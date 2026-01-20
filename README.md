# ML Inference System

> **Distributed ML Model Inference System with Manager-Worker Architecture**

A production-ready, scalable distributed system for serving machine learning models with high performance and reliability.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Python 3.9+](https://img.shields.io/badge/Python-3.9+-blue.svg)](https://www.python.org/)

## 🚀 Features

### Architecture
- **Manager Node**: Orchestrates workers, handles model registry, API keys, and load balancing
- **Worker Nodes**: Execute ML inferences directly for clients (CPU/GPU support)
- **Direct Communication**: Clients connect to workers after discovery via manager
- **Horizontal Scalability**: Add workers dynamically without downtime

### Core Capabilities
- ✅ **ONNX Runtime**: Support for models from Keras, PyTorch, TensorFlow
- ✅ **GPU Acceleration**: CUDA support with automatic GPU memory management
- ✅ **Multi-Model Serving**: Workers can serve multiple models concurrently
- ✅ **Lazy Loading**: Models loaded on-demand, unloaded after timeout
- ✅ **Thread Pool**: Concurrent request handling with configurable workers
- ✅ **gRPC Binary Protocol**: High-performance binary communication
- ✅ **REST API Gateway**: FastAPI-based HTTP interface
- ✅ **Web Dashboard**: Real-time monitoring and model management
- ✅ **CLI Tool**: Complete command-line interface
- ✅ **Python SDK**: Easy-to-use client library
- ✅ **C++ Client**: High-performance native client

### Advanced Features
- 🔐 **API Key Authentication**: Granular permissions (inference, upload, admin)
- 📊 **Metrics & Monitoring**: Prometheus-compatible metrics
- 🏷️ **Model Versioning**: Semantic versioning with stages (dev/staging/prod)
- 📋 **Schema Validation**: JSON Schema for input/output validation
- 💓 **Health Checks**: Automatic worker monitoring with heartbeats
- ⚖️ **Load Balancing**: Intelligent worker assignment
- 🔄 **Failover**: Automatic fallback if worker fails

## 📋 Table of Contents

- [Quick Start](#-quick-start)
- [Architecture](#-architecture)
- [Installation](#-installation)
- [Configuration](#️-configuration)
- [Usage](#-usage)
- [Development](#-development)
- [Deployment](#-deployment)
- [API Reference](#-api-reference)
- [Contributing](#-contributing)

## 🎯 Quick Start

### Prerequisites
- Docker & Docker Compose (recommended)
- OR: C++17 compiler, CMake 3.20+, Python 3.9+

### Using Docker (Easiest)

```bash
# Clone repository
git clone https://github.com/yourusername/ml-inference-system.git
cd ml-inference-system

# Start all services
make quickstart

# System will be available at:
# - Manager:     localhost:50051 (gRPC)
# - API Gateway: http://localhost:8000
# - Web UI:      http://localhost:3000
# - Grafana:     http://localhost:3001
```

### Manual Installation

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install -y \
    build-essential cmake git \
    libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc \
    libspdlog-dev libyaml-cpp-dev libssl-dev

# Build C++ components
make build-cpp

# Install Python components
pip install -e python/

# Start Manager
./build/bin/manager --config configs/manager_config.yaml

# Start Worker (in another terminal)
./build/bin/worker --config configs/worker_config.yaml

# Start API Gateway (in another terminal)
cd python/api_gateway && python main.py
```

## 🏗️ Architecture

```
┌─────────────┐         ┌──────────────────┐
│   Client    │────1───▶│  Manager Node    │
│             │         │  (Orchestrator)  │
└─────────────┘         └──────────────────┘
       │                         │
       │                    ┌────┴────┐
       │                    │         │
       │              ┌─────▼───┐ ┌──▼──────┐
       └────2────────▶│ Worker  │ │ Worker  │
            (Direct)  │ Node 1  │ │ Node 2  │
                      └─────────┘ └─────────┘
```

**Flow:**
1. Client requests worker assignment from Manager
2. Manager returns optimal worker address
3. Client connects directly to Worker for inference
4. Worker streams results back to client

### Components

#### Manager Node (C++ + gRPC)
- Worker registry and health monitoring
- Model registry with versioning
- API key management
- Load balancing
- Metrics aggregation

#### Worker Node (C++ + gRPC + ONNX Runtime)
- Model loading/unloading (lazy + timeout)
- GPU/CPU inference execution
- Thread pool for concurrency
- Direct client connections
- Metrics reporting

#### API Gateway (Python + FastAPI)
- REST interface for HTTP clients
- WebSocket support for streaming
- Authentication middleware
- Request validation

#### Web UI (React + TypeScript)
- Real-time dashboard
- Model upload interface
- Inference playground
- Worker monitoring
- Schema visualization

#### CLI (Python + Click)
- Model management
- Worker status
- API key operations
- Configuration

## 📦 Installation

### System Requirements

**Minimum:**
- CPU: 4 cores
- RAM: 8 GB
- Storage: 20 GB

**Recommended (with GPU):**
- CPU: 8+ cores
- RAM: 16+ GB
- GPU: NVIDIA with 8+ GB VRAM
- Storage: 50+ GB

### Dependencies

#### C++ Dependencies
```bash
# Ubuntu/Debian
sudo apt-get install -y \
    cmake build-essential \
    libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc \
    libspdlog-dev libyaml-cpp-dev libssl-dev \
    libonnxruntime-dev

# GPU support (optional)
sudo apt-get install -y \
    nvidia-cuda-toolkit \
    libcudnn8 libcudnn8-dev
```

#### Python Dependencies
```bash
pip install -r python/requirements.txt
```

### Building from Source

```bash
# Generate protobuf files
make proto

# Build C++ (Manager + Worker + Client)
make build-cpp

# Build Python (API Gateway + CLI + SDK)
make build-python

# Build Web UI
make build-web

# Or build everything
make build
```

## ⚙️ Configuration

### Manager Configuration (`configs/manager_config.yaml`)

```yaml
server:
  address: "0.0.0.0"
  port: 50051
  max_workers: 100
  thread_pool_size: 4

worker:
  heartbeat_interval_seconds: 30
  heartbeat_timeout_seconds: 90
  max_missed_heartbeats: 3

storage:
  models_directory: "./models"
  database_path: "./manager.db"
  max_model_size_mb: 1024

security:
  enable_tls: false
  admin_api_key: "your-secure-admin-key"

logging:
  log_file: "./manager.log"
  log_level: "info"
```

### Worker Configuration (`configs/worker_config.yaml`)

```yaml
server:
  address: "0.0.0.0"
  port: 50052
  thread_pool_size: 8
  max_concurrent_requests: 10

manager:
  address: "localhost"
  port: 50051
  heartbeat_interval_seconds: 30

inference:
  enable_gpu: true
  gpu_device_id: 0
  num_cpu_threads: 4
  model_unload_timeout_seconds: 300
  max_memory_usage_mb: 4096

storage:
  models_cache_directory: "./models_cache"
  max_cache_size_mb: 10240

logging:
  log_file: "./worker.log"
  log_level: "info"
```

## 💻 Usage

### Python SDK

```python
from sdk import InferenceClient
import numpy as np

# Create client
client = InferenceClient(
    manager_address="localhost:50051",
    api_key="your-api-key"
)

# Single inference
result = client.predict(
    model_id="my-model",
    inputs={
        "input": np.array([[1.0, 2.0, 3.0, 4.0]])
    },
    model_version="1.0.0"
)

print(f"Output: {result.outputs}")
print(f"Inference time: {result.inference_time_ms:.2f}ms")

# Batch inference
results = client.batch_predict(
    model_id="my-model",
    batch_inputs=[
        {"input": np.array([[1.0, 2.0, 3.0, 4.0]])},
        {"input": np.array([[5.0, 6.0, 7.0, 8.0]])},
    ]
)

# List models
models = client.list_models(stage="prod")
for model in models:
    print(f"{model.name} v{model.version}")

# Close connection
client.close()
```

### CLI

```bash
# Upload model
ml-inference model upload \
    --file model.onnx \
    --name "my-classifier" \
    --version "1.0.0" \
    --stage "prod" \
    --description "Image classifier"

# List models
ml-inference model list

# Get model info
ml-inference model info my-classifier --version 1.0.0

# Update model stage
ml-inference model update my-classifier --version 1.0.0 --stage prod

# Delete model
ml-inference model delete my-classifier --version 1.0.0

# List workers
ml-inference worker list

# Get worker status
ml-inference worker status worker-1

# Create API key
ml-inference apikey create \
    --name "prod-api-key" \
    --permissions inference,model_upload \
    --rate-limit 1000

# List API keys
ml-inference apikey list

# System status
ml-inference status
```

### REST API

```bash
# Health check
curl http://localhost:8000/health

# List models
curl http://localhost:8000/api/v1/models \
    -H "X-API-Key: your-api-key"

# Predict
curl -X POST http://localhost:8000/api/v1/inference/predict \
    -H "Content-Type: application/json" \
    -H "X-API-Key: your-api-key" \
    -d '{
        "model_id": "my-model",
        "model_version": "1.0.0",
        "inputs": {
            "input": [[1.0, 2.0, 3.0, 4.0]]
        }
    }'

# Upload model
curl -X POST http://localhost:8000/api/v1/models/upload \
    -H "X-API-Key: your-api-key" \
    -F "file=@model.onnx" \
    -F "metadata={\"name\":\"my-model\",\"version\":\"1.0.0\"}"
```

### C++ Client

```cpp
#include <client/inference_client.hpp>

int main() {
    // Create client
    mlinference::client::InferenceClient client(
        "localhost:50051",
        "your-api-key"
    );
    
    // Prepare inputs
    std::map<std::string, Tensor> inputs;
    inputs["input"] = create_tensor({1.0f, 2.0f, 3.0f, 4.0f});
    
    // Predict
    auto result = client.predict("my-model", inputs, "1.0.0");
    
    if (result.success) {
        std::cout << "Inference time: " 
                  << result.inference_time_ms << "ms" << std::endl;
    }
    
    return 0;
}
```

## 🔧 Development

### Project Structure
```
ml-inference-system/
├── proto/              # Protocol Buffer definitions
├── cpp/                # C++ implementation
│   ├── common/         # Shared utilities
│   ├── manager/        # Manager node
│   ├── worker/         # Worker node
│   └── client/         # C++ client library
├── python/             # Python components
│   ├── api_gateway/    # FastAPI REST gateway
│   ├── cli/            # CLI tool
│   └── sdk/            # Python client SDK
├── web/                # React web UI
├── deploy/             # Deployment configs
├── tests/              # Test suites
└── docs/               # Documentation
```

### Running Tests

```bash
# C++ tests
make test-cpp

# Python tests
make test-python

# All tests
make test
```

### Code Formatting

```bash
# Format C++
make format-cpp

# Format Python
make format-python

# Lint
make lint-cpp lint-python
```

## 🚀 Deployment

### Docker Compose (Development)

```bash
docker compose up -d
```

### Kubernetes (Production)

```bash
# Apply configurations
kubectl apply -f deploy/k8s/

# Check status
kubectl get pods -n ml-inference

# Scale workers
kubectl scale deployment worker --replicas=5 -n ml-inference
```

### Production Checklist

- [ ] Configure TLS certificates
- [ ] Set strong API keys
- [ ] Enable authentication
- [ ] Configure resource limits
- [ ] Setup monitoring (Prometheus + Grafana)
- [ ] Configure log aggregation
- [ ] Setup backup for model registry
- [ ] Configure autoscaling
- [ ] Test failover scenarios

## 📚 API Reference

See full API documentation at:
- **REST API**: http://localhost:8000/docs (Swagger UI)
- **gRPC**: See `proto/*.proto` files
- **Python SDK**: `docs/python_sdk.md`
- **C++ Client**: `docs/cpp_client.md`

## 🤝 Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## 📄 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- ONNX Runtime team
- gRPC project
- FastAPI framework
- React community

## 📞 Support

- Issues: [GitHub Issues](https://github.com/yourusername/ml-inference-system/issues)
- Discussions: [GitHub Discussions](https://github.com/yourusername/ml-inference-system/discussions)
- Email: support@yourdomain.com

---

**Made with ❤️ by [Your Name]**
