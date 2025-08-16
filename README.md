# EnviroNet Analyzer - C++ Implementation

A production-quality C++ implementation of the EnviroNet Analyzer, designed to correlate environmental factors with WiFi network performance. This project demonstrates low-level networking skills using libpcap, libnl/nl80211, and embedded device communications via I²C.

## 🎯 Project Overview

EnviroNet Analyzer combines ultrasonic and IR sensors to detect environmental changes, while continuously monitoring WiFi signal strength and network performance. This enables detailed analysis of how physical factors—like movement or object placement—affect wireless connectivity and reliability.

**Key Features:**
- **Environmental Sensing**: IR motion detection and ultrasonic distance measurement via Arduino I²C
- **WiFi Network Analysis**: Real-time scanning using nl80211 with iw command fallback
- **Packet Capture**: High-performance libpcap integration with BPF filtering
- **Network Metrics**: Ping and iperf3 testing with automated correlation
- **Event Correlation**: Time-windowed analysis linking sensor events to network changes
- **Production Ready**: Systemd service, comprehensive logging, and monitoring integration

## 🏗️ Architecture

The system is built with a modular, multi-threaded architecture:

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Sensor Layer  │    │  Network Layer  │    │ Analysis Layer  │
│                 │    │                 │    │                 │
│ • Arduino I2C   │    │ • WiFi Scanner  │    │ • Correlator    │
│ • Mock Generator│    │ • PCAP Capture  │    │ • Time Windows  │
│ • CRC Validation│    │ • Network Tests │    │ • Event Finding │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         └───────────────────────┼───────────────────────┘
                                 │
                    ┌─────────────────┐
                    │  Output Layer   │
                    │                 │
                    │ • JSONL Logs    │
                    │ • PCAP Files    │
                    │ • Systemd Logs  │
                    │ • Prometheus    │
                    └─────────────────┘
```

## 🚀 Quick Start

### Prerequisites

- **OS**: Raspberry Pi OS, Ubuntu 20.04+, or Debian 11+
- **Hardware**: Raspberry Pi 4 (8GB recommended), Panda AC1200 WiFi adapter
- **Dependencies**: See [Installation](#installation) section

### Quick Demo (Mock Mode)

```bash
# Clone and setup
git clone <your-repo>
cd environet-analyzer
chmod +x scripts/*.sh

# Install dependencies
./scripts/install_deps.sh

# Build the project
./scripts/build.sh --test

# Run demo (no hardware required)
cd examples
./run_demo.sh --duration 30
```

### Quick Test Commands

```bash
# Test sensors (mock mode)
./build/environet --test-sensors

# Test network scanning
./build/environet --test-network

# Test packet capture
./build/environet --test-pcap

# Full monitoring (mock mode)
./build/environet --config config/config.json
```

## 📦 Installation

### Automated Installation

```bash
# Install all dependencies
./scripts/install_deps.sh

# Build and install
./scripts/build.sh --install

# Setup systemd service
sudo systemctl enable environet
sudo systemctl start environet
```

### Manual Installation

```bash
# Install system dependencies
sudo apt update
sudo apt install -y build-essential cmake g++ clang-format clang-tidy git \
  libpcap-dev libnl-3-dev libnl-genl-3-dev libgpiod-dev i2c-tools libi2c-dev \
  iperf3 pkg-config libspdlog-dev libgtest-dev valgrind doxygen graphviz

# Build from source
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)
sudo make install
```

## 🔧 Configuration

### Configuration File

Create `/etc/environet/config.json` or use local `config/config.json`:

```json
{
  "i2c": {
    "mock_mode": true,
    "bus_id": 1,
    "addr": 16,
    "sample_interval_ms": 100
  },
  "wifi": {
    "iface_ap": "wlan1",
    "iface_scan": "wlan0",
    "scan_interval_ms": 5000,
    "monitor_mode": false
  },
  "pcap": {
    "bpf": "not (type mgt)",
    "output_dir": "captures",
    "max_file_size_mb": 100,
    "max_files": 10
  },
  "correlator": {
    "sensor_threshold": 200,
    "window_ms": 5000,
    "findings_dir": "findings"
  },
  "logging": {
    "level": "info",
    "file": "/var/log/environet/environet.log",
    "console": true,
    "max_size_mb": 5,
    "max_files": 3
  }
}
```

### Hardware Configuration

When ready for real hardware:

1. **Set `mock_mode: false`** in configuration
2. **Connect Arduino** via I²C with logic level shifter
3. **Verify I2C bus**: `sudo i2cdetect -y 1`
4. **Test communication**: `./environet --test-sensors`

See [Hardware Setup](#hardware-setup) for detailed wiring instructions.

## 🧪 Testing

### Unit Tests

```bash
# Run all tests
./scripts/build.sh --test

# Run specific test suite
cd build
./environet_tests --gtest_filter=ArduinoI2CTest*
```

### Integration Tests

```bash
# Test full pipeline
cd examples
./run_demo.sh --full --duration 60

# Test individual components
./run_demo.sh --sensors
./run_demo.sh --network
./run_demo.sh --pcap
```

### Performance Testing

```bash
# High-throughput packet capture test
./build/environet --config test_configs/high_throughput.json

# Memory usage monitoring
valgrind --tool=massif ./build/environet --test-sensors
```

## 🔌 Hardware Setup

### Required Components

| Component | Purpose | Notes |
|-----------|---------|-------|
| Raspberry Pi 4 | Main processing unit | 8GB recommended |
| Panda AC1200 | WiFi access point | USB 3.0 dual-band |
| Arduino Uno/Nano | Sensor interface | I²C slave device |
| IR Sensor | Motion detection | Analog output |
| Ultrasonic Sensor | Distance measurement | HC-SR04 compatible |
| Logic Level Shifter | **REQUIRED** | 3.3V ↔ 5V conversion |

### Wiring Diagram

```
Raspberry Pi 4    Logic Level Shifter    Arduino
    3.3V ────────── LV (3.3V)
   GPIO2 ────────── LV1 (SDA) ────────── A4
   GPIO3 ────────── LV2 (SCL) ────────── A5
     GND ────────── GND ──────────────── GND
```

**⚠️ CRITICAL**: Never connect 5V Arduino directly to 3.3V Pi!

### Arduino Firmware

Upload `examples/arduino/environet_sensor.ino` to your Arduino:

```bash
# Install Arduino IDE or use arduino-cli
arduino-cli compile --fqbn arduino:avr:uno examples/arduino/environet_sensor.ino
arduino-cli upload --fqbn arduino:avr:uno --port /dev/ttyUSB0
```

## 📊 Usage Examples

### Basic Monitoring

```bash
# Start monitoring with default config
./environet --config config/config.json

# Monitor specific interface
./environet --config config/config.json --interface wlan0

# Verbose logging
./environet --config config/config.json --log-level debug
```

### Data Collection

```bash
# Collect sensor data for 1 hour
timeout 3600 ./environet --config config/config.json

# Analyze collected data
ls -la findings/
cat findings/*.json | jq '.event_type' | sort | uniq -c
```

### Network Analysis

```bash
# WiFi scan only
./environet --test-network

# Packet capture analysis
tcpdump -r captures/*.pcap | head -20

# Performance metrics
./environet --config config/config.json --metrics-only
```

## 🏭 Production Deployment

### Systemd Service

```bash
# Install service
sudo cp systemd/environet.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable environet
sudo systemctl start environet

# Check status
sudo systemctl status environet
sudo journalctl -u environet -f
```

### Log Management

```bash
# View logs
sudo journalctl -u environet -f
tail -f /var/log/environet/environet.log

# Log rotation (handled automatically)
ls -la /var/log/environet/
```

### Monitoring Integration

```bash
# Check system resources
htop
iotop
iftop

# Monitor findings
watch -n 5 'ls -la findings/ | wc -l'
```

## 🔍 Troubleshooting

### Common Issues

| Problem | Solution |
|---------|----------|
| I2C devices not found | Enable I2C in `raspi-config` |
| Permission denied | Run with sudo or add user to i2c group |
| WiFi scan fails | Check interface permissions and driver support |
| PCAP capture errors | Verify interface exists and has traffic |

### Debug Commands

```bash
# Check I2C bus
sudo i2cdetect -y 1

# Test WiFi interface
iw dev wlan0 scan | head -20

# Verify packet capture
sudo tcpdump -i wlan0 -c 10

# Check system logs
sudo journalctl -u environet --since "1 hour ago"
```

### Performance Tuning

```bash
# Increase file descriptor limits
echo "* soft nofile 65536" | sudo tee -a /etc/security/limits.conf
echo "* hard nofile 65536" | sudo tee -a /etc/security/limits.conf

# Optimize network buffers
echo 'net.core.rmem_max = 134217728' | sudo tee -a /etc/sysctl.conf
echo 'net.core.wmem_max = 134217728' | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

## 🚀 Development

### Building from Source

```bash
# Development build with sanitizers
BUILD_TYPE=Debug ./scripts/build.sh --clean --test

# Release build
BUILD_TYPE=Release ./scripts/build.sh --clean

# Install to custom location
INSTALL_DIR=/opt/environet ./scripts/build.sh --install
```

### Code Quality

```bash
# Format code
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Static analysis
clang-tidy src/*.cpp -- -I include/

# Memory checking
valgrind --tool=memcheck --leak-check=full ./build/environet --test-sensors
```

### Adding New Features

1. **Create header** in `include/` directory
2. **Implement source** in `src/` directory  
3. **Add tests** in `tests/` directory
4. **Update CMakeLists.txt** with new files
5. **Document** with Doxygen comments

## 📚 API Reference

### Core Classes

- **`environet::core::Config`**: Configuration management
- **`environet::core::Logger`**: Structured logging system
- **`environet::sensors::ArduinoI2C`**: I²C sensor communication
- **`environet::net::WifiScan`**: WiFi network scanning
- **`environet::net::PcapSniffer`**: Packet capture and analysis
- **`environet::correlate::Correlator`**: Event correlation engine

### Data Structures

- **`SensorFrame`**: 16-byte packed sensor data with CRC
- **`BssInfo`**: WiFi access point information
- **`PacketMeta`**: Packet metadata and statistics
- **`Finding`**: Correlated event results

See `docs/` directory for detailed API documentation.

## 🤝 Contributing

1. **Fork** the repository
2. **Create** feature branch: `git checkout -b feat/amazing-feature`
3. **Commit** changes: `git commit -m 'Add amazing feature'`
4. **Push** to branch: `git push origin feat/amazing-feature`
5. **Open** Pull Request

### Development Guidelines

- Follow existing code style (clang-format enforced)
- Add unit tests for new functionality
- Update documentation for API changes
- Ensure all tests pass before submitting

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **libpcap** for packet capture capabilities
- **libnl** for netlink communication
- **spdlog** for structured logging
- **GoogleTest** for testing framework
- **Arduino community** for sensor integration examples

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/environet-analyzer/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/environet-analyzer/discussions)
- **Documentation**: [Wiki](https://github.com/yourusername/environet-analyzer/wiki)

---

**Built with ❤️ for network analysis and environmental correlation**
