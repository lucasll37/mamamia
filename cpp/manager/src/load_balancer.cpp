// #include "manager/load_balancer.hpp"
// #include "common/logger.hpp"
// #include <algorithm>
// #include <limits>

// namespace mlinference {
// namespace manager {

// LoadBalancer::LoadBalancer(std::shared_ptr<WorkerRegistry> worker_registry,
//                            LoadBalancingStrategy strategy)
//     : worker_registry_(worker_registry)
//     , strategy_(strategy) {
// }

// std::optional<std::string> LoadBalancer::select_worker(
//     const std::string& model_id,
//     bool require_gpu) {
    
//     auto candidates = get_candidate_workers(require_gpu);
    
//     if (candidates.empty()) {
//         LOG_WARN("No available workers found (require_gpu={})", require_gpu);
//         return std::nullopt;
//     }
    
//     std::optional<std::string> selected;
    
//     switch (strategy_) {
//         case LoadBalancingStrategy::ROUND_ROBIN:
//             selected = select_round_robin(candidates);
//             break;
            
//         case LoadBalancingStrategy::LEAST_LOADED:
//             selected = select_least_loaded(candidates);
//             break;
            
//         case LoadBalancingStrategy::MODEL_AFFINITY:
//             selected = select_model_affinity(model_id, candidates);
//             break;
//     }
    
//     if (selected) {
//         LOG_DEBUG("Selected worker {} for model {} (strategy={})", 
//                  *selected, model_id, static_cast<int>(strategy_));
//     }
    
//     return selected;
// }

// void LoadBalancer::set_strategy(LoadBalancingStrategy strategy) {
//     strategy_ = strategy;
//     LOG_INFO("Load balancing strategy changed to {}", static_cast<int>(strategy));
// }

// std::optional<std::string> LoadBalancer::select_round_robin(
//     const std::vector<RegisteredWorker>& candidates) {
    
//     if (candidates.empty()) {
//         return std::nullopt;
//     }
    
//     // Use a simple round-robin per model
//     static size_t global_index = 0;
//     size_t index = global_index++ % candidates.size();
    
//     return candidates[index].worker_id;
// }

// std::optional<std::string> LoadBalancer::select_least_loaded(
//     const std::vector<RegisteredWorker>& candidates) {
    
//     if (candidates.empty()) {
//         return std::nullopt;
//     }
    
//     // Find worker with lowest load score
//     auto min_it = std::min_element(candidates.begin(), candidates.end(),
//         [this](const RegisteredWorker& a, const RegisteredWorker& b) {
//             return calculate_load_score(a) < calculate_load_score(b);
//         });
    
//     return min_it->worker_id;
// }

// std::optional<std::string> LoadBalancer::select_model_affinity(
//     const std::string& model_id,
//     const std::vector<RegisteredWorker>& candidates) {
    
//     if (candidates.empty()) {
//         return std::nullopt;
//     }
    
//     // First, try to find workers that already have the model loaded
//     std::vector<RegisteredWorker> with_model;
//     std::vector<RegisteredWorker> without_model;
    
//     for (const auto& worker : candidates) {
//         auto it = std::find(worker.loaded_models.begin(),
//                            worker.loaded_models.end(),
//                            model_id);
        
//         if (it != worker.loaded_models.end()) {
//             with_model.push_back(worker);
//         } else {
//             without_model.push_back(worker);
//         }
//     }
    
//     // If we have workers with the model loaded, use least-loaded among them
//     if (!with_model.empty()) {
//         auto min_it = std::min_element(with_model.begin(), with_model.end(),
//             [this](const RegisteredWorker& a, const RegisteredWorker& b) {
//                 return calculate_load_score(a) < calculate_load_score(b);
//             });
//         return min_it->worker_id;
//     }
    
//     // Otherwise, use least-loaded among all workers
//     return select_least_loaded(without_model);
// }

// std::vector<RegisteredWorker> LoadBalancer::get_candidate_workers(bool require_gpu) const {
//     auto all_workers = worker_registry_->list_workers();
    
//     std::vector<RegisteredWorker> candidates;
    
//     for (const auto& worker : all_workers) {
//         // Skip offline or overloaded workers
//         if (worker.status == common::WorkerStatus::OFFLINE ||
//             worker.status == common::WorkerStatus::OVERLOADED) {
//             continue;
//         }
        
//         // Filter by GPU requirement
//         if (require_gpu && !worker.capabilities.has_gpu()) {
//             continue;
//         }
        
//         candidates.push_back(worker);
//     }
    
//     return candidates;
// }

// double LoadBalancer::calculate_load_score(const RegisteredWorker& worker) const {
//     // Composite score based on multiple factors
//     // Lower score = better choice
    
//     double score = 0.0;
    
//     // Factor 1: Active requests (0-1 scale)
//     uint32_t active = worker.metrics.active_requests();
//     uint32_t max_concurrent = worker.capabilities.max_concurrent_requests();
//     if (max_concurrent == 0) max_concurrent = 10;
    
//     double request_load = static_cast<double>(active) / max_concurrent;
//     score += request_load * 40.0;  // Weight: 40%
    
//     // Factor 2: CPU usage (0-1 scale)
//     double cpu_load = worker.metrics.cpu_usage_percent() / 100.0;
//     score += cpu_load * 30.0;  // Weight: 30%
    
//     // Factor 3: Memory usage (0-1 scale)
//     double mem_load = worker.metrics.memory_usage_percent() / 100.0;
//     score += mem_load * 20.0;  // Weight: 20%
    
//     // Factor 4: Queue length
//     double queue_load = static_cast<double>(worker.metrics.queued_requests()) / 10.0;
//     score += std::min(queue_load, 1.0) * 10.0;  // Weight: 10%
    
//     return score;
// }

// } // namespace manager
// } // namespace mlinference