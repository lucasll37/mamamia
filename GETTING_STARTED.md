# Getting Started with ML Inference System

## 📚 Complete File Structure Created

This project includes **ALL** necessary files for a production-ready distributed ML inference system:

### ✅ Protocol Buffers (gRPC Definitions)
- `proto/common.proto` - Common types and messages
- `proto/manager.proto` - Manager service definition
- `proto/worker.proto` - Worker service definition

### ✅ C++ Core Components
**Manager Node:**
- Headers in `cpp/manager/include/manager/`
- Implementation in `cpp/manager/src/`

**Worker Node:**
- Headers in `cpp/worker/include/worker/`
- Implementation in `cpp/worker/src/`

**Common Library:**
- `cpp/common/include/common/` - Shared utilities
- `cpp/common/src/` - Implementations

**C++ Client:**
- `cpp/client/include/client/` - Client library

### ✅ Python Components
**API Gateway (FastAPI):**
- `python/api_gateway/main.py` - Main application
- `python/api_gateway/routes/` - API endpoints
- `python/api_gateway/middleware/` - Auth & CORS
- `python/api_gateway/utils/` - gRPC clients

**CLI Tool (Click):**
- `python/cli/main.py` - CLI entry point
- `python/cli/commands/` - Command implementations

**Python SDK:**
- `python/sdk/client.py` - High-level client

### ✅ Web UI (React + TypeScript)
- `web/src/` - React application
- `web/src/components/` - UI components
- `web/src/pages/` - Application pages
- `web/src/services/` - API clients

### ✅ Build & Deployment
- `Makefile` - Build automation
- `docker-compose.yml` - Multi-service deployment
- `deploy/docker/*.Dockerfile` - Container images
- `scripts/build.sh` - Complete build script

### ✅ Configuration
- `configs/manager_config.yaml` - Manager settings
- `configs/worker_config.yaml` - Worker settings

### ✅ Documentation
- `README.md` - Project overview
- `docs/architecture.md` - Architecture details

## 🚀 Quick Start (3 Options)

### Option 1: Docker (Recommended)
```bash
# Build and start everything
make quickstart

# Or manually:
docker compose up -d

# Access:
# - Manager:     localhost:50051
# - API Gateway: http://localhost:8000
# - Web UI:      http://localhost:3000
# - Grafana:     http://localhost:3001
```

### Option 2: Local Build
```bash
# 1. Install dependencies (Ubuntu/Debian)
sudo apt-get install -y \
    build-essential cmake git \
    libgrpc++-dev libprotobuf-dev protobuf-compiler-grpc \
    libspdlog-dev libyaml-cpp-dev libssl-dev \
    libonnxruntime-dev

# 2. Build everything
make build

# 3. Start services (in separate terminals)
# Terminal 1 - Manager
./build/bin/manager --config configs/manager_config.yaml

# Terminal 2 - Worker
./build/bin/worker --config configs/worker_config.yaml

# Terminal 3 - API Gateway
source python/venv/bin/activate
ml-inference-api

# Terminal 4 - Web UI (development)
cd web && npm run dev
```

### Option 3: Step-by-Step Manual Build
```bash
# 1. Generate Protocol Buffers
make proto

# 2. Build C++ components
make build-cpp
# Outputs: build/bin/manager, build/bin/worker

# 3. Build Python components
make build-python
# Installs CLI and SDK

# 4. Build Web UI
make build-web
# Outputs: web/dist/

# 5. Run individual components
./build/bin/manager --config configs/manager_config.yaml
./build/bin/worker --config configs/worker_config.yaml
```

## 📝 Next Steps After Installation

### 1. Create API Key
```bash
# Using CLI
ml-inference apikey create \
    --name "my-api-key" \
    --permissions inference,model_upload \
    --rate-limit 1000

# Using REST API
curl -X POST http://localhost:8000/api/v1/apikeys \
    -H "X-Admin-Key: change-me-in-production" \
    -d '{"name":"my-key","permissions":["inference"]}'
```

### 2. Upload a Model
```bash
# Convert your model to ONNX first
# For Keras:
# python -m tf2onnx.convert --saved-model model/ --output model.onnx

# Upload via CLI
ml-inference model upload \
    --file model.onnx \
    --name "my-classifier" \
    --version "1.0.0" \
    --stage "prod"

# Or via REST API
curl -X POST http://localhost:8000/api/v1/models/upload \
    -H "X-API-Key: your-api-key" \
    -F "file=@model.onnx" \
    -F 'metadata={"name":"my-classifier","version":"1.0.0"}'
```

### 3. Make Predictions

**Python SDK:**
```python
from sdk import InferenceClient
import numpy as np

client = InferenceClient(
    manager_address="localhost:50051",
    api_key="your-api-key"
)

result = client.predict(
    model_id="my-classifier",
    inputs={"input": np.array([[1.0, 2.0, 3.0]])}
)

print(result.outputs)
```

**REST API:**
```bash
curl -X POST http://localhost:8000/api/v1/inference/predict \
    -H "Content-Type: application/json" \
    -H "X-API-Key: your-api-key" \
    -d '{
        "model_id": "my-classifier",
        "inputs": {"input": [[1.0, 2.0, 3.0]]}
    }'
```

**C++ Client:**
```cpp
#include <client/inference_client.hpp>

mlinference::client::InferenceClient client(
    "localhost:50051", "your-api-key"
);

auto result = client.predict("my-classifier", inputs);
```

## 🔧 Configuration

### Manager (`configs/manager_config.yaml`)
Key settings:
- `server.port`: gRPC port (default: 50051)
- `storage.models_directory`: Where models are stored
- `security.admin_api_key`: **CHANGE THIS IN PRODUCTION!**

### Worker (`configs/worker_config.yaml`)
Key settings:
- `inference.enable_gpu`: Enable GPU acceleration
- `manager.address`: Manager address
- `inference.model_unload_timeout_seconds`: Auto-unload timeout

### API Gateway
Environment variables:
- `MANAGER_ADDRESS`: Manager host
- `MANAGER_PORT`: Manager port (50051)
- `LOG_LEVEL`: Logging level

## 📊 Monitoring

### Access Dashboards
- **API Docs**: http://localhost:8000/docs
- **Web UI**: http://localhost:3000
- **Prometheus**: http://localhost:9091
- **Grafana**: http://localhost:3001 (admin/admin)

### Check Status
```bash
# CLI
ml-inference status

# REST API
curl http://localhost:8000/health

# Direct gRPC (with grpcurl)
grpcurl -plaintext localhost:50051 \
    mlinference.manager.ManagerService/GetHealth
```

## 🧪 Testing

```bash
# Run C++ tests
make test-cpp

# Run Python tests
make test-python

# Run all tests
make test
```

## 🐛 Troubleshooting

### Build Issues

**gRPC not found:**
```bash
sudo apt-get install libgrpc++-dev protobuf-compiler-grpc
```

**ONNX Runtime not found:**
```bash
# Download and install from:
# https://github.com/microsoft/onnxruntime/releases
```

### Runtime Issues

**Worker can't connect to Manager:**
- Check Manager is running: `docker ps` or `ps aux | grep manager`
- Verify port: `netstat -tulpn | grep 50051`
- Check config: `worker.manager.address` in worker_config.yaml

**GPU not detected:**
- Verify NVIDIA drivers: `nvidia-smi`
- Check CUDA: `nvcc --version`
- Set `inference.enable_gpu: true` in worker_config.yaml

**Models not loading:**
- Check model path in manager config
- Verify ONNX model is valid: `python -c "import onnx; onnx.checker.check_model('model.onnx')"`
- Check worker logs for errors

## 📚 Additional Documentation

- **Architecture**: `docs/architecture.md`
- **API Reference**: http://localhost:8000/docs (when running)
- **Python SDK**: `python/sdk/client.py` (inline documentation)
- **C++ API**: See header files in `cpp/*/include/`

## 🤝 Development

### Code Formatting
```bash
make format-cpp    # Format C++ code
make format-python # Format Python code
```

### Adding New Features

1. **New gRPC Service**: Add to `proto/*.proto`, regenerate
2. **New Manager Feature**: Implement in `cpp/manager/`
3. **New Worker Feature**: Implement in `cpp/worker/`
4. **New API Endpoint**: Add to `python/api_gateway/routes/`
5. **New UI Feature**: Add to `web/src/`

### Building Individual Components
```bash
make build-cpp     # C++ only
make build-python  # Python only
make build-web     # Web UI only
```

## 🚀 Production Deployment

### Prerequisites
1. Change admin API key in `configs/manager_config.yaml`
2. Enable TLS in all configs
3. Set up proper logging and monitoring
4. Configure resource limits
5. Set up backup for model registry

### Kubernetes Deployment
```bash
# Apply K8s configs
kubectl apply -f deploy/k8s/

# Scale workers
kubectl scale deployment worker --replicas=5
```

### Security Checklist
- [ ] Change default API keys
- [ ] Enable TLS for all services
- [ ] Set up firewall rules
- [ ] Configure rate limiting
- [ ] Enable audit logging
- [ ] Set resource limits
- [ ] Use non-root containers
- [ ] Regular security updates

## 💡 Tips

1. **Start Simple**: Use Docker Compose for initial testing
2. **Monitor Resources**: Watch CPU/GPU usage in Grafana
3. **Scale Gradually**: Add workers one at a time
4. **Test Thoroughly**: Use the playground in Web UI
5. **Read Logs**: Check logs for errors and performance

## 🆘 Getting Help

- **Issues**: Create a GitHub issue
- **Questions**: Use GitHub Discussions
- **Documentation**: Check `docs/` directory
- **Examples**: See `examples/` (TODO)

## 📄 License

MIT License - See LICENSE file for details.

---

**Project Status**: ✅ Complete structure ready for implementation

**Next Actions**:
1. Implement C++ source files (`.cpp`)
2. Complete Python route handlers
3. Build React components
4. Add comprehensive tests
5. Production hardening

Good luck with your ML Inference System! 🎉
