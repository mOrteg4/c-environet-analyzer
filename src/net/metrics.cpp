#include "net/metrics.hpp"
#include "core/log.hpp"

#include <chrono>
#include <regex>
#include <cstdio>
#include <array>
#include <sstream>
#include <cstdlib>

namespace environet { namespace net {

Metrics::Metrics(const std::string& /*config_path*/)
    : ping_interval_ms_(10000), iperf3_duration_(10), ping_tests_run_(0), iperf3_tests_run_(0),
      ping_errors_(0), iperf3_errors_(0), start_time_ms_(0) {}

Metrics::~Metrics() {}

bool Metrics::init() {
    start_time_ms_ = get_current_time_ms();
    return true;
}

PingStats Metrics::ping_test(const std::string& target, int count, int timeout_ms) {
    ++ping_tests_run_;
    PingStats ps; ps.target = target; ps.timestamp_ms = get_current_time_ms();

#ifndef __linux__
    set_error("ping not implemented on this platform in current build");
    ++ping_errors_;
    return ps;
#else
    if (!check_ping_available()) {
        set_error("ping command not found");
        ++ping_errors_;
        return ps;
    }

    // Linux ping uses seconds for -W timeout
    int timeout_s = std::max(1, timeout_ms / 1000);
    std::ostringstream cmd;
    cmd << "ping -n -c " << count << " -W " << timeout_s << " " << target << " 2>&1";

    std::string out = execute_command(cmd.str());
    PingStats parsed = parse_ping_output(out, target);
    if (!parsed.reachable) {
        ++ping_errors_;
    }
    return parsed;
#endif
}

std::vector<PingStats> Metrics::ping_multiple(const std::vector<std::string>& targets, int count, int timeout_ms) {
    std::vector<PingStats> out;
    out.reserve(targets.size());
    for (const auto& t : targets) out.push_back(ping_test(t, count, timeout_ms));
    return out;
}

Iperf3Results Metrics::iperf3_test(const std::string& server, int duration, const std::string& protocol, int port) {
    ++iperf3_tests_run_;
    Iperf3Results r; r.server = server; r.duration_seconds = duration; r.protocol = protocol; r.timestamp_ms = get_current_time_ms();

#ifndef __linux__
    set_error("iperf3 not implemented on this platform in current build");
    ++iperf3_errors_;
    return r;
#else
    if (server.empty()) {
        set_error("iperf3 server not configured");
        ++iperf3_errors_;
        return r;
    }
    if (!check_iperf3_available()) {
        set_error("iperf3 command not found");
        ++iperf3_errors_;
        return r;
    }

    std::ostringstream cmd;
    cmd << "iperf3 -c " << server << " -p " << port << " -t " << duration << " -J";
    if (!protocol.empty() && (protocol == "UDP" || protocol == "udp")) {
        cmd << " -u";
    }
    cmd << " 2>&1";

    std::string out = execute_command(cmd.str());
    Iperf3Results parsed = parse_iperf3_output(out, server);
    if (!parsed.success) {
        ++iperf3_errors_;
    }
    return parsed;
#endif
}

nlohmann::json Metrics::get_stats() const {
    nlohmann::json j;
    j["ping_tests_run"] = ping_tests_run_;
    j["iperf3_tests_run"] = iperf3_tests_run_;
    j["ping_errors"] = ping_errors_;
    j["iperf3_errors"] = iperf3_errors_;
    return j;
}

PingStats Metrics::parse_ping_output(const std::string& output, const std::string& target) {
    PingStats ps; ps.target = target; ps.timestamp_ms = get_current_time_ms();

    // Packets: "X packets transmitted, Y received, Z% packet loss"
    std::regex pkt_rx(R"((\d+)\s+packets\s+transmitted,\s+(\d+)\s+(?:packets\s+)?received,\s+(?:\+?\d+\s+errors,\s+)?([0-9.]+)%\s+packet\s+loss)");
    std::smatch m;
    if (std::regex_search(output, m, pkt_rx) && m.size() >= 4) {
        ps.packets_sent = std::stoi(m[1].str());
        ps.packets_received = std::stoi(m[2].str());
        ps.packets_lost = ps.packets_sent - ps.packets_received;
        ps.loss_percentage = std::stod(m[3].str());
        ps.reachable = (ps.packets_received > 0);
    }

    // RTT: "rtt min/avg/max/mdev = a/b/c/d ms"
    std::regex rtt_rx(R"(rtt\s+min/avg/max/(?:mdev|stddev)\s*=\s*([0-9.]+)/([0-9.]+)/([0-9.]+)/([0-9.]+)\s*ms)");
    if (std::regex_search(output, m, rtt_rx) && m.size() >= 5) {
        ps.min_rtt_ms = std::stod(m[1].str());
        ps.avg_rtt_ms = std::stod(m[2].str());
        ps.max_rtt_ms = std::stod(m[3].str());
        ps.stddev_rtt_ms = std::stod(m[4].str());
    }

    return ps;
}

Iperf3Results Metrics::parse_iperf3_output(const std::string& output, const std::string& server) {
    Iperf3Results r; r.server = server; r.timestamp_ms = get_current_time_ms();

    try {
        // Prefer JSON parsing when -J is used
        auto trimmed_start = output.find_first_not_of(" \n\r\t");
        if (trimmed_start != std::string::npos && output[trimmed_start] == '{') {
            auto j = nlohmann::json::parse(output);
            if (j.contains("end")) {
                const auto& end = j["end"];
                if (end.contains("sum_received")) {
                    const auto& sum = end["sum_received"];
                    if (sum.contains("bits_per_second")) {
                        r.bandwidth_mbps = sum["bits_per_second"].get<double>() / 1e6;
                    }
                } else if (end.contains("sum_sent")) {
                    const auto& sum = end["sum_sent"];
                    if (sum.contains("bits_per_second")) {
                        r.bandwidth_mbps = sum["bits_per_second"].get<double>() / 1e6;
                    }
                }
                if (end.contains("sum")) {
                    const auto& sum = end["sum"];
                    if (sum.contains("jitter_ms")) r.jitter_ms = sum["jitter_ms"].get<double>();
                    if (sum.contains("lost_percent")) r.packet_loss = sum["lost_percent"].get<double>();
                }
                r.success = true;
                return r;
            }
        }
    } catch (const std::exception& e) {
        r.success = false; r.error_message = std::string("iperf3 JSON parse error: ") + e.what();
        return r;
    }

    // Fallback: parse text for bandwidth line
    std::regex bw_rx(R"((\d+\.?\d*)\s+Mbits/sec)");
    std::smatch m;
    if (std::regex_search(output, m, bw_rx) && m.size() >= 2) {
        r.bandwidth_mbps = std::stod(m[1].str());
        r.success = true;
    } else {
        r.success = false;
        r.error_message = "Unable to parse iperf3 output";
    }
    return r;
}

std::string Metrics::execute_command(const std::string& command) {
    std::array<char, 256> buf{};
    std::string result;

#if defined(_WIN32)
    // Not supported in current build context; return empty
    (void)command;
    return result;
#else
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return result;
    while (fgets(buf.data(), static_cast<int>(buf.size()), pipe) != nullptr) {
        result.append(buf.data());
    }
    pclose(pipe);
    return result;
#endif
}

void Metrics::set_error(const std::string& e) { last_error_ = e; }

uint64_t Metrics::get_current_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

double Metrics::extract_numeric_value(const std::string& text, const std::string& pattern) {
    std::regex rx(pattern);
    std::smatch m;
    if (std::regex_search(text, m, rx) && m.size() >= 2) {
        return std::stod(m[1].str());
    }
    return 0.0;
}

bool Metrics::check_iperf3_available() {
#if defined(_WIN32)
    return false;
#else
    // Use a direct popen here since this is a static context
    std::array<char, 128> buf{};
    std::string out;
    FILE* pipe = popen("command -v iperf3 2>&1", "r");
    if (pipe) {
        while (fgets(buf.data(), static_cast<int>(buf.size()), pipe) != nullptr) out.append(buf.data());
        pclose(pipe);
    }
    return !out.empty();
#endif
}

bool Metrics::check_ping_available() {
#if defined(_WIN32)
    return false;
#else
    std::array<char, 128> buf{};
    std::string out;
    FILE* pipe = popen("command -v ping 2>&1", "r");
    if (pipe) {
        while (fgets(buf.data(), static_cast<int>(buf.size()), pipe) != nullptr) out.append(buf.data());
        pclose(pipe);
    }
    return !out.empty();
#endif
}

}} // namespace
