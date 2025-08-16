#include "net/wifi_scan.hpp"
#include "core/config.hpp"
#include "core/log.hpp"

namespace environet { namespace net {

WifiScan::WifiScan(const std::string& /*config_path*/)
    : iface_scan_("wlan0"), iface_ap_("wlan1"), scan_interval_ms_(5000), monitor_mode_(false),
      nl_sock_(nullptr), nl_cache_(nullptr), nl_family_(0), scan_count_(0), scan_errors_(0) {}

WifiScan::~WifiScan() { cleanup_libnl(); }

bool WifiScan::init() {
    // For now, just verify that the scan interface exists via `iw dev`
#ifndef __linux__
    set_error("WiFi scanning supported only on Linux in current build");
    return false;
#else
    std::string out = execute_command("iw dev 2>&1");
    if (out.find(iface_scan_) == std::string::npos) {
        set_error("Scan interface not found: " + iface_scan_);
        // Still allow init; fallback parser may handle later configuration changes
    }
    return true;
#endif
}

std::vector<BssInfo> WifiScan::scan() {
#ifndef __linux__
    last_scan_results_.clear();
    ++scan_count_;
    return last_scan_results_;
#else
    // Try libnl path first (not yet implemented), then fallback to `iw`
    auto results = scan_libnl();
    if (results.empty()) {
        results = scan_fallback();
    }
    last_scan_results_ = std::move(results);
    ++scan_count_;
    return last_scan_results_;
#endif
}

BssInfo WifiScan::get_connected_network() { return BssInfo(); }

nlohmann::json WifiScan::get_scan_stats() const {
    nlohmann::json j;
    j["scan_count"] = scan_count_;
    j["scan_errors"] = scan_errors_;
    return j;
}

bool WifiScan::init_libnl() { return false; }
bool WifiScan::init_interface() { return false; }
std::vector<BssInfo> WifiScan::scan_libnl() { return {}; }

std::vector<BssInfo> WifiScan::scan_fallback() {
#ifndef __linux__
    return {};
#else
    // Use `iw dev <iface> scan` which often requires CAP_NET_ADMIN or root
    std::string cmd = std::string("iw dev ") + iface_scan_ + " scan 2>&1";
    std::string out = execute_command(cmd);
    if (out.find("command failed") != std::string::npos || out.find("Operation not permitted") != std::string::npos) {
        // As a weaker fallback, try reading /proc/net/wireless
        return parse_proc_wireless();
    }
    return parse_scan_results(out);
#endif
}

std::vector<BssInfo> WifiScan::parse_iw_output(const std::string& output) { return parse_scan_results(output); }

std::vector<BssInfo> WifiScan::parse_proc_wireless() {
#ifndef __linux__
    return {};
#else
    std::string out = execute_command("cat /proc/net/wireless 2>&1");
    std::vector<BssInfo> results;
    // Very thin parsing just to surface the interface itself with a signal metric
    // Format lines like: wlan0: 0000   54.  -256.  -256.        0      0      0      0      0        0
    std::istringstream iss(out);
    std::string line;
    while (std::getline(iss, line)) {
        auto pos = line.find(':');
        if (pos == std::string::npos) continue;
        std::string ifname = line.substr(0, pos);
        // trim spaces
        ifname.erase(0, ifname.find_first_not_of(" \t"));
        ifname.erase(ifname.find_last_not_of(" \t") + 1);
        if (ifname == iface_scan_) {
            // Not enough info for BSSID/SSID; report a synthetic entry
            BssInfo b; b.ssid = ""; b.bssid = ""; b.freq = 0; b.signal_mbm = 0; b.is_connected = false;
            results.push_back(b);
        }
    }
    return results;
#endif
}

std::vector<BssInfo> WifiScan::parse_scan_results(const std::string& output) {
    std::vector<BssInfo> results;
#ifndef __linux__
    return results;
#else
    // Parse sections starting with "BSS <mac>(on <iface>)"
    // We’ll gather a few common fields: SSID, freq, signal
    std::istringstream iss(output);
    std::string line;
    BssInfo current;
    bool in_bss = false;
    while (std::getline(iss, line)) {
        if (line.rfind("BSS ", 0) == 0) {
            if (in_bss) {
                results.push_back(current);
            }
            in_bss = true;
            current = BssInfo();
            // Extract BSSID between "BSS " and the following space or parenthesis
            size_t start = 4; // after "BSS "
            size_t end = line.find_first_of(" (\t\r\n", start);
            if (end != std::string::npos) {
                current.bssid = line.substr(start, end - start);
            } else {
                current.bssid = line.substr(start);
            }
            continue;
        }
        if (!in_bss) continue;
        // Trim leading spaces
        auto first = line.find_first_not_of(" \t");
        if (first != std::string::npos) line = line.substr(first);

        if (line.rfind("freq:", 0) == 0) {
            try { current.freq = std::stoi(line.substr(5)); current.channel = freq_to_channel(current.freq); } catch (...) {}
        } else if (line.rfind("signal:", 0) == 0) {
            // e.g., signal: -45.00 dBm
            try {
                auto dpos = line.find("dBm");
                std::string val = (dpos != std::string::npos) ? line.substr(7, dpos - 7) : line.substr(7);
                double dbm = std::stod(val);
                current.signal_mbm = dbm_to_mbm(static_cast<int>(std::round(dbm)));
            } catch (...) {}
        } else if (line.rfind("SSID:", 0) == 0) {
            current.ssid = line.substr(5);
            // trim
            current.ssid.erase(0, current.ssid.find_first_not_of(" \t"));
            current.ssid.erase(current.ssid.find_last_not_of(" \t") + 1);
        } else if (line.empty()) {
            // End of block
            if (in_bss) { results.push_back(current); in_bss = false; }
        }
    }
    if (in_bss) results.push_back(current);
    return results;
#endif
}

std::string WifiScan::execute_command(const std::string& command) {
#if defined(_WIN32)
    (void)command; return {};
#else
    std::array<char, 256> buf{};
    std::string result;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return result;
    while (fgets(buf.data(), static_cast<int>(buf.size()), pipe) != nullptr) {
        result.append(buf.data());
    }
    pclose(pipe);
    return result;
#endif
}
void WifiScan::set_error(const std::string& e) { last_error_ = e; }
void WifiScan::cleanup_libnl() {}
int WifiScan::freq_to_channel(int freq) { return freq; }
int WifiScan::dbm_to_mbm(int dbm) { return dbm * 100; }

}} // namespace
