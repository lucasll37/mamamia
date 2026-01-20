// #ifndef ML_INFERENCE_COMMON_CONFIG_HPP
// #define ML_INFERENCE_COMMON_CONFIG_HPP

// #include <string>
// #include <map>
// #include <vector>
// #include <memory>
// #include <optional>
// #include <yaml-cpp/yaml.h>
// #include "types.hpp"

// namespace mlinference {
// namespace common {

// // Base configuration class
// class Config {
// public:
//     virtual ~Config() = default;
    
//     // Load configuration from YAML file
//     virtual bool load_from_file(const std::string& file_path);
    
//     // Load configuration from YAML string
//     virtual bool load_from_string(const std::string& yaml_str);
    
//     // Save configuration to file
//     virtual bool save_to_file(const std::string& file_path) const;
    
//     // Get YAML representation
//     virtual std::string to_yaml() const;
    
// protected:
//     YAML::Node config_;
    
//     // Helper methods for derived classes
//     template<typename T>
//     std::optional<T> get_value(const std::string& key) const {
//         try {
//             if (config_[key]) {
//                 return config_[key].as<T>();
//             }
//         } catch (const std::exception& e) {
//             return std::nullopt;
//         }
//         return std::nullopt;
//     }
    
//     template<typename T>
//     T get_value_or(const std::string& key, const T& default_value) const {
//         auto value = get_value<T>(key);
//         return value.value_or(default_value);
//     }
    
//     template<typename T>
//     void set_value(const std::string& key, const T& value) {
//         config_[key] = value;
//     }
// };

// // Manager configuration
// struct ManagerConfig : public Config {
//     struct Server {
//         std::string address = "0.0.0.0";
//         uint32_t port = 50051;
//         uint32_t max_workers = 100;
//         uint32_t thread_pool_size = 4;
//     } server;
    
//     struct Worker {
//         uint32_t heartbeat_interval_seconds = 30;
//         uint32_t heartbeat_timeout_seconds = 90;
//         uint32_t max_missed_heartbeats = 3;
//     } worker;
    
//     struct Storage {
//         std::string models_directory = "./models";
//         std::string database_path = "./manager.db";
//         uint64_t max_model_size_mb = 1024;
//     } storage;
    
//     struct Security {
//         bool enable_tls = false;
//         std::string tls_cert_path = "";
//         std::string tls_key_path = "";
//         std::string admin_api_key = "";
//     } security;
    
//     struct Logging {
//         std::string log_file = "./manager.log";
//         std::string log_level = "info";
//         uint64_t max_log_size_mb = 10;
//         uint32_t max_log_files = 3;
//     } logging;
    
//     // Load specific manager config
//     bool load(const std::string& file_path);
    
//     // Validate configuration
//     bool validate() const;
// };

// // Worker configuration
// struct WorkerConfig : public Config {
//     struct Server {
//         std::string address = "0.0.0.0";
//         uint32_t port = 50052;
//         uint32_t thread_pool_size = 8;
//         uint32_t max_concurrent_requests = 10;
//     } server;
    
//     struct Manager {
//         std::string address = "localhost";
//         uint32_t port = 50051;
//         uint32_t heartbeat_interval_seconds = 30;
//         uint32_t reconnect_interval_seconds = 5;
//     } manager;
    
//     struct Inference {
//         bool enable_gpu = true;
//         uint32_t gpu_device_id = 0;
//         uint32_t num_cpu_threads = 4;
//         uint32_t model_unload_timeout_seconds = 300;
//         uint64_t max_memory_usage_mb = 4096;
//     } inference;
    
//     struct Storage {
//         std::string models_cache_directory = "./models_cache";
//         uint64_t max_cache_size_mb = 10240;
//     } storage;
    
//     struct Security {
//         bool enable_tls = false;
//         std::string tls_cert_path = "";
//     } security;
    
//     struct Logging {
//         std::string log_file = "./worker.log";
//         std::string log_level = "info";
//         uint64_t max_log_size_mb = 10;
//         uint32_t max_log_files = 3;
//     } logging;
    
//     // Worker identification
//     std::string worker_id = "";  // Auto-generated if empty
    
//     // Load specific worker config
//     bool load(const std::string& file_path);
    
//     // Validate configuration
//     bool validate() const;
// };

// // API Gateway configuration (for reference, actual config in Python)
// struct ApiGatewayConfig : public Config {
//     struct Server {
//         std::string host = "0.0.0.0";
//         uint32_t port = 8000;
//         uint32_t workers = 4;
//     } server;
    
//     struct Manager {
//         std::string address = "localhost";
//         uint32_t port = 50051;
//     } manager;
    
//     struct Security {
//         bool enable_cors = true;
//         std::vector<std::string> cors_origins = {"*"};
//         std::string jwt_secret = "";
//     } security;
    
//     bool load(const std::string& file_path);
//     bool validate() const;
// };

// } // namespace common
// } // namespace mlinference

// #endif // ML_INFERENCE_COMMON_CONFIG_HPP
