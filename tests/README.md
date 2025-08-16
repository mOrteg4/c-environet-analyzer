# EnviroNet Analyzer - Testing Framework

This document describes the comprehensive testing framework for the EnviroNet Analyzer C++ project.

## Overview

The testing framework provides:
- **Unit Tests**: Individual component testing with GoogleTest
- **Integration Tests**: Full system testing
- **Mock Mode Testing**: Hardware-free development and testing
- **Performance Testing**: Load and stress testing
- **Memory Testing**: Valgrind-based memory leak detection
- **Automated Test Running**: Scripts for different platforms

## Test Structure

### Test Files

- `test_main.cpp` - Main test framework and basic tests
- `test_sensors.cpp` - Sensor component tests (Arduino I2C, mock mode)
- `test_config.cpp` - Configuration system tests
- `test_time.cpp` - Time utility function tests
- `test_configs.json` - Test configuration scenarios

### Test Categories

1. **Basic Tests** - Verify fundamental functionality
2. **Component Tests** - Test individual modules
3. **Integration Tests** - Test system-wide functionality
4. **Mock Mode Tests** - Test without hardware
5. **Performance Tests** - Load and stress testing
6. **Memory Tests** - Memory leak detection
7. **Edge Case Tests** - Boundary condition testing

## Running Tests

### Prerequisites

1. **Build the project first**:
   ```bash
   ./scripts/build.sh --test
   # or on Windows
   .\scripts\build.ps1 -Test
   ```

2. **Install dependencies** (if not already done):
   ```bash
   ./scripts/install_deps.sh
   ```

### Basic Test Execution

#### Linux/macOS
```bash
# Run all tests
./scripts/run_tests.sh

# Run specific test categories
./scripts/run_tests.sh --no-memory --no-performance

# Run with custom timeout
./scripts/run_tests.sh --timeout 600

# Run stress tests
./scripts/run_tests.sh --stress
```

#### Windows (PowerShell)
```powershell
# Run all tests
.\scripts\run_tests.ps1

# Run specific test categories
.\scripts\run_tests.ps1 -NoMemoryTests -NoPerformanceTests

# Run with custom timeout
.\scripts\run_tests.ps1 -TestTimeout 600

# Run stress tests
.\scripts\run_tests.ps1 -StressTests
```

### Direct Test Execution

```bash
# Run all tests
cd build
./environet_tests

# Run specific test suites
./environet_tests --gtest_filter="*Sensor*"
./environet_tests --gtest_filter="*Config*"
./environet_tests --gtest_filter="*Time*"

# Run with XML output
./environet_tests --gtest_output=xml:test_results.xml

# Run tests multiple times
./environet_tests --gtest_repeat=5

# Run with verbose output
./environet_tests --gtest_verbose
```

## Test Configuration

### Test Scenarios

The `test_configs.json` file defines different test scenarios:

- **unit_tests**: Basic unit tests (60s timeout)
- **sensor_tests**: Comprehensive sensor testing (120s timeout)
- **config_tests**: Configuration validation (90s timeout)
- **time_tests**: Time utility functions (60s timeout)
- **integration_tests**: Full system integration (300s timeout)
- **mock_mode_tests**: Mock mode functionality (180s timeout)
- **performance_tests**: Performance testing (600s timeout)
- **stress_tests**: Stress testing (900s timeout)
- **rapid_test**: Quick development tests (30s timeout)
- **full_test_suite**: Complete CI/CD testing (1800s timeout)

### Configuration Overrides

Each test scenario can override default configuration:

```json
{
  "sensor_tests": {
    "config_overrides": {
      "i2c": {
        "mock_mode": true,
        "sample_interval_ms": 50
      }
    }
  }
}
```

## Mock Mode Testing

### What is Mock Mode?

Mock mode allows testing without physical hardware by:
- Generating synthetic sensor data
- Simulating network conditions
- Providing predictable test environments

### Mock Mode Configuration

```json
{
  "i2c": {
    "mock_mode": true,
    "sample_interval_ms": 100
  }
}
```

### Mock Data Validation

The framework validates mock data against expected ranges:
- **IR Sensor**: -512 to 511 (10-bit signed)
- **Ultrasonic**: 0 to 4000mm
- **Status Flags**: Valid bit combinations
- **CRC Validation**: Proper checksum calculation

## Performance Testing

### Performance Metrics

- **Response Time**: Maximum 1000ms for sensor reads
- **Throughput**: Minimum 10 frames per second
- **Memory Usage**: Maximum 100MB
- **CPU Usage**: Maximum 80%

### Load Testing

```bash
# Run performance tests
./environet_tests --gtest_filter="*Performance*" --gtest_repeat=10

# Run stress tests
./environet_tests --gtest_filter="*Stress*" --gtest_repeat=20
```

## Memory Testing

### Valgrind Integration

Memory testing uses Valgrind for:
- Memory leak detection
- Invalid memory access detection
- Memory corruption detection

### Memory Test Execution

```bash
# Run with Valgrind
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
         --track-origins=yes --verbose --log-file=valgrind.log \
         ./environet_tests
```

### Memory Test Analysis

```bash
# Analyze Valgrind results
python3 scripts/analyze_tests.py --charts
```

## Test Results Analysis

### Automated Analysis

The test framework provides comprehensive result analysis:

```bash
# Generate summary report
python3 scripts/analyze_tests.py

# Generate detailed HTML report
python3 scripts/analyze_tests.py --html-report report.html

# Generate charts and visualizations
python3 scripts/analyze_tests.py --charts
```

### Report Types

1. **Console Summary**: Quick overview of test results
2. **HTML Report**: Detailed web-based report
3. **Charts**: Visual representations of test data
4. **XML Output**: Machine-readable test results

### Test Metrics

- **Success Rate**: Percentage of passing tests
- **Execution Time**: Time per test suite
- **Memory Usage**: Memory consumption analysis
- **Performance**: Throughput and response times

## Continuous Integration

### CI/CD Integration

The testing framework supports CI/CD pipelines:

```yaml
# GitHub Actions example
- name: Run Tests
  run: |
    ./scripts/build.sh --test
    ./scripts/run_tests.sh --timeout 1800
    python3 scripts/analyze_tests.py --charts
```

### Test Automation

Automated testing includes:
- **Pre-commit Hooks**: Run tests before commits
- **Nightly Builds**: Comprehensive testing schedule
- **Release Validation**: Full test suite before releases

## Troubleshooting

### Common Issues

1. **Tests Fail to Compile**
   - Ensure all dependencies are installed
   - Check CMake configuration
   - Verify C++17 support

2. **Mock Mode Tests Fail**
   - Verify configuration file exists
   - Check mock mode is enabled
   - Ensure test data directories exist

3. **Performance Tests Timeout**
   - Increase timeout values
   - Check system resources
   - Verify no background processes

4. **Memory Tests Fail**
   - Install Valgrind
   - Check for system memory issues
   - Verify test isolation

### Debug Mode

Enable debug output:

```bash
# Verbose test output
./environet_tests --gtest_verbose

# Debug logging
./environet_tests --gtest_filter="*" --gtest_break_on_failure
```

### Test Isolation

Ensure tests don't interfere:

```bash
# Clean build
./scripts/build.sh --clean

# Run tests in isolation
./environet_tests --gtest_shuffle --gtest_random_seed=42
```

## Best Practices

### Writing Tests

1. **Test Naming**: Use descriptive test names
2. **Test Isolation**: Each test should be independent
3. **Setup/Teardown**: Use proper test fixtures
4. **Assertions**: Use appropriate assertion macros
5. **Error Handling**: Test both success and failure cases

### Test Data

1. **Mock Data**: Use realistic but predictable values
2. **Edge Cases**: Test boundary conditions
3. **Error Conditions**: Test error handling paths
4. **Performance**: Test under various loads

### Test Maintenance

1. **Regular Updates**: Keep tests current with code
2. **Documentation**: Document complex test scenarios
3. **Performance Monitoring**: Track test execution times
4. **Coverage Analysis**: Ensure adequate test coverage

## Advanced Features

### Custom Test Filters

```bash
# Run specific test patterns
./environet_tests --gtest_filter="*Sensor*:*Mock*"

# Exclude certain tests
./environet_tests --gtest_filter="*:-*Slow*"

# Run tests in specific order
./environet_tests --gtest_shuffle=false
```

### Test Output Formats

```bash
# XML output for CI tools
./environet_tests --gtest_output=xml:results.xml

# JSON output for custom analysis
./environet_tests --gtest_output=json:results.json

# JUnit output for Jenkins
./environet_tests --gtest_output=junit:junit.xml
```

### Parallel Testing

```bash
# Run tests in parallel (if supported)
./environet_tests --gtest_parallel=4

# Shuffle test execution order
./environet_tests --gtest_shuffle --gtest_random_seed=42
```

## Support and Contributing

### Getting Help

1. **Documentation**: Check this README and main project docs
2. **Issues**: Report bugs in the project issue tracker
3. **Discussions**: Use project discussion forums

### Contributing Tests

1. **Follow Patterns**: Use existing test structure
2. **Add Documentation**: Document new test scenarios
3. **Update Configs**: Add new test configurations
4. **Maintain Coverage**: Ensure adequate test coverage

### Test Standards

- **GoogleTest**: Follow GoogleTest best practices
- **Naming**: Use consistent naming conventions
- **Structure**: Follow established test organization
- **Documentation**: Document complex test logic

## Conclusion

The EnviroNet Analyzer testing framework provides comprehensive testing capabilities for development, validation, and deployment. By following the guidelines in this document, you can effectively test all aspects of the system and ensure reliable operation in production environments.

For additional information, refer to:
- [GoogleTest Documentation](https://google.github.io/googletest/)
- [CMake Testing](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
- [Valgrind User Manual](https://valgrind.org/docs/manual/manual.html)
