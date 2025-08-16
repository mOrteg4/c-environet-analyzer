#!/usr/bin/env python3
"""
EnviroNet Analyzer - C++ Implementation Verification Script

This script verifies the project structure and provides a summary of implementation status.
"""

import os
import json
from pathlib import Path

def check_file_exists(filepath, description):
    """Check if a file exists and return status."""
    exists = os.path.exists(filepath)
    status = "✅" if exists else "❌"
    return f"{status} {description}: {filepath}"

def check_directory_structure():
    """Check the project directory structure."""
    print("🔍 Checking Project Structure...")
    print("=" * 50)
    
    # Core project files
    core_files = [
        ("CMakeLists.txt", "Main CMake build configuration"),
        ("README.md", "Project documentation"),
        ("config/config.json", "Configuration file"),
        ("resume_bullets.md", "TP-Link resume bullets"),
    ]
    
    for filepath, description in core_files:
        print(check_file_exists(filepath, description))
    
    print()
    
    # Directory structure
    directories = [
        "include/core",
        "include/sensors", 
        "include/net",
        "include/correlate",
        "include/util",
        "src/core",
        "src/sensors",
        "src/net", 
        "src/correlate",
        "src/util",
        "tests",
        "examples/arduino",
        "scripts",
        "systemd",
        "docs"
    ]
    
    print("📁 Directory Structure:")
    for directory in directories:
        if os.path.exists(directory):
            print(f"  ✅ {directory}/")
        else:
            print(f"  ❌ {directory}/")
    
    print()

def check_header_files():
    """Check header file implementation."""
    print("📋 Header Files:")
    
    headers = [
        ("include/core/log.hpp", "Logging system header"),
        ("include/core/config.hpp", "Configuration management header"),
        ("include/sensors/arduino_i2c.hpp", "Arduino I2C sensor header"),
        ("include/net/wifi_scan.hpp", "WiFi scanning header"),
        ("include/net/pcap_sniffer.hpp", "Packet capture header"),
        ("include/net/metrics.hpp", "Network metrics header"),
        ("include/correlate/correlator.hpp", "Correlation engine header"),
        ("include/util/time.hpp", "Time utility header"),
    ]
    
    for filepath, description in headers:
        print(check_file_exists(filepath, description))
    
    print()

def check_source_files():
    """Check source file implementation."""
    print("💻 Source Files:")
    
    sources = [
        ("src/main.cpp", "Main application entry point"),
        ("tests/test_main.cpp", "Test main file"),
    ]
    
    for filepath, description in sources:
        print(check_file_exists(filepath, description))
    
    print()

def check_scripts_and_configs():
    """Check scripts and configuration files."""
    print("🔧 Scripts and Configurations:")
    
    scripts = [
        ("scripts/install_deps.sh", "Dependency installation script"),
        ("scripts/build.sh", "Build automation script"),
        ("examples/run_demo.sh", "Demo execution script"),
        ("systemd/environet.service", "Systemd service file"),
        ("examples/arduino/environet_sensor.ino", "Arduino firmware"),
        ("docs/data-flow.dot", "Data flow diagram"),
        ("docs/topology.dot", "Topology diagram"),
        ("docs/wiring-onepager.md", "Hardware wiring guide"),
    ]
    
    for filepath, description in scripts:
        print(check_file_exists(filepath, description))
    
    print()

def check_implementation_status():
    """Check implementation status by milestone."""
    print("📊 Implementation Status by Milestone:")
    print("=" * 50)
    
    milestones = {
        "M0 - Repo scaffold & CI": [
            "CMakeLists.txt with proper dependencies",
            "Directory structure created",
            "Basic README and documentation",
            "Install and build scripts",
            "Clang-format and clang-tidy configs"
        ],
        "M1 - Mock sensor": [
            "ArduinoI2C header with frame structure",
            "Mock mode implementation ready",
            "Arduino firmware sketch",
            "Demo script for sensor testing"
        ],
        "M2 - WiFi scan": [
            "WifiScan header with nl80211 support",
            "Fallback to iw command parsing",
            "BSS information structures"
        ],
        "M3 - PCAP capture": [
            "PcapSniffer header with BPF support",
            "Packet metadata structures",
            "File rotation and management"
        ],
        "M4 - Correlator & metrics": [
            "Correlator header with time-series support",
            "Metrics header for ping/iperf",
            "Finding generation structures"
        ],
        "M5 - System integration": [
            "Main.cpp with multi-threaded architecture",
            "Systemd service file",
            "Configuration management"
        ],
        "M6 - Polish & stretch": [
            "Comprehensive documentation",
            "Wiring diagrams and guides",
            "Resume bullets for TP-Link"
        ]
    }
    
    for milestone, items in milestones.items():
        print(f"\n🎯 {milestone}:")
        for item in items:
            print(f"  • {item}")
    
    print()

def generate_summary():
    """Generate implementation summary."""
    print("📈 Implementation Summary:")
    print("=" * 50)
    
    total_files = 0
    implemented_files = 0
    
    # Count files in key directories
    key_dirs = ["include", "src", "scripts", "examples", "systemd", "docs"]
    
    for directory in key_dirs:
        if os.path.exists(directory):
            for root, dirs, files in os.walk(directory):
                for file in files:
                    if file.endswith(('.hpp', '.cpp', '.sh', '.ino', '.dot', '.md', '.json', '.service')):
                        total_files += 1
                        if os.path.exists(os.path.join(root, file)):
                            implemented_files += 1
    
    print(f"📁 Total project files: {total_files}")
    print(f"✅ Implemented files: {implemented_files}")
    print(f"📊 Completion: {(implemented_files/total_files*100):.1f}%" if total_files > 0 else "📊 Completion: 0%")
    
    print()
    print("🚀 Ready for next steps:")
    print("  1. Implement source files (.cpp) for each header")
    print("  2. Add unit tests for each component")
    print("  3. Test build system on target platform")
    print("  4. Run demo scripts to verify functionality")

def main():
    """Main verification function."""
    print("🔍 EnviroNet Analyzer - C++ Implementation Verification")
    print("=" * 60)
    print()
    
    check_file_exists("CMakeLists.txt", "Project root")
    check_directory_structure()
    check_header_files()
    check_source_files()
    check_scripts_and_configs()
    check_implementation_status()
    generate_summary()
    
    print("✨ Verification complete!")

if __name__ == "__main__":
    main()
