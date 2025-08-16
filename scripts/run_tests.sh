#!/bin/bash

# EnviroNet Analyzer - Comprehensive Test Runner
# This script runs all tests with different configurations and provides detailed output

set -e

# Color definitions
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR=${BUILD_DIR:-"build"}
TEST_TIMEOUT=${TEST_TIMEOUT:-300}
VERBOSE=${VERBOSE:-false}
RUN_MEMORY_TESTS=${RUN_MEMORY_TESTS:-true}
RUN_PERFORMANCE_TESTS=${RUN_PERFORMANCE_TESTS:-true}
RUN_STRESS_TESTS=${RUN_STRESS_TESTS:-false}

# Test results
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SKIPPED_TESTS=0

# Helper functions
print_header() {
    echo -e "\n${BLUE}========================================${NC}"
    echo -e "${BLUE} $1${NC}"
    echo -e "${BLUE}========================================${NC}\n"
}

print_section() {
    echo -e "\n${CYAN}--- $1 ---${NC}"
}

print_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

# Check if build directory exists
check_build_dir() {
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory '$BUILD_DIR' not found!"
        print_info "Please run './scripts/build.sh' first"
        exit 1
    fi
    
    if [ ! -f "$BUILD_DIR/environet_tests" ]; then
        print_error "Test executable not found in '$BUILD_DIR'!"
        print_info "Please run './scripts/build.sh --test' first"
        exit 1
    fi
}

# Run basic tests
run_basic_tests() {
    print_section "Running Basic Tests"
    
    cd "$BUILD_DIR"
    
    # Run all tests
    print_info "Running all tests..."
    if ./environet_tests --gtest_output=xml:test_results.xml; then
        print_success "All basic tests passed!"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_error "Some basic tests failed!"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    cd ..
}

# Run individual test suites
run_test_suites() {
    print_section "Running Individual Test Suites"
    
    cd "$BUILD_DIR"
    
    local suites=("Sensor" "Config" "Time" "Integration")
    
    for suite in "${suites[@]}"; do
        print_info "Running ${suite} tests..."
        if timeout $TEST_TIMEOUT ./environet_tests --gtest_filter="*${suite}*" --gtest_output=xml:${suite}_tests.xml; then
            print_success "${suite} tests passed!"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            print_error "${suite} tests failed!"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    done
    
    cd ..
}

# Run memory tests
run_memory_tests() {
    if [ "$RUN_MEMORY_TESTS" = "false" ]; then
        print_warning "Memory tests skipped (RUN_MEMORY_TESTS=false)"
        return
    fi
    
    print_section "Running Memory Tests"
    
    if ! command -v valgrind &> /dev/null; then
        print_warning "Valgrind not found, skipping memory tests"
        SKIPPED_TESTS=$((SKIPPED_TESTS + 1))
        return
    fi
    
    cd "$BUILD_DIR"
    
    print_info "Running tests with Valgrind (this may take a while)..."
    if timeout $((TEST_TIMEOUT * 2)) valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind.log ./environet_tests; then
        print_success "Memory tests completed!"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_error "Memory tests failed!"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    # Check for memory leaks
    if grep -q "definitely lost: 0 bytes" valgrind.log; then
        print_success "No memory leaks detected!"
    else
        print_warning "Potential memory leaks detected. Check valgrind.log for details."
    fi
    
    cd ..
}

# Run performance tests
run_performance_tests() {
    if [ "$RUN_PERFORMANCE_TESTS" = "false" ]; then
        print_warning "Performance tests skipped (RUN_PERFORMANCE_TESTS=false)"
        return
    fi
    
    print_section "Running Performance Tests"
    
    cd "$BUILD_DIR"
    
    print_info "Running performance tests..."
    if timeout $TEST_TIMEOUT ./environet_tests --gtest_filter="*Performance*" --gtest_repeat=3; then
        print_success "Performance tests passed!"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_error "Performance tests failed!"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    cd ..
}

# Run stress tests
run_stress_tests() {
    if [ "$RUN_STRESS_TESTS" = "false" ]; then
        print_warning "Stress tests skipped (RUN_STRESS_TESTS=false)"
        return
    fi
    
    print_section "Running Stress Tests"
    
    cd "$BUILD_DIR"
    
    print_info "Running stress tests (this may take a while)..."
    if timeout $((TEST_TIMEOUT * 3)) ./environet_tests --gtest_filter="*Stress*" --gtest_repeat=10; then
        print_success "Stress tests passed!"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_error "Stress tests failed!"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    cd ..
}

# Run mock mode tests
run_mock_tests() {
    print_section "Running Mock Mode Tests"
    
    cd "$BUILD_DIR"
    
    print_info "Testing mock sensor functionality..."
    if timeout $TEST_TIMEOUT ./environet --test-sensors; then
        print_success "Mock sensor tests passed!"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_error "Mock sensor tests failed!"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    print_info "Testing mock network functionality..."
    if timeout $TEST_TIMEOUT ./environet --test-network; then
        print_success "Mock network tests passed!"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_error "Mock network tests failed!"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    cd ..
}

# Generate test report
generate_report() {
    print_section "Generating Test Report"
    
    TOTAL_TESTS=$((PASSED_TESTS + FAILED_TESTS + SKIPPED_TESTS))
    
    echo -e "\n${BLUE}Test Summary:${NC}"
    echo -e "  Total Test Suites: $TOTAL_TESTS"
    echo -e "  Passed: ${GREEN}$PASSED_TESTS${NC}"
    echo -e "  Failed: ${RED}$FAILED_TESTS${NC}"
    echo -e "  Skipped: ${YELLOW}$SKIPPED_TESTS${NC}"
    
    if [ $FAILED_TESTS -eq 0 ]; then
        echo -e "\n${GREEN}🎉 All tests passed!${NC}"
        return 0
    else
        echo -e "\n${RED}❌ Some tests failed!${NC}"
        return 1
    fi
}

# Cleanup function
cleanup() {
    print_info "Cleaning up test artifacts..."
    
    # Remove test output files
    if [ -d "$BUILD_DIR" ]; then
        cd "$BUILD_DIR"
        rm -f test_results.xml *_tests.xml valgrind.log
        cd ..
    fi
}

# Main function
main() {
    print_header "EnviroNet Analyzer - Comprehensive Test Suite"
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --build-dir)
                BUILD_DIR="$2"
                shift 2
                ;;
            --timeout)
                TEST_TIMEOUT="$2"
                shift 2
                ;;
            --verbose)
                VERBOSE=true
                shift
                ;;
            --no-memory)
                RUN_MEMORY_TESTS=false
                shift
                ;;
            --no-performance)
                RUN_PERFORMANCE_TESTS=false
                shift
                ;;
            --stress)
                RUN_STRESS_TESTS=true
                shift
                ;;
            --help|-h)
                echo "Usage: $0 [options]"
                echo "Options:"
                echo "  --build-dir DIR    Build directory (default: build)"
                echo "  --timeout SEC      Test timeout in seconds (default: 300)"
                echo "  --verbose          Enable verbose output"
                echo "  --no-memory       Skip memory tests"
                echo "  --no-performance  Skip performance tests"
                echo "  --stress          Enable stress tests"
                echo "  --help, -h        Show this help message"
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                exit 1
                ;;
        esac
    done
    
    # Set up cleanup trap
    trap cleanup EXIT
    
    # Check prerequisites
    check_build_dir
    
    # Print configuration
    print_info "Build directory: $BUILD_DIR"
    print_info "Test timeout: ${TEST_TIMEOUT}s"
    print_info "Memory tests: $RUN_MEMORY_TESTS"
    print_info "Performance tests: $RUN_PERFORMANCE_TESTS"
    print_info "Stress tests: $RUN_STRESS_TESTS"
    
    # Run tests
    run_basic_tests
    run_test_suites
    run_mock_tests
    run_memory_tests
    run_performance_tests
    run_stress_tests
    
    # Generate report
    generate_report
}

# Run main function
main "$@"