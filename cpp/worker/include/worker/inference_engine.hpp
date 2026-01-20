// #ifndef ML_INFERENCE_INFERENCE_ENGINE_HPP
// #define ML_INFERENCE_INFERENCE_ENGINE_HPP

// #include <string>
// #include <memory>
// #include <map>
// #include <mutex>
// #include "common/types.hpp"
// #include "common.pb.h"

// namespace mlinference {
// namespace worker {

// struct InferenceResult {
//     bool success;
//     std::map<std::string, std::vector<float>> outputs;
//     double inference_time_ms;
//     std::string error_message;
// };

// // Simplified ONNX Runtime wrapper
// class InferenceEngine {
// public:
//     explicit InferenceEngine(bool enable_gpu = false, 
//                             uint32_t gpu_device_id = 0,
//                             uint32_t num_threads = 4);
//     ~InferenceEngine();
    
//     // Model management
//     bool load_model(const std::string& model_id,
//                    const std::string& model_path);
    
//     bool unload_model(const std::string& model_id);
    
//     bool is_model_loaded(const std::string& model_id) const;
    
//     // Inference
//     InferenceResult predict(const std::string& model_id,
//                            const std::map<std::string, std::vector<float>>& inputs);
    
//     // Get loaded models
//     std::vector<std::string> get_loaded_models() const;
    
// private:
//     bool enable_gpu_;
//     uint32_t gpu_device_id_;
//     uint32_t num_threads_;
    
//     mutable std::mutex mutex_;
    
//     // In real implementation, would store ONNX Runtime sessions
//     std::map<std::string, std::string> loaded_models_;  // model_id -> path
    
//     // Metrics
//     std::map<std::string, uint64_t> inference_counts_;
//     std::map<std::string, double> total_inference_time_;
// };

// } // namespace worker
// } // namespace mlinference

// #endif // ML_INFERENCE_INFERENCE_ENGINE_HPP