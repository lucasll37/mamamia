// #include "manager/worker_registry.hpp"
// #include "common/logger.hpp"
// #include <chrono>
// #include <algorithm>

// namespace mlinference {
// namespace manager {

// WorkerRegistry::WorkerRegistry(uint32_t heartbeat_timeout_seconds,
//                                uint32_t max_missed_heartbeats)
//     : heartbeat_timeout_seconds_(heartbeat_timeout_seconds)
//     , max_missed_heartbeats_(max_missed_heartbeats) {
// }

// bool WorkerRegistry::register_worker(const std::string& worker_id,
//                                      const std::string& address,
//                                      uint32_t port,
//                                      const mlinference::common::WorkerCapabilities& capabilities) {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     // Check if worker already registered
//     if (workers_.find(worker_id) != workers_.end()) {
//         LOG_WARN("Worker {} already registered, updating registration", worker_id);
//         workers_.erase(worker_id);
//     }
    
//     RegisteredWorker worker;
//     worker.worker_id = worker_id;
//     worker.address = address;
//     worker.port = port;
//     worker.status = common::WorkerStatus::IDLE;
//     worker.capabilities = capabilities;
//     worker.last_heartbeat = common::now();
//     worker.registered_at = common::now();
//     worker.missed_heartbeats = 0;
    
//     workers_[worker_id] = worker;
    
//     LOG_INFO("Worker registered: id={}, address={}:{}, gpu={}", 
//              worker_id, address, port, capabilities.has_gpu());
    
//     return true;
// }

// bool WorkerRegistry::unregister_worker(const std::string& worker_id) {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     auto it = workers_.find(worker_id);
//     if (it == workers_.end()) {
//         LOG_WARN("Attempted to unregister unknown worker: {}", worker_id);
//         return false;
//     }
    
//     workers_.erase(it);
//     LOG_INFO("Worker unregistered: {}", worker_id);
    
//     return true;
// }

// bool WorkerRegistry::update_heartbeat(const std::string& worker_id,
//                                       const mlinference::common::WorkerMetrics& metrics,
//                                       const std::vector<std::string>& loaded_models) {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     auto it = workers_.find(worker_id);
//     if (it == workers_.end()) {
//         LOG_WARN("Heartbeat from unknown worker: {}", worker_id);
//         return false;
//     }
    
//     RegisteredWorker& worker = it->second;
//     worker.last_heartbeat = common::now();
//     worker.missed_heartbeats = 0;
//     worker.metrics = metrics;
//     worker.loaded_models = loaded_models;
    
//     // Update status based on metrics
//     update_worker_status(worker);
    
//     LOG_DEBUG("Heartbeat received from worker {}: cpu={:.1f}%, mem={:.1f}%, active_reqs={}", 
//               worker_id, 
//               metrics.cpu_usage_percent(),
//               metrics.memory_usage_percent(),
//               metrics.active_requests());
    
//     return true;
// }

// std::optional<RegisteredWorker> WorkerRegistry::get_worker(const std::string& worker_id) const {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     auto it = workers_.find(worker_id);
//     if (it == workers_.end()) {
//         return std::nullopt;
//     }
    
//     return it->second;
// }

// std::vector<RegisteredWorker> WorkerRegistry::list_workers(
//     std::optional<common::WorkerStatus> status_filter) const {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     std::vector<RegisteredWorker> result;
//     result.reserve(workers_.size());
    
//     for (const auto& [id, worker] : workers_) {
//         if (!status_filter || worker.status == *status_filter) {
//             result.push_back(worker);
//         }
//     }
    
//     return result;
// }

// std::vector<std::string> WorkerRegistry::get_workers_for_model(const std::string& model_id) const {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     std::vector<std::string> result;
    
//     for (const auto& [id, worker] : workers_) {
//         // Check if worker has this model loaded
//         auto it = std::find(worker.loaded_models.begin(), 
//                            worker.loaded_models.end(), 
//                            model_id);
        
//         if (it != worker.loaded_models.end() && 
//             (worker.status == common::WorkerStatus::IDLE || 
//              worker.status == common::WorkerStatus::BUSY)) {
//             result.push_back(id);
//         }
//     }
    
//     return result;
// }

// void WorkerRegistry::check_worker_health() {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     auto now = common::now();
//     auto timeout = std::chrono::seconds(heartbeat_timeout_seconds_);
    
//     std::vector<std::string> offline_workers;
    
//     for (auto& [id, worker] : workers_) {
//         auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
//             now - worker.last_heartbeat);
        
//         if (elapsed > timeout) {
//             worker.missed_heartbeats++;
            
//             if (worker.missed_heartbeats >= max_missed_heartbeats_) {
//                 if (worker.status != common::WorkerStatus::OFFLINE) {
//                     LOG_WARN("Worker {} marked as OFFLINE (missed {} heartbeats)",
//                             id, worker.missed_heartbeats);
//                     worker.status = common::WorkerStatus::OFFLINE;
//                     offline_workers.push_back(id);
//                 }
//             }
//         }
//     }
    
//     // Optionally auto-remove offline workers after extended period
//     // For now, just log
//     if (!offline_workers.empty()) {
//         LOG_INFO("Total offline workers: {}", offline_workers.size());
//     }
// }

// size_t WorkerRegistry::count_workers(std::optional<common::WorkerStatus> status_filter) const {
//     std::lock_guard<std::mutex> lock(mutex_);
    
//     if (!status_filter) {
//         return workers_.size();
//     }
    
//     return std::count_if(workers_.begin(), workers_.end(),
//         [status_filter](const auto& pair) {
//             return pair.second.status == *status_filter;
//         });
// }

// void WorkerRegistry::update_worker_status(RegisteredWorker& worker) {
//     // Determine status based on metrics
//     uint32_t active = worker.metrics.active_requests();
//     uint32_t max_concurrent = worker.capabilities.max_concurrent_requests();
    
//     if (max_concurrent == 0) max_concurrent = 10; // Default
    
//     double load = static_cast<double>(active) / max_concurrent;
    
//     if (load == 0) {
//         worker.status = common::WorkerStatus::IDLE;
//     } else if (load < 0.8) {
//         worker.status = common::WorkerStatus::BUSY;
//     } else {
//         worker.status = common::WorkerStatus::OVERLOADED;
//     }
// }

// } // namespace manager
// } // namespace mlinference