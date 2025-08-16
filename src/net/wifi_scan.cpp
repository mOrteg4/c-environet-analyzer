#include "net/wifi_scan.hpp"
#include "core/config.hpp"
#include "core/log.hpp"

namespace environet { namespace net {

WifiScan::WifiScan(const std::string& /*config_path*/)
    : iface_scan_("wlan0"), iface_ap_("wlan1"), scan_interval_ms_(5000), monitor_mode_(false),
      nl_sock_(nullptr), nl_cache_(nullptr), nl_family_(0), scan_count_(0), scan_errors_(0) {}

WifiScan::~WifiScan() { cleanup_libnl(); }

bool WifiScan::init() {
    // Minimal stub: no libnl init yet
    return true;
}

std::vector<BssInfo> WifiScan::scan() {
    // Stub: return empty list
    last_scan_results_.clear();
    ++scan_count_;
    return last_scan_results_;
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
std::vector<BssInfo> WifiScan::scan_fallback() { return {}; }
std::vector<BssInfo> WifiScan::parse_iw_output(const std::string&) { return {}; }
std::vector<BssInfo> WifiScan::parse_proc_wireless() { return {}; }
std::vector<BssInfo> WifiScan::parse_scan_results(const std::string&) { return {}; }
std::string WifiScan::execute_command(const std::string&) { return {}; }
void WifiScan::set_error(const std::string& e) { last_error_ = e; }
void WifiScan::cleanup_libnl() {}
int WifiScan::freq_to_channel(int freq) { return freq; }
int WifiScan::dbm_to_mbm(int dbm) { return dbm * 100; }

}} // namespace
