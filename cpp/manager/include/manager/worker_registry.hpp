// #ifndef ML_INFERENCE_WORKER_REGISTRY_HPP
// #define ML_INFERENCE_WORKER_REGISTRY_HPP

// #include <map>
// #include <string>
// #include <mutex>
// #include <memory>
// #include "common/types.hpp"
// #include "common.pb.h"

// namespace mlinference {
// namespace manager {

// struct RegisteredWorker {
//     std::string worker_id;
//     std::string address;
//     uint32_t port;
//     common::WorkerStatus status;
//     mlinference::common::WorkerCapabilities capabilities;
//     mlinference::common::WorkerMetrics metrics;
//     common::TimePoint last_heartbeat;
//     common::TimePoint registered_at;
//     uint32_t missed_heartbeats;
//     std::vector<std::string> loaded_models;
// };

// class WorkerRegistry {
// public:
//     WorkerRegistry(uint32_t heartbeat_timeout_seconds,
//                    uint32_t max_missed_heartbeats);
//     ~WorkerRegistry() = default;
    
//     // Worker management
//     bool register_worker(const std::string& worker_id,
//                         const std::string& address,
//                         uint32_t port,
//                         const mlinference::common::WorkerCapabilities& capabilities);
    
//     bool unregister_worker(const std::string& worker_id);
    
//     bool update_heartbeat(const std::string& worker_id,
//                          const mlinference::common::WorkerMetrics& metrics,
//                          const std::vector<std::string>& loaded_models);
    
//     // Worker queries
//     std::optional<RegisteredWorker> get_worker(const std::string& worker_id) const;
    
//     std::vector<RegisteredWorker> list_workers(
//         std::optional<common::WorkerStatus> status_filter = std::nullopt) const;
    
//     std::vector<std::string> get_workers_for_model(const std::string& model_id) const;
    
//     // Health monitoring
//     void check_worker_health();
    
//     size_t count_workers(std::optional<common::WorkerStatus> status_filter = std::nullopt) const;
    
// private:
//     mutable std::mutex mutex_;
//     std::map<std::string, RegisteredWorker> workers_;
    
//     uint32_t heartbeat_timeout_seconds_;
//     uint32_t max_missed_heartbeats_;
    
//     // Helper to update worker status
//     void update_worker_status(RegisteredWorker& worker);
// };

// } // namespace manager
// } // namespace mlinference

// #endif // ML_INFERENCE_WORKER_REGISTRY_HPP