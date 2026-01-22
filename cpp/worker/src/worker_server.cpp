#include "worker/worker_server.hpp"
#include <iostream>
#include <thread>

namespace mlinference {
namespace worker {

WorkerServiceImpl::WorkerServiceImpl(
    const std::string& worker_id,
    bool enable_gpu,
    uint32_t num_threads)
    : worker_id_(worker_id)
    , inference_engine_(std::make_shared<InferenceEngine>(enable_gpu, 0, num_threads))
    , start_time_(std::chrono::steady_clock::now()) {
    
    std::cout << "WorkerService initialized: " << worker_id_ << std::endl;
}

WorkerServiceImpl::~WorkerServiceImpl() {
    std::cout << "WorkerService destroyed: " << worker_id_ << std::endl;
}

grpc::Status WorkerServiceImpl::Predict(
    grpc::ServerContext* context,
    const PredictRequest* request,
    PredictResponse* response) {
    
    active_requests_++;
    total_requests_++;
    
    std::cout << "Predict request for model: " << request->model_id() << std::endl;
    
    // Convert protobuf inputs to map
    std::map<std::string, std::vector<float>> inputs;
    for (const auto& input_pair : request->inputs()) {
        const auto& tensor = input_pair.second;
        
        // Convert bytes to floats (simplified - assumes float32)
        const std::string& data = tensor.data();
        std::vector<float> values;
        values.reserve(data.size() / sizeof(float));
        
        const float* float_data = reinterpret_cast<const float*>(data.data());
        size_t num_floats = data.size() / sizeof(float);
        
        for (size_t i = 0; i < num_floats; ++i) {
            values.push_back(float_data[i]);
        }
        
        inputs[input_pair.first] = values;
    }
    
    // Perform inference
    auto result = inference_engine_->predict(request->model_id(), inputs);
    
    // Set response
    response->set_success(result.success);
    response->set_inference_time_ms(result.inference_time_ms);
    
    if (result.success) {
        // Convert outputs back to protobuf
        for (const auto& output_pair : result.outputs) {
            auto* tensor = (*response->mutable_outputs())[output_pair.first].mutable_data();
            tensor->assign(
                reinterpret_cast<const char*>(output_pair.second.data()),
                output_pair.second.size() * sizeof(float)
            );
            
            (*response->mutable_outputs())[output_pair.first].set_dtype(
                mlinference::common::DataType::FLOAT32
            );
            (*response->mutable_outputs())[output_pair.first].add_shape(output_pair.second.size());
        }
        
        successful_requests_++;
    } else {
        response->set_error_message(result.error_message);
        failed_requests_++;
    }
    
    active_requests_--;
    
    std::cout << "Predict completed in " << result.inference_time_ms << "ms" << std::endl;
    
    return grpc::Status::OK;
}

grpc::Status WorkerServiceImpl::PredictStream(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<PredictResponse, PredictRequest>* stream) {
    
    PredictRequest request;
    while (stream->Read(&request)) {
        PredictResponse response;
        
        // Use the same logic as Predict
        Predict(context, &request, &response);
        
        if (!stream->Write(response)) {
            break;
        }
    }
    
    return grpc::Status::OK;
}

grpc::Status WorkerServiceImpl::BatchPredict(
    grpc::ServerContext* context,
    const BatchPredictRequest* request,
    BatchPredictResponse* response) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    response->set_success(true);
    
    for (const auto& predict_request : request->requests()) {
        PredictResponse predict_response;
        auto status = Predict(context, &predict_request, &predict_response);
        
        if (!status.ok()) {
            response->set_success(false);
            response->set_error_message(status.error_message());
            break;
        }
        
        *response->add_responses() = predict_response;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    response->set_total_time_ms(duration.count() / 1000.0);
    
    return grpc::Status::OK;
}

grpc::Status WorkerServiceImpl::LoadModel(
    grpc::ServerContext* context,
    const LoadModelRequest* request,
    LoadModelResponse* response) {
    
    std::cout << "Loading model: " << request->model_id() << " from " << request->model_path() << std::endl;
    
    bool success = inference_engine_->load_model(request->model_id(), request->model_path());
    
    response->set_success(success);
    if (success) {
        response->set_message("Model loaded successfully");
        
        // Set model info
        auto model_info = response->mutable_model_info();
        model_info->set_model_id(request->model_id());
        model_info->set_version(request->version());
        model_info->add_input_names("input");
        model_info->add_output_names("output");
    } else {
        response->set_message("Failed to load model");
    }
    
    return grpc::Status::OK;
}

grpc::Status WorkerServiceImpl::UnloadModel(
    grpc::ServerContext* context,
    const UnloadModelRequest* request,
    UnloadModelResponse* response) {
    
    std::cout << "Unloading model: " << request->model_id() << std::endl;
    
    bool success = inference_engine_->unload_model(request->model_id());
    
    response->set_success(success);
    if (success) {
        response->set_message("Model unloaded successfully");
    } else {
        response->set_message("Failed to unload model");
    }
    
    return grpc::Status::OK;
}

grpc::Status WorkerServiceImpl::ListLoadedModels(
    grpc::ServerContext* context,
    const ListLoadedModelsRequest* request,
    ListLoadedModelsResponse* response) {
    
    auto model_ids = inference_engine_->get_loaded_models();
    
    for (const auto& model_id : model_ids) {
        response->add_model_ids(model_id);
        
        // Add model info
        auto model_data = inference_engine_->get_model_info(model_id);
        if (model_data) {
            auto* model_info = (*response->mutable_models())[model_id].mutable_model_id();
            *model_info = model_id;
        }
    }
    
    return grpc::Status::OK;
}

grpc::Status WorkerServiceImpl::GetModelInfo(
    grpc::ServerContext* context,
    const GetModelInfoRequest* request,
    GetModelInfoResponse* response) {
    
    auto model_data = inference_engine_->get_model_info(request->model_id());
    
    if (model_data) {
        response->set_success(true);
        
        auto* model_info = response->mutable_model_info();
        model_info->set_model_id(model_data->model_id);
        
        for (const auto& name : model_data->input_names) {
            model_info->add_input_names(name);
        }
        for (const auto& name : model_data->output_names) {
            model_info->add_output_names(name);
        }
    } else {
        response->set_success(false);
        response->set_error_message("Model not found");
    }
    
    return grpc::Status::OK;
}

grpc::Status WorkerServiceImpl::GetStatus(
    grpc::ServerContext* context,
    const GetStatusRequest* request,
    GetStatusResponse* response) {
    
    response->set_worker_id(worker_id_);
    response->set_status(mlinference::common::WorkerStatus::READY);
    
    // Set capabilities
    auto* caps = response->mutable_capabilities();
    caps->set_supports_gpu(false);  // Simplified
    caps->set_num_cpu_cores(std::thread::hardware_concurrency());
    
    // Set metrics
    auto metrics = get_current_metrics();
    *response->mutable_metrics() = metrics;
    
    // List loaded models
    auto model_ids = inference_engine_->get_loaded_models();
    for (const auto& model_id : model_ids) {
        response->add_loaded_models(model_id);
    }
    
    return grpc::Status::OK;
}

grpc::Status WorkerServiceImpl::GetMetrics(
    grpc::ServerContext* context,
    const GetMetricsRequest* request,
    GetMetricsResponse* response) {
    
    auto metrics = get_current_metrics();
    *response->mutable_metrics() = metrics;
    
    // Add per-model metrics
    auto model_ids = inference_engine_->get_loaded_models();
    for (const auto& model_id : model_ids) {
        auto model_metrics = inference_engine_->get_model_metrics(model_id);
        
        auto* proto_metrics = (*response->mutable_model_metrics())[model_id].clear_total_requests();
        *proto_metrics = model_metrics.inference_count;
        (*response->mutable_model_metrics())[model_id].set_total_inference_time_ms(
            model_metrics.total_inference_time_ms
        );
        (*response->mutable_model_metrics())[model_id].set_avg_inference_time_ms(
            model_metrics.avg_inference_time_ms
        );
    }
    
    return grpc::Status::OK;
}

grpc::Status WorkerServiceImpl::HealthCheck(
    grpc::ServerContext* context,
    const HealthCheckRequest* request,
    HealthCheckResponse* response) {
    
    response->set_healthy(true);
    response->set_message("Worker is healthy");
    
    return grpc::Status::OK;
}

void WorkerServiceImpl::update_metrics(bool success) {
    if (success) {
        successful_requests_++;
    } else {
        failed_requests_++;
    }
}

mlinference::common::WorkerMetrics WorkerServiceImpl::get_current_metrics() {
    mlinference::common::WorkerMetrics metrics;
    
    metrics.set_total_requests(total_requests_.load());
    metrics.set_successful_requests(successful_requests_.load());
    metrics.set_failed_requests(failed_requests_.load());
    metrics.set_active_requests(active_requests_.load());
    
    // Calculate uptime
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    metrics.set_uptime_seconds(uptime.count());
    
    // Calculate average response time
    uint64_t total = total_requests_.load();
    if (total > 0) {
        // Simplified - in production, track actual response times
        metrics.set_avg_response_time_ms(25.0);
    }
    
    return metrics;
}

// Worker Server Implementation

WorkerServer::WorkerServer(
    const std::string& worker_id,
    const std::string& server_address,
    bool enable_gpu,
    uint32_t num_threads)
    : worker_id_(worker_id)
    , server_address_(server_address)
    , service_(std::make_unique<WorkerServiceImpl>(worker_id, enable_gpu, num_threads)) {
    
    std::cout << "WorkerServer created: " << worker_id_ << " at " << server_address_ << std::endl;
}

WorkerServer::~WorkerServer() {
    stop();
}

void WorkerServer::run() {
    grpc::ServerBuilder builder;
    
    // Listen on the given address
    builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());
    
    // Register service
    builder.RegisterService(service_.get());
    
    // Build and start server
    server_ = builder.BuildAndStart();
    
    if (server_) {
        std::cout << "Worker server listening on " << server_address_ << std::endl;
        server_->Wait();
    } else {
        std::cerr << "Failed to start server" << std::endl;
    }
}

void WorkerServer::stop() {
    if (server_) {
        std::cout << "Shutting down worker server..." << std::endl;
        server_->Shutdown();
        server_.reset();
    }
}

} // namespace worker
} // namespace mlinference