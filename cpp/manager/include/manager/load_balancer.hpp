// #ifndef ML_INFERENCE_LOAD_BALANCER_HPP
// #define ML_INFERENCE_LOAD_BALANCER_HPP

// #include <string>
// #include <map>
// #include <optional>
// #include <memory>
// #include "worker_registry.hpp"

// namespace mlinference {
// namespace manager {

// enum class LoadBalancingStrategy {
//     ROUND_ROBIN,
//     LEAST_LOADED,
//     MODEL_AFFINITY  // Prefer workers with model already loaded
// };

// class LoadBalancer {
// public:
//     explicit LoadBalancer(std::shared_ptr<WorkerRegistry> worker_registry,
//                          LoadBalancingStrategy strategy = LoadBalancingStrategy::MODEL_AFFINITY);
//     ~LoadBalancer() = default;
    
//     // Select worker for inference
//     std::optional<std::string> select_worker(
//         const std::string& model_id,
//         bool require_gpu = false);
    
//     // Set load balancing strategy
//     void set_strategy(LoadBalancingStrategy strategy);
    
// private:
//     std::shared_ptr<WorkerRegistry> worker_registry_;
//     LoadBalancingStrategy strategy_;
    
//     // Round-robin state
//     mutable std::map<std::string, size_t> round_robin_index_;
    
//     // Strategy implementations
//     std::optional<std::string> select_round_robin(
//         const std::vector<RegisteredWorker>& candidates);
    
//     std::optional<std::string> select_least_loaded(
//         const std::vector<RegisteredWorker>& candidates);
    
//     std::optional<std::string> select_model_affinity(
//         const std::string& model_id,
//         const std::vector<RegisteredWorker>& candidates);
    
//     // Helper to filter candidates
//     std::vector<RegisteredWorker> get_candidate_workers(
//         bool require_gpu = false) const;
    
//     // Calculate worker load score (lower is better)
//     double calculate_load_score(const RegisteredWorker& worker) const;
// };

// } // namespace manager
// } // namespace mlinference

// #endif // ML_INFERENCE_LOAD_BALANCER_HPP