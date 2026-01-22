#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include "client/inference_client.hpp"

void print_separator() {
    std::cout << "========================================" << std::endl;
}

void print_vector(const std::string& name, const std::vector<float>& data) {
    std::cout << name << ": [";
    for (size_t i = 0; i < data.size() && i < 10; ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << data[i];
    }
    if (data.size() > 10) {
        std::cout << ", ... (" << data.size() << " total)";
    }
    std::cout << "]" << std::endl;
}

int main(int argc, char** argv) {
    std::cout << "==================================" << std::endl;
    std::cout << "ML Inference Client Example" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << std::endl;
    
    // Parse command line arguments
    std::string server_address = "localhost:50052";
    
    if (argc > 1) {
        server_address = argv[1];
    }
    
    std::cout << "Connecting to server: " << server_address << std::endl;
    
    // Create client
    mlinference::client::InferenceClient client(server_address);
    
    // Connect to server
    if (!client.connect()) {
        std::cerr << "Failed to connect to server" << std::endl;
        return 1;
    }
    
    print_separator();
    
    // Health check
    std::cout << "Performing health check..." << std::endl;
    if (client.health_check()) {
        std::cout << "✓ Server is healthy" << std::endl;
    } else {
        std::cout << "✗ Server health check failed" << std::endl;
    }
    
    print_separator();
    
    // Load a model
    std::cout << "Loading model..." << std::endl;
    std::string model_id = "test-model";
    std::string model_path = "/models/test_model.onnx";
    
    if (client.load_model(model_id, model_path)) {
        std::cout << "✓ Model loaded successfully" << std::endl;
    } else {
        std::cout << "✗ Failed to load model" << std::endl;
    }
    
    print_separator();
    
    // List loaded models
    std::cout << "Listing loaded models..." << std::endl;
    auto models = client.list_loaded_models();
    std::cout << "Loaded models (" << models.size() << "):" << std::endl;
    for (const auto& model : models) {
        std::cout << "  - " << model << std::endl;
    }
    
    print_separator();
    
    // Single prediction
    std::cout << "Testing single prediction..." << std::endl;
    
    // Create input data
    std::map<std::string, std::vector<float>> inputs;
    std::vector<float> input_data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    inputs["input"] = input_data;
    
    print_vector("Input", input_data);
    
    auto result = client.predict(model_id, inputs);
    
    if (result.success) {
        std::cout << "✓ Prediction successful" << std::endl;
        std::cout << "  Inference time: " << result.inference_time_ms << " ms" << std::endl;
        
        for (const auto& output_pair : result.outputs) {
            print_vector("Output (" + output_pair.first + ")", output_pair.second);
        }
    } else {
        std::cout << "✗ Prediction failed: " << result.error_message << std::endl;
    }
    
    print_separator();
    
    // Batch prediction
    std::cout << "Testing batch prediction..." << std::endl;
    
    std::vector<std::map<std::string, std::vector<float>>> batch_inputs;
    
    for (int i = 0; i < 3; ++i) {
        std::map<std::string, std::vector<float>> batch_input;
        std::vector<float> data(5);
        for (int j = 0; j < 5; ++j) {
            data[j] = static_cast<float>((i + 1) * (j + 1));
        }
        batch_input["input"] = data;
        batch_inputs.push_back(batch_input);
    }
    
    auto batch_results = client.batch_predict(model_id, batch_inputs);
    
    std::cout << "Batch results: " << batch_results.size() << " predictions" << std::endl;
    for (size_t i = 0; i < batch_results.size(); ++i) {
        if (batch_results[i].success) {
            std::cout << "  [" << i << "] ✓ " << batch_results[i].inference_time_ms << " ms" << std::endl;
        } else {
            std::cout << "  [" << i << "] ✗ " << batch_results[i].error_message << std::endl;
        }
    }
    
    print_separator();
    
    // Get worker status
    std::cout << "Getting worker status..." << std::endl;
    auto status = client.get_status();
    
    std::cout << "Worker ID: " << status.worker_id << std::endl;
    std::cout << "Uptime: " << status.uptime_seconds << " seconds" << std::endl;
    std::cout << "Total requests: " << status.total_requests << std::endl;
    std::cout << "Successful: " << status.successful_requests << std::endl;
    std::cout << "Failed: " << status.failed_requests << std::endl;
    std::cout << "Active: " << status.active_requests << std::endl;
    std::cout << "Loaded models: " << status.loaded_models.size() << std::endl;
    
    print_separator();
    
    // Stress test (optional)
    std::cout << "Running stress test (10 requests)..." << std::endl;
    
    auto stress_start = std::chrono::high_resolution_clock::now();
    
    int successful = 0;
    int failed = 0;
    
    for (int i = 0; i < 10; ++i) {
        auto test_result = client.predict(model_id, inputs);
        if (test_result.success) {
            successful++;
        } else {
            failed++;
        }
    }
    
    auto stress_end = std::chrono::high_resolution_clock::now();
    auto stress_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        stress_end - stress_start
    );
    
    std::cout << "Stress test completed:" << std::endl;
    std::cout << "  Successful: " << successful << std::endl;
    std::cout << "  Failed: " << failed << std::endl;
    std::cout << "  Total time: " << stress_duration.count() << " ms" << std::endl;
    std::cout << "  Avg time: " << (stress_duration.count() / 10.0) << " ms per request" << std::endl;
    
    print_separator();
    
    // Unload model
    std::cout << "Unloading model..." << std::endl;
    if (client.unload_model(model_id)) {
        std::cout << "✓ Model unloaded successfully" << std::endl;
    } else {
        std::cout << "✗ Failed to unload model" << std::endl;
    }
    
    print_separator();
    
    std::cout << "Client example completed!" << std::endl;
    
    return 0;
}