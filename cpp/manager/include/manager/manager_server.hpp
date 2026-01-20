#ifndef ML_INFERENCE_MANAGER_SERVER_HPP
#define ML_INFERENCE_MANAGER_SERVER_HPP

#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "manager.grpc.pb.h"
#include "common/config.hpp"
#include "worker_registry.hpp"
#include "model_registry.hpp"
#include "api_key_manager.hpp"
#include "load_balancer.hpp"
#include "health_monitor.hpp"

namespace mlinference {
namespace manager {

// Manager Service Implementation
class ManagerServiceImpl final : public mlinference::manager::ManagerService::Service {
public:
    explicit ManagerServiceImpl(const common::ManagerConfig& config);
    ~ManagerServiceImpl() override;
    
    // Initialize all subsystems
    bool initialize();
    
    // Shutdown gracefully
    void shutdown();
    
    // Worker Management
    grpc::Status RegisterWorker(
        grpc::ServerContext* context,
        const RegisterWorkerRequest* request,
        RegisterWorkerResponse* response) override;
    
    grpc::Status Heartbeat(
        grpc::ServerContext* context,
        const HeartbeatRequest* request,
        HeartbeatResponse* response) override;
    
    grpc::Status UnregisterWorker(
        grpc::ServerContext* context,
        const UnregisterWorkerRequest* request,
        UnregisterWorkerResponse* response) override;
    
    grpc::Status ListWorkers(
        grpc::ServerContext* context,
        const ListWorkersRequest* request,
        ListWorkersResponse* response) override;
    
    // Worker Assignment
    grpc::Status GetWorkerForInference(
        grpc::ServerContext* context,
        const GetWorkerRequest* request,
        GetWorkerResponse* response) override;
    
    // Model Registry
    grpc::Status RegisterModel(
        grpc::ServerContext* context,
        const RegisterModelRequest* request,
        RegisterModelResponse* response) override;
    
    grpc::Status GetModel(
        grpc::ServerContext* context,
        const GetModelRequest* request,
        GetModelResponse* response) override;
    
    grpc::Status ListModels(
        grpc::ServerContext* context,
        const ListModelsRequest* request,
        ListModelsResponse* response) override;
    
    grpc::Status UpdateModel(
        grpc::ServerContext* context,
        const UpdateModelRequest* request,
        UpdateModelResponse* response) override;
    
    grpc::Status DeleteModel(
        grpc::ServerContext* context,
        const DeleteModelRequest* request,
        DeleteModelResponse* response) override;
    
    grpc::Status UploadModelChunk(
        grpc::ServerContext* context,
        grpc::ServerReader<UploadModelChunkRequest>* reader,
        UploadModelResponse* response) override;
    
    // API Key Management
    grpc::Status CreateApiKey(
        grpc::ServerContext* context,
        const CreateApiKeyRequest* request,
        CreateApiKeyResponse* response) override;
    
    grpc::Status ValidateApiKey(
        grpc::ServerContext* context,
        const ValidateApiKeyRequest* request,
        ValidateApiKeyResponse* response) override;
    
    grpc::Status ListApiKeys(
        grpc::ServerContext* context,
        const ListApiKeysRequest* request,
        ListApiKeysResponse* response) override;
    
    grpc::Status RevokeApiKey(
        grpc::ServerContext* context,
        const RevokeApiKeyRequest* request,
        RevokeApiKeyResponse* response) override;
    
    // Health and Metrics
    grpc::Status GetHealth(
        grpc::ServerContext* context,
        const GetHealthRequest* request,
        GetHealthResponse* response) override;
    
    grpc::Status GetMetrics(
        grpc::ServerContext* context,
        const GetMetricsRequest* request,
        GetMetricsResponse* response) override;
    
    grpc::Status GetAggregatedMetrics(
        grpc::ServerContext* context,
        const GetAggregatedMetricsRequest* request,
        GetAggregatedMetricsResponse* response) override;

private:
    // Configuration
    common::ManagerConfig config_;
    
    // Subsystems
    std::unique_ptr<WorkerRegistry> worker_registry_;
    std::unique_ptr<ModelRegistry> model_registry_;
    std::unique_ptr<ApiKeyManager> api_key_manager_;
    std::unique_ptr<LoadBalancer> load_balancer_;
    std::unique_ptr<HealthMonitor> health_monitor_;
    
    // Helper methods
    bool validate_api_key(const std::string& api_key, const std::string& required_permission);
    std::string generate_session_token(const std::string& worker_id);
    bool verify_session_token(const std::string& token);
};

// Manager Server
class ManagerServer {
public:
    explicit ManagerServer(const common::ManagerConfig& config);
    ~ManagerServer();
    
    // Start the server (blocking)
    bool run();
    
    // Start the server (non-blocking)
    bool start();
    
    // Stop the server
    void stop();
    
    // Wait for server to finish
    void wait();
    
    // Check if server is running
    bool is_running() const { return running_; }

private:
    common::ManagerConfig config_;
    std::unique_ptr<ManagerServiceImpl> service_;
    std::unique_ptr<grpc::Server> server_;
    bool running_{false};
    
    // Build server credentials
    std::shared_ptr<grpc::ServerCredentials> build_credentials();
};

} // namespace manager
} // namespace mlinference

#endif // ML_INFERENCE_MANAGER_SERVER_HPP
