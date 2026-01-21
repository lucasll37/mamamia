// #include "client/inference_client.hpp"
// #include <iostream>
// #include <chrono>

// namespace mlinference {
// namespace client {

// InferenceClient::InferenceClient(const std::string& server_address)
//     : server_address_(server_address)
//     , connected_(false) {
    
//     std::cout << "InferenceClient created for server: " << server_address_ << std::endl;
// }

// InferenceClient::~InferenceClient() {
//     std::cout << "InferenceClient destroyed" << std::endl;
// }

// bool InferenceClient::connect() {
//     try {
//         // Create channel
//         channel_ = grpc::CreateChannel(
//             server_address_,
//             grpc::InsecureChannelCredentials()
//         );
        
//         // Create stub
//         stub_ = mlinference::worker::WorkerService::NewStub(channel_);
        
//         // Test connection with health check
//         grpc::ClientContext context;
//         mlinference::worker::HealthCheckRequest request;
//         mlinference::worker::HealthCheckResponse response;
        
//         auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
//         context.set_deadline(deadline);
        
//         grpc::Status status = stub_->HealthCheck(&context, request, &response);
        
//         if (status.ok() && response.healthy()) {
//             connected_ = true;
//             std::cout << "Successfully connected to server" << std::endl;
//             return true;
//         } else {
//             std::cerr << "Health check failed: " << status.error_message() << std::endl;
//             return false;
//         }
        
//     } catch (const std::exception& e) {
//         std::cerr << "Connection error: " << e.what() << std::endl;
//         return false;
//     }
// }

// bool InferenceClient::is_connected() const {
//     return connected_;
// }

// bool InferenceClient::load_model(
//     const std::string& model_id,
//     const std::string& model_path,
//     const std::string& version) {
    
//     if (!connected_) {
//         std::cerr << "Not connected to server" << std::endl;
//         return false;
//     }
    
//     grpc::ClientContext context;
//     mlinference::worker::LoadModelRequest request;
//     mlinference::worker::LoadModelResponse response;
    
//     request.set_model_id(model_id);
//     request.set_model_path(model_path);
//     request.set_version(version);
    
//     grpc::Status status = stub_->LoadModel(&context, request, &response);
    
//     if (status.ok()) {
//         if (response.success()) {
//             std::cout << "Model loaded: " << model_id << std::endl;
//             return true;
//         } else {
//             std::cerr << "Load model failed: " << response.message() << std::endl;
//             return false;
//         }
//     } else {
//         std::cerr << "RPC failed: " << status.error_message() << std::endl;
//         return false;
//     }
// }

// bool InferenceClient::unload_model(const std::string& model_id) {
//     if (!connected_) {
//         std::cerr << "Not connected to server" << std::endl;
//         return false;
//     }
    
//     grpc::ClientContext context;
//     mlinference::worker::UnloadModelRequest request;
//     mlinference::worker::UnloadModelResponse response;
    
//     request.set_model_id(model_id);
    
//     grpc::Status status = stub_->UnloadModel(&context, request, &response);
    
//     if (status.ok()) {
//         if (response.success()) {
//             std::cout << "Model unloaded: " << model_id << std::endl;
//             return true;
//         } else {
//             std::cerr << "Unload model failed: " << response.message() << std::endl;
//             return false;
//         }
//     } else {
//         std::cerr << "RPC failed: " << status.error_message() << std::endl;
//         return false;
//     }
// }

// std::vector<std::string> InferenceClient::list_loaded_models() {
//     std::vector<std::string> models;
    
//     if (!connected_) {
//         std::cerr << "Not connected to server" << std::endl;
//         return models;
//     }
    
//     grpc::ClientContext context;
//     mlinference::worker::ListLoadedModelsRequest request;
//     mlinference::worker::ListLoadedModelsResponse response;
    
//     grpc::Status status = stub_->ListLoadedModels(&context, request, &response);
    
//     if (status.ok()) {
//         for (const auto& model_id : response.model_ids()) {
//             models.push_back(model_id);
//         }
//     } else {
//         std::cerr << "RPC failed: " << status.error_message() << std::endl;
//     }
    
//     return models;
// }

// PredictionResult InferenceClient::predict(
//     const std::string& model_id,
//     const std::map<std::string, std::vector<float>>& inputs,
//     const std::string& version) {
    
//     PredictionResult result;
//     result.success = false;
    
//     if (!connected_) {
//         result.error_message = "Not connected to server";
//         return result;
//     }
    
//     grpc::ClientContext context;
//     mlinference::worker::PredictRequest request;
//     mlinference::worker::PredictResponse response;
    
//     request.set_model_id(model_id);
//     request.set_version(version);
    
//     // Convert inputs to protobuf
//     for (const auto& input_pair : inputs) {
//         auto* tensor = (*request.mutable_inputs())[input_pair.first].mutable_data();
//         set_tensor_data(&(*request.mutable_inputs())[input_pair.first], input_pair.second);
//     }
    
//     auto start_time = std::chrono::high_resolution_clock::now();
//     grpc::Status status = stub_->Predict(&context, request, &response);
//     auto end_time = std::chrono::high_resolution_clock::now();
    
//     auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
//     double total_time_ms = duration.count() / 1000.0;
    
//     if (status.ok()) {
//         result.success = response.success();
//         result.inference_time_ms = response.inference_time_ms();
//         result.error_message = response.error_message();
        
//         if (response.success()) {
//             // Convert outputs from protobuf
//             for (const auto& output_pair : response.outputs()) {
//                 result.outputs[output_pair.first] = get_tensor_data(output_pair.second);
//             }
            
//             std::cout << "Prediction successful (inference: " << result.inference_time_ms 
//                      << "ms, total: " << total_time_ms << "ms)" << std::endl;
//         } else {
//             std::cerr << "Prediction failed: " << result.error_message << std::endl;
//         }
//     } else {
//         result.error_message = "RPC failed: " + status.error_message();
//         std::cerr << result.error_message << std::endl;
//     }
    
//     return result;
// }

// std::vector<PredictionResult> InferenceClient::batch_predict(
//     const std::string& model_id,
//     const std::vector<std::map<std::string, std::vector<float>>>& batch_inputs,
//     const std::string& version) {
    
//     std::vector<PredictionResult> results;
    
//     if (!connected_) {
//         std::cerr << "Not connected to server" << std::endl;
//         return results;
//     }
    
//     grpc::ClientContext context;
//     mlinference::worker::BatchPredictRequest request;
//     mlinference::worker::BatchPredictResponse response;
    
//     request.set_model_id(model_id);
//     request.set_version(version);
    
//     // Add all requests
//     for (const auto& inputs : batch_inputs) {
//         auto* predict_request = request.add_requests();
//         predict_request->set_model_id(model_id);
//         predict_request->set_version(version);
        
//         for (const auto& input_pair : inputs) {
//             set_tensor_data(&(*predict_request->mutable_inputs())[input_pair.first], input_pair.second);
//         }
//     }
    
//     grpc::Status status = stub_->BatchPredict(&context, request, &response);
    
//     if (status.ok()) {
//         if (response.success()) {
//             std::cout << "Batch prediction successful (total time: " 
//                      << response.total_time_ms() << "ms)" << std::endl;
            
//             for (const auto& predict_response : response.responses()) {
//                 PredictionResult result;
//                 result.success = predict_response.success();
//                 result.inference_time_ms = predict_response.inference_time_ms();
//                 result.error_message = predict_response.error_message();
                
//                 if (predict_response.success()) {
//                     for (const auto& output_pair : predict_response.outputs()) {
//                         result.outputs[output_pair.first] = get_tensor_data(output_pair.second);
//                     }
//                 }
                
//                 results.push_back(result);
//             }
//         } else {
//             std::cerr << "Batch prediction failed: " << response.error_message() << std::endl;
//         }
//     } else {
//         std::cerr << "RPC failed: " << status.error_message() << std::endl;
//     }
    
//     return results;
// }

// bool InferenceClient::health_check() {
//     if (!connected_) {
//         return false;
//     }
    
//     grpc::ClientContext context;
//     mlinference::worker::HealthCheckRequest request;
//     mlinference::worker::HealthCheckResponse response;
    
//     grpc::Status status = stub_->HealthCheck(&context, request, &response);
    
//     return status.ok() && response.healthy();
// }

// InferenceClient::WorkerStatus InferenceClient::get_status() {
//     WorkerStatus worker_status;
    
//     if (!connected_) {
//         std::cerr << "Not connected to server" << std::endl;
//         return worker_status;
//     }
    
//     grpc::ClientContext context;
//     mlinference::worker::GetStatusRequest request;
//     mlinference::worker::GetStatusResponse response;
    
//     grpc::Status status = stub_->GetStatus(&context, request, &response);
    
//     if (status.ok()) {
//         worker_status.worker_id = response.worker_id();
//         worker_status.total_requests = response.metrics().total_requests();
//         worker_status.successful_requests = response.metrics().successful_requests();
//         worker_status.failed_requests = response.metrics().failed_requests();
//         worker_status.active_requests = response.metrics().active_requests();
//         worker_status.uptime_seconds = response.metrics().uptime_seconds();
        
//         for (const auto& model_id : response.loaded_models()) {
//             worker_status.loaded_models.push_back(model_id);
//         }
//     } else {
//         std::cerr << "RPC failed: " << status.error_message() << std::endl;
//     }
    
//     return worker_status;
// }

// void InferenceClient::set_tensor_data(
//     mlinference::common::Tensor* tensor,
//     const std::vector<float>& data) {
    
//     tensor->set_dtype(mlinference::common::DataType::FLOAT32);
//     tensor->add_shape(data.size());
    
//     tensor->mutable_data()->assign(
//         reinterpret_cast<const char*>(data.data()),
//         data.size() * sizeof(float)
//     );
// }

// std::vector<float> InferenceClient::get_tensor_data(
//     const mlinference::common::Tensor& tensor) {
    
//     const std::string& data = tensor.data();
//     std::vector<float> values;
    
//     if (tensor.dtype() == mlinference::common::DataType::FLOAT32) {
//         const float* float_data = reinterpret_cast<const float*>(data.data());
//         size_t num_floats = data.size() / sizeof(float);
        
//         values.reserve(num_floats);
//         for (size_t i = 0; i < num_floats; ++i) {
//             values.push_back(float_data[i]);
//         }
//     }
    
//     return values;
// }

// } // namespace client
// } // namespace mlinference