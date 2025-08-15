# EnviroNet Analyzer

A Raspberry Pi-based system that correlates environmental factors with WiFi network performance, designed to demonstrate embedded Linux networking protocols and analysis with real-world, data-driven insight.

## Project Overview

EnviroNet Analyzer combines ultrasonic and IR sensors to detect environmental changes, while continuously monitoring WiFi signal strength and network performance. This enables detailed analysis of how physical factors—like movement or object placement—affect wireless connectivity and reliability.

**Key features:**
- Environmental sensing (IR motion detection, ultrasonic distance measurement)
- WiFi network scanning and signal strength monitoring (RSSI, SSID, BSSID, channel)
- Network performance testing (ping, throughput, packet loss)
- Correlation of environmental events with real-time network metrics
- Rboust automated data logging with timestamped sensor and network events
- Modular architecture with interchangeable real and mock hardware for development and testing
- Configurable analysis scripts for visualization and post-processing

## Hardware Requirements

- Raspberry Pi 4 Model B (8GB) running Ubuntu 22.04 LTS Server
- PandaWireless AC1200 WiFi adapter (stable dual-band support)
- SparkFun TinkerKit with:
  - IR sensor
  - Ultrasonic sensor
  - Jumper wires and breadboard

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/environet-analyzer.git
   cd environet-analyzer
   ```

2. Install dependencies:
   ```bash
   ./scripts/install_dependencies.sh
   ```

3. Configure the system:
   Edit `config.json` to match your hardware setup and preferences.

## Usage

### Mock Testing Mode

For development without physical sensors:
```bash
# Set mock mode to true in config.json
python main.py --test-sensors
```

### Network Testing

Test PandaWireless adapter functionality:
```bash
python main.py --test-network
```

### Continuous Monitoring

Start the environmental/network correlation monitor:
```bash
python main.py --monitor
```

### Run Full Analysis

Perform a one-time comprehensive analysis:
```bash
python main.py --analyze
```

### Set Up as a Service

For unattended, long-term operation:
```bash
./scripts/setup_service.sh
sudo systemctl start environet-analyzer
```

## Project Structure

```
environet-analyzer/
├── main.py              # Main integration script
├── config.json          # Configuration file
├── README.md            # Project documentation
├── sensors/             # Sensor modules
│   ├── interface.py     # Common sensor interface
│   ├── ir_mock.py       # IR sensor mock implementation
│   └── ultrasonic_mock.py # Ultrasonic sensor mock
├── network/             # Network modules
│   ├── scanner.py       # WiFi network scanning
│   └── performance.py   # Network performance testing
├── scripts/             # Utility scripts
│   ├── install_dependencies.sh
│   ├── run_tests.sh
│   ├── setup_panda_wifi.sh
│   └── setup_service.sh
├── tests/               # Unit tests
│   ├── test_sensors.py
│   └── test_network.py
└── logs/                # Output logs (created at runtime)
```

## Development Workflow

This project follows a modular approach:

1. Each hardware component has both real and mock implementations
2. Core functionality is tested independently
3. The main script integrates all modules for end-to-end operation
4. Automated, timestamped logging enables correlation of sensor events and network performance for later analysis
## Homelab Learning Outcomes

Building and operating this homelab will help you develop and reinforce skills in:

- **Embedded Linux:** Package management, service configuration, and automation scripting
- **Networking:** WiFi configuration, signal monitoring, performance diagnostics, and driver integration
- **Hardware Integration:** Sensor interfacing (GPIO/I2C), circuit prototyping, and robust error handling
- **Software Engineering:** Modular design, test-driven development, and comprehensive logging strategies
- **Automation:** Scripted setup, unattended operation, and systemd service integration
- **Data Analysis:** Structured event logging, correlation analysis, and result visualization

## License

This project is open source, licensed under the MIT License.
