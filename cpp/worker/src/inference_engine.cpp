#include "worker/inference_engine.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace mlinference {
namespace worker {

InferenceEngine::InferenceEngine(bool enable_gpu, uint32_t gpu_device_id, uint32_t num_threads)
    : env_(ORT_LOGGING_LEVEL_WARNING, "MLInference")
    , enable_gpu_(enable_gpu)
    , gpu_device_id_(gpu_device_id)
    , num_threads_(num_threads)
    , auto_unload_enabled_(false)
    , idle_timeout_seconds_(300) { // 5 minutes default
    
    std::cout << "InferenceEngine initialized with ONNX Runtime" << std::endl;
    std::cout << "  ONNX Runtime version: " << ORT_API_VERSION << std::endl;
    std::cout << "  GPU enabled: " << (enable_gpu_ ? "yes" : "no") << std::endl;
    if (enable_gpu_) {
        std::cout << "  GPU device ID: " << gpu_device_id_ << std::endl;
    }
    std::cout << "  Number of threads: " << num_threads_ << std::endl;
    
    // Configure session options
    session_options_.SetIntraOpNumThreads(num_threads_);
    session_options_.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    
    // Configure execution provider
    if (enable_gpu_) {
#ifdef USE_CUDA
        try {
            OrtCUDAProviderOptions cuda_options;
            cuda_options.device_id = gpu_device_id_;
            session_options_.AppendExecutionProvider_CUDA(cuda_options);
            std::cout << "  CUDA execution provider enabled" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "  Warning: Failed to enable CUDA: " << e.what() << std::endl;
            std::cerr << "  Falling back to CPU execution" << std::endl;
        }
#else
        std::cerr << "  Warning: GPU requested but not compiled with CUDA support" << std::endl;
        std::cerr << "  Falling back to CPU execution" << std::endl;
#endif
    }
    
    std::cout << "InferenceEngine ready" << std::endl;
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
    
    std::cout << "Loading model: " << model_id << " from " << model_path << std::endl;
    
    try {
        auto model_data = std::make_shared<ModelData>();
        model_data->model_id = model_id;
        model_data->path = model_path;
        model_data->load_time = std::chrono::steady_clock::now();
        model_data->last_used_time = model_data->load_time;
        model_data->inference_count = 0;
        model_data->total_inference_time_ms = 0.0;
        
        // Create ONNX Runtime session
#ifdef _WIN32
        std::wstring wide_path(model_path.begin(), model_path.end());
        model_data->session = std::make_unique<Ort::Session>(env_, wide_path.c_str(), session_options_);
#else
        model_data->session = std::make_unique<Ort::Session>(env_, model_path.c_str(), session_options_);
#endif
        
        // Extract model metadata
        extract_model_metadata(*model_data);
        
        // Store model
        loaded_models_[model_id] = model_data;
        
        std::cout << "Model loaded successfully: " << model_id << std::endl;
        std::cout << "  Inputs: " << model_data->input_names.size() << std::endl;
        for (size_t i = 0; i < model_data->input_names.size(); ++i) {
            std::cout << "    - " << model_data->input_names[i] << " [";
            for (size_t j = 0; j < model_data->input_shapes[i].size(); ++j) {
                if (j > 0) std::cout << ", ";
                std::cout << model_data->input_shapes[i][j];
            }
            std::cout << "]" << std::endl;
        }
        std::cout << "  Outputs: " << model_data->output_names.size() << std::endl;
        for (size_t i = 0; i < model_data->output_names.size(); ++i) {
            std::cout << "    - " << model_data->output_names[i] << " [";
            for (size_t j = 0; j < model_data->output_shapes[i].size(); ++j) {
                if (j > 0) std::cout << ", ";
                std::cout << model_data->output_shapes[i][j];
            }
            std::cout << "]" << std::endl;
        }
        
        return true;
        
    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime error loading model: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Error loading model: " << e.what() << std::endl;
        return false;
    }
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
    
    std::shared_ptr<ModelData> model_data;
    
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
        
        model_data = it->second;
        model_data->last_used_time = std::chrono::steady_clock::now();
    }
    
    try {
        // Prepare memory info
        Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(
            OrtArenaAllocator, OrtMemTypeDefault);
        
        // Prepare input tensors
        std::vector<Ort::Value> input_tensors;
        std::vector<const char*> input_names_cstr;
        
        for (size_t i = 0; i < model_data->input_names.size(); ++i) {
            const auto& input_name = model_data->input_names[i];
            
            auto it = inputs.find(input_name);
            if (it == inputs.end()) {
                return InferenceResult{
                    false,
                    {},
                    0.0,
                    "Missing input: " + input_name
                };
            }
            
            const auto& input_data = it->second;
            const auto& input_shape = model_data->input_shapes[i];
            
            // Validate input size
            int64_t expected_size = std::accumulate(
                input_shape.begin(), input_shape.end(), 
                1LL, std::multiplies<int64_t>());
            
            if (static_cast<int64_t>(input_data.size()) != expected_size) {
                return InferenceResult{
                    false,
                    {},
                    0.0,
                    "Input size mismatch for " + input_name + 
                    ": expected " + std::to_string(expected_size) + 
                    ", got " + std::to_string(input_data.size())
                };
            }
            
            // Create input tensor (const_cast is safe because ONNX Runtime won't modify it)
            input_tensors.push_back(Ort::Value::CreateTensor<float>(
                memory_info,
                const_cast<float*>(input_data.data()),
                input_data.size(),
                input_shape.data(),
                input_shape.size()
            ));
            
            input_names_cstr.push_back(input_name.c_str());
        }
        
        // Prepare output names
        std::vector<const char*> output_names_cstr;
        for (const auto& name : model_data->output_names) {
            output_names_cstr.push_back(name.c_str());
        }
        
        // Run inference
        auto output_tensors = model_data->session->Run(
            Ort::RunOptions{nullptr},
            input_names_cstr.data(),
            input_tensors.data(),
            input_tensors.size(),
            output_names_cstr.data(),
            output_names_cstr.size()
        );
        
        // Extract outputs
        std::map<std::string, std::vector<float>> outputs;
        
        for (size_t i = 0; i < output_tensors.size(); ++i) {
            auto& tensor = output_tensors[i];
            
            // Get tensor data
            float* float_data = tensor.GetTensorMutableData<float>();
            
            // Get tensor shape info
            auto type_info = tensor.GetTensorTypeAndShapeInfo();
            size_t element_count = type_info.GetElementCount();
            
            // Copy data
            std::vector<float> output_data(float_data, float_data + element_count);
            outputs[model_data->output_names[i]] = std::move(output_data);
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);
        double inference_time_ms = duration.count() / 1000.0;
        
        // Update metrics
        {
            std::lock_guard<std::mutex> lock(mutex_);
            model_data->inference_count++;
            model_data->total_inference_time_ms += inference_time_ms;
        }
        
        return InferenceResult{
            true,
            outputs,
            inference_time_ms,
            ""
        };
        
    } catch (const Ort::Exception& e) {
        return InferenceResult{
            false,
            {},
            0.0,
            std::string("ONNX Runtime error: ") + e.what()
        };
    } catch (const std::exception& e) {
        return InferenceResult{
            false,
            {},
            0.0,
            std::string("Error: ") + e.what()
        };
    }
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
        
        auto now = std::chrono::steady_clock::now();
        auto time_since_use = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second->last_used_time);
        
        return ModelMetrics{
            it->second->inference_count,
            it->second->total_inference_time_ms,
            avg_time,
            time_since_use.count()
        };
    }
    
    return ModelMetrics{0, 0.0, 0.0, 0};
}

void InferenceEngine::enable_auto_unload(uint32_t idle_timeout_seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto_unload_enabled_ = true;
    idle_timeout_seconds_ = idle_timeout_seconds;
    std::cout << "Auto-unload enabled: " << idle_timeout_seconds << " seconds idle timeout" << std::endl;
}

void InferenceEngine::disable_auto_unload() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto_unload_enabled_ = false;
    std::cout << "Auto-unload disabled" << std::endl;
}

InferenceEngine::EngineInfo InferenceEngine::get_engine_info() const {
    EngineInfo info;
    info.gpu_enabled = enable_gpu_;
    info.num_threads = num_threads_;
    info.onnx_runtime_version = std::to_string(ORT_API_VERSION);
    
    // Get available providers
    std::vector<std::string> providers = Ort::GetAvailableProviders();
    info.available_providers = providers;
    
    return info;
}

void InferenceEngine::extract_model_metadata(ModelData& model_data) {
    Ort::AllocatorWithDefaultOptions allocator;
    
    // Extract input information
    size_t num_inputs = model_data.session->GetInputCount();
    for (size_t i = 0; i < num_inputs; ++i) {
        // Get input name
        auto input_name_ptr = model_data.session->GetInputNameAllocated(i, allocator);
        model_data.input_names.push_back(input_name_ptr.get());
        
        // Get input shape
        auto type_info = model_data.session->GetInputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        model_data.input_shapes.push_back(tensor_info.GetShape());
    }
    
    // Extract output information
    size_t num_outputs = model_data.session->GetOutputCount();
    for (size_t i = 0; i < num_outputs; ++i) {
        // Get output name
        auto output_name_ptr = model_data.session->GetOutputNameAllocated(i, allocator);
        model_data.output_names.push_back(output_name_ptr.get());
        
        // Get output shape
        auto type_info = model_data.session->GetOutputTypeInfo(i);
        auto tensor_info = type_info.GetTensorTypeAndShapeInfo();
        model_data.output_shapes.push_back(tensor_info.GetShape());
    }
}

void InferenceEngine::check_auto_unload() {
    if (!auto_unload_enabled_) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    std::vector<std::string> models_to_unload;
    
    for (const auto& pair : loaded_models_) {
        auto idle_time = std::chrono::duration_cast<std::chrono::seconds>(
            now - pair.second->last_used_time);
        
        if (idle_time.count() > static_cast<int64_t>(idle_timeout_seconds_)) {
            models_to_unload.push_back(pair.first);
        }
    }
    
    for (const auto& model_id : models_to_unload) {
        std::cout << "Auto-unloading idle model: " << model_id << std::endl;
        loaded_models_.erase(model_id);
    }
}

} // namespace worker
} // namespace mlinference