#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>

#include "core/log.hpp"
#include "core/config.hpp"
#include "sensors/arduino_i2c.hpp"
#include "net/wifi_scan.hpp"
#include "net/pcap_sniffer.hpp"
#include "net/metrics.hpp"
#include "correlate/correlator.hpp"

// Global shutdown flag
std::atomic<bool> g_shutdown_requested(false);

// Signal handler for graceful shutdown
void signal_handler(int signal) {
    LOGI("Received signal {}, initiating graceful shutdown...", signal);
    g_shutdown_requested.store(true);
}

// Forward declarations
void setup_signal_handlers();
void create_directories(const environet::core::Config& config);
void sensor_thread_func(std::shared_ptr<environet::sensors::ArduinoI2C> sensor,
                       std::shared_ptr<environet::correlate::Correlator> correlator,
                       const environet::core::Config& config);
void wifi_scan_thread_func(std::shared_ptr<environet::net::WifiScan> wifi_scan,
                          std::shared_ptr<environet::correlate::Correlator> correlator,
                          const environet::core::Config& config);
void pcap_thread_func(std::shared_ptr<environet::net::PcapSniffer> pcap_sniffer,
                     std::shared_ptr<environet::correlate::Correlator> correlator);
void metrics_thread_func(std::shared_ptr<environet::net::Metrics> metrics,
                        std::shared_ptr<environet::correlate::Correlator> correlator,
                        const environet::core::Config& config);
void correlation_thread_func(std::shared_ptr<environet::correlate::Correlator> correlator);

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        std::string config_path = "config/config.json";
        bool mock_mode = true;
        bool test_sensors = false;
        bool test_network = false;
        bool test_pcap = false;
        
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "--config" && i + 1 < argc) {
                config_path = argv[++i];
            } else if (arg == "--mock") {
                mock_mode = true;
            } else if (arg == "--real") {
                mock_mode = false;
            } else if (arg == "--test-sensors") {
                test_sensors = true;
            } else if (arg == "--test-network") {
                test_network = true;
            } else if (arg == "--test-pcap") {
                test_pcap = true;
            } else if (arg == "--help" || arg == "-h") {
                std::cout << "EnviroNet Analyzer - C++ Implementation\n"
                          << "Usage: " << argv[0] << " [options]\n"
                          << "Options:\n"
                          << "  --config <path>    Configuration file path\n"
                          << "  --mock             Enable mock mode (default)\n"
                          << "  --real             Enable real hardware mode\n"
                          << "  --test-sensors     Test sensor functionality\n"
                          << "  --test-network     Test network functionality\n"
                          << "  --test-pcap        Test packet capture\n"
                          << "  --help, -h         Show this help message\n";
                return 0;
            }
        }
        
        // Load configuration
        LOGI("Loading configuration from: {}", config_path);
        environet::core::Config config;
        try {
            config = environet::core::Config::load(config_path);
        } catch (const std::exception& e) {
            LOGE("Failed to load configuration: {}", e.what());
            std::cerr << "Error: Failed to load configuration: " << e.what() << std::endl;
            return 1;
        }
        
        // Override mock mode if specified on command line
        if (!mock_mode) {
            config.i2c.mock_mode = false;
            LOGI("Command line override: Real hardware mode enabled");
        }
        
        // Initialize logging
        LOGI("Initializing logging system");
        environet::core::init_logger(config.logging.level, config.logging.file, 
                                   config.logging.max_size_mb * 1024 * 1024, 
                                   config.logging.max_files);
        
        LOGI("EnviroNet Analyzer starting up...");
        LOGI("Configuration: mock_i2c={}, wifi_scan_interval={}ms, pcap_bpf='{}'", 
             config.i2c.mock_mode, config.wifi.scan_interval_ms, config.pcap.bpf);
        
        // Create necessary directories
        create_directories(config);
        
        // Setup signal handlers for graceful shutdown
        setup_signal_handlers();
        
        // Initialize components
        LOGI("Initializing components...");
        
    auto sensor = std::make_shared<environet::sensors::ArduinoI2C>(config);
        if (!sensor->init()) {
            LOGE("Failed to initialize sensor: {}", sensor->get_last_error());
            return 1;
        }
        
        auto wifi_scan = std::make_shared<environet::net::WifiScan>(config_path);
        if (!wifi_scan->init()) {
            LOGW("Failed to initialize WiFi scan: {}", wifi_scan->get_last_error());
            LOGW("Continuing without WiFi scanning...");
        }
        
        auto pcap_sniffer = std::make_shared<environet::net::PcapSniffer>(config_path);
        if (!pcap_sniffer->init()) {
            LOGW("Failed to initialize PCAP sniffer: {}", pcap_sniffer->get_last_error());
            LOGW("Continuing without packet capture...");
        }
        
        auto metrics = std::make_shared<environet::net::Metrics>(config_path);
        if (!metrics->init()) {
            LOGW("Failed to initialize metrics: {}", metrics->get_last_error());
            LOGW("Continuing without network metrics...");
        }
        
        auto correlator = std::make_shared<environet::correlate::Correlator>(config_path);
        if (!correlator->init()) {
            LOGE("Failed to initialize correlator: {}", correlator->get_last_error());
            return 1;
        }
        
        // Set up finding callback
        correlator->set_finding_callback([](const environet::correlate::Finding& finding) {
            LOGI("New finding: {} - {}", finding.event_type, finding.description);
            // TODO: Save to database, send alerts, etc.
        });
        
        LOGI("All components initialized successfully");
        
        // Run tests if requested
        if (test_sensors) {
            LOGI("Running sensor tests...");
            environet::sensors::SensorFrame frame;
            for (int i = 0; i < 5; i++) {
                if (sensor->read_frame(frame)) {
                    LOGI("Sensor frame {}: IR={}, Ultra={}mm, Status=0x{:02x}", 
                         i, frame.ir_raw, frame.ultra_mm, frame.status);
                } else {
                    LOGE("Failed to read sensor frame {}", i);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
            LOGI("Sensor tests completed");
            return 0;
        }
        
        if (test_network) {
            LOGI("Running network tests...");
            auto bss_list = wifi_scan->scan();
            LOGI("Found {} WiFi networks", bss_list.size());
            for (const auto& bss : bss_list) {
                LOGI("  SSID: {}, BSSID: {}, Signal: {} dBm", 
                     bss.ssid, bss.bssid, bss.signal_mbm / 100.0);
            }
            
            auto ping_stats = metrics->ping_test("8.8.8.8", 4);
            if (ping_stats.reachable) {
                LOGI("Ping test: avg={:.2f}ms, loss={:.1f}%", 
                     ping_stats.avg_rtt_ms, ping_stats.loss_percentage);
            } else {
                LOGW("Ping test failed");
            }
            LOGI("Network tests completed");
            return 0;
        }
        
        if (test_pcap) {
            LOGI("Running PCAP tests...");
            bool pcap_started = pcap_sniffer->start([correlator](const environet::net::PacketMeta& meta, const uint8_t* data) {
                (void)data; // Suppress unused parameter warning
                correlator->push_packet(meta);
                LOGD("Packet: {} -> {}, {} bytes", meta.src_mac, meta.dst_mac, meta.length);
            });
            
            if (pcap_started) {
                LOGI("PCAP capture started, running for 10 seconds...");
                std::this_thread::sleep_for(std::chrono::seconds(10));
                pcap_sniffer->stop();
                LOGI("PCAP tests completed");
            } else {
                LOGE("Failed to start PCAP capture");
            }
            return 0;
        }
        
        // Start main monitoring threads
        LOGI("Starting monitoring threads...");
        
        std::thread sensor_thread(sensor_thread_func, sensor, correlator, std::ref(config));
        std::thread wifi_thread(wifi_scan_thread_func, wifi_scan, correlator, std::ref(config));
        std::thread pcap_thread(pcap_thread_func, pcap_sniffer, correlator);
        std::thread metrics_thread(metrics_thread_func, metrics, correlator, std::ref(config));
        std::thread correlation_thread(correlation_thread_func, correlator);
        
        // Main loop - wait for shutdown signal
        LOGI("Monitoring started. Press Ctrl+C to stop.");
        while (!g_shutdown_requested.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        // Shutdown sequence
        LOGI("Shutting down...");
        
        // Stop components first
        pcap_sniffer->stop();
        
        // Wait for threads to finish
        if (sensor_thread.joinable()) {
            // For std::thread, we can't timeout on join, so we just join
            // In a real application, you might want to use std::future with timeout
            sensor_thread.join();
            LOGI("Sensor thread joined successfully");
        }
        
        if (wifi_thread.joinable()) {
            wifi_thread.join();
            LOGI("WiFi thread joined successfully");
        }
        
        if (pcap_thread.joinable()) {
            pcap_thread.join();
            LOGI("PCAP thread joined successfully");
        }
        
        if (metrics_thread.joinable()) {
            metrics_thread.join();
            LOGI("Metrics thread joined successfully");
        }
        
        if (correlation_thread.joinable()) {
            correlation_thread.join();
            LOGI("Correlation thread joined successfully");
        }
        
        // Cleanup
        sensor->stop();
        
        LOGI("Shutdown complete");
        environet::core::shutdown_logger();
        
    } catch (const std::exception& e) {
        LOGE("Fatal error: {}", e.what());
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

void setup_signal_handlers() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
}

void create_directories(const environet::core::Config& config) {
    // Create log directory
    std::string log_dir = config.logging.file.substr(0, config.logging.file.find_last_of('/'));
    if (!log_dir.empty()) {
        mkdir(log_dir.c_str(), 0755);
    }
    
    // Create findings directory
    mkdir(config.correlator.findings_dir.c_str(), 0755);
    
    // Create captures directory
    mkdir(config.pcap.output_dir.c_str(), 0755);
}

void sensor_thread_func(std::shared_ptr<environet::sensors::ArduinoI2C> sensor,
                       std::shared_ptr<environet::correlate::Correlator> correlator,
                       const environet::core::Config& config) {
    LOGI("Sensor thread started");
    
    environet::sensors::SensorFrame frame;
    while (!g_shutdown_requested.load()) {
        if (sensor->read_frame(frame)) {
            correlator->push_sensor(frame);
            LOGD("Sensor frame: IR={}, Ultra={}mm, Status=0x{:02x}", 
                 frame.ir_raw, frame.ultra_mm, frame.status);
        } else {
            LOGW("Failed to read sensor frame: {}", sensor->get_last_error());
        }
        
        // Sleep for sample interval
        std::this_thread::sleep_for(std::chrono::milliseconds(config.i2c.sample_interval_ms));
    }
    
    LOGI("Sensor thread stopped");
}

void wifi_scan_thread_func(std::shared_ptr<environet::net::WifiScan> wifi_scan,
                          std::shared_ptr<environet::correlate::Correlator> correlator,
                          const environet::core::Config& config) {
    LOGI("WiFi scan thread started");
    
    while (!g_shutdown_requested.load()) {
        try {
            auto bss_list = wifi_scan->scan();
            for (const auto& bss : bss_list) {
                correlator->push_bss(bss);
            }
            LOGD("WiFi scan completed: {} networks found", bss_list.size());
        } catch (const std::exception& e) {
            LOGW("WiFi scan failed: {}", e.what());
        }
        
        // Sleep for scan interval
        std::this_thread::sleep_for(std::chrono::milliseconds(config.wifi.scan_interval_ms));
    }
    
    LOGI("WiFi scan thread stopped");
}

void pcap_thread_func(std::shared_ptr<environet::net::PcapSniffer> pcap_sniffer,
                     std::shared_ptr<environet::correlate::Correlator> correlator) {
    LOGI("PCAP thread started");
    
    bool started = pcap_sniffer->start([correlator](const environet::net::PacketMeta& meta, const uint8_t* data) {
        (void)data; // Suppress unused parameter warning
        correlator->push_packet(meta);
        LOGD("Packet: {} -> {}, {} bytes", meta.src_mac, meta.dst_mac, meta.length);
    });
    
    if (!started) {
        LOGE("Failed to start PCAP capture");
        return;
    }
    
    // Wait for shutdown
    while (!g_shutdown_requested.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    LOGI("PCAP thread stopped");
}

void metrics_thread_func(std::shared_ptr<environet::net::Metrics> metrics,
                        std::shared_ptr<environet::correlate::Correlator> correlator,
                        const environet::core::Config& config) {
    LOGI("Metrics thread started");
    
    while (!g_shutdown_requested.load()) {
        try {
            // Run ping tests
            for (const auto& target : config.metrics.ping_targets) {
                auto ping_stats = metrics->ping_test(target, 4);
                correlator->push_ping_stats(ping_stats);
                LOGD("Ping {}: avg={:.2f}ms, loss={:.1f}%", 
                     target, ping_stats.avg_rtt_ms, ping_stats.loss_percentage);
            }
            
            // Run iperf3 test if server is configured
            if (!config.metrics.iperf_server.empty()) {
                auto iperf_results = metrics->iperf3_test(config.metrics.iperf_server, 
                                                        config.metrics.iperf_duration);
                correlator->push_iperf3_results(iperf_results);
                if (iperf_results.success) {
                    LOGD("iPerf3: {} Mbps", iperf_results.bandwidth_mbps);
                }
            }
        } catch (const std::exception& e) {
            LOGW("Metrics collection failed: {}", e.what());
        }
        
        // Sleep for metrics interval
        std::this_thread::sleep_for(std::chrono::milliseconds(config.metrics.ping_interval_ms));
    }
    
    LOGI("Metrics thread stopped");
}

void correlation_thread_func(std::shared_ptr<environet::correlate::Correlator> correlator) {
    LOGI("Correlation thread started");
    
    while (!g_shutdown_requested.load()) {
        try {
            auto findings = correlator->process();
            if (!findings.empty()) {
                LOGI("Generated {} new findings", findings.size());
            }
        } catch (const std::exception& e) {
            LOGW("Correlation processing failed: {}", e.what());
        }
        
        // Process correlations every second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    LOGI("Correlation thread stopped");
}
