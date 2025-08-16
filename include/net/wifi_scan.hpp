#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <nlohmann/json.hpp>

namespace environet {
namespace net {

/**
 * @brief Basic Service Set (BSS) information
 * 
 * Contains information about a detected WiFi access point
 */
struct BssInfo {
    std::string ssid;           // Service Set Identifier
    std::string bssid;          // Basic Service Set Identifier (MAC address)
    int freq;                   // Frequency in MHz
    int signal_mbm;             // Signal strength in mBm (dBm * 100)
    uint64_t last_seen_ms;      // Last seen timestamp in milliseconds
    int channel;                // Channel number
    std::string capabilities;    // Capability information
    bool is_connected;          // Whether this is the currently connected AP
    
    // Default constructor
    BssInfo() : freq(0), signal_mbm(0), last_seen_ms(0), channel(0), is_connected(false) {}
    
    // Constructor with parameters
    BssInfo(const std::string& ssid, const std::string& bssid, int freq, int signal_mbm)
        : ssid(ssid), bssid(bssid), freq(freq), signal_mbm(signal_mbm), 
          last_seen_ms(0), channel(0), is_connected(false) {}
};

/**
 * @brief WiFi scanning class using libnl/nl80211
 * 
 * Provides WiFi network scanning capabilities with fallback to iw command
 */
class WifiScan {
public:
    /**
     * @brief Constructor
     * 
     * @param config_path Path to configuration file
     */
    explicit WifiScan(const std::string& config_path);
    
    /**
     * @brief Destructor
     */
    ~WifiScan();
    
    /**
     * @brief Initialize WiFi scanning
     * 
     * @return true if successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Perform a WiFi scan
     * 
     * @return Vector of detected BSS information
     */
    std::vector<BssInfo> scan();
    
    /**
     * @brief Get currently connected network
     * 
     * @return BssInfo of connected network, or empty BssInfo if not connected
     */
    BssInfo get_connected_network();
    
    /**
     * @brief Get scan statistics
     * 
     * @return JSON object with scan statistics
     */
    nlohmann::json get_scan_stats() const;
    
    /**
     * @brief Get last error message
     * 
     * @return Error message string
     */
    std::string get_last_error() const { return last_error_; }

private:
    // Configuration
    std::string iface_scan_;
    std::string iface_ap_;
    int scan_interval_ms_;
    bool monitor_mode_;
    
    // libnl/netlink
    struct nl_sock* nl_sock_;
    struct nl_cache* nl_cache_;
    int nl_family_;
    
    // Scan state
    std::vector<BssInfo> last_scan_results_;
    std::chrono::steady_clock::time_point last_scan_time_;
    int scan_count_;
    int scan_errors_;
    
    // Error handling
    std::string last_error_;
    
    // Private methods
    bool init_libnl();
    bool init_interface();
    std::vector<BssInfo> scan_libnl();
    std::vector<BssInfo> scan_fallback();
    std::vector<BssInfo> parse_iw_output(const std::string& output);
    std::vector<BssInfo> parse_proc_wireless();
    
    /**
     * @brief Parse iw scan results
     * 
     * @param output Output from 'iw dev <iface> scan' command
     * @return Vector of BSS information
     */
    std::vector<BssInfo> parse_scan_results(const std::string& output);
    
    /**
     * @brief Execute shell command and return output
     * 
     * @param command Command to execute
     * @return Command output as string
     */
    std::string execute_command(const std::string& command);
    
    /**
     * @brief Set error message
     * 
     * @param error Error message
     */
    void set_error(const std::string& error);
    
    /**
     * @brief Clean up libnl resources
     */
    void cleanup_libnl();
    
    /**
     * @brief Convert frequency to channel number
     * 
     * @param freq Frequency in MHz
     * @return Channel number
     */
    static int freq_to_channel(int freq);
    
    /**
     * @brief Convert signal strength from dBm to mBm
     * 
     * @param dbm Signal strength in dBm
     * @return Signal strength in mBm
     */
    static int dbm_to_mbm(int dbm);
};

} // namespace net
} // namespace environet
