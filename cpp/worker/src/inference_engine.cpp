#include "worker/inference_engine.hpp"
#include <iostream>
#include <random>
#include <cmath>
#include <thread>
#include <cstdlib>


namespace mlinference {
namespace worker {

InferenceEngine::InferenceEngine(bool enable_gpu, uint32_t gpu_device_id, uint32_t num_threads)
    : enable_gpu_(enable_gpu)
    , gpu_device_id_(gpu_device_id)
    , num_threads_(num_threads) {
    
    std::cout << "InferenceEngine initialized" << std::endl;
    std::cout << "  GPU enabled: " << (enable_gpu_ ? "yes" : "no") << std::endl;
    if (enable_gpu_) {
        std::cout << "  GPU device ID: " << gpu_device_id_ << std::endl;
    }
    std::cout << "  Number of threads: " << num_threads_ << std::endl;
}

InferenceEngine::~InferenceEngine() {
    std::lock_guard<std::mutex> lock(mutex_);
    loaded_models_.clear();
    std::cout << "InferenceEngine destroyed" << std::endl;
}

bool InferenceEngine::load_model(const std::string& model_id, const std::string& model_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (loaded_models_.find(model_id) != loaded_models_.end()) {
        std::cerr << "Model " << model_id << " is already loaded" << std::endl;
        return false;
    }
    
    // In production, load actual model here using ONNX Runtime or similar
    auto model_data = std::make_shared<ModelData>();
    model_data->model_id = model_id;
    model_data->path = model_path;
    model_data->input_names = {"input"};  // Simplified
    model_data->output_names = {"output"};  // Simplified
    model_data->load_time = std::chrono::steady_clock::now();
    model_data->inference_count = 0;
    model_data->total_inference_time_ms = 0.0;
    
    loaded_models_[model_id] = model_data;
    
    std::cout << "Model loaded: " << model_id << " from " << model_path << std::endl;
    return true;
}

bool InferenceEngine::unload_model(const std::string& model_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = loaded_models_.find(model_id);
    if (it == loaded_models_.end()) {
        std::cerr << "Model " << model_id << " is not loaded" << std::endl;
        return false;
    }
    
    loaded_models_.erase(it);
    std::cout << "Model unloaded: " << model_id << std::endl;
    return true;
}

bool InferenceEngine::is_model_loaded(const std::string& model_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return loaded_models_.find(model_id) != loaded_models_.end();
}

InferenceResult InferenceEngine::predict(
    const std::string& model_id,
    const std::map<std::string, std::vector<float>>& inputs) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = loaded_models_.find(model_id);
        if (it == loaded_models_.end()) {
            return InferenceResult{
                false,
                {},
                0.0,
                "Model not loaded: " + model_id
            };
        }
    }
    
    // Simulate inference
    InferenceResult result = simulate_inference(model_id, inputs);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    result.inference_time_ms = duration.count() / 1000.0;
    
    // Update metrics
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = loaded_models_.find(model_id);
        if (it != loaded_models_.end()) {
            it->second->inference_count++;
            it->second->total_inference_time_ms += result.inference_time_ms;
        }
    }
    
    return result;
}

std::vector<std::string> InferenceEngine::get_loaded_models() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> model_ids;
    model_ids.reserve(loaded_models_.size());
    
    for (const auto& pair : loaded_models_) {
        model_ids.push_back(pair.first);
    }
    
    return model_ids;
}

std::shared_ptr<ModelData> InferenceEngine::get_model_info(const std::string& model_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = loaded_models_.find(model_id);
    if (it != loaded_models_.end()) {
        return it->second;
    }
    
    return nullptr;
}

InferenceEngine::ModelMetrics InferenceEngine::get_model_metrics(const std::string& model_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = loaded_models_.find(model_id);
    if (it != loaded_models_.end()) {
        double avg_time = 0.0;
        if (it->second->inference_count > 0) {
            avg_time = it->second->total_inference_time_ms / it->second->inference_count;
        }
        
        return ModelMetrics{
            it->second->inference_count,
            it->second->total_inference_time_ms,
            avg_time
        };
    }
    
    return ModelMetrics{0, 0.0, 0.0};
}

InferenceResult InferenceEngine::simulate_inference(
    const std::string& model_id,
    const std::map<std::string, std::vector<float>>& inputs) {
    
    // Simulate some processing time
    std::this_thread::sleep_for(std::chrono::milliseconds(10 + (rand() % 40)));
    
    // Generate dummy output
    std::map<std::string, std::vector<float>> outputs;
    
    // Simple transformation: multiply inputs by 2 and add random noise
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> noise(0.0f, 0.1f);
    
    for (const auto& input_pair : inputs) {
        std::vector<float> output;
        output.reserve(input_pair.second.size());
        
        for (float value : input_pair.second) {
            output.push_back(value * 2.0f + noise(gen));
        }
        
        outputs["output"] = output;
    }
    
    return InferenceResult{
        true,
        outputs,
        0.0,  // Will be set by caller
        ""
    };
}

} // namespace worker
} // namespace mlinference