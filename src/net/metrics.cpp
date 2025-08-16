#include "net/metrics.hpp"
#include "core/log.hpp"

#include <chrono>

namespace environet { namespace net {

Metrics::Metrics(const std::string& /*config_path*/)
    : ping_interval_ms_(10000), iperf3_duration_(10), ping_tests_run_(0), iperf3_tests_run_(0),
      ping_errors_(0), iperf3_errors_(0), start_time_ms_(0) {}

Metrics::~Metrics() {}

bool Metrics::init() {
    start_time_ms_ = get_current_time_ms();
    return true;
}

PingStats Metrics::ping_test(const std::string& target, int /*count*/, int /*timeout_ms*/) {
    ++ping_tests_run_;
    PingStats ps; ps.target = target; ps.timestamp_ms = get_current_time_ms(); ps.reachable = false;
    return ps;
}

std::vector<PingStats> Metrics::ping_multiple(const std::vector<std::string>& targets, int count, int timeout_ms) {
    std::vector<PingStats> out;
    out.reserve(targets.size());
    for (const auto& t : targets) out.push_back(ping_test(t, count, timeout_ms));
    return out;
}

Iperf3Results Metrics::iperf3_test(const std::string& server, int duration, const std::string& protocol, int /*port*/) {
    ++iperf3_tests_run_;
    Iperf3Results r; r.server = server; r.duration_seconds = duration; r.protocol = protocol; r.timestamp_ms = get_current_time_ms(); r.success = false;
    return r;
}

nlohmann::json Metrics::get_stats() const {
    nlohmann::json j;
    j["ping_tests_run"] = ping_tests_run_;
    j["iperf3_tests_run"] = iperf3_tests_run_;
    j["ping_errors"] = ping_errors_;
    j["iperf3_errors"] = iperf3_errors_;
    return j;
}

Metrics::PingStats Metrics::parse_ping_output(const std::string&, const std::string&) { return {}; }
Metrics::Iperf3Results Metrics::parse_iperf3_output(const std::string&, const std::string&) { return {}; }
std::string Metrics::execute_command(const std::string&) { return {}; }
void Metrics::set_error(const std::string& e) { last_error_ = e; }
uint64_t Metrics::get_current_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
double Metrics::extract_numeric_value(const std::string&, const std::string&) { return 0.0; }
bool Metrics::check_iperf3_available() { return false; }
bool Metrics::check_ping_available() { return false; }

}} // namespace
