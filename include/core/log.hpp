#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace environet {
namespace core {

/**
 * @brief Initialize the logging system
 * 
 * Sets up rotating file logger and console logger with specified level
 * 
 * @param level Log level (trace, debug, info, warn, error, critical)
 * @param file_path Path to log file (optional, defaults to console only)
 * @param max_size Maximum size of each log file in MB (default: 5MB)
 * @param max_files Maximum number of log files to keep (default: 3)
 */
void init_logger(const std::string& level = "info", 
                 const std::string& file_path = "", 
                 size_t max_size = 5 * 1024 * 1024,  // 5MB
                 size_t max_files = 3);

/**
 * @brief Get the main logger instance
 * 
 * @return Shared pointer to the main logger
 */
std::shared_ptr<spdlog::logger> get_logger();

/**
 * @brief Shutdown logging system gracefully
 */
void shutdown_logger();

} // namespace core
} // namespace environet

// Convenient logging macros
#define LOGI(...) SPDLOG_LOGGER_INFO(environet::core::get_logger(), __VA_ARGS__)
#define LOGW(...) SPDLOG_LOGGER_WARN(environet::core::get_logger(), __VA_ARGS__)
#define LOGE(...) SPDLOG_LOGGER_ERROR(environet::core::get_logger(), __VA_ARGS__)
#define LOGD(...) SPDLOG_LOGGER_DEBUG(environet::core::get_logger(), __VA_ARGS__)
#define LOGT(...) SPDLOG_LOGGER_TRACE(environet::core::get_logger(), __VA_ARGS__)
#define LOGC(...) SPDLOG_LOGGER_CRITICAL(environet::core::get_logger(), __VA_ARGS__)
