#ifndef ML_INFERENCE_COMMON_LOGGER_HPP
#define ML_INFERENCE_COMMON_LOGGER_HPP

#include <memory>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace mlinference {
namespace common {

// Log levels
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

// Logger class - wrapper around spdlog
class Logger {
public:
    // Get singleton instance
    static Logger& get_instance();
    
    // Initialize logger with configuration
    void init(const std::string& log_file_path = "",
              LogLevel level = LogLevel::INFO,
              size_t max_file_size = 10 * 1024 * 1024,  // 10 MB
              size_t max_files = 3);
    
    // Set log level
    void set_level(LogLevel level);
    
    // Log messages
    template<typename... Args>
    void trace(const char* fmt, Args&&... args) {
        logger_->trace(fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void debug(const char* fmt, Args&&... args) {
        logger_->debug(fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void info(const char* fmt, Args&&... args) {
        logger_->info(fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void warn(const char* fmt, Args&&... args) {
        logger_->warn(fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void error(const char* fmt, Args&&... args) {
        logger_->error(fmt, std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void critical(const char* fmt, Args&&... args) {
        logger_->critical(fmt, std::forward<Args>(args)...);
    }
    
    // Flush logs
    void flush();
    
    // Get underlying spdlog logger
    std::shared_ptr<spdlog::logger> get_spdlog() { return logger_; }
    
private:
    Logger() = default;
    ~Logger() = default;
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    std::shared_ptr<spdlog::logger> logger_;
    spdlog::level::level_enum to_spdlog_level(LogLevel level);
};

// Convenience macros
#define LOG_TRACE(...) mlinference::common::Logger::get_instance().trace(__VA_ARGS__)
#define LOG_DEBUG(...) mlinference::common::Logger::get_instance().debug(__VA_ARGS__)
#define LOG_INFO(...) mlinference::common::Logger::get_instance().info(__VA_ARGS__)
#define LOG_WARN(...) mlinference::common::Logger::get_instance().warn(__VA_ARGS__)
#define LOG_ERROR(...) mlinference::common::Logger::get_instance().error(__VA_ARGS__)
#define LOG_CRITICAL(...) mlinference::common::Logger::get_instance().critical(__VA_ARGS__)

} // namespace common
} // namespace mlinference

#endif // ML_INFERENCE_COMMON_LOGGER_HPP
