// #include "worker/worker_server.hpp"
// #include "common/logger.hpp"
// #include <grpcpp/server_builder.h>
// #include <thread>

// namespace mlinference {
// namespace worker {

// // ============================================================================
// // WorkerServiceImpl
// // ============================================================================

// WorkerServiceImpl::WorkerServiceImpl(
//     const common::WorkerConfig& config,
//     std::shared_ptr<InferenceEngine> inference_engine,
//     std::shared_ptr<ModelLoader> model_loader)
//     : config_(config)
//     , inference_engine_(inference_engine)
//     , model_loader_(model_loader) {
// }

// WorkerServiceImpl::~WorkerServiceImpl() = default;

// grpc::Status WorkerServiceImpl::Predict(
//     grpc::ServerContext* context,
//     const PredictRequest* request,
//     PredictResponse* response) {
    
//     active_requests_++;
//     auto cleanup = [this]() { active_requests_--; };
//     std::shared_ptr<void> guard(nullptr, [cleanup](void*) { cleanup(); });
    
//     auto start = std::chrono::high_resolution_clock::now();
    
//     LOG_DEBUG("Predict request: model={}, request_id={}", 
//              request->model_id(), request->request_id());
    
//     // Verify session token (simplified)
//     if (!verify_session_token(request->session_token())) {
//         return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, 
//                            "Invalid session token");
//     }
    
//     // Ensure model is loaded
//     std::string model_key = request->model_id();  // + version
//     if (!inference_engine_->is_model_loaded(model_key)) {
//         // TODO: Load model from cache or download
//         LOG_WARN("Model {} not loaded, attempting to load...", model_key);
//     }
    
//     // Convert protobuf inputs to native format
//     std::map<std::string, std::vector<float>> inputs;
//     for (const auto& tensor : request->inputs()) {
//         // TODO: Properly decode tensor data
//         std::vector<float> data(100, 1.0f);  // Mock data
//         inputs[tensor.name()] = data;
//     }
    
//     // Run inference
//     auto result = inference_engine_->predict(model_key, inputs);
    
//     // Build response
//     response->set_request_id(request->request_id());
//     response->set_success(result.success);
//     response->set_inference_time_ms(result.inference_time_ms);
//     response->set_worker_id(config_.worker_id);
    
//     if (!result.success) {
//         response->set_error_message(result.error_message);
//         failed_requests_++;
//         return grpc::Status(grpc::StatusCode::INTERNAL, result.error_message);
//     }
    
//     // Convert outputs to protobuf
//     for (const auto& [name, data] : result.outputs) {
//         auto* output_tensor = response->add_outputs();
//         output_tensor->set_name(name);
//         // TODO: Properly encode tensor data
//     }
    
//     auto end = std::chrono::high_resolution_clock::now();
//     double total_time = std::chrono::duration<double, std::milli>(end - start).count();
    
//     total_requests_++;
//     update_metrics(true, total_time);
    
//     return grpc::Status::OK;
// }

// grpc::Status WorkerServiceImpl::GetStatus(
//     grpc::ServerContext* context,
//     const GetStatusRequest* request,
//     GetStatusResponse* response) {
    
//     response->set_worker_id(config_.worker_id);
    
//     uint32_t active = active_requests_.load();
//     if (active == 0) {
//         response->set_status(mlinference::common::WORKER_STATUS_IDLE);
//     } else if (active < config_.server.max_concurrent_requests) {
//         response->set_status(mlinference::common::WORKER_STATUS_BUSY);
//     } else {
//         response->set_status(mlinference::common::WORKER_STATUS_OVERLOADED);
//     }
    
//     auto models = inference_engine_->get_loaded_models();
//     for (const auto& model : models) {
//         response->add_loaded_models(model);
//     }
    
//     return grpc::Status::OK;
// }

// grpc::Status WorkerServiceImpl::HealthCheck(
//     grpc::ServerContext* context,
//     const HealthCheckRequest* request,
//     HealthCheckResponse* response) {
    
//     auto* health = response->mutable_health();
//     health->set_healthy(true);
//     health->set_message("Worker is healthy");
    
//     return grpc::Status::OK;
// }

// bool WorkerServiceImpl::verify_session_token(const std::string& token) {
//     // Simplified - in production, verify with Manager
//     return !token.empty();
// }

// void WorkerServiceImpl::update_metrics(bool success, double latency_ms) {
//     // Update internal metrics
// }

// // ============================================================================
// // ManagerClient
// // ============================================================================

// ManagerClient::ManagerClient(const common::WorkerConfig& config)
//     : config_(config)
//     , worker_id_(config.worker_id) {
// }

// ManagerClient::~ManagerClient() {
//     stop_heartbeat_loop();
// }

// bool ManagerClient::connect() {
//     std::string manager_address = config_.manager.address + ":" + 
//                                   std::to_string(config_.manager.port);
    
//     channel_ = grpc::CreateChannel(manager_address, 
//                                    grpc::InsecureChannelCredentials());
    
//     // stub_ = mlinference::manager::ManagerService::NewStub(channel_);
    
//     LOG_INFO("Connected to Manager at {}", manager_address);
//     return true;
// }

// bool ManagerClient::register_worker() {
//     LOG_INFO("Registering worker with Manager...");
    
//     // TODO: Call Manager RegisterWorker RPC
//     // RegisterWorkerRequest request;
//     // request.set_worker_id(worker_id_);
//     // request.set_address(config_.server.address);
//     // request.set_port(config_.server.port);
//     // ... set capabilities
    
//     LOG_INFO("Worker registered: {}", worker_id_);
//     return true;
// }

// bool ManagerClient::send_heartbeat() {
//     // TODO: Call Manager Heartbeat RPC
//     LOG_DEBUG("Sending heartbeat to Manager");
//     return true;
// }

// void ManagerClient::start_heartbeat_loop() {
//     heartbeat_running_ = true;
    
//     heartbeat_thread_ = std::make_unique<std::thread>([this]() {
//         while (heartbeat_running_) {
//             send_heartbeat();
//             std::this_thread::sleep_for(
//                 std::chrono::seconds(config_.manager.heartbeat_interval_seconds));
//         }
//     });
    
//     LOG_INFO("Heartbeat loop started");
// }

// void ManagerClient::stop_heartbeat_loop() {
//     if (heartbeat_running_) {
//         heartbeat_running_ = false;
        
//         if (heartbeat_thread_ && heartbeat_thread_->joinable()) {
//             heartbeat_thread_->join();
//         }
        
//         LOG_INFO("Heartbeat loop stopped");
//     }
// }

// // ============================================================================
// // WorkerServer
// // ============================================================================

// WorkerServer::WorkerServer(const common::WorkerConfig& config)
//     : config_(config) {
// }

// WorkerServer::~WorkerServer() {
//     stop();
// }

// bool WorkerServer::initialize() {
//     LOG_INFO("Initializing Worker...");
    
//     // Create inference engine
//     inference_engine_ = std::make_shared<InferenceEngine>(
//         config_.inference.enable_gpu,
//         config_.inference.gpu_device_id,
//         config_.inference.num_cpu_threads);
    
//     // Create model loader (simplified - not implemented)
//     // model_loader_ = std::make_shared<ModelLoader>();
    
//     // Create service
//     service_ = std::make_unique<WorkerServiceImpl>(
//         config_, inference_engine_, nullptr);
    
//     // Create manager client
//     manager_client_ = std::make_unique<ManagerClient>(config_);
    
//     if (!manager_client_->connect()) {
//         LOG_ERROR("Failed to connect to Manager");
//         return false;
//     }
    
//     if (!manager_client_->register_worker()) {
//         LOG_ERROR("Failed to register with Manager");
//         return false;
//     }
    
//     LOG_INFO("Worker initialized successfully");
//     return true;
// }

// bool WorkerServer::run() {
//     if (!start()) {
//         return false;
//     }
    
//     wait();
//     return true;
// }

// bool WorkerServer::start() {
//     if (!initialize()) {
//         return false;
//     }
    
//     std::string server_address = config_.server.address + ":" + 
//                                  std::to_string(config_.server.port);
    
//     grpc::ServerBuilder builder;
//     builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
//     builder.RegisterService(service_.get());
    
//     server_ = builder.BuildAndStart();
    
//     if (!server_) {
//         LOG_ERROR("Failed to start Worker server");
//         return false;
//     }
    
//     running_ = true;
    
//     // Start heartbeat loop
//     manager_client_->start_heartbeat_loop();
    
//     LOG_INFO("Worker server listening on {}", server_address);
//     return true;
// }

// void WorkerServer::stop() {
//     if (running_ && server_) {
//         LOG_INFO("Stopping Worker server...");
        
//         // Stop heartbeat
//         if (manager_client_) {
//             manager_client_->stop_heartbeat_loop();
//             manager_client_->unregister_worker("Server shutdown");
//         }
        
//         server_->Shutdown();
//         running_ = false;
        
//         LOG_INFO("Worker server stopped");
//     }
// }

// void WorkerServer::wait() {
//     if (server_) {
//         server_->Wait();
//     }
// }

// } // namespace worker
// } // namespace mlinference