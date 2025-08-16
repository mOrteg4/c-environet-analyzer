#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>

#include "core/config.hpp"

using namespace environet::core;

class ConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directories
        system("mkdir -p test_data");
    }
    
    void TearDown() override {
        // Clean up test data
        system("rm -rf test_data");
    }
    
    void CreateTestConfigFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        file << content;
        file.close();
    }
    
    std::string GetTestConfigPath(const std::string& filename) {
        return "test_data/" + filename;
    }
};

// Test default configuration
TEST_F(ConfigTest, DefaultConfiguration) {
    Config config = Config::get_defaults();
    
    // Test I2C defaults
    EXPECT_TRUE(config.i2c.mock_mode);
    EXPECT_EQ(config.i2c.bus_id, 1);
    EXPECT_EQ(config.i2c.addr, 16);
    EXPECT_EQ(config.i2c.sample_interval_ms, 100);
    
    // Test WiFi defaults
    EXPECT_EQ(config.wifi.iface_ap, "wlan1");
    EXPECT_EQ(config.wifi.iface_scan, "wlan0");
    EXPECT_EQ(config.wifi.scan_interval_ms, 5000);
    EXPECT_FALSE(config.wifi.monitor_mode);
    
    // Test PCAP defaults
    EXPECT_EQ(config.pcap.bpf, "not (type mgt)");
    EXPECT_EQ(config.pcap.output_dir, "captures");
    EXPECT_EQ(config.pcap.max_file_size_mb, 100);
    EXPECT_EQ(config.pcap.max_files, 10);
    
    // Test Correlator defaults
    EXPECT_EQ(config.correlator.sensor_threshold, 200);
    EXPECT_EQ(config.correlator.window_ms, 5000);
    EXPECT_EQ(config.correlator.findings_dir, "findings");
    
    // Test Logging defaults
    EXPECT_EQ(config.logging.level, "info");
    EXPECT_EQ(config.logging.file, "/var/log/environet/environet.log");
    EXPECT_TRUE(config.logging.console);
    EXPECT_EQ(config.logging.max_size_mb, 5);
    EXPECT_EQ(config.logging.max_files, 3);
    
    // Test Metrics defaults
    EXPECT_EQ(config.metrics.ping_targets.size(), 3);
    EXPECT_EQ(config.metrics.ping_targets[0], "8.8.8.8");
    EXPECT_EQ(config.metrics.ping_targets[1], "1.1.1.1");
    EXPECT_EQ(config.metrics.ping_targets[2], "google.com");
    EXPECT_TRUE(config.metrics.iperf_server.empty());
    EXPECT_EQ(config.metrics.ping_interval_ms, 10000);
    EXPECT_EQ(config.metrics.iperf3_duration, 10);
}

// Test configuration loading from file
TEST_F(ConfigTest, LoadFromFile) {
    std::string config_content = R"({
        "i2c": {
            "mock_mode": false,
            "bus_id": 2,
            "addr": 32,
            "sample_interval_ms": 200
        },
        "wifi": {
            "iface_ap": "wlan2",
            "iface_scan": "wlan1",
            "scan_interval_ms": 10000,
            "monitor_mode": true
        }
    })";
    
    std::string config_file = GetTestConfigPath("test_config.json");
    CreateTestConfigFile(config_file, config_content);
    
    Config config = Config::load(config_file);
    
    // Test loaded values
    EXPECT_FALSE(config.i2c.mock_mode);
    EXPECT_EQ(config.i2c.bus_id, 2);
    EXPECT_EQ(config.i2c.addr, 32);
    EXPECT_EQ(config.i2c.sample_interval_ms, 200);
    
    EXPECT_EQ(config.wifi.iface_ap, "wlan2");
    EXPECT_EQ(config.wifi.iface_scan, "wlan1");
    EXPECT_EQ(config.wifi.scan_interval_ms, 10000);
    EXPECT_TRUE(config.wifi.monitor_mode);
    
    // Default values should still be present for non-specified fields
    EXPECT_EQ(config.pcap.bpf, "not (type mgt)");
    EXPECT_EQ(config.correlator.sensor_threshold, 200);
}

// Test configuration loading from JSON string
TEST_F(ConfigTest, LoadFromJsonString) {
    std::string json_string = R"({
        "i2c": {
            "mock_mode": true,
            "sample_interval_ms": 50
        },
        "pcap": {
            "max_file_size_mb": 50,
            "max_files": 5
        }
    })";
    
    Config config = Config::from_json(json_string);
    
    // Test specified values
    EXPECT_TRUE(config.i2c.mock_mode);
    EXPECT_EQ(config.i2c.sample_interval_ms, 50);
    EXPECT_EQ(config.pcap.max_file_size_mb, 50);
    EXPECT_EQ(config.pcap.max_files, 5);
    
    // Default values should be preserved
    EXPECT_EQ(config.i2c.bus_id, 1);
    EXPECT_EQ(config.i2c.addr, 16);
    EXPECT_EQ(config.pcap.bpf, "not (type mgt)");
}

// Test configuration validation
TEST_F(ConfigTest, ConfigurationValidation) {
    Config config = Config::get_defaults();
    
    // Valid configuration should not throw
    EXPECT_NO_THROW(config.validate());
    
    // Test invalid configurations
    Config invalid_config = config;
    
    // Invalid I2C bus ID
    invalid_config.i2c.bus_id = -1;
    EXPECT_THROW(invalid_config.validate(), std::runtime_error);
    
    // Invalid I2C address
    invalid_config = config;
    invalid_config.i2c.addr = 0;
    EXPECT_THROW(invalid_config.validate(), std::runtime_error);
    
    // Invalid sample interval
    invalid_config = config;
    invalid_config.i2c.sample_interval_ms = 0;
    EXPECT_THROW(invalid_config.validate(), std::runtime_error);
    
    // Invalid scan interval
    invalid_config = config;
    invalid_config.wifi.scan_interval_ms = 0;
    EXPECT_THROW(invalid_config.validate(), std::runtime_error);
    
    // Invalid file size
    invalid_config = config;
    invalid_config.pcap.max_file_size_mb = 0;
    EXPECT_THROW(invalid_config.validate(), std::runtime_error);
    
    // Invalid correlation window
    invalid_config = config;
    invalid_config.correlator.window_ms = 0;
    EXPECT_THROW(invalid_config.validate(), std::runtime_error);
}

// Test configuration serialization
TEST_F(ConfigTest, ConfigurationSerialization) {
    Config original_config = Config::get_defaults();
    
    // Modify some values
    original_config.i2c.mock_mode = false;
    original_config.i2c.bus_id = 3;
    original_config.wifi.scan_interval_ms = 15000;
    
    // Serialize to JSON
    nlohmann::json json = original_config.to_json();
    
    // Deserialize back
    Config deserialized_config = Config::from_json(json.dump());
    
    // Values should match
    EXPECT_EQ(original_config.i2c.mock_mode, deserialized_config.i2c.mock_mode);
    EXPECT_EQ(original_config.i2c.bus_id, deserialized_config.i2c.bus_id);
    EXPECT_EQ(original_config.wifi.scan_interval_ms, deserialized_config.wifi.scan_interval_ms);
    
    // All other values should also match
    EXPECT_EQ(original_config.i2c.addr, deserialized_config.i2c.addr);
    EXPECT_EQ(original_config.pcap.bpf, deserialized_config.pcap.bpf);
    EXPECT_EQ(original_config.correlator.sensor_threshold, deserialized_config.correlator.sensor_threshold);
}

// Test error handling for invalid JSON
TEST_F(ConfigTest, InvalidJsonHandling) {
    // Test malformed JSON
    std::string invalid_json = "{ invalid json }";
    EXPECT_THROW(Config::from_json(invalid_json), std::runtime_error);
    
    // Test empty JSON
    std::string empty_json = "";
    EXPECT_THROW(Config::from_json(empty_json), std::runtime_error);
    
    // Test missing required fields
    std::string incomplete_json = R"({
        "i2c": {
            "mock_mode": true
        }
    })";
    
    // Should not throw for incomplete JSON (uses defaults)
    EXPECT_NO_THROW(Config::from_json(incomplete_json));
}

// Test configuration file not found
TEST_F(ConfigTest, FileNotFound) {
    std::string nonexistent_file = "nonexistent_config.json";
    EXPECT_THROW(Config::load(nonexistent_file), std::runtime_error);
}

// Test configuration with empty arrays
TEST_F(ConfigTest, EmptyArrays) {
    std::string config_with_empty_arrays = R"({
        "metrics": {
            "ping_targets": []
        }
    })";
    
    Config config = Config::from_json(config_with_empty_arrays);
    EXPECT_TRUE(config.metrics.ping_targets.empty());
}

// Test configuration with very large values
TEST_F(ConfigTest, LargeValues) {
    std::string config_with_large_values = R"({
        "i2c": {
            "sample_interval_ms": 86400000
        },
        "pcap": {
            "max_file_size_mb": 1073741824
        }
    })";
    
    Config config = Config::from_json(config_with_large_values);
    EXPECT_EQ(config.i2c.sample_interval_ms, 86400000);
    EXPECT_EQ(config.pcap.max_file_size_mb, 1073741824);
}

// Test configuration with special characters
TEST_F(ConfigTest, SpecialCharacters) {
    std::string config_with_special_chars = R"({
        "pcap": {
            "bpf": "not (type mgt) and (host 192.168.1.1 or host 10.0.0.1)"
        },
        "logging": {
            "file": "/var/log/environet/analyzer-$(date +%Y%m%d).log"
        }
    })";
    
    Config config = Config::from_json(config_with_special_chars);
    EXPECT_EQ(config.pcap.bpf, "not (type mgt) and (host 192.168.1.1 or host 10.0.0.1)");
    EXPECT_EQ(config.logging.file, "/var/log/environet/analyzer-$(date +%Y%m%d).log");
}

// Test configuration inheritance
TEST_F(ConfigTest, ConfigurationInheritance) {
    Config base_config = Config::get_defaults();
    
    // Create a partial override
    std::string override_json = R"({
        "i2c": {
            "mock_mode": false
        }
    })";
    
    Config override_config = Config::from_json(override_json);
    
    // Override values should be applied
    EXPECT_FALSE(override_config.i2c.mock_mode);
    
    // Base values should be preserved
    EXPECT_EQ(override_config.i2c.bus_id, base_config.i2c.bus_id);
    EXPECT_EQ(override_config.i2c.addr, base_config.i2c.addr);
    EXPECT_EQ(override_config.wifi.iface_ap, base_config.wifi.iface_ap);
}

// Test configuration with nested objects
TEST_F(ConfigTest, NestedObjects) {
    std::string nested_config = R"({
        "i2c": {
            "mock_mode": true,
            "advanced": {
                "timeout_ms": 1000,
                "retry_count": 3
            }
        }
    })";
    
    // Should handle unknown nested fields gracefully
    EXPECT_NO_THROW(Config::from_json(nested_config));
}

// Test configuration performance
TEST_F(ConfigTest, ConfigurationPerformance) {
    std::string large_config = R"({
        "i2c": {
            "mock_mode": true
        }
    })";
    
    // Add many ping targets
    for (int i = 0; i < 1000; i++) {
        large_config += R"(, "ping_target_)" + std::to_string(i) + R"(": "192.168.1.)" + std::to_string(i) + R"(")";
    }
    large_config += "}";
    
    auto start_time = std::chrono::steady_clock::now();
    
    // Should parse large configs quickly
    Config config = Config::from_json(large_config);
    
    auto end_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Should complete in less than 100ms
    EXPECT_LT(duration.count(), 100);
}
