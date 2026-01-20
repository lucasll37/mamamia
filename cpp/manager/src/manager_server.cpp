// #include "manager/manager_server.hpp"
// #include "common/logger.hpp"
// #include <grpcpp/security/server_credentials.h>
// #include <grpcpp/server_builder.h>

// namespace mlinference {
// namespace manager {

// // ============================================================================
// // ManagerServiceImpl Implementation
// // ============================================================================

// ManagerServiceImpl::ManagerServiceImpl(const common::ManagerConfig& config)
//     : config_(config) {
// }

// ManagerServiceImpl::~ManagerServiceImpl() {
//     shutdown();
// }

// bool ManagerServiceImpl::initialize() {
//     LOG_INFO("Initializing Manager subsystems...");
    
//     // Initialize Worker Registry
//     worker_registry_ = std::make_unique<WorkerRegistry>(
//         config_.worker.heartbeat_timeout_seconds,
//         config_.worker.max_missed_heartbeats);
    
//     // Initialize API Key Manager
//     api_key_manager_ = std::make_unique<ApiKeyManager>(
//         config_.security.admin_api_key);
    
//     // Initialize Load Balancer
//     load_balancer_ = std::make_unique<LoadBalancer>(
//         std::shared_ptr<WorkerRegistry>(worker_registry_.get(), [](auto*){}),
//         LoadBalancingStrategy::MODEL_AFFINITY);
    
//     LOG_INFO("Manager subsystems initialized successfully");
//     return true;
// }

// void ManagerServiceImpl::shutdown() {
//     LOG_INFO("Shutting down Manager...");
//     // Cleanup resources
// }

// // Worker Management RPCs

// grpc::Status ManagerServiceImpl::RegisterWorker(
//     grpc::ServerContext* context,
//     const RegisterWorkerRequest* request,
//     RegisterWorkerResponse* response) {
    
//     LOG_INFO("Worker registration request: {}", request->worker_id());
    
//     bool success = worker_registry_->register_worker(
//         request->worker_id(),
//         request->address(),
//         request->port(),
//         request->capabilities());
    
//     if (success) {
//         response->set_success(true);
//         response->set_message("Worker registered successfully");
//         response->set_heartbeat_interval_seconds(config_.worker.heartbeat_interval_seconds);
        
//         LOG_INFO("Worker registered: {}", request->worker_id());
//         return grpc::Status::OK;
//     } else {
//         response->set_success(false);
//         response->set_message("Failed to register worker");
//         return grpc::Status(grpc::StatusCode::INTERNAL, "Registration failed");
//     }
// }

// grpc::Status ManagerServiceImpl::Heartbeat(
//     grpc::ServerContext* context,
//     const HeartbeatRequest* request,
//     HeartbeatResponse* response) {
    
//     std::vector<std::string> loaded_models;
//     for (const auto& model : request->loaded_models()) {
//         loaded_models.push_back(model);
//     }
    
//     bool success = worker_registry_->update_heartbeat(
//         request->worker_id(),
//         request->metrics(),
//         loaded_models);
    
//     response->set_acknowledged(success);
    
//     return grpc::Status::OK;
// }

// grpc::Status ManagerServiceImpl::UnregisterWorker(
//     grpc::ServerContext* context,
//     const UnregisterWorkerRequest* request,
//     UnregisterWorkerResponse* response) {
    
//     LOG_INFO("Worker unregistration: {} (reason: {})", 
//              request->worker_id(), request->reason());
    
//     bool success = worker_registry_->unregister_worker(request->worker_id());
    
//     response->set_success(success);
//     response->set_message(success ? "Worker unregistered" : "Worker not found");
    
//     return grpc::Status::OK;
// }

// grpc::Status ManagerServiceImpl::ListWorkers(
//     grpc::ServerContext* context,
//     const ListWorkersRequest* request,
//     ListWorkersResponse* response) {
    
//     auto workers = worker_registry_->list_workers();
    
//     for (const auto& worker : workers) {
//         auto* worker_info = response->add_workers();
//         worker_info->set_worker_id(worker.worker_id);
//         worker_info->set_address(worker.address);
//         worker_info->set_port(worker.port);
        
//         if (request->include_metrics()) {
//             *worker_info->mutable_metrics() = worker.metrics;
//         }
//     }
    
//     return grpc::Status::OK;
// }

// grpc::Status ManagerServiceImpl::GetWorkerForInference(
//     grpc::ServerContext* context,
//     const GetWorkerRequest* request,
//     GetWorkerResponse* response) {
    
//     // Validate API key
//     if (!validate_api_key(request->api_key(), common::Permission::INFERENCE)) {
//         response->set_success(false);
//         response->set_error_message("Invalid or unauthorized API key");
//         return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Invalid API key");
//     }
    
//     // Get GPU requirement
//     bool require_gpu = false;
//     if (request->requirements().count("gpu_required")) {
//         require_gpu = (request->requirements().at("gpu_required") == "true");
//     }
    
//     // Select worker
//     auto worker_id = load_balancer_->select_worker(request->model_id(), require_gpu);
    
//     if (!worker_id) {
//         response->set_success(false);
//         response->set_error_message("No available workers");
//         return grpc::Status(grpc::StatusCode::UNAVAILABLE, "No workers available");
//     }
    
//     // Get worker info
//     auto worker_opt = worker_registry_->get_worker(*worker_id);
//     if (!worker_opt) {
//         response->set_success(false);
//         response->set_error_message("Worker not found");
//         return grpc::Status(grpc::StatusCode::INTERNAL, "Worker disappeared");
//     }
    
//     // Generate session token
//     std::string session_token = generate_session_token(*worker_id);
    
//     response->set_success(true);
//     response->set_worker_id(*worker_id);
//     response->set_worker_address(worker_opt->address);
//     response->set_worker_port(worker_opt->port);
//     response->set_session_token(session_token);
    
//     LOG_INFO("Assigned worker {} for model {}", *worker_id, request->model_id());
    
//     return grpc::Status::OK;
// }

// // Health Check

// grpc::Status ManagerServiceImpl::GetHealth(
//     grpc::ServerContext* context,
//     const GetHealthRequest* request,
//     GetHealthResponse* response) {
    
//     auto* health = response->mutable_manager_health();
//     health->set_healthy(true);
//     health->set_message("Manager is healthy");
    
//     if (request->include_workers()) {
//         auto workers = worker_registry_->list_workers();
//         for (const auto& worker : workers) {
//             auto* worker_health = (*response->mutable_worker_health())[worker.worker_id].mutable_health();
//             worker_health->set_healthy(worker.status != common::WorkerStatus::OFFLINE);
//         }
//     }
    
//     return grpc::Status::OK;
// }

// // Helper methods

// bool ManagerServiceImpl::validate_api_key(const std::string& api_key, 
//                                           const std::string& required_permission) {
//     if (api_key.empty()) {
//         return false;
//     }
    
//     std::optional<common::Permission> perm;
//     if (!required_permission.empty()) {
//         perm = common::permission_from_string(required_permission);
//     }
    
//     return api_key_manager_->validate_api_key(api_key, perm);
// }

// std::string ManagerServiceImpl::generate_session_token(const std::string& worker_id) {
//     // Simple token: worker_id + timestamp + random
//     return common::sha256_hash(worker_id + 
//                                 common::timestamp_to_string(common::now()) + 
//                                 common::generate_uuid());
// }

// bool ManagerServiceImpl::verify_session_token(const std::string& token) {
//     // Simple verification - in production, use JWT or similar
//     return !token.empty() && token.length() == 64;  // SHA256 length
// }

// // ============================================================================
// // ManagerServer Implementation
// // ============================================================================

// ManagerServer::ManagerServer(const common::ManagerConfig& config)
//     : config_(config) {
// }

// ManagerServer::~ManagerServer() {
//     stop();
// }

// bool ManagerServer::run() {
//     if (!start()) {
//         return false;
//     }
    
//     wait();
//     return true;
// }

// bool ManagerServer::start() {
//     LOG_INFO("Starting Manager Server...");
    
//     // Create service implementation
//     service_ = std::make_unique<ManagerServiceImpl>(config_);
    
//     if (!service_->initialize()) {
//         LOG_ERROR("Failed to initialize Manager service");
//         return false;
//     }
    
//     // Build server
//     std::string server_address = config_.server.address + ":" + 
//                                  std::to_string(config_.server.port);
    
//     grpc::ServerBuilder builder;
    
//     // Add listening port
//     builder.AddListeningPort(server_address, build_credentials());
    
//     // Register service
//     builder.RegisterService(service_.get());
    
//     // Set options
//     builder.SetMaxReceiveMessageSize(100 * 1024 * 1024);  // 100 MB
//     builder.SetMaxSendMessageSize(100 * 1024 * 1024);
    
//     // Build and start server
//     server_ = builder.BuildAndStart();
    
//     if (!server_) {
//         LOG_ERROR("Failed to start Manager server");
//         return false;
//     }
    
//     running_ = true;
//     LOG_INFO("Manager Server listening on {}", server_address);
    
//     return true;
// }

// void ManagerServer::stop() {
//     if (running_ && server_) {
//         LOG_INFO("Stopping Manager Server...");
        
//         server_->Shutdown();
//         running_ = false;
        
//         LOG_INFO("Manager Server stopped");
//     }
// }

// void ManagerServer::wait() {
//     if (server_) {
//         server_->Wait();
//     }
// }

// std::shared_ptr<grpc::ServerCredentials> ManagerServer::build_credentials() {
//     if (config_.security.enable_tls) {
//         // Load TLS credentials
//         grpc::SslServerCredentialsOptions ssl_opts;
//         // TODO: Load cert and key files
//         LOG_INFO("Using TLS credentials");
//         return grpc::SslServerCredentials(ssl_opts);
//     } else {
//         LOG_WARN("Using insecure credentials (no TLS)");
//         return grpc::InsecureServerCredentials();
//     }
// }

// } // namespace manager
// } // namespace mlinference