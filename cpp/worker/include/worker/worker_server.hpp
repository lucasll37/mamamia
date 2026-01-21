// #ifndef ML_INFERENCE_WORKER_SERVER_HPP
// #define ML_INFERENCE_WORKER_SERVER_HPP

// #include <memory>
// #include <string>
// #include <atomic>
// #include <thread>
// #include <chrono>
// #include <grpcpp/grpcpp.h>
// #include "worker.grpc.pb.h"
// #include "inference_engine.hpp"

// namespace mlinference {
// namespace worker {

// // Worker Service Implementation
// class WorkerServiceImpl final : public mlinference::worker::WorkerService::Service {
// public:
//     explicit WorkerServiceImpl(
//         const std::string& worker_id,
//         bool enable_gpu = false,
//         uint32_t num_threads = 4);
    
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
//     std::string worker_id_;
//     std::shared_ptr<InferenceEngine> inference_engine_;
    
//     // Metrics tracking
//     std::atomic<uint64_t> total_requests_{0};
//     std::atomic<uint64_t> successful_requests_{0};
//     std::atomic<uint64_t> failed_requests_{0};
//     std::atomic<uint32_t> active_requests_{0};
    
//     std::chrono::steady_clock::time_point start_time_;
    
//     // Helper methods
//     void update_metrics(bool success);
//     mlinference::common::WorkerMetrics get_current_metrics();
// };

// // Worker Server
// class WorkerServer {
// public:
//     explicit WorkerServer(
//         const std::string& worker_id,
//         const std::string& server_address,
//         bool enable_gpu = false,
//         uint32_t num_threads = 4);
    
//     ~WorkerServer();
    
//     // Start the server (blocking)
//     void run();
    
//     // Stop the server
//     void stop();

// private:
//     std::string worker_id_;
//     std::string server_address_;
//     std::unique_ptr<WorkerServiceImpl> service_;
//     std::unique_ptr<grpc::Server> server_;
// };

// } // namespace worker
// } // namespace mlinference

// #endif // ML_INFERENCE_WORKER_SERVER_HPP