#pragma once

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <chrono>
#include <functional>
#include <nlohmann/json.hpp>

// Forward declarations
namespace environet {
namespace sensors {
    struct SensorFrame;
}
namespace net {
    struct BssInfo;
    struct PacketMeta;
    struct PingStats;
    struct Iperf3Results;
}
}

namespace environet {
namespace correlate {

/**
 * @brief Finding structure for correlation results
 * 
 * Contains correlated environmental and network data
 */
struct Finding {
    uint64_t timestamp_ms;      // Timestamp when finding was generated
    std::string event_type;     // Type of event (motion, signal_drop, etc.)
    std::string description;    // Human-readable description
    
    // Sensor data
    double ir_raw_delta;        // Change in IR sensor reading
    double ultra_distance_delta; // Change in ultrasonic distance
    uint8_t sensor_status;      // Sensor status flags
    
    // Network data
    double rssi_avg;            // Average RSSI during correlation window
    double rssi_delta;          // RSSI change during correlation window
    double ping_latency_delta;  // Change in ping latency
    double packet_loss_delta;   // Change in packet loss
    double throughput_delta;    // Change in throughput
    
    // Correlation metadata
    int correlation_window_ms;  // Correlation window size in milliseconds
    int sensor_threshold;       // Sensor threshold that triggered correlation
    std::vector<std::string> affected_networks; // Networks affected by event
    
    // Default constructor
    Finding() : timestamp_ms(0), ir_raw_delta(0.0), ultra_distance_delta(0.0), 
                 sensor_status(0), rssi_avg(0.0), rssi_delta(0.0), 
                 ping_latency_delta(0.0), packet_loss_delta(0.0), 
                 throughput_delta(0.0), correlation_window_ms(0), sensor_threshold(0) {}
};

/**
 * @brief Time-series data point
 * 
 * Template for storing time-series data with timestamps
 */
template<typename T>
struct TimeSeriesPoint {
    uint64_t timestamp_ms;
    T value;
    
    TimeSeriesPoint(uint64_t ts, const T& val) : timestamp_ms(ts), value(val) {}
};

/**
 * @brief Correlation engine class
 * 
 * Correlates environmental sensor data with network performance metrics
 * to identify patterns and generate findings
 */
class Correlator {
public:
    /**
     * @brief Constructor
     * 
     * @param config_path Path to configuration file
     */
    explicit Correlator(const std::string& config_path);
    
    /**
     * @brief Destructor
     */
    ~Correlator();
    
    /**
     * @brief Initialize correlation engine
     * 
     * @return true if successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Add sensor data to correlation buffer
     * 
     * @param frame Sensor frame data
     */
    void push_sensor(const sensors::SensorFrame& frame);
    
    /**
     * @brief Add WiFi BSS information to correlation buffer
     * 
     * @param bss BSS information
     */
    void push_bss(const net::BssInfo& bss);
    
    /**
     * @brief Add packet metadata to correlation buffer
     * 
     * @param packet Packet metadata
     */
    void push_packet(const net::PacketMeta& packet);
    
    /**
     * @brief Add ping statistics to correlation buffer
     * 
     * @param ping_stats Ping statistics
     */
    void push_ping_stats(const net::PingStats& ping_stats);
    
    /**
     * @brief Add iperf3 results to correlation buffer
     * 
     * @param iperf_results iperf3 test results
     */
    void push_iperf3_results(const net::Iperf3Results& iperf_results);
    
    /**
     * @brief Process correlation data and generate findings
     * 
     * @return Vector of new findings
     */
    std::vector<Finding> process();
    
    /**
     * @brief Get all findings
     * 
     * @return Vector of all findings
     */
    std::vector<Finding> get_findings() const;
    
    /**
     * @brief Get correlation statistics
     * 
     * @return JSON object with correlation statistics
     */
    nlohmann::json get_stats() const;
    
    /**
     * @brief Set correlation callback
     * 
     * @param callback Function to call when new findings are generated
     */
    void set_finding_callback(std::function<void(const Finding&)> callback);
    
    /**
     * @brief Get last error message
     * 
     * @return Error message string
     */
    std::string get_last_error() const { return last_error_; }

private:
    // Configuration
    int sensor_threshold_;
    int correlation_window_ms_;
    std::string findings_dir_;
    
    // Time-series buffers
    std::vector<TimeSeriesPoint<sensors::SensorFrame>> sensor_buffer_;
    std::vector<TimeSeriesPoint<net::BssInfo>> bss_buffer_;
    std::vector<TimeSeriesPoint<net::PacketMeta>> packet_buffer_;
    std::vector<TimeSeriesPoint<net::PingStats>> ping_buffer_;
    std::vector<TimeSeriesPoint<net::Iperf3Results>> iperf_buffer_;
    
    // Findings
    std::vector<Finding> findings_;
    
    // Callbacks
    std::function<void(const Finding&)> finding_callback_;
    
    // Statistics
    uint64_t sensor_events_;
    uint64_t network_events_;
    uint64_t correlations_found_;
    uint64_t start_time_ms_;
    
    // Thread safety
    mutable std::mutex data_mutex_;
    mutable std::mutex findings_mutex_;
    
    // Error handling
    std::string last_error_;
    
    // Private methods
    void cleanup_old_data();
    std::vector<Finding> correlate_sensor_event(const sensors::SensorFrame& frame);
    
    /**
     * @brief Calculate statistics for a time window
     * 
     * @param start_time Start time in milliseconds
     * @param end_time End time in milliseconds
     * @return JSON object with window statistics
     */
    nlohmann::json calculate_window_stats(uint64_t start_time, uint64_t end_time);
    
    /**
     * @brief Save finding to file
     * 
     * @param finding Finding to save
     */
    void save_finding(const Finding& finding);
    
    /**
     * @brief Create findings directory if it doesn't exist
     */
    void ensure_findings_dir();
    
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
     * @brief Check if timestamp is within correlation window
     * 
     * @param timestamp Timestamp to check
     * @param window_start Start of correlation window
     * @return true if timestamp is within window
     */
    bool is_in_window(uint64_t timestamp, uint64_t window_start) const;
    
    /**
     * @brief Calculate average RSSI for a time window
     * 
     * @param start_time Start time in milliseconds
     * @param end_time End time in milliseconds
     * @return Average RSSI value
     */
    double calculate_avg_rssi(uint64_t start_time, uint64_t end_time) const;
    
    /**
     * @brief Calculate RSSI change for a time window
     * 
     * @param start_time Start time in milliseconds
     * @param end_time End time in milliseconds
     * @return RSSI change value
     */
    double calculate_rssi_delta(uint64_t start_time, uint64_t end_time) const;
};

} // namespace correlate
} // namespace environet
