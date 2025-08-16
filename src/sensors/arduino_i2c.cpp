#include "sensors/arduino_i2c.hpp"
#include "core/log.hpp"
#include "core/config.hpp"

#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace environet {
namespace sensors {

// CRC-16-CCITT parameters
static constexpr uint16_t CRC16_POLY = 0x1021;
static constexpr uint16_t CRC16_INIT = 0xFFFF;

ArduinoI2C::ArduinoI2C(const environet::core::Config& cfg)
    : mock_mode_(cfg.i2c.mock_mode),
      bus_id_(cfg.i2c.bus_id),
      addr_(cfg.i2c.addr),
      sample_interval_ms_(cfg.i2c.sample_interval_ms),
      fd_(-1),
      mock_timestamp_(0) {}

ArduinoI2C::ArduinoI2C(const std::string& config_path)
    : mock_mode_(true), bus_id_(1), addr_(16), sample_interval_ms_(100), fd_(-1), mock_timestamp_(0) {
    try {
        auto cfg = environet::core::Config::load(config_path);
        mock_mode_ = cfg.i2c.mock_mode;
        bus_id_ = cfg.i2c.bus_id;
        addr_ = cfg.i2c.addr;
        sample_interval_ms_ = cfg.i2c.sample_interval_ms;
    } catch (const std::exception& e) {
        set_error(std::string("Failed to load config: ") + e.what());
    }
}

ArduinoI2C::~ArduinoI2C() {
    stop();
}

bool ArduinoI2C::init() {
    if (mock_mode_) {
        return init_mock_i2c();
    }
    return init_real_i2c();
}

bool ArduinoI2C::init_real_i2c() {
    // Minimal stub for now; real implementation requires linux/i2c-dev/ioctl
    set_error("Real I2C mode not implemented yet in this build");
    return false;
}

bool ArduinoI2C::init_mock_i2c() {
    // Seed RNG deterministically using config values
    auto seed = static_cast<uint32_t>(bus_id_ * 131 + addr_ * 17 + sample_interval_ms_);
    rng_ = std::make_unique<std::mt19937>(seed);
    last_sample_ = std::chrono::steady_clock::now();
    mock_timestamp_ = 0;
    return true;
}

bool ArduinoI2C::read_frame(SensorFrame& frame) {
    if (mock_mode_) {
        return read_frame_mock(frame);
    }
    return read_frame_real(frame);
}

bool ArduinoI2C::read_frame_real(SensorFrame& frame) {
    (void)frame;
    // Not implemented yet
    set_error("read_frame_real not implemented");
    return false;
}

void ArduinoI2C::stop() {
    if (!mock_mode_ && fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

uint16_t ArduinoI2C::compute_crc16(const uint8_t* data, size_t len) {
    uint16_t crc = CRC16_INIT;
    for (size_t i = 0; i < len; ++i) {
        crc ^= (static_cast<uint16_t>(data[i]) << 8);
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ CRC16_POLY;
            } else {
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

bool ArduinoI2C::validate_crc16(const SensorFrame& frame) {
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&frame);
    uint16_t crc = compute_crc16(bytes, sizeof(SensorFrame) - sizeof(uint16_t));
    return crc == frame.crc16;
}

void ArduinoI2C::generate_mock_frame(SensorFrame& frame) {
    using namespace std::chrono;

    // Enforce sampling interval
    wait_for_sample_interval();

    mock_timestamp_ += static_cast<uint32_t>(sample_interval_ms_);
    frame.ts_ms = mock_timestamp_;

    // Deterministic pseudo-random sensors
    static double phase = 0.0;
    std::normal_distribution<double> noise(0.0, 10.0);
    int base = 100;
    int amp = 400;
    phase += 0.15; // advance phase
    int ir = static_cast<int>(base + amp * std::sin(phase) + noise(*rng_));
    if (ir < -512) ir = -512;
    if (ir > 511) ir = 511;
    frame.ir_raw = static_cast<int16_t>(ir);

    std::uniform_int_distribution<int> jitter(-30, 30);
    int ultra = 1200 + jitter(*rng_); // around 1.2m
    if (ultra < 50) ultra = 50;
    if (ultra > 4000) ultra = 4000;
    frame.ultra_mm = static_cast<uint16_t>(ultra);

    // Occasionally set motion bit
    std::bernoulli_distribution motion(0.1);
    frame.status = motion(*rng_) ? SensorFrame::STATUS_MOTION : 0;
    frame.reserved = 0;

    // Compute CRC over all prior bytes
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&frame);
    frame.crc16 = compute_crc16(bytes, sizeof(SensorFrame) - sizeof(uint16_t));
}

bool ArduinoI2C::read_frame_mock(SensorFrame& frame) {
    generate_mock_frame(frame);
    return true;
}

void ArduinoI2C::set_error(const std::string& error) {
    last_error_ = error;
}

void ArduinoI2C::wait_for_sample_interval() {
    using namespace std::chrono;
    auto now = steady_clock::now();
    auto next_time = last_sample_ + milliseconds(sample_interval_ms_);
    if (now < next_time) {
        std::this_thread::sleep_until(next_time);
    }
    last_sample_ = steady_clock::now();
}

} // namespace sensors
} // namespace environet
