#pragma once

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <nlohmann/json.hpp>

namespace environet {
namespace net {

/**
 * @brief Ping statistics structure
 * 
 * Contains ping test results and statistics
 */
struct PingStats {
    std::string target;         // Target hostname/IP
    bool reachable;             // Whether target is reachable
    double min_rtt_ms;          // Minimum round-trip time in milliseconds
    double avg_rtt_ms;          // Average round-trip time in milliseconds
    double max_rtt_ms;          // Maximum round-trip time in milliseconds
    double stddev_rtt_ms;       // Standard deviation of RTT
    int packets_sent;           // Number of packets sent
    int packets_received;       // Number of packets received
    int packets_lost;           // Number of packets lost
    double loss_percentage;     // Packet loss percentage
    uint64_t timestamp_ms;      // Test timestamp in milliseconds
    
    // Default constructor
    PingStats() : reachable(false), min_rtt_ms(0.0), avg_rtt_ms(0.0), 
                   max_rtt_ms(0.0), stddev_rtt_ms(0.0), packets_sent(0), 
                   packets_received(0), packets_lost(0), loss_percentage(0.0), 
                   timestamp_ms(0) {}
};

/**
 * @brief iperf3 test results structure
 * 
 * Contains iperf3 bandwidth test results
 */
struct Iperf3Results {
    std::string server;         // iperf3 server address
    std::string protocol;       // Protocol used (TCP/UDP)
    double bandwidth_mbps;      // Bandwidth in Mbps
    double jitter_ms;           // Jitter in milliseconds (UDP only)
    double packet_loss;         // Packet loss percentage (UDP only)
    int duration_seconds;       // Test duration in seconds
    uint64_t timestamp_ms;      // Test timestamp in milliseconds
    bool success;               // Whether test completed successfully
    std::string error_message;  // Error message if test failed
    
    // Default constructor
    Iperf3Results() : bandwidth_mbps(0.0), jitter_ms(0.0), packet_loss(0.0), 
                       duration_seconds(0), timestamp_ms(0), success(false) {}
};

/**
 * @brief Network metrics class
 * 
 * Provides network performance testing capabilities including ping and iperf3
 */
class Metrics {
public:
    /**
     * @brief Constructor
     * 
     * @param config_path Path to configuration file
     */
    explicit Metrics(const std::string& config_path);
    
    /**
     * @brief Destructor
     */
    ~Metrics();
    
    /**
     * @brief Initialize metrics collection
     * 
     * @return true if successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Perform ping test to a target
     * 
     * @param target Target hostname or IP address
     * @param count Number of ping packets to send
     * @param timeout_ms Timeout for each ping in milliseconds
     * @return PingStats with test results
     */
    PingStats ping_test(const std::string& target, int count = 4, int timeout_ms = 1000);
    
    /**
     * @brief Perform ping test to multiple targets
     * 
     * @param targets Vector of target hostnames/IPs
     * @param count Number of ping packets per target
     * @param timeout_ms Timeout for each ping in milliseconds
     * @return Vector of PingStats for each target
     */
    std::vector<PingStats> ping_multiple(const std::vector<std::string>& targets, 
                                         int count = 4, int timeout_ms = 1000);
    
    /**
     * @brief Perform iperf3 bandwidth test
     * 
     * @param server iperf3 server address
     * @param duration Test duration in seconds
     * @param protocol Protocol to use (TCP or UDP)
     * @param port Server port (default: 5201)
     * @return Iperf3Results with test results
     */
    Iperf3Results iperf3_test(const std::string& server, int duration = 10, 
                              const std::string& protocol = "TCP", int port = 5201);
    
    /**
     * @brief Get metrics statistics
     * 
     * @return JSON object with metrics statistics
     */
    nlohmann::json get_stats() const;
    
    /**
     * @brief Get last error message
     * 
     * @return Error message string
     */
    std::string get_last_error() const { return last_error_; }

private:
    // Configuration
    std::vector<std::string> ping_targets_;
    std::string iperf3_server_;
    int ping_interval_ms_;
    int iperf3_duration_;
    
    // Statistics
    int ping_tests_run_;
    int iperf3_tests_run_;
    int ping_errors_;
    int iperf3_errors_;
    uint64_t start_time_ms_;
    
    // Error handling
    std::string last_error_;
    
    // Private methods
    PingStats parse_ping_output(const std::string& output, const std::string& target);
    Iperf3Results parse_iperf3_output(const std::string& output, const std::string& server);
    
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
     * @brief Get current timestamp in milliseconds
     * 
     * @return Current timestamp
     */
    static uint64_t get_current_time_ms();
    
    /**
     * @brief Extract numeric value from string using regex
     * 
     * @param text Text to search in
     * @param pattern Regex pattern to match
     * @return Extracted numeric value, or 0.0 if not found
     */
    static double extract_numeric_value(const std::string& text, const std::string& pattern);
    
    /**
     * @brief Check if iperf3 is available
     * 
     * @return true if iperf3 command is available
     */
    static bool check_iperf3_available();
    
    /**
     * @brief Check if ping command is available
     * 
     * @return true if ping command is available
     */
    static bool check_ping_available();
};

} // namespace net
} // namespace environet
