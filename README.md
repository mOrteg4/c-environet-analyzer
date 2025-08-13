# EnviroNet Analyzer

A Raspberry Pi-based system that correlates environmental factors with WiFi network performance, designed to demonstrate embedded Linux networking expertise.

## Project Overview

EnviroNet Analyzer uses ultrasonic and IR sensors to detect environmental changes, while simultaneously monitoring WiFi network performance. This creates a comprehensive picture of how physical factors impact wireless connectivity.

Key features:
- Environmental sensing (IR motion detection, ultrasonic distance measurement)
- WiFi network scanning and signal strength monitoring
- Network performance testing (ping, throughput, packet loss)
- Correlation of environmental events with network performance
- Automated data logging and analysis
- Modular architecture with support for both real and mock hardware

## Hardware Requirements

- Raspberry Pi 4 Model B (8GB) running Ubuntu 22.04 LTS Server
- PandaWireless AC1200 WiFi adapter
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

For continuous operation:
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
3. The main script integrates all components
4. Logging provides comprehensive data for analysis

## Demonstrated Skills

This project showcases skills relevant to embedded networking roles:

- **Embedded Linux**: Package management, service configuration, scripting
- **Networking**: WiFi configuration, performance testing, driver integration
- **Hardware Integration**: Sensor interfacing, GPIO programming
- **Software Engineering**: Modular design, testing, logging
- **Automation**: Scripted setup, continuous monitoring

## License

This project is open source, licensed under the MIT License.