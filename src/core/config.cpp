#include "core/config.hpp"

#include <fstream>
#include <sstream>

namespace environet {
namespace core {

using nlohmann::json;

static bool file_exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

Config Config::load(const std::string& path) {
    if (!file_exists(path)) {
        throw std::runtime_error("Config file not found: " + path);
    }
    std::ifstream in(path);
    if (!in.is_open()) {
        throw std::runtime_error("Unable to open config file: " + path);
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    in.close();
    return Config::from_json(buffer.str());
}

Config Config::from_json(const std::string& json_str) {
    if (json_str.empty()) {
        throw std::runtime_error("Empty JSON string");
    }
    json j;
    try {
        j = json::parse(json_str);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Invalid JSON: ") + e.what());
    }
    Config cfg = Config::get_defaults();
    cfg.from_json_object(j);
    cfg.validate();
    return cfg;
}

Config Config::get_defaults() {
    Config cfg;
    cfg.set_defaults();
    return cfg;
}

void Config::validate() const {
    if (i2c.bus_id < 0) {
        throw std::runtime_error("i2c.bus_id must be >= 0");
    }
    if (i2c.addr <= 0 || i2c.addr > 0x7f) {
        throw std::runtime_error("i2c.addr must be 1..127");
    }
    if (i2c.sample_interval_ms <= 0) {
        throw std::runtime_error("i2c.sample_interval_ms must be > 0");
    }
    if (wifi.scan_interval_ms <= 0) {
        throw std::runtime_error("wifi.scan_interval_ms must be > 0");
    }
    if (pcap.max_file_size_mb <= 0) {
        throw std::runtime_error("pcap.max_file_size_mb must be > 0");
    }
    if (pcap.max_files <= 0) {
        throw std::runtime_error("pcap.max_files must be > 0");
    }
    if (correlator.window_ms <= 0) {
        throw std::runtime_error("correlator.window_ms must be > 0");
    }
    if (logging.max_size_mb <= 0) {
        throw std::runtime_error("logging.max_size_mb must be > 0");
    }
    if (logging.max_files <= 0) {
        throw std::runtime_error("logging.max_files must be > 0");
    }
    if (metrics.ping_interval_ms <= 0) {
        throw std::runtime_error("metrics.ping_interval_ms must be > 0");
    }
    if (metrics.iperf3_duration <= 0) {
        throw std::runtime_error("metrics.iperf3_duration must be > 0");
    }
}

nlohmann::json Config::to_json() const {
    json j;
    j["i2c"] = {
        {"mock_mode", i2c.mock_mode},
        {"bus_id", i2c.bus_id},
        {"addr", i2c.addr},
        {"sample_interval_ms", i2c.sample_interval_ms}
    };
    j["wifi"] = {
        {"iface_ap", wifi.iface_ap},
        {"iface_scan", wifi.iface_scan},
        {"scan_interval_ms", wifi.scan_interval_ms},
        {"monitor_mode", wifi.monitor_mode}
    };
    j["pcap"] = {
        {"bpf", pcap.bpf},
        {"output_dir", pcap.output_dir},
        {"max_file_size_mb", pcap.max_file_size_mb},
        {"max_files", pcap.max_files}
    };
    j["correlator"] = {
        {"sensor_threshold", correlator.sensor_threshold},
        {"window_ms", correlator.window_ms},
        {"findings_dir", correlator.findings_dir}
    };
    j["logging"] = {
        {"level", logging.level},
        {"file", logging.file},
        {"console", logging.console},
        {"max_size_mb", logging.max_size_mb},
        {"max_files", logging.max_files}
    };
    j["metrics"] = {
        {"ping_targets", metrics.ping_targets},
        {"iperf_server", metrics.iperf_server},
        {"ping_interval_ms", metrics.ping_interval_ms},
        {"iperf_duration", metrics.iperf3_duration}
    };
    return j;
}

void Config::from_json_object(const json& j) {
    if (j.contains("i2c") && j["i2c"].is_object()) {
        auto& ji = j["i2c"];
        if (ji.contains("mock_mode")) i2c.mock_mode = ji["mock_mode"].get<bool>();
        if (ji.contains("bus_id")) i2c.bus_id = ji["bus_id"].get<int>();
        if (ji.contains("addr")) i2c.addr = ji["addr"].get<int>();
        if (ji.contains("sample_interval_ms")) i2c.sample_interval_ms = ji["sample_interval_ms"].get<int>();
    }
    if (j.contains("wifi") && j["wifi"].is_object()) {
        auto& jw = j["wifi"];
        if (jw.contains("iface_ap")) wifi.iface_ap = jw["iface_ap"].get<std::string>();
        if (jw.contains("iface_scan")) wifi.iface_scan = jw["iface_scan"].get<std::string>();
        if (jw.contains("scan_interval_ms")) wifi.scan_interval_ms = jw["scan_interval_ms"].get<int>();
        if (jw.contains("monitor_mode")) wifi.monitor_mode = jw["monitor_mode"].get<bool>();
    }
    if (j.contains("pcap") && j["pcap"].is_object()) {
        auto& jp = j["pcap"];
        if (jp.contains("bpf")) pcap.bpf = jp["bpf"].get<std::string>();
        if (jp.contains("output_dir")) pcap.output_dir = jp["output_dir"].get<std::string>();
        if (jp.contains("max_file_size_mb")) pcap.max_file_size_mb = jp["max_file_size_mb"].get<size_t>();
        if (jp.contains("max_files")) pcap.max_files = jp["max_files"].get<int>();
    }
    if (j.contains("correlator") && j["correlator"].is_object()) {
        auto& jc = j["correlator"];
        if (jc.contains("sensor_threshold")) correlator.sensor_threshold = jc["sensor_threshold"].get<int>();
        if (jc.contains("window_ms")) correlator.window_ms = jc["window_ms"].get<int>();
        if (jc.contains("findings_dir")) correlator.findings_dir = jc["findings_dir"].get<std::string>();
    }
    if (j.contains("logging") && j["logging"].is_object()) {
        auto& jl = j["logging"];
        if (jl.contains("level")) logging.level = jl["level"].get<std::string>();
        if (jl.contains("file")) logging.file = jl["file"].get<std::string>();
        if (jl.contains("console")) logging.console = jl["console"].get<bool>();
        if (jl.contains("max_size_mb")) logging.max_size_mb = jl["max_size_mb"].get<size_t>();
        if (jl.contains("max_files")) logging.max_files = jl["max_files"].get<int>();
    }
    if (j.contains("metrics") && j["metrics"].is_object()) {
        auto& jm = j["metrics"];
        if (jm.contains("ping_targets")) metrics.ping_targets = jm["ping_targets"].get<std::vector<std::string>>();
        if (jm.contains("iperf_server")) metrics.iperf_server = jm["iperf_server"].get<std::string>();
        if (jm.contains("ping_interval_ms")) metrics.ping_interval_ms = jm["ping_interval_ms"].get<int>();
        if (jm.contains("iperf3_duration")) metrics.iperf3_duration = jm["iperf3_duration"].get<int>();
        else if (jm.contains("iperf_duration")) metrics.iperf3_duration = jm["iperf_duration"].get<int>();
    }
}

void Config::set_defaults() {
    // defaults set by member initializers
}

} // namespace core
} // namespace environet
