#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <chrono>
#include <thread>

#include "util/time.hpp"

using namespace environet::util;

class TimeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Test setup
    }
    
    void TearDown() override {
        // Test cleanup
    }
};

// Test current time functions
TEST_F(TimeTest, CurrentTimeFunctions) {
    // Test that we can get current time
    uint64_t ms_now = Time::get_current_time_ms();
    uint64_t us_now = Time::get_current_time_us();
    uint64_t ns_now = Time::get_current_time_ns();
    
    // Times should be reasonable (not zero, not too far in the past)
    EXPECT_GT(ms_now, 0);
    EXPECT_GT(us_now, 0);
    EXPECT_GT(ns_now, 0);
    
    // Microseconds should be greater than milliseconds
    EXPECT_GE(us_now, ms_now * 1000);
    
    // Nanoseconds should be greater than microseconds
    EXPECT_GE(ns_now, us_now * 1000);
    
    // Times should be close to each other (within 1 second)
    uint64_t ms_from_us = us_now / 1000;
    uint64_t ms_from_ns = ns_now / 1000000;
    
    EXPECT_NEAR(ms_now, ms_from_us, 1000);
    EXPECT_NEAR(ms_now, ms_from_ns, 1000);
}

// Test time conversion functions
TEST_F(TimeTest, TimeConversionFunctions) {
    // Test milliseconds to seconds
    EXPECT_DOUBLE_EQ(Time::ms_to_seconds(1000), 1.0);
    EXPECT_DOUBLE_EQ(Time::ms_to_seconds(500), 0.5);
    EXPECT_DOUBLE_EQ(Time::ms_to_seconds(100), 0.1);
    EXPECT_DOUBLE_EQ(Time::ms_to_seconds(0), 0.0);
    
    // Test seconds to milliseconds
    EXPECT_EQ(Time::seconds_to_ms(1.0), 1000);
    EXPECT_EQ(Time::seconds_to_ms(0.5), 500);
    EXPECT_EQ(Time::seconds_to_ms(0.1), 100);
    EXPECT_EQ(Time::seconds_to_ms(0.0), 0);
    
    // Test milliseconds to microseconds
    EXPECT_EQ(Time::ms_to_us(1), 1000);
    EXPECT_EQ(Time::ms_to_us(100), 100000);
    EXPECT_EQ(Time::ms_to_us(0), 0);
    
    // Test microseconds to milliseconds
    EXPECT_EQ(Time::us_to_ms(1000), 1);
    EXPECT_EQ(Time::us_to_ms(100000), 100);
    EXPECT_EQ(Time::us_to_ms(0), 0);
}

// Test time formatting functions
TEST_F(TimeTest, TimeFormattingFunctions) {
    // Test ISO 8601 formatting
    uint64_t test_time = 1640995200000; // 2022-01-01T00:00:00Z
    
    std::string iso_string = Time::format_iso8601(test_time);
    EXPECT_FALSE(iso_string.empty());
    EXPECT_EQ(iso_string.length(), 24); // ISO 8601 format length
    
    // Should contain expected date components
    EXPECT_NE(iso_string.find("2022"), std::string::npos);
    EXPECT_NE(iso_string.find("01"), std::string::npos);
    EXPECT_NE(iso_string.find("00"), std::string::npos);
    
    // Test human readable formatting
    std::string human_string = Time::format_human_readable(test_time);
    EXPECT_FALSE(human_string.empty());
    
    // Should contain readable date/time
    EXPECT_NE(human_string.find("2022"), std::string::npos);
}

// Test time parsing functions
TEST_F(TimeTest, TimeParsingFunctions) {
    // Test valid ISO 8601 strings
    std::string valid_iso = "2022-01-01T00:00:00Z";
    uint64_t parsed_time = Time::parse_iso8601(valid_iso);
    EXPECT_GT(parsed_time, 0);
    
    // Test invalid ISO 8601 strings
    std::string invalid_iso = "invalid-date";
    uint64_t invalid_parsed = Time::parse_iso8601(invalid_iso);
    EXPECT_EQ(invalid_parsed, 0);
    
    // Test empty string
    uint64_t empty_parsed = Time::parse_iso8601("");
    EXPECT_EQ(empty_parsed, 0);
}

// Test time difference functions
TEST_F(TimeTest, TimeDifferenceFunctions) {
    uint64_t start_time = 1000;
    uint64_t end_time = 2000;
    
    // Test positive difference
    int64_t diff = Time::get_time_diff_ms(start_time, end_time);
    EXPECT_EQ(diff, 1000);
    
    // Test negative difference
    diff = Time::get_time_diff_ms(end_time, start_time);
    EXPECT_EQ(diff, -1000);
    
    // Test zero difference
    diff = Time::get_time_diff_ms(start_time, start_time);
    EXPECT_EQ(diff, 0);
    
    // Test large time differences
    uint64_t large_start = 0;
    uint64_t large_end = 86400000; // 24 hours in ms
    diff = Time::get_time_diff_ms(large_start, large_end);
    EXPECT_EQ(diff, 86400000);
}

// Test time recency functions
TEST_F(TimeTest, TimeRecencyFunctions) {
    uint64_t now = Time::get_current_time_ms();
    uint64_t recent_time = now - 1000; // 1 second ago
    uint64_t old_time = now - 10000;   // 10 seconds ago
    
    // Test recent time
    EXPECT_TRUE(Time::is_recent(recent_time, 5000));  // Within 5 seconds
    EXPECT_FALSE(Time::is_recent(old_time, 5000));    // Not within 5 seconds
    
    // Test edge cases
    EXPECT_TRUE(Time::is_recent(now, 0));             // Exactly at boundary
    EXPECT_FALSE(Time::is_recent(now - 1, 0));       // Just outside boundary
}

// Test sleep functions
TEST_F(TimeTest, SleepFunctions) {
    auto start_time = std::chrono::steady_clock::now();
    
    // Test millisecond sleep
    Time::sleep_ms(100);
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Should sleep for at least 100ms
    EXPECT_GE(duration.count(), 100);
    
    // Test microsecond sleep
    start_time = std::chrono::steady_clock::now();
    Time::sleep_us(1000); // 1ms
    
    end_time = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Should sleep for at least 1ms
    EXPECT_GE(duration.count(), 1);
}

// Test monotonic time
TEST_F(TimeTest, MonotonicTime) {
    uint64_t mono1 = Time::get_monotonic_time_ms();
    
    // Sleep a bit
    Time::sleep_ms(10);
    
    uint64_t mono2 = Time::get_monotonic_time_ms();
    
    // Monotonic time should always increase
    EXPECT_GT(mono2, mono1);
    
    // Should have increased by approximately 10ms
    uint64_t diff = mono2 - mono1;
    EXPECT_GE(diff, 10);
    EXPECT_LE(diff, 50); // Allow some tolerance
}

// Test edge cases
TEST_F(TimeTest, EdgeCases) {
    // Test zero values
    EXPECT_DOUBLE_EQ(Time::ms_to_seconds(0), 0.0);
    EXPECT_EQ(Time::seconds_to_ms(0.0), 0);
    EXPECT_EQ(Time::ms_to_us(0), 0);
    EXPECT_EQ(Time::us_to_ms(0), 0);
    
    // Test very large values
    uint64_t large_ms = 86400000ULL * 365ULL; // 1 year in ms
    EXPECT_DOUBLE_EQ(Time::ms_to_seconds(large_ms), 86400.0 * 365);
    EXPECT_EQ(Time::seconds_to_ms(86400.0 * 365), large_ms);
    
    // Test very small values
    EXPECT_DOUBLE_EQ(Time::ms_to_seconds(1), 0.001);
    EXPECT_EQ(Time::seconds_to_ms(0.001), 1);
}

// Test performance
TEST_F(TimeTest, Performance) {
    // Test time function performance
    auto start_time = std::chrono::steady_clock::now();
    
    // Call time functions many times
    for (int i = 0; i < 10000; i++) {
        Time::get_current_time_ms();
        Time::get_current_time_us();
        Time::get_current_time_ns();
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Should complete quickly (less than 100ms for 10k calls)
    EXPECT_LT(duration.count(), 100);
}

// Test thread safety
TEST_F(TimeTest, ThreadSafety) {
    std::vector<std::thread> threads;
    std::vector<uint64_t> results(10);
    
    // Create multiple threads calling time functions
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&results, i]() {
            results[i] = Time::get_current_time_ms();
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All calls should succeed
    for (uint64_t result : results) {
        EXPECT_GT(result, 0);
    }
}

// Test time consistency
TEST_F(TimeTest, TimeConsistency) {
    // Test that time functions are consistent
    uint64_t ms1 = Time::get_current_time_ms();
    uint64_t us1 = Time::get_current_time_us();
    uint64_t ns1 = Time::get_current_time_ns();
    
    // Small delay
    Time::sleep_ms(1);
    
    uint64_t ms2 = Time::get_current_time_ms();
    uint64_t us2 = Time::get_current_time_us();
    uint64_t ns2 = Time::get_current_time_ns();
    
    // All times should increase
    EXPECT_GT(ms2, ms1);
    EXPECT_GT(us2, us1);
    EXPECT_GT(ns2, ns1);
    
    // Microsecond time should be consistent with millisecond time
    uint64_t ms_from_us1 = us1 / 1000;
    uint64_t ms_from_us2 = us2 / 1000;
    
    EXPECT_NEAR(ms1, ms_from_us1, 1);
    EXPECT_NEAR(ms2, ms_from_us2, 1);
}

// Test boundary conditions
TEST_F(TimeTest, BoundaryConditions) {
    // Test maximum values
    uint64_t max_uint64 = std::numeric_limits<uint64_t>::max();
    
    // These should not crash
    EXPECT_NO_THROW(Time::ms_to_seconds(max_uint64));
    EXPECT_NO_THROW(Time::seconds_to_ms(1e9)); // Large but reasonable
    
    // Test minimum values
    EXPECT_NO_THROW(Time::ms_to_seconds(0));
    EXPECT_NO_THROW(Time::seconds_to_ms(0.0));
}

// Test time formatting edge cases
TEST_F(TimeTest, TimeFormattingEdgeCases) {
    // Test zero timestamp
    std::string zero_iso = Time::format_iso8601(0);
    EXPECT_FALSE(zero_iso.empty());
    
    // Test very large timestamp
    uint64_t large_timestamp = 9999999999999; // Far future
    std::string large_iso = Time::format_iso8601(large_timestamp);
    EXPECT_FALSE(large_iso.empty());
    
    // Test human readable formatting for edge cases
    std::string zero_human = Time::format_human_readable(0);
    EXPECT_FALSE(zero_human.empty());
    
    std::string large_human = Time::format_human_readable(large_timestamp);
    EXPECT_FALSE(large_human.empty());
}
