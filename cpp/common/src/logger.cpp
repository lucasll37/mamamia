// #include "common/logger.hpp"
// #include <spdlog/spdlog.h>
// #include <spdlog/sinks/stdout_color_sinks.h>
// #include <spdlog/sinks/rotating_file_sink.h>
// #include <memory>

// namespace mlinference {
// namespace common {

// Logger& Logger::get_instance() {
//     static Logger instance;
//     return instance;
// }

// void Logger::init(const std::string& log_file_path,
//                   LogLevel level,
//                   size_t max_file_size,
//                   size_t max_files) {
//     try {
//         std::vector<spdlog::sink_ptr> sinks;
        
//         // Console sink
//         auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//         console_sink->set_level(to_spdlog_level(level));
//         console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
//         sinks.push_back(console_sink);
        
//         // File sink (if path provided)
//         if (!log_file_path.empty()) {
//             auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
//                 log_file_path, max_file_size, max_files);
//             file_sink->set_level(to_spdlog_level(level));
//             file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
//             sinks.push_back(file_sink);
//         }
        
//         // Create logger
//         logger_ = std::make_shared<spdlog::logger>("ml-inference", 
//                                                     sinks.begin(), 
//                                                     sinks.end());
//         logger_->set_level(to_spdlog_level(level));
//         logger_->flush_on(spdlog::level::warn);
        
//         // Register as default logger
//         spdlog::set_default_logger(logger_);
        
//         logger_->info("Logger initialized: level={}, file={}", 
//                      static_cast<int>(level), 
//                      log_file_path.empty() ? "console-only" : log_file_path);
        
//     } catch (const spdlog::spdlog_ex& ex) {
//         std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
//     }
// }

// void Logger::set_level(LogLevel level) {
//     if (logger_) {
//         logger_->set_level(to_spdlog_level(level));
//     }
// }

// void Logger::flush() {
//     if (logger_) {
//         logger_->flush();
//     }
// }

// spdlog::level::level_enum Logger::to_spdlog_level(LogLevel level) {
//     switch (level) {
//         case LogLevel::TRACE:    return spdlog::level::trace;
//         case LogLevel::DEBUG:    return spdlog::level::debug;
//         case LogLevel::INFO:     return spdlog::level::info;
//         case LogLevel::WARN:     return spdlog::level::warn;
//         case LogLevel::ERROR:    return spdlog::level::err;
//         case LogLevel::CRITICAL: return spdlog::level::critical;
//         default:                 return spdlog::level::info;
//     }
// }

// } // namespace common
// } // namespace mlinference