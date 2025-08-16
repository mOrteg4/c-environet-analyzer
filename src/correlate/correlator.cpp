#include "correlate/correlator.hpp"
#include "core/log.hpp"

#include <chrono>

namespace environet { namespace correlate {

Correlator::Correlator(const std::string& /*config_path*/)
    : sensor_threshold_(200), correlation_window_ms_(5000), findings_dir_("findings"),
      sensor_events_(0), network_events_(0), correlations_found_(0), start_time_ms_(0) {}

Correlator::~Correlator() {}

bool Correlator::init() {
    using namespace std::chrono;
    start_time_ms_ = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    return true;
}

void Correlator::push_sensor(const sensors::SensorFrame& frame) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    sensor_buffer_.emplace_back(get_current_time_ms(), frame);
}

void Correlator::push_bss(const net::BssInfo& bss) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    bss_buffer_.emplace_back(get_current_time_ms(), bss);
}

void Correlator::push_packet(const net::PacketMeta& pkt) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    packet_buffer_.emplace_back(get_current_time_ms(), pkt);
}

void Correlator::push_ping_stats(const net::PingStats& ps) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    ping_buffer_.emplace_back(get_current_time_ms(), ps);
}

void Correlator::push_iperf3_results(const net::Iperf3Results& r) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    iperf_buffer_.emplace_back(get_current_time_ms(), r);
}

std::vector<Finding> Correlator::process() {
    // Stub: no real correlation yet
    return {};
}

std::vector<Finding> Correlator::get_findings() const { return findings_; }

nlohmann::json Correlator::get_stats() const {
    nlohmann::json j;
    j["sensor_events"] = sensor_events_;
    j["network_events"] = network_events_;
    j["correlations_found"] = correlations_found_;
    return j;
}

void Correlator::set_finding_callback(std::function<void(const Finding&)> cb) { finding_callback_ = std::move(cb); }

void Correlator::cleanup_old_data() {}
std::vector<Finding> Correlator::correlate_sensor_event(const sensors::SensorFrame&) { return {}; }

nlohmann::json Correlator::calculate_window_stats(uint64_t, uint64_t) { return {}; }
void Correlator::save_finding(const Finding&) {}
void Correlator::ensure_findings_dir() {}
void Correlator::set_error(const std::string& e) { last_error_ = e; }
uint64_t Correlator::get_current_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}
bool Correlator::is_in_window(uint64_t ts, uint64_t window_start) const {
    return ts >= window_start && ts <= (window_start + static_cast<uint64_t>(correlation_window_ms_));
}
double Correlator::calculate_avg_rssi(uint64_t, uint64_t) const { return 0.0; }
double Correlator::calculate_rssi_delta(uint64_t, uint64_t) const { return 0.0; }

}} // namespace
