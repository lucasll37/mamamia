#ifndef ML_INFERENCE_CLIENT_HPP
#define ML_INFERENCE_CLIENT_HPP

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <grpcpp/grpcpp.h>
#include "worker.grpc.pb.h"
#include "common.pb.h"

namespace mlinference {
namespace client {

struct PredictionResult {
    bool success;
    std::map<std::string, std::vector<float>> outputs;
    double inference_time_ms;
    std::string error_message;
};

class InferenceClient {
public:
    explicit InferenceClient(const std::string& server_address);
    ~InferenceClient();
    
    // Connection management
    bool connect();
    bool is_connected() const;
    
    // Model management
    bool load_model(const std::string& model_id,
                   const std::string& model_path,
                   const std::string& version = "1.0.0");
    
    bool unload_model(const std::string& model_id);
    
    std::vector<std::string> list_loaded_models();
    
    // Inference
    PredictionResult predict(const std::string& model_id,
                            const std::map<std::string, std::vector<float>>& inputs,
                            const std::string& version = "");
    
    std::vector<PredictionResult> batch_predict(
        const std::string& model_id,
        const std::vector<std::map<std::string, std::vector<float>>>& batch_inputs,
        const std::string& version = "");
    
    // Health and status
    bool health_check();
    
    struct WorkerStatus {
        std::string worker_id;
        uint64_t total_requests;
        uint64_t successful_requests;
        uint64_t failed_requests;
        uint32_t active_requests;
        int64_t uptime_seconds;
        std::vector<std::string> loaded_models;
    };
    
    WorkerStatus get_status();

private:
    std::string server_address_;
    bool connected_;
    
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<mlinference::worker::WorkerService::Stub> stub_;
    
    // Helper methods
    void set_tensor_data(mlinference::common::Tensor* tensor,
                        const std::vector<float>& data);
    
    std::vector<float> get_tensor_data(const mlinference::common::Tensor& tensor);
};

} // namespace client
} // namespace mlinference

#endif // ML_INFERENCE_CLIENT_HPP