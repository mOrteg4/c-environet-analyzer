cmake_minimum_required(VERSION 3.16)
project(EnviroNet-Analyzer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fsanitize=address,undefined -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -Wall -Wextra")

# Find required packages
find_package(PkgConfig REQUIRED)

# System packages
find_package(Threads REQUIRED)
find_package(spdlog REQUIRED)

# PkgConfig packages
pkg_check_modules(PCAP REQUIRED libpcap)
pkg_check_modules(LIBNL REQUIRED libnl-3.0)
pkg_check_modules(LIBNL_GENL REQUIRED libnl-genl-3.0)
pkg_check_modules(GPIOD REQUIRED libgpiod)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${PCAP_INCLUDE_DIRS})
include_directories(${LIBNL_INCLUDE_DIRS})
include_directories(${LIBNL_GENL_INCLUDE_DIRS})
include_directories(${GPIOD_INCLUDE_DIRS})

# Link directories
link_directories(${PCAP_LIBRARY_DIRS})
link_directories(${LIBNL_LIBRARY_DIRS})
link_directories(${LIBNL_GENL_LIBRARY_DIRS})
link_directories(${GPIOD_LIBRARY_DIRS})

# Fetch nlohmann/json (header-only)
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Source files (only include files that actually exist)
set(SOURCES
    src/main.cpp
    src/core/log.cpp
)

# Header files
set(HEADERS
    include/core/log.hpp
    include/core/config.hpp
    include/sensors/arduino_i2c.hpp
    include/net/pcap_sniffer.hpp
    include/net/wifi_scan.hpp
    include/net/metrics.hpp
    include/correlate/correlator.hpp
    include/util/time.hpp
)

# Create main executable
add_executable(environet ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(environet
    Threads::Threads
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    ${PCAP_LIBRARIES}
    ${LIBNL_LIBRARIES}
    ${LIBNL_GENL_LIBRARIES}
    ${GPIOD_LIBRARIES}
    rt
    m
)

# Compiler-specific flags
target_compile_options(environet PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# Enable testing
# enable_testing()

# Find GTest
# find_package(GTest REQUIRED)

# Test source files
# set(TEST_SOURCES
#     tests/test_main.cpp
#     tests/test_sensors.cpp
#     tests/test_config.cpp
#     tests/test_time.cpp
# )

# Test executable
# add_executable(environet_tests ${TEST_SOURCES})
# target_link_libraries(environet_tests
#     GTest::gtest
#     GTest::gtest_main
#     GTest::gmock
# )

# Add individual test suites
# add_test(NAME SensorTests COMMAND environet_tests --gtest_filter="*Sensor*")
# add_test(NAME ConfigTests COMMAND environet_tests --gtest_filter="*Config*")
# add_test(NAME TimeTests COMMAND environet_tests --gtest_filter="*Time*")
# add_test(NAME IntegrationTests COMMAND environet_tests --gtest_filter="*Integration*")

# Set test properties (after tests are created)
# set_tests_properties(SensorTests ConfigTests TimeTests IntegrationTests PROPERTIES
#     TIMEOUT 300
#     ENVIRONMENT "GTEST_COLOR=1"
# )

# Add test discovery
# include(GoogleTest)
# gtest_discover_tests(environet_tests)

# Install targets
install(TARGETS environet
    RUNTIME DESTINATION bin
)

# Install configuration
install(FILES config/config.json
    DESTINATION /etc/environet
)

# Install systemd service
install(FILES systemd/environet.service
    DESTINATION /etc/systemd/system
)

# Install scripts
install(DIRECTORY scripts/
    DESTINATION /usr/local/bin/environet
    FILES_MATCHING PATTERN "*.sh"
    PATTERN "*.py"
)

# Set executable permissions for scripts
install(CODE "
    file(GLOB_RECURSE SCRIPTS \"\${CMAKE_INSTALL_PREFIX}/usr/local/bin/environet/*.sh\")
    foreach(SCRIPT \${SCRIPTS})
        execute_process(COMMAND chmod +x \${SCRIPT})
    endforeach()
")

# Print configuration summary
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Testing enabled: NO (test files not yet implemented)")
message(STATUS "Source files: ${SOURCES}")
