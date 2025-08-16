#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <random>
#include <chrono>
#include <thread>

namespace environet {
namespace sensors {

/**
 * @brief Sensor frame structure matching Arduino firmware
 * 
 * Packed structure for efficient I2C transmission
 * All values are little-endian
 */
#pragma pack(push, 1)
struct SensorFrame {
    uint32_t ts_ms;      // Milliseconds since Arduino boot
    int16_t ir_raw;      // Raw ADC reading from IR sensor
    uint16_t ultra_mm;   // Ultrasonic distance in millimeters
    uint8_t status;      // Status bitflags (bit0=motion, bit1=error, etc.)
    uint8_t reserved;    // Reserved for future use / alignment
    uint16_t crc16;      // CRC-16-CCITT of previous bytes
    
    // Status bit definitions
    static constexpr uint8_t STATUS_MOTION = 0x01;    // Motion detected
    static constexpr uint8_t STATUS_ERROR = 0x02;     // Sensor error
    static constexpr uint8_t STATUS_CALIBRATING = 0x04; // Calibrating
    static constexpr uint8_t STATUS_LOW_BATTERY = 0x08; // Low battery
};
#pragma pack(pop)

/**
 * @brief Arduino I2C communication class
 * 
 * Supports both real hardware I2C and mock mode for development
 */
class ArduinoI2C {
public:
    /**
     * @brief Constructor
     * 
     * @param config Configuration object
     */
    explicit ArduinoI2C(const std::string& config_path);
    
    /**
     * @brief Destructor
     */
    ~ArduinoI2C();
    
    /**
     * @brief Initialize I2C communication
     * 
     * In mock mode: sets up RNG and timers
     * In real mode: opens I2C device and sets slave address
     * 
     * @return true if successful, false otherwise
     */
    bool init();
    
    /**
     * @brief Read a sensor frame from Arduino
     * 
     * @param frame Reference to frame to fill
     * @return true if successful, false otherwise
     */
    bool read_frame(SensorFrame& frame);
    
    /**
     * @brief Stop I2C communication
     */
    void stop();
    
    /**
     * @brief Check if mock mode is enabled
     * 
     * @return true if in mock mode
     */
    bool is_mock_mode() const { return mock_mode_; }
    
    /**
     * @brief Get last error message
     * 
     * @return Error message string
     */
    std::string get_last_error() const { return last_error_; }

private:
    // Configuration
    bool mock_mode_;
    int bus_id_;
    int addr_;
    int sample_interval_ms_;
    
    // Real hardware mode
    int fd_;  // I2C file descriptor
    
    // Mock mode
    std::unique_ptr<std::mt19937> rng_;
    std::chrono::steady_clock::time_point last_sample_;
    uint32_t mock_timestamp_;
    
    // Error handling
    std::string last_error_;
    
    // Private methods
    bool init_real_i2c();
    bool init_mock_i2c();
    bool read_frame_real(SensorFrame& frame);
    bool read_frame_mock(SensorFrame& frame);
    
    /**
     * @brief Compute CRC-16-CCITT
     * 
     * @param data Data buffer
     * @param len Length of data
     * @return CRC-16 value
     */
    static uint16_t compute_crc16(const uint8_t* data, size_t len);
    
    /**
     * @brief Validate CRC-16 in frame
     * 
     * @param frame Frame to validate
     * @return true if CRC is valid
     */
    static bool validate_crc16(const SensorFrame& frame);
    
    /**
     * @brief Generate mock sensor data
     * 
     * @param frame Frame to fill with mock data
     */
    void generate_mock_frame(SensorFrame& frame);
    
    /**
     * @brief Set error message
     * 
     * @param error Error message
     */
    void set_error(const std::string& error);
    
    /**
     * @brief Wait for next sample interval
     */
    void wait_for_sample_interval();
};

} // namespace sensors
} // namespace environet
