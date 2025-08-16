#include "net/pcap_sniffer.hpp"
#include "core/log.hpp"

#include <cstring>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <arpa/inet.h>
#include <atomic>

namespace fs = std::filesystem;
namespace environet { namespace net {

PcapSniffer::PcapSniffer(const std::string& config_path)
    : interface_("wlan0"), bpf_filter_(""), output_dir_("captures"), max_file_size_mb_(100),
      max_files_(10), promiscuous_(true), pcap_handle_(nullptr), pcap_dumper_(nullptr),
      running_(false), packets_captured_(0), packets_dropped_(0), bytes_captured_(0),
      start_time_ms_(0), file_index_(0) {
    // Load minimal fields from config json if available
    try {
        std::ifstream in(config_path);
        if (in.is_open()) {
            nlohmann::json j; in >> j;
            if (j.contains("pcap")) {
                auto &p = j["pcap"];
                if (p.contains("bpf")) bpf_filter_ = p["bpf"].get<std::string>();
                if (p.contains("output_dir")) output_dir_ = p["output_dir"].get<std::string>();
                if (p.contains("max_file_size_mb")) max_file_size_mb_ = p["max_file_size_mb"].get<size_t>();
                if (p.contains("max_files")) max_files_ = p["max_files"].get<int>();
            }
            if (j.contains("wifi")) {
                auto &w = j["wifi"];
                if (w.contains("iface_scan")) interface_ = w["iface_scan"].get<std::string>();
            }
        }
    } catch (...) {
        // ignore config parse errors; keep defaults
    }
}

PcapSniffer::~PcapSniffer() { stop(); cleanup(); }
bool PcapSniffer::init() {
    // Ensure output directory exists
    try {
        if (!fs::exists(output_dir_)) {
            fs::create_directories(output_dir_);
        }
    } catch (const std::exception& e) {
        LOGE("Failed to create output directory: {}", e.what());
        return false;
    }
    return true;
}

bool PcapSniffer::start(PacketCallback callback) {
    packet_callback_ = std::move(callback);
    if (!init_interface()) return false;
    if (!compile_bpf()) return false;
    if (!open_pcap_file()) return false;

    running_ = true;
    capture_thread_ = std::thread([this]() { capture_loop(); });
    return true;
}

void PcapSniffer::stop() {
    if (running_) {
        running_ = false;
        if (pcap_handle_) pcap_breakloop(pcap_handle_);
        if (capture_thread_.joinable()) capture_thread_.join();
    }
}

nlohmann::json PcapSniffer::get_stats() const {
    nlohmann::json j;
    j["packets_captured"] = packets_captured_;
    j["packets_dropped"] = packets_dropped_;
    j["bytes_captured"] = bytes_captured_;
    return j;
}

bool PcapSniffer::init_interface() {
    char errbuf[PCAP_ERRBUF_SIZE] = {};
    pcap_handle_ = pcap_open_live(interface_.c_str(), 65535, promiscuous_ ? 1 : 0, 1000, errbuf);
    if (!pcap_handle_) {
        set_error(std::string("pcap_open_live failed: ") + errbuf);
        return false;
    }
    datalink_ = pcap_datalink(pcap_handle_);
    return true;
}

bool PcapSniffer::compile_bpf() {
    if (bpf_filter_.empty()) return true; // nothing to do
    bpf_program prog{};
    if (pcap_compile(pcap_handle_, &prog, bpf_filter_.c_str(), 1, PCAP_NETMASK_UNKNOWN) < 0) {
        set_error(std::string("pcap_compile failed: ") + pcap_geterr(pcap_handle_));
        return false;
    }
    if (pcap_setfilter(pcap_handle_, &prog) < 0) {
        pcap_freecode(&prog);
        set_error(std::string("pcap_setfilter failed: ") + pcap_geterr(pcap_handle_));
        return false;
    }
    pcap_freecode(&prog);
    return true;
}

bool PcapSniffer::open_pcap_file() {
    // Rotate history if needed
    rotate_pcap_file();
    // Create a new file name with index and timestamp
    char fname[256];
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    int file_idx = file_index_.fetch_add(1, std::memory_order_relaxed);
    std::snprintf(fname, sizeof(fname), "%s/capture_%04d%02d%02d_%02d%02d%02d_%03d.pcap",
                  output_dir_.c_str(), tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec, file_idx);
    current_pcap_file_ = fname;
    pcap_dumper_ = pcap_dump_open(pcap_handle_, current_pcap_file_.c_str());
    if (!pcap_dumper_) {
        set_error(std::string("pcap_dump_open failed: ") + pcap_geterr(pcap_handle_));
        return false;
    }
    return true;
}

void PcapSniffer::close_pcap_file() {
    if (pcap_dumper_) {
        pcap_dump_flush(pcap_dumper_);
        pcap_dump_close(pcap_dumper_);
        pcap_dumper_ = nullptr;
    }
}

void PcapSniffer::rotate_pcap_file() {
    // Close current dumper if any
    close_pcap_file();
    // Enforce retention
    if (max_files_ > 0 && file_history_.size() >= static_cast<size_t>(max_files_)) {
        try {
            fs::remove(file_history_.front());
        } catch (...) {}
        file_history_.erase(file_history_.begin());
    }
}

void PcapSniffer::capture_loop() {
    const pcap_pkthdr* header = nullptr;
    const u_char* data = nullptr;
    while (running_ && pcap_handle_) {
        int rc = pcap_next_ex(pcap_handle_, &header, &data);
        if (rc == 1 && header && data) {
            // Write to pcap file
            if (pcap_dumper_) pcap_dump(reinterpret_cast<u_char*>(pcap_dumper_), header, data);
            bytes_captured_ += header->caplen;
            packets_captured_++;
            try {
                if (!current_pcap_file_.empty()) {
                    auto sz = fs::file_size(current_pcap_file_);
                    if (sz > max_file_size_mb_ * 1024ULL * 1024ULL) {
                        file_history_.push_back(current_pcap_file_);
                        open_pcap_file();
                    }
                }
            } catch (const std::exception& e) {
                LOGE("Error during file size check or rotation: {}", e.what());
            }
            // Process packet
            if (packet_callback_) {
                process_packet(header, reinterpret_cast<const uint8_t*>(data));
            }
        } else if (rc == 0) {
            // timeout; continue
            continue;
        } else if (rc == -1) {
            set_error(std::string("pcap_next_ex error: ") + pcap_geterr(pcap_handle_));
            break;
        } else if (rc == -2) {
            // breakloop
            break;
        }
    }
    close_pcap_file();
    // Gather stats
    if (pcap_handle_) {
        struct pcap_stat ps{};
        if (pcap_stats(pcap_handle_, &ps) == 0) {
            packets_dropped_ += ps.ps_drop;
        }
    }
    cleanup();
}

void PcapSniffer::process_packet(const pcap_pkthdr* header, const uint8_t* packet) {
    PacketMeta meta;
    meta.timestamp_ms = static_cast<uint64_t>(header->ts.tv_sec) * 1000ULL + header->ts.tv_usec / 1000ULL;
    meta.length = header->len;

    const uint8_t* cursor = packet;
    // Attempt Ethernet first
    if (!parse_ethernet_header(cursor, meta)) {
        // Some WLAN captures with radiotap may not start with Ethernet
        parse_radiotap_header(cursor, meta); // best-effort
    }
    if (packet_callback_) packet_callback_(meta, packet);
}

static std::string hex2(const uint8_t* p, size_t n) {
    static const char* hexd = "0123456789abcdef";
    std::string s; s.reserve(n * 3);
    for (size_t i = 0; i < n; ++i) {
        s.push_back(hexd[(p[i] >> 4) & 0xF]);
        s.push_back(hexd[p[i] & 0xF]);
        if (i + 1 < n) s.push_back(':');
    }
    return s;
}

bool PcapSniffer::parse_ethernet_header(const uint8_t* packet, PacketMeta& meta) {
    if (!packet) return false;
    const uint8_t* dst = packet;
    const uint8_t* src = packet + 6;
    uint16_t type = (packet[12] << 8) | packet[13];
    meta.dst_mac = hex2(dst, 6);
    meta.src_mac = hex2(src, 6);
    meta.ethertype = type;

    const uint8_t* payload = packet + 14;
    if (type == 0x0800) {
        // IPv4
        parse_ip_header(payload, meta);
    } else if (type == 0x86DD) {
        // IPv6 (basic parsing)
        meta.protocol = payload[6];
        meta.src_ip = ip_to_string(payload + 8, 6);
        meta.dst_ip = ip_to_string(payload + 24, 6);
    }
    return true;
}

bool PcapSniffer::parse_ip_header(const uint8_t* packet, PacketMeta& meta) {
    if (!packet) return false;
    uint8_t ver_ihl = packet[0];
    uint8_t ihl = (ver_ihl & 0x0F) * 4;
    if (ihl < 20) return false;
    meta.protocol = packet[9];
    meta.src_ip = ip_to_string(packet + 12, 4);
    meta.dst_ip = ip_to_string(packet + 16, 4);
    const uint8_t* l4 = packet + ihl;
    if (meta.protocol == 6) parse_tcp_header(l4, meta);
    else if (meta.protocol == 17) parse_udp_header(l4, meta);
    return true;
}

bool PcapSniffer::parse_tcp_header(const uint8_t* packet, PacketMeta& meta) {
    if (!packet) return false;
    meta.src_port = (packet[0] << 8) | packet[1];
    meta.dst_port = (packet[2] << 8) | packet[3];
    return true;
}

bool PcapSniffer::parse_udp_header(const uint8_t* packet, PacketMeta& meta) {
    if (!packet) return false;
    meta.src_port = (packet[0] << 8) | packet[1];
    meta.dst_port = (packet[2] << 8) | packet[3];
    return true;
}

bool PcapSniffer::parse_radiotap_header(const uint8_t* /*packet*/, PacketMeta& /*meta*/) {
    // Placeholder: radiotap parsing can be added later for RSSI
    return false;
}

void PcapSniffer::set_error(const std::string& e) { last_error_ = e; }

void PcapSniffer::cleanup() {
    if (pcap_handle_) {
        pcap_close(pcap_handle_);
        pcap_handle_ = nullptr;
    }
}

uint64_t PcapSniffer::get_current_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

std::string PcapSniffer::mac_to_string(const uint8_t* mac) { return hex2(mac, 6); }

std::string PcapSniffer::ip_to_string(const uint8_t* ip, int version) {
    char buf[INET6_ADDRSTRLEN] = {};
    if (version == 4) {
        inet_ntop(AF_INET, ip, buf, sizeof(buf));
    } else {
        inet_ntop(AF_INET6, ip, buf, sizeof(buf));
    }
    return std::string(buf);
}

}} // namespace
