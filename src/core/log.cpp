#include "core/log.hpp"
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <iostream>

namespace environet {
namespace core {

// Global logger instance
static std::shared_ptr<spdlog::logger> g_logger;

void init_logger(const std::string& level, 
                 const std::string& file_path, 
                 size_t max_size, 
                 size_t max_files) {
    
    // Create sinks vector
    std::vector<spdlog::sink_ptr> sinks;
    
    // Always add console sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::from_str(level));
    sinks.push_back(console_sink);
    
    // Add file sink if path is provided
    if (!file_path.empty()) {
        try {
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                file_path, max_size, max_files);
            file_sink->set_level(spdlog::level::from_str(level));
            sinks.push_back(file_sink);
        } catch (const std::exception& e) {
            std::cerr << "Warning: Failed to create log file: " << e.what() << std::endl;
            std::cerr << "Continuing with console logging only." << std::endl;
        }
    }
    
    // Create logger with multiple sinks
    g_logger = std::make_shared<spdlog::logger>("environet", sinks.begin(), sinks.end());
    g_logger->set_level(spdlog::level::from_str(level));
    
    // Set as default logger
    spdlog::set_default_logger(g_logger);
    
    // Log initialization
    g_logger->info("Logging system initialized with level: {}", level);
    if (!file_path.empty()) {
        g_logger->info("Log file: {} (max: {}MB, keep: {} files)", 
                      file_path, max_size / (1024 * 1024), max_files);
    }
}

std::shared_ptr<spdlog::logger> get_logger() {
    if (!g_logger) {
        // If logger hasn't been initialized, create a basic console logger
        std::cerr << "Warning: Logger not initialized, creating default console logger" << std::endl;
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        g_logger = std::make_shared<spdlog::logger>("environet_default", console_sink);
        g_logger->set_level(spdlog::level::info);
        spdlog::set_default_logger(g_logger);
    }
    return g_logger;
}

void shutdown_logger() {
    if (g_logger) {
        g_logger->info("Shutting down logging system");
        g_logger->flush();
        spdlog::shutdown();
        g_logger.reset();
    }
}

} // namespace core
} // namespace environet
