// #include "common/config.hpp"
// #include <fstream>
// #include <sstream>

// namespace mlinference {
// namespace common {

// // ============================================================================
// // Base Config Implementation
// // ============================================================================

// bool Config::load_from_file(const std::string& file_path) {
//     try {
//         config_ = YAML::LoadFile(file_path);
//         return true;
//     } catch (const YAML::Exception& e) {
//         return false;
//     }
// }

// bool Config::load_from_string(const std::string& yaml_str) {
//     try {
//         config_ = YAML::Load(yaml_str);
//         return true;
//     } catch (const YAML::Exception& e) {
//         return false;
//     }
// }

// bool Config::save_to_file(const std::string& file_path) const {
//     try {
//         std::ofstream file(file_path);
//         if (!file.good()) return false;
        
//         file << config_;
//         return true;
//     } catch (const std::exception& e) {
//         return false;
//     }
// }

// std::string Config::to_yaml() const {
//     std::ostringstream oss;
//     oss << config_;
//     return oss.str();
// }

// // ============================================================================
// // ManagerConfig Implementation
// // ============================================================================

// bool ManagerConfig::load(const std::string& file_path) {
//     if (!load_from_file(file_path)) {
//         return false;
//     }
    
//     try {
//         // Server configuration
//         if (config_["server"]) {
//             auto server_node = config_["server"];
//             server.address = server_node["address"].as<std::string>(server.address);
//             server.port = server_node["port"].as<uint32_t>(server.port);
//             server.max_workers = server_node["max_workers"].as<uint32_t>(server.max_workers);
//             server.thread_pool_size = server_node["thread_pool_size"].as<uint32_t>(server.thread_pool_size);
//         }
        
//         // Worker configuration
//         if (config_["worker"]) {
//             auto worker_node = config_["worker"];
//             worker.heartbeat_interval_seconds = 
//                 worker_node["heartbeat_interval_seconds"].as<uint32_t>(worker.heartbeat_interval_seconds);
//             worker.heartbeat_timeout_seconds = 
//                 worker_node["heartbeat_timeout_seconds"].as<uint32_t>(worker.heartbeat_timeout_seconds);
//             worker.max_missed_heartbeats = 
//                 worker_node["max_missed_heartbeats"].as<uint32_t>(worker.max_missed_heartbeats);
//         }
        
//         // Storage configuration
//         if (config_["storage"]) {
//             auto storage_node = config_["storage"];
//             storage.models_directory = 
//                 storage_node["models_directory"].as<std::string>(storage.models_directory);
//             storage.database_path = 
//                 storage_node["database_path"].as<std::string>(storage.database_path);
//             storage.max_model_size_mb = 
//                 storage_node["max_model_size_mb"].as<uint64_t>(storage.max_model_size_mb);
//         }
        
//         // Security configuration
//         if (config_["security"]) {
//             auto security_node = config_["security"];
//             security.enable_tls = security_node["enable_tls"].as<bool>(security.enable_tls);
//             security.tls_cert_path = 
//                 security_node["tls_cert_path"].as<std::string>(security.tls_cert_path);
//             security.tls_key_path = 
//                 security_node["tls_key_path"].as<std::string>(security.tls_key_path);
//             security.admin_api_key = 
//                 security_node["admin_api_key"].as<std::string>(security.admin_api_key);
//         }
        
//         // Logging configuration
//         if (config_["logging"]) {
//             auto logging_node = config_["logging"];
//             logging.log_file = logging_node["log_file"].as<std::string>(logging.log_file);
//             logging.log_level = logging_node["log_level"].as<std::string>(logging.log_level);
//             logging.max_log_size_mb = 
//                 logging_node["max_log_size_mb"].as<uint64_t>(logging.max_log_size_mb);
//             logging.max_log_files = 
//                 logging_node["max_log_files"].as<uint32_t>(logging.max_log_files);
//         }
        
//         return validate();
        
//     } catch (const YAML::Exception& e) {
//         return false;
//     }
// }

// bool ManagerConfig::validate() const {
//     // Validate port range
//     if (server.port < 1024 || server.port > 65535) {
//         return false;
//     }
    
//     // Validate thread pool size
//     if (server.thread_pool_size == 0 || server.thread_pool_size > 1000) {
//         return false;
//     }
    
//     // Validate heartbeat configuration
//     if (worker.heartbeat_interval_seconds == 0 ||
//         worker.heartbeat_timeout_seconds < worker.heartbeat_interval_seconds) {
//         return false;
//     }
    
//     // Validate TLS configuration
//     if (security.enable_tls) {
//         if (security.tls_cert_path.empty() || security.tls_key_path.empty()) {
//             return false;
//         }
//     }
    
//     return true;
// }

// // ============================================================================
// // WorkerConfig Implementation
// // ============================================================================

// bool WorkerConfig::load(const std::string& file_path) {
//     if (!load_from_file(file_path)) {
//         return false;
//     }
    
//     try {
//         // Server configuration
//         if (config_["server"]) {
//             auto server_node = config_["server"];
//             server.address = server_node["address"].as<std::string>(server.address);
//             server.port = server_node["port"].as<uint32_t>(server.port);
//             server.thread_pool_size = 
//                 server_node["thread_pool_size"].as<uint32_t>(server.thread_pool_size);
//             server.max_concurrent_requests = 
//                 server_node["max_concurrent_requests"].as<uint32_t>(server.max_concurrent_requests);
//         }
        
//         // Manager configuration
//         if (config_["manager"]) {
//             auto manager_node = config_["manager"];
//             manager.address = manager_node["address"].as<std::string>(manager.address);
//             manager.port = manager_node["port"].as<uint32_t>(manager.port);
//             manager.heartbeat_interval_seconds = 
//                 manager_node["heartbeat_interval_seconds"].as<uint32_t>(manager.heartbeat_interval_seconds);
//             manager.reconnect_interval_seconds = 
//                 manager_node["reconnect_interval_seconds"].as<uint32_t>(manager.reconnect_interval_seconds);
//         }
        
//         // Inference configuration
//         if (config_["inference"]) {
//             auto inference_node = config_["inference"];
//             inference.enable_gpu = inference_node["enable_gpu"].as<bool>(inference.enable_gpu);
//             inference.gpu_device_id = 
//                 inference_node["gpu_device_id"].as<uint32_t>(inference.gpu_device_id);
//             inference.num_cpu_threads = 
//                 inference_node["num_cpu_threads"].as<uint32_t>(inference.num_cpu_threads);
//             inference.model_unload_timeout_seconds = 
//                 inference_node["model_unload_timeout_seconds"].as<uint32_t>(inference.model_unload_timeout_seconds);
//             inference.max_memory_usage_mb = 
//                 inference_node["max_memory_usage_mb"].as<uint64_t>(inference.max_memory_usage_mb);
//         }
        
//         // Storage configuration
//         if (config_["storage"]) {
//             auto storage_node = config_["storage"];
//             storage.models_cache_directory = 
//                 storage_node["models_cache_directory"].as<std::string>(storage.models_cache_directory);
//             storage.max_cache_size_mb = 
//                 storage_node["max_cache_size_mb"].as<uint64_t>(storage.max_cache_size_mb);
//         }
        
//         // Security configuration
//         if (config_["security"]) {
//             auto security_node = config_["security"];
//             security.enable_tls = security_node["enable_tls"].as<bool>(security.enable_tls);
//             security.tls_cert_path = 
//                 security_node["tls_cert_path"].as<std::string>(security.tls_cert_path);
//         }
        
//         // Logging configuration
//         if (config_["logging"]) {
//             auto logging_node = config_["logging"];
//             logging.log_file = logging_node["log_file"].as<std::string>(logging.log_file);
//             logging.log_level = logging_node["log_level"].as<std::string>(logging.log_level);
//             logging.max_log_size_mb = 
//                 logging_node["max_log_size_mb"].as<uint64_t>(logging.max_log_size_mb);
//             logging.max_log_files = 
//                 logging_node["max_log_files"].as<uint32_t>(logging.max_log_files);
//         }
        
//         // Worker ID (optional, auto-generate if empty)
//         if (config_["worker_id"]) {
//             worker_id = config_["worker_id"].as<std::string>(worker_id);
//         }
//         if (worker_id.empty()) {
//             worker_id = "worker-" + generate_uuid().substr(0, 8);
//         }
        
//         return validate();
        
//     } catch (const YAML::Exception& e) {
//         return false;
//     }
// }

// bool WorkerConfig::validate() const {
//     // Validate port range
//     if (server.port < 1024 || server.port > 65535) {
//         return false;
//     }
    
//     // Validate thread pool size
//     if (server.thread_pool_size == 0 || server.thread_pool_size > 1000) {
//         return false;
//     }
    
//     // Validate manager address
//     if (manager.address.empty() || manager.port == 0) {
//         return false;
//     }
    
//     // Validate GPU configuration
//     if (inference.enable_gpu && inference.gpu_device_id > 16) {
//         return false;
//     }
    
//     // Validate TLS configuration
//     if (security.enable_tls && security.tls_cert_path.empty()) {
//         return false;
//     }
    
//     return true;
// }

// // ============================================================================
// // ApiGatewayConfig Implementation
// // ============================================================================

// bool ApiGatewayConfig::load(const std::string& file_path) {
//     if (!load_from_file(file_path)) {
//         return false;
//     }
    
//     try {
//         // Server configuration
//         if (config_["server"]) {
//             auto server_node = config_["server"];
//             server.host = server_node["host"].as<std::string>(server.host);
//             server.port = server_node["port"].as<uint32_t>(server.port);
//             server.workers = server_node["workers"].as<uint32_t>(server.workers);
//         }
        
//         // Manager configuration
//         if (config_["manager"]) {
//             auto manager_node = config_["manager"];
//             manager.address = manager_node["address"].as<std::string>(manager.address);
//             manager.port = manager_node["port"].as<uint32_t>(manager.port);
//         }
        
//         // Security configuration
//         if (config_["security"]) {
//             auto security_node = config_["security"];
//             security.enable_cors = security_node["enable_cors"].as<bool>(security.enable_cors);
            
//             if (security_node["cors_origins"]) {
//                 security.cors_origins.clear();
//                 for (const auto& origin : security_node["cors_origins"]) {
//                     security.cors_origins.push_back(origin.as<std::string>());
//                 }
//             }
            
//             security.jwt_secret = 
//                 security_node["jwt_secret"].as<std::string>(security.jwt_secret);
//         }
        
//         return validate();
        
//     } catch (const YAML::Exception& e) {
//         return false;
//     }
// }

// bool ApiGatewayConfig::validate() const {
//     // Validate port range
//     if (server.port < 1024 || server.port > 65535) {
//         return false;
//     }
    
//     // Validate workers
//     if (server.workers == 0 || server.workers > 100) {
//         return false;
//     }
    
//     // Validate manager address
//     if (manager.address.empty() || manager.port == 0) {
//         return false;
//     }
    
//     return true;
// }

// } // namespace common
// } // namespace mlinference