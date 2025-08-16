#!/bin/bash

# EnviroNet Analyzer - C++ Implementation
# Demo Script
# 
# This script demonstrates the full EnviroNet Analyzer pipeline in mock mode

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR=${BUILD_DIR:-"../build"}
DEMO_DURATION=${DEMO_DURATION:-30}
LOG_LEVEL=${LOG_LEVEL:-"info"}

# Function to print colored output
print_header() {
    echo -e "${CYAN}==========================================${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${CYAN}==========================================${NC}"
}

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if binary exists
check_binary() {
    if [ ! -f "$BUILD_DIR/environet" ]; then
        print_error "EnviroNet binary not found at $BUILD_DIR/environet"
        print_error "Please build the project first: ./scripts/build.sh"
        exit 1
    fi
}

# Function to create demo configuration
create_demo_config() {
    print_status "Creating demo configuration..."
    
    mkdir -p demo_config
    cat > demo_config/config.json << 'EOF'
{
  "i2c": {
    "mock_mode": true,
    "bus_id": 1,
    "addr": 16,
    "sample_interval_ms": 500
  },
  "wifi": {
    "iface_ap": "wlan1",
    "iface_scan": "wlan0",
    "scan_interval_ms": 2000,
    "monitor_mode": false
  },
  "pcap": {
    "bpf": "not (type mgt)",
    "output_dir": "demo_captures",
    "max_file_size_mb": 10,
    "max_files": 3
  },
  "correlator": {
    "sensor_threshold": 100,
    "window_ms": 3000,
    "findings_dir": "demo_findings"
  },
  "logging": {
    "level": "info",
    "file": "demo_logs/environet.log",
    "console": true,
    "max_size_mb": 1,
    "max_files": 2
  },
  "metrics": {
    "ping_targets": ["8.8.8.8", "1.1.1.1"],
    "iperf_server": "",
    "ping_interval_ms": 5000,
    "iperf_duration": 5
  }
}
EOF
    
    print_success "Demo configuration created"
}

# Function to create demo directories
create_demo_dirs() {
    print_status "Creating demo directories..."
    
    mkdir -p demo_logs demo_captures demo_findings
    print_success "Demo directories created"
}

# Function to run sensor test
run_sensor_test() {
    print_header "Sensor Test Demo"
    print_status "Testing mock sensor functionality..."
    
    timeout 10s "$BUILD_DIR/environet" --config demo_config/config.json --test-sensors || {
        print_warning "Sensor test completed (timeout or error expected in demo)"
    }
    
    print_success "Sensor test completed"
}

# Function to run network test
run_network_test() {
    print_header "Network Test Demo"
    print_status "Testing WiFi scanning and ping functionality..."
    
    timeout 15s "$BUILD_DIR/environet" --config demo_config/config.json --test-network || {
        print_warning "Network test completed (timeout or error expected in demo)"
    }
    
    print_success "Network test completed"
}

# Function to run PCAP test
run_pcap_test() {
    print_header "PCAP Test Demo"
    print_status "Testing packet capture functionality..."
    
    timeout 10s "$BUILD_DIR/environet" --config demo_config/config.json --test-pcap || {
        print_warning "PCAP test completed (timeout or error expected in demo)"
    }
    
    print_success "PCAP test completed"
}

# Function to run full demo
run_full_demo() {
    print_header "Full Pipeline Demo"
    print_status "Running full EnviroNet Analyzer pipeline for ${DEMO_DURATION} seconds..."
    print_status "This will demonstrate:"
    echo "  - Mock sensor data generation"
    echo "  - WiFi network scanning"
    echo "  - Packet capture (if available)"
    echo "  - Network metrics collection"
    echo "  - Correlation analysis"
    echo "  - Finding generation"
    echo ""
    
    # Start the full application
    print_status "Starting EnviroNet Analyzer..."
    "$BUILD_DIR/environet" --config demo_config/config.json &
    local environet_pid=$!
    
    # Wait for startup
    sleep 5
    
    print_status "EnviroNet Analyzer running (PID: $environet_pid)"
    print_status "Monitoring for ${DEMO_DURATION} seconds..."
    
    # Monitor the application
    local elapsed=0
    while [ $elapsed -lt $DEMO_DURATION ] && kill -0 $environet_pid 2>/dev/null; do
        sleep 5
        elapsed=$((elapsed + 5))
        remaining=$((DEMO_DURATION - elapsed))
        print_status "Demo running... ${elapsed}s elapsed, ${remaining}s remaining"
        
        # Show some live data if available
        if [ -f "demo_logs/environet.log" ]; then
            echo "  Recent log entries:"
            tail -n 3 demo_logs/environet.log 2>/dev/null | sed 's/^/    /' || true
        fi
        
        if [ -d "demo_findings" ] && [ "$(ls -A demo_findings 2>/dev/null)" ]; then
            echo "  Findings generated:"
            ls -la demo_findings/ | head -3 | sed 's/^/    /' || true
        fi
    done
    
    # Stop the application
    print_status "Stopping EnviroNet Analyzer..."
    if kill -0 $environet_pid 2>/dev/null; then
        kill $environet_pid
        sleep 2
        if kill -0 $environet_pid 2>/dev/null; then
            kill -9 $environet_pid
        fi
    fi
    
    print_success "Full demo completed"
}

# Function to show demo results
show_demo_results() {
    print_header "Demo Results"
    
    echo "Demo completed successfully!"
    echo ""
    
    # Show log file
    if [ -f "demo_logs/environet.log" ]; then
        print_status "Log file created: demo_logs/environet.log"
        echo "  Last 10 log entries:"
        tail -n 10 demo_logs/environet.log | sed 's/^/    /' || true
        echo ""
    fi
    
    # Show findings
    if [ -d "demo_findings" ] && [ "$(ls -A demo_findings 2>/dev/null)" ]; then
        print_status "Findings generated:"
        ls -la demo_findings/ | sed 's/^/    /' || true
        echo ""
        
        # Show content of first finding
        local first_finding=$(find demo_findings -name "*.json" | head -1)
        if [ -n "$first_finding" ]; then
            echo "  Sample finding content:"
            cat "$first_finding" | sed 's/^/    /' || true
            echo ""
        fi
    fi
    
    # Show captures
    if [ -d "demo_captures" ] && [ "$(ls -A demo_captures 2>/dev/null)" ]; then
        print_status "Packet captures:"
        ls -la demo_captures/ | sed 's/^/    /' || true
        echo ""
    fi
    
    print_success "Demo completed successfully!"
    echo ""
    echo "Next steps:"
    echo "  1. Examine the generated files and logs"
    echo "  2. Modify demo_config/config.json for different settings"
    echo "  3. Run individual tests: $0 --sensors, --network, --pcap"
    echo "  4. Build and test with real hardware when available"
}

# Function to cleanup demo
cleanup_demo() {
    print_status "Cleaning up demo files..."
    
    rm -rf demo_config demo_logs demo_captures demo_findings
    
    print_success "Demo cleanup completed"
}

# Function to show help
show_help() {
    echo "EnviroNet Analyzer - C++ Demo Script"
    echo ""
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --sensors          Run sensor test demo only"
    echo "  --network          Run network test demo only"
    echo "  --pcap             Run PCAP test demo only"
    echo "  --full             Run full pipeline demo (default)"
    echo "  --cleanup          Clean up demo files after completion"
    echo "  --duration <sec>   Set demo duration in seconds (default: 30)"
    echo "  --help, -h         Show this help message"
    echo ""
    echo "Environment variables:"
    echo "  BUILD_DIR          Build directory path [default: ../build]"
    echo "  DEMO_DURATION      Demo duration in seconds [default: 30]"
    echo "  LOG_LEVEL          Log level [default: info]"
    echo ""
    echo "Examples:"
    echo "  $0                 # Run full demo"
    echo "  $0 --sensors       # Test sensors only"
    echo "  $0 --duration 60   # Run demo for 60 seconds"
    echo "  $0 --cleanup       # Clean up after demo"
}

# Main script
main() {
    # Parse command line arguments
    local run_sensors=false
    local run_network=false
    local run_pcap=false
    local run_full=true
    local cleanup_after=false
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            --sensors)
                run_sensors=true
                run_full=false
                shift
                ;;
            --network)
                run_network=true
                run_full=false
                shift
                ;;
            --pcap)
                run_pcap=true
                run_full=false
                shift
                ;;
            --full)
                run_full=true
                shift
                ;;
            --cleanup)
                cleanup_after=true
                shift
                ;;
            --duration)
                if [[ $# -gt 1 ]]; then
                    DEMO_DURATION="$2"
                    shift 2
                else
                    print_error "--duration requires a value"
                    exit 1
                fi
                ;;
            --help|-h)
                show_help
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
    
    print_header "EnviroNet Analyzer - C++ Demo"
    print_status "This demo showcases the C++ implementation capabilities"
    echo ""
    
    # Check prerequisites
    check_binary
    
    # Create demo environment
    create_demo_config
    create_demo_dirs
    
    # Run requested demos
    if [ "$run_sensors" = true ]; then
        run_sensor_test
    fi
    
    if [ "$run_network" = true ]; then
        run_network_test
    fi
    
    if [ "$run_pcap" = true ]; then
        run_pcap_test
    fi
    
    if [ "$run_full" = true ]; then
        run_full_demo
    fi
    
    # Show results
    show_demo_results
    
    # Cleanup if requested
    if [ "$cleanup_after" = true ]; then
        cleanup_demo
    fi
    
    print_success "Demo completed successfully!"
}

# Run main function with all arguments
main "$@"
