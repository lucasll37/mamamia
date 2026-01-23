#ifndef ML_INFERENCE_INFERENCE_ENGINE_HPP
#define ML_INFERENCE_INFERENCE_ENGINE_HPP

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <onnxruntime_cxx_api.h>
#include "common.pb.h"

namespace mlinference {
namespace worker {

// Inference result structure
struct InferenceResult {
    bool success;
    std::map<std::string, std::vector<float>> outputs;
    double inference_time_ms;
    std::string error_message;
};

// Model metadata structure
struct ModelData {
    std::string model_id;
    std::string path;
    std::vector<std::string> input_names;
    std::vector<std::string> output_names;
    std::vector<std::vector<int64_t>> input_shapes;
    std::vector<std::vector<int64_t>> output_shapes;
    std::chrono::steady_clock::time_point load_time;
    std::chrono::steady_clock::time_point last_used_time;
    uint64_t inference_count;
    double total_inference_time_ms;
    
    // ONNX Runtime session
    std::unique_ptr<Ort::Session> session;
};

// ONNX Runtime Inference Engine
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
        int64_t seconds_since_last_use;
    };
    
    ModelMetrics get_model_metrics(const std::string& model_id) const;
    
    // Automatic model unloading after idle timeout (in seconds)
    void enable_auto_unload(uint32_t idle_timeout_seconds);
    void disable_auto_unload();
    
    // Get inference engine info
    struct EngineInfo {
        bool gpu_enabled;
        uint32_t num_threads;
        std::string onnx_runtime_version;
        std::vector<std::string> available_providers;
    };
    
    EngineInfo get_engine_info() const;

private:
    // ONNX Runtime environment and configuration
    Ort::Env env_;
    Ort::SessionOptions session_options_;
    
    bool enable_gpu_;
    uint32_t gpu_device_id_;
    uint32_t num_threads_;
    
    mutable std::mutex mutex_;
    
    // Model storage
    std::map<std::string, std::shared_ptr<ModelData>> loaded_models_;
    
    // Auto-unload configuration
    bool auto_unload_enabled_;
    uint32_t idle_timeout_seconds_;
    
    // Helper methods
    std::vector<int64_t> get_tensor_shape(const Ort::Session& session, 
                                          size_t index, 
                                          bool is_input) const;
    
    std::string get_tensor_name(const Ort::Session& session,
                               size_t index,
                               bool is_input) const;
    
    void extract_model_metadata(ModelData& model_data);
    
    void check_auto_unload();
};

} // namespace worker
} // namespace mlinference

#endif // ML_INFERENCE_INFERENCE_ENGINE_HPP