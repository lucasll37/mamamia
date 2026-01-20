#include <iostream>
#include <csignal>
#include <memory>
#include "worker/worker_server.hpp"

std::unique_ptr<mlinference::worker::WorkerServer> g_server;

void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
}

int main(int argc, char** argv) {
    std::cout << "==================================" << std::endl;
    std::cout << "ML Inference Worker Server" << std::endl;
    std::cout << "==================================" << std::endl;
    
    // Parse command line arguments
    std::string worker_id = "worker-1";
    std::string server_address = "0.0.0.0:50052";
    bool enable_gpu = false;
    uint32_t num_threads = 4;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--id" && i + 1 < argc) {
            worker_id = argv[++i];
        } else if (arg == "--address" && i + 1 < argc) {
            server_address = argv[++i];
        } else if (arg == "--gpu") {
            enable_gpu = true;
        } else if (arg == "--threads" && i + 1 < argc) {
            num_threads = std::stoi(argv[++i]);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --id <worker_id>        Worker ID (default: worker-1)" << std::endl;
            std::cout << "  --address <addr:port>   Server address (default: 0.0.0.0:50052)" << std::endl;
            std::cout << "  --gpu                   Enable GPU support" << std::endl;
            std::cout << "  --threads <num>         Number of threads (default: 4)" << std::endl;
            std::cout << "  --help, -h              Show this help message" << std::endl;
            return 0;
        }
    }
    
    std::cout << "Configuration:" << std::endl;
    std::cout << "  Worker ID: " << worker_id << std::endl;
    std::cout << "  Address: " << server_address << std::endl;
    std::cout << "  GPU: " << (enable_gpu ? "enabled" : "disabled") << std::endl;
    std::cout << "  Threads: " << num_threads << std::endl;
    std::cout << std::endl;
    
    // Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    try {
        // Create and start server
        g_server = std::make_unique<mlinference::worker::WorkerServer>(
            worker_id,
            server_address,
            enable_gpu,
            num_threads
        );
        
        std::cout << "Starting worker server..." << std::endl;
        g_server->run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Worker server stopped" << std::endl;
    return 0;
}