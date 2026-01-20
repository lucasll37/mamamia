// #include "worker/worker_server.hpp"
// #include "common/logger.hpp"
// #include "common/config.hpp"
// #include <iostream>
// #include <csignal>
// #include <atomic>

// using namespace mlinference;

// std::atomic<bool> shutdown_requested(false);

// void signal_handler(int signal) {
//     if (signal == SIGINT || signal == SIGTERM) {
//         std::cout << "\nShutdown signal received..." << std::endl;
//         shutdown_requested = true;
//     }
// }

// void print_usage(const char* program_name) {
//     std::cout << "Usage: " << program_name << " [OPTIONS]\n"
//               << "\nOptions:\n"
//               << "  --config PATH    Path to configuration file (required)\n"
//               << "  --help           Show this help message\n"
//               << "  --version        Show version information\n"
//               << "  --health-check   Check if worker is healthy and exit\n"
//               << std::endl;
// }

// void print_version() {
//     std::cout << "ML Inference System - Worker Node\n"
//               << "Version: 1.0.0\n"
//               << "Build: " << __DATE__ << " " << __TIME__ << std::endl;
// }

// int main(int argc, char** argv) {
//     std::string config_path;
//     bool show_help = false;
//     bool show_version = false;
//     bool health_check = false;
    
//     // Parse command-line arguments
//     for (int i = 1; i < argc; ++i) {
//         std::string arg = argv[i];
        
//         if (arg == "--help" || arg == "-h") {
//             show_help = true;
//         } else if (arg == "--version" || arg == "-v") {
//             show_version = true;
//         } else if (arg == "--health-check") {
//             health_check = true;
//         } else if (arg == "--config") {
//             if (i + 1 < argc) {
//                 config_path = argv[++i];
//             } else {
//                 std::cerr << "Error: --config requires a path" << std::endl;
//                 return 1;
//             }
//         } else {
//             std::cerr << "Unknown argument: " << arg << std::endl;
//             print_usage(argv[0]);
//             return 1;
//         }
//     }
    
//     if (show_help) {
//         print_usage(argv[0]);
//         return 0;
//     }
    
//     if (show_version) {
//         print_version();
//         return 0;
//     }
    
//     if (health_check) {
//         std::cout << "Health check not implemented yet" << std::endl;
//         return 1;
//     }
    
//     if (config_path.empty()) {
//         std::cerr << "Error: --config is required" << std::endl;
//         print_usage(argv[0]);
//         return 1;
//     }
    
//     // Load configuration
//     common::WorkerConfig config;
//     if (!config.load(config_path)) {
//         std::cerr << "Error: Failed to load configuration from " << config_path << std::endl;
//         return 1;
//     }
    
//     // Initialize logger
//     common::LogLevel log_level = common::LogLevel::INFO;
//     if (config.logging.log_level == "trace") log_level = common::LogLevel::TRACE;
//     else if (config.logging.log_level == "debug") log_level = common::LogLevel::DEBUG;
//     else if (config.logging.log_level == "warn") log_level = common::LogLevel::WARN;
//     else if (config.logging.log_level == "error") log_level = common::LogLevel::ERROR;
    
//     common::Logger::get_instance().init(
//         config.logging.log_file,
//         log_level,
//         config.logging.max_log_size_mb * 1024 * 1024,
//         config.logging.max_log_files);
    
//     LOG_INFO("========================================");
//     LOG_INFO("ML Inference System - Worker Node");
//     LOG_INFO("Version: 1.0.0");
//     LOG_INFO("========================================");
    
//     LOG_INFO("Configuration loaded from: {}", config_path);
//     LOG_INFO("Worker ID: {}", config.worker_id);
//     LOG_INFO("Server address: {}:{}", config.server.address, config.server.port);
//     LOG_INFO("Manager address: {}:{}", config.manager.address, config.manager.port);
//     LOG_INFO("GPU enabled: {}", config.inference.enable_gpu);
    
//     // Register signal handlers
//     std::signal(SIGINT, signal_handler);
//     std::signal(SIGTERM, signal_handler);
    
//     // Create and start server
//     worker::WorkerServer server(config);
    
//     if (!server.start()) {
//         LOG_CRITICAL("Failed to start Worker server");
//         return 1;
//     }
    
//     LOG_INFO("Worker server started successfully");
//     LOG_INFO("Press Ctrl+C to stop...");
    
//     // Wait for shutdown signal
//     while (!shutdown_requested) {
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//     }
    
//     LOG_INFO("Shutting down...");
//     server.stop();
    
//     LOG_INFO("Worker server stopped");
//     common::Logger::get_instance().flush();
    
//     return 0;
// }