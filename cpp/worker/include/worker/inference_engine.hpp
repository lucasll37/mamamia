#ifndef ML_INFERENCE_INFERENCE_ENGINE_HPP
#define ML_INFERENCE_INFERENCE_ENGINE_HPP

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <mutex>
#include "common.pb.h"

namespace mlinference {
namespace worker {

struct InferenceResult {
    bool success;
    std::map<std::string, std::vector<float>> outputs;
    double inference_time_ms;
    std::string error_message;
};

struct ModelData {
    std::string model_id;
    std::string path;
    std::vector<std::string> input_names;
    std::vector<std::string> output_names;
    std::chrono::steady_clock::time_point load_time;
    uint64_t inference_count;
    double total_inference_time_ms;
};

// Simplified Inference Engine
// In production, this would use ONNX Runtime or other ML framework
class InferenceEngine {
public:
    explicit InferenceEngine(bool enable_gpu = false, 
                            uint32_t gpu_device_id = 0,
                            uint32_t num_threads = 4);
    ~InferenceEngine();
    
    // Model management
    bool load_model(const std::string& model_id,
                   const std::string& model_path);
    
    bool unload_model(const std::string& model_id);
    
    bool is_model_loaded(const std::string& model_id) const;
    
    // Inference
    InferenceResult predict(const std::string& model_id,
                           const std::map<std::string, std::vector<float>>& inputs);
    
    // Get loaded models
    std::vector<std::string> get_loaded_models() const;
    
    // Get model info
    std::shared_ptr<ModelData> get_model_info(const std::string& model_id) const;
    
    // Get model metrics
    struct ModelMetrics {
        uint64_t inference_count;
        double total_inference_time_ms;
        double avg_inference_time_ms;
    };
    
    ModelMetrics get_model_metrics(const std::string& model_id) const;

private:
    bool enable_gpu_;
    uint32_t gpu_device_id_;
    uint32_t num_threads_;
    
    mutable std::mutex mutex_;
    
    // Model storage
    std::map<std::string, std::shared_ptr<ModelData>> loaded_models_;
    
    // Simulate inference (in production, use real ML framework)
    InferenceResult simulate_inference(
        const std::string& model_id,
        const std::map<std::string, std::vector<float>>& inputs);
};

} // namespace worker
} // namespace mlinference

#endif // ML_INFERENCE_INFERENCE_ENGINE_HPP