#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace environet {
namespace core {

/**
 * @brief Configuration class for EnviroNet Analyzer
 * 
 * Loads and validates configuration from JSON files with sensible defaults
 */
class Config {
public:
    struct I2CConfig {
        bool mock_mode = true;  // Default to mock mode for development
        int bus_id = 1;         // Default I2C bus
        int addr = 16;          // Default slave address (0x10)
        int sample_interval_ms = 100;  // Sample interval in milliseconds
    };

    struct WifiConfig {
        std::string iface_ap = "wlan1";      // Access point interface
        std::string iface_scan = "wlan0";    // Scanning interface
        int scan_interval_ms = 5000;         // Scan interval in milliseconds
        bool monitor_mode = false;           // Enable monitor mode capture
    };

    struct PcapConfig {
        std::string bpf = "not (type mgt)";  // BPF filter string
        std::string output_dir = "captures";  // Output directory for pcap files
        size_t max_file_size_mb = 100;       // Max pcap file size
        int max_files = 10;                  // Max number of pcap files
    };

    struct CorrelatorConfig {
        int sensor_threshold = 200;          // Sensor change threshold
        int window_ms = 5000;                // Correlation window in milliseconds
        std::string findings_dir = "findings"; // Output directory for findings
    };

    struct LoggingConfig {
        std::string level = "info";          // Log level
        std::string file = "/var/log/environet/environet.log"; // Log file path
        bool console = true;                 // Enable console logging
        size_t max_size_mb = 5;              // Max log file size
        int max_files = 3;                   // Max log files to keep
    };

    struct MetricsConfig {
        std::vector<std::string> ping_targets = {"8.8.8.8", "1.1.1.1"};
        std::string iperf_server = "";       // iperf3 server (optional)
        int ping_interval_ms = 10000;        // Ping interval in milliseconds
        int iperf_duration = 10;             // iperf3 test duration in seconds
    };

    // Configuration sections
    I2CConfig i2c;
    WifiConfig wifi;
    PcapConfig pcap;
    CorrelatorConfig correlator;
    LoggingConfig logging;
    MetricsConfig metrics;

    /**
     * @brief Load configuration from JSON file
     * 
     * @param path Path to configuration file
     * @return Config instance
     * @throws std::runtime_error if file cannot be loaded or is invalid
     */
    static Config load(const std::string& path);

    /**
     * @brief Load configuration from JSON string
     * 
     * @param json_str JSON configuration string
     * @return Config instance
     * @throws std::runtime_error if JSON is invalid
     */
    static Config from_json(const std::string& json_str);

    /**
     * @brief Get default configuration
     * 
     * @return Config instance with sensible defaults
     */
    static Config get_defaults();

    /**
     * @brief Validate configuration
     * 
     * @throws std::runtime_error if configuration is invalid
     */
    void validate() const;

    /**
     * @brief Convert configuration to JSON
     * 
     * @return JSON representation
     */
    nlohmann::json to_json() const;

private:
    /**
     * @brief Load configuration from JSON object
     * 
     * @param j JSON object
     */
    void from_json_object(const nlohmann::json& j);

    /**
     * @brief Set default values
     */
    void set_defaults();
};

} // namespace core
} // namespace environet
