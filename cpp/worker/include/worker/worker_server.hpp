// #ifndef ML_INFERENCE_WORKER_SERVER_HPP
// #define ML_INFERENCE_WORKER_SERVER_HPP

// #include <memory>
// #include <string>
// #include <atomic>
// #include <thread>
// #include <grpcpp/grpcpp.h>
// #include "worker.grpc.pb.h"
// #include "manager.grpc.pb.h"
// #include "common/config.hpp"
// #include "inference_engine.hpp"
// #include "model_loader.hpp"
// #include "thread_pool.hpp"

// namespace mlinference {
// namespace worker {

// // Worker Service Implementation
// class WorkerServiceImpl final : public mlinference::worker::WorkerService::Service {
// public:
//     explicit WorkerServiceImpl(
//         const common::WorkerConfig& config,
//         std::shared_ptr<InferenceEngine> inference_engine,
//         std::shared_ptr<ModelLoader> model_loader);
    
//     ~WorkerServiceImpl() override;
    
//     // Inference Operations
//     grpc::Status Predict(
//         grpc::ServerContext* context,
//         const PredictRequest* request,
//         PredictResponse* response) override;
    
//     grpc::Status PredictStream(
//         grpc::ServerContext* context,
//         grpc::ServerReaderWriter<PredictResponse, PredictRequest>* stream) override;
    
//     grpc::Status BatchPredict(
//         grpc::ServerContext* context,
//         const BatchPredictRequest* request,
//         BatchPredictResponse* response) override;
    
//     // Model Management
//     grpc::Status LoadModel(
//         grpc::ServerContext* context,
//         const LoadModelRequest* request,
//         LoadModelResponse* response) override;
    
//     grpc::Status UnloadModel(
//         grpc::ServerContext* context,
//         const UnloadModelRequest* request,
//         UnloadModelResponse* response) override;
    
//     grpc::Status ListLoadedModels(
//         grpc::ServerContext* context,
//         const ListLoadedModelsRequest* request,
//         ListLoadedModelsResponse* response) override;
    
//     grpc::Status GetModelInfo(
//         grpc::ServerContext* context,
//         const GetModelInfoRequest* request,
//         GetModelInfoResponse* response) override;
    
//     // Worker Status
//     grpc::Status GetStatus(
//         grpc::ServerContext* context,
//         const GetStatusRequest* request,
//         GetStatusResponse* response) override;
    
//     grpc::Status GetMetrics(
//         grpc::ServerContext* context,
//         const GetMetricsRequest* request,
//         GetMetricsResponse* response) override;
    
//     grpc::Status HealthCheck(
//         grpc::ServerContext* context,
//         const HealthCheckRequest* request,
//         HealthCheckResponse* response) override;

// private:
//     common::WorkerConfig config_;
//     std::shared_ptr<InferenceEngine> inference_engine_;
//     std::shared_ptr<ModelLoader> model_loader_;
//     std::shared_ptr<ThreadPool> thread_pool_;
    
//     // Metrics tracking
//     std::atomic<uint64_t> total_requests_{0};
//     std::atomic<uint64_t> failed_requests_{0};
//     std::atomic<uint32_t> active_requests_{0};
    
//     // Helper methods
//     bool verify_session_token(const std::string& token);
//     void update_metrics(bool success, double latency_ms);
// };

// // Manager Communication Client
// class ManagerClient {
// public:
//     explicit ManagerClient(const common::WorkerConfig& config);
//     ~ManagerClient();
    
//     // Connect to manager
//     bool connect();
    
//     // Register worker with manager
//     bool register_worker();
    
//     // Send heartbeat
//     bool send_heartbeat();
    
//     // Unregister worker
//     bool unregister_worker(const std::string& reason);
    
//     // Start heartbeat loop
//     void start_heartbeat_loop();
    
//     // Stop heartbeat loop
//     void stop_heartbeat_loop();

// private:
//     common::WorkerConfig config_;
//     std::string worker_id_;
//     std::unique_ptr<mlinference::manager::ManagerService::Stub> stub_;
//     std::shared_ptr<grpc::Channel> channel_;
    
//     // Heartbeat thread
//     std::unique_ptr<std::thread> heartbeat_thread_;
//     std::atomic<bool> heartbeat_running_{false};
    
//     // Capabilities and metrics
//     mlinference::common::WorkerCapabilities get_capabilities();
//     mlinference::common::WorkerMetrics get_metrics();
    
//     // Build channel credentials
//     std::shared_ptr<grpc::ChannelCredentials> build_credentials();
// };

// // Worker Server
// class WorkerServer {
// public:
//     explicit WorkerServer(const common::WorkerConfig& config);
//     ~WorkerServer();
    
//     // Initialize worker
//     bool initialize();
    
//     // Start the server (blocking)
//     bool run();
    
//     // Start the server (non-blocking)
//     bool start();
    
//     // Stop the server
//     void stop();
    
//     // Wait for server to finish
//     void wait();
    
//     // Check if server is running
//     bool is_running() const { return running_; }

// private:
//     common::WorkerConfig config_;
//     std::unique_ptr<WorkerServiceImpl> service_;
//     std::unique_ptr<grpc::Server> server_;
//     std::unique_ptr<ManagerClient> manager_client_;
    
//     // Subsystems
//     std::shared_ptr<InferenceEngine> inference_engine_;
//     std::shared_ptr<ModelLoader> model_loader_;
    
//     bool running_{false};
    
//     // Build server credentials
//     std::shared_ptr<grpc::ServerCredentials> build_credentials();
// };

// } // namespace worker
// } // namespace mlinference

// #endif // ML_INFERENCE_WORKER_SERVER_HPP
