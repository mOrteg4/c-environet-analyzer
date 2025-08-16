#pragma once

#include <cstdint>
#include <string>
#include <chrono>

namespace environet {
namespace util {

/**
 * @brief Time utility functions
 * 
 * Provides common time-related operations and conversions
 */
class Time {
public:
    /**
     * @brief Get current timestamp in milliseconds since epoch
     * 
     * @return Current timestamp in milliseconds
     */
    static uint64_t get_current_time_ms();
    
    /**
     * @brief Get current timestamp in microseconds since epoch
     * 
     * @return Current timestamp in microseconds
     */
    static uint64_t get_current_time_us();
    
    /**
     * @brief Get current timestamp in nanoseconds since epoch
     * 
     * @return Current timestamp in nanoseconds
     */
    static uint64_t get_current_time_ns();
    
    /**
     * @brief Convert milliseconds to seconds
     * 
     * @param ms Milliseconds
     * @return Seconds
     */
    static double ms_to_seconds(uint64_t ms);
    
    /**
     * @brief Convert seconds to milliseconds
     * 
     * @param seconds Seconds
     * @return Milliseconds
     */
    static uint64_t seconds_to_ms(double seconds);
    
    /**
     * @brief Convert milliseconds to microseconds
     * 
     * @param ms Milliseconds
     * @return Microseconds
     */
    static uint64_t ms_to_us(uint64_t ms);
    
    /**
     * @brief Convert microseconds to milliseconds
     * 
     * @param us Microseconds
     * @return Milliseconds
     */
    static uint64_t us_to_ms(uint64_t us);
    
    /**
     * @brief Format timestamp as ISO 8601 string
     * 
     * @param timestamp_ms Timestamp in milliseconds
     * @return ISO 8601 formatted string
     */
    static std::string format_iso8601(uint64_t timestamp_ms);
    
    /**
     * @brief Format timestamp as human-readable string
     * 
     * @param timestamp_ms Timestamp in milliseconds
     * @return Human-readable string
     */
    static std::string format_human_readable(uint64_t timestamp_ms);
    
    /**
     * @brief Parse ISO 8601 string to timestamp
     * 
     * @param iso_string ISO 8601 formatted string
     * @return Timestamp in milliseconds, or 0 if parsing failed
     */
    static uint64_t parse_iso8601(const std::string& iso_string);
    
    /**
     * @brief Get time difference in milliseconds
     * 
     * @param start_time Start time in milliseconds
     * @param end_time End time in milliseconds
     * @return Time difference in milliseconds
     */
    static int64_t get_time_diff_ms(uint64_t start_time, uint64_t end_time);
    
    /**
     * @brief Check if timestamp is recent (within specified window)
     * 
     * @param timestamp Timestamp to check
     * @param window_ms Time window in milliseconds
     * @return true if timestamp is within window
     */
    static bool is_recent(uint64_t timestamp, uint64_t window_ms);
    
    /**
     * @brief Sleep for specified duration
     * 
     * @param duration_ms Duration in milliseconds
     */
    static void sleep_ms(uint64_t duration_ms);
    
    /**
     * @brief Sleep for specified duration
     * 
     * @param duration_us Duration in microseconds
     */
    static void sleep_us(uint64_t duration_us);
    
    /**
     * @brief Get monotonic clock time in milliseconds
     * 
     * @return Monotonic clock time in milliseconds
     */
    static uint64_t get_monotonic_time_ms();

private:
    // Prevent instantiation
    Time() = delete;
};

} // namespace util
} // namespace environet
