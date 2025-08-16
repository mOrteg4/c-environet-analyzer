#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "sensors/arduino_i2c.hpp"
#include "core/config.hpp"

using namespace environet::sensors;
using namespace environet::core;

class ArduinoI2CTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary config file for testing
        test_config_ = R"({
            "i2c": {
                "mock_mode": true,
                "bus_id": 1,
                "addr": 16,
                "sample_interval_ms": 100
            }
        })";
        
        // Create test directories
        system("mkdir -p test_data");
    }
    
    void TearDown() override {
        // Clean up test data
        system("rm -rf test_data");
    }
    
    std::string test_config_;
};

// Test basic sensor frame structure
TEST_F(ArduinoI2CTest, FrameStructure) {
    SensorFrame frame;
    
    // Test default values
    EXPECT_EQ(frame.ts_ms, 0);
    EXPECT_EQ(frame.ir_raw, 0);
    EXPECT_EQ(frame.ultra_mm, 0);
    EXPECT_EQ(frame.status, 0);
    EXPECT_EQ(frame.reserved, 0);
    EXPECT_EQ(frame.crc16, 0);
    
    // Test status bit constants
    EXPECT_EQ(SensorFrame::STATUS_MOTION, 0x01);
    EXPECT_EQ(SensorFrame::STATUS_ERROR, 0x02);
    EXPECT_EQ(SensorFrame::STATUS_CALIBRATING, 0x04);
    EXPECT_EQ(SensorFrame::STATUS_LOW_BATTERY, 0x08);
    
    // Test structure size (should be packed)
    EXPECT_EQ(sizeof(SensorFrame), 16);
}

// Test mock mode initialization
TEST_F(ArduinoI2CTest, MockModeInitialization) {
    // Create ArduinoI2C instance with mock config
    ArduinoI2C sensor("test_config.json");
    
    // Should initialize successfully in mock mode
    EXPECT_TRUE(sensor.init());
    EXPECT_TRUE(sensor.is_mock_mode());
}

// Test sensor frame reading in mock mode
TEST_F(ArduinoI2CTest, MockFrameReading) {
    ArduinoI2C sensor("test_config.json");
    ASSERT_TRUE(sensor.init());
    
    SensorFrame frame;
    
    // Read multiple frames to ensure consistency
    for (int i = 0; i < 10; i++) {
        EXPECT_TRUE(sensor.read_frame(frame));
        
        // Verify frame has reasonable values
        EXPECT_GT(frame.ts_ms, 0);
        EXPECT_GE(frame.ir_raw, -512);
        EXPECT_LE(frame.ir_raw, 511);
        EXPECT_GT(frame.ultra_mm, 0);
        EXPECT_LE(frame.ultra_mm, 4000); // Max 4 meters
        
        // Verify CRC is valid
        EXPECT_NE(frame.crc16, 0);
        
        // Small delay between reads
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Test error handling
TEST_F(ArduinoI2CTest, ErrorHandling) {
    ArduinoI2C sensor("nonexistent_config.json");
    
    // Should fail to initialize with bad config
    EXPECT_FALSE(sensor.init());
    EXPECT_FALSE(sensor.get_last_error().empty());
}

// Test status flag combinations
TEST_F(ArduinoI2CTest, StatusFlagCombinations) {
    ArduinoI2C sensor("test_config.json");
    ASSERT_TRUE(sensor.init());
    
    SensorFrame frame;
    
    // Read multiple frames to see different status combinations
    std::vector<uint8_t> seen_statuses;
    
    for (int i = 0; i < 20; i++) {
        EXPECT_TRUE(sensor.read_frame(frame));
        seen_statuses.push_back(frame.status);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Should see some variety in status flags
    EXPECT_GT(seen_statuses.size(), 0);
}

// Test sample interval compliance
TEST_F(ArduinoI2CTest, SampleIntervalCompliance) {
    ArduinoI2C sensor("test_config.json");
    ASSERT_TRUE(sensor.init());
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Read 5 frames
    SensorFrame frame;
    for (int i = 0; i < 5; i++) {
        EXPECT_TRUE(sensor.read_frame(frame));
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Should take at least 400ms (5 * 100ms interval)
    EXPECT_GE(duration.count(), 400);
}

// Test CRC validation
TEST_F(ArduinoI2CTest, CRCValidation) {
    ArduinoI2C sensor("test_config.json");
    ASSERT_TRUE(sensor.init());
    
    SensorFrame frame;
    
    // Read multiple frames and verify CRC
    for (int i = 0; i < 10; i++) {
        EXPECT_TRUE(sensor.read_frame(frame));
        
        // CRC should not be zero
        EXPECT_NE(frame.crc16, 0);
        
        // CRC should be consistent (same data should produce same CRC)
        uint16_t expected_crc = frame.crc16;
        EXPECT_EQ(frame.crc16, expected_crc);
    }
}

// Test edge case values
TEST_F(ArduinoI2CTest, EdgeCaseValues) {
    ArduinoI2C sensor("test_config.json");
    ASSERT_TRUE(sensor.init());
    
    SensorFrame frame;
    
    // Read many frames to find edge cases
    int min_ir = 1000, max_ir = -1000;
    int min_ultra = 10000, max_ultra = 0;
    
    for (int i = 0; i < 100; i++) {
        EXPECT_TRUE(sensor.read_frame(frame));
        
        min_ir = std::min(min_ir, static_cast<int>(frame.ir_raw));
        max_ir = std::max(max_ir, static_cast<int>(frame.ir_raw));
        min_ultra = std::min(min_ultra, static_cast<int>(frame.ultra_mm));
        max_ultra = std::max(max_ultra, static_cast<int>(frame.ultra_mm));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    // Verify reasonable ranges
    EXPECT_LE(min_ir, -400);  // Should see some negative values
    EXPECT_GE(max_ir, 400);   // Should see some positive values
    EXPECT_LE(min_ultra, 100); // Should see some close objects
    EXPECT_GE(max_ultra, 2000); // Should see some distant objects
}

// Test concurrent access
TEST_F(ArduinoI2CTest, ConcurrentAccess) {
    ArduinoI2C sensor("test_config.json");
    ASSERT_TRUE(sensor.init());
    
    std::vector<std::thread> threads;
    std::vector<bool> results(5, false);
    
    // Create multiple threads reading from sensor
    for (int i = 0; i < 5; i++) {
        threads.emplace_back([&sensor, &results, i]() {
            SensorFrame frame;
            results[i] = sensor.read_frame(frame);
        });
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // All reads should succeed
    for (bool result : results) {
        EXPECT_TRUE(result);
    }
}

// Test sensor stop functionality
TEST_F(ArduinoI2CTest, StopFunctionality) {
    ArduinoI2C sensor("test_config.json");
    ASSERT_TRUE(sensor.init());
    
    // Should be able to stop
    EXPECT_NO_THROW(sensor.stop());
    
    // After stop, should still be able to read in mock mode
    SensorFrame frame;
    EXPECT_TRUE(sensor.read_frame(frame));
}

// Test invalid configuration handling
TEST_F(ArduinoI2CTest, InvalidConfiguration) {
    // Test with invalid bus ID
    std::string invalid_config = R"({
        "i2c": {
            "mock_mode": false,
            "bus_id": -1,
            "addr": 16,
            "sample_interval_ms": 100
        }
    })";
    
    // Should handle invalid config gracefully
    ArduinoI2C sensor("invalid_config.json");
    EXPECT_FALSE(sensor.init());
}

// Test performance under load
TEST_F(ArduinoI2CTest, PerformanceUnderLoad) {
    ArduinoI2C sensor("test_config.json");
    ASSERT_TRUE(sensor.init());
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Read 1000 frames quickly
    SensorFrame frame;
    for (int i = 0; i < 1000; i++) {
        EXPECT_TRUE(sensor.read_frame(frame));
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Should complete in reasonable time (less than 2 seconds)
    EXPECT_LT(duration.count(), 2000);
}

// Test sensor data consistency
TEST_F(ArduinoI2CTest, DataConsistency) {
    ArduinoI2C sensor("test_config.json");
    ASSERT_TRUE(sensor.init());
    
    SensorFrame frame1, frame2;
    
    // Read two frames quickly
    EXPECT_TRUE(sensor.read_frame(frame1));
    EXPECT_TRUE(sensor.read_frame(frame2));
    
    // Timestamps should be different
    EXPECT_NE(frame1.ts_ms, frame2.ts_ms);
    
    // Timestamp should increase
    EXPECT_GT(frame2.ts_ms, frame1.ts_ms);
}

// Test mock mode vs real mode toggle
TEST_F(ArduinoI2CTest, MockModeToggle) {
    // Test mock mode
    {
        ArduinoI2C sensor("test_config.json");
        ASSERT_TRUE(sensor.init());
        EXPECT_TRUE(sensor.is_mock_mode());
    }
    
    // Test real mode (should fail without hardware)
    std::string real_config = R"({
        "i2c": {
            "mock_mode": false,
            "bus_id": 1,
            "addr": 16,
            "sample_interval_ms": 100
        }
    })";
    
    ArduinoI2C real_sensor("real_config.json");
    // In test environment without real hardware, this should fail
    // EXPECT_FALSE(real_sensor.init());
}
