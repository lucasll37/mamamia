# ML Inference System - Architecture Documentation

## Overview

The ML Inference System is a distributed, high-performance inference platform designed for serving machine learning models at scale. It uses a Manager-Worker architecture with direct client-to-worker communication for optimal performance.

## Architecture Diagram

```
┌──────────────────────────────────────────────────────────────────┐
│                         Client Layer                              │
├────────────────┬─────────────────┬───────────────┬───────────────┤
│  Python SDK    │  C++ Client     │  REST API     │  Web UI       │
└────────┬───────┴────────┬────────┴───────┬───────┴───────┬───────┘
         │                │                │               │
         └────────────────┴────────┬───────┘               │
                                   │                       │
                              1. Discovery                 │
                                   ↓                       │
         ┌─────────────────────────────────────────────────┘
         │                   Manager Node                   
         │            (Orchestration & Registry)            
         │  ┌───────────────────────────────────────────┐  
         │  │ - Worker Registry                         │  
         │  │ - Model Registry                          │  
         │  │ - API Key Management                      │  
         │  │ - Load Balancing                          │  
         │  │ - Health Monitoring                       │  
         │  └───────────────────────────────────────────┘  
         │                                                  
         └──────┬────────────────────────┬─────────────────
                │                        │                  
           2. Assign                3. Assign              
                │                        │                  
                ↓                        ↓                  
    ┌─────────────────────┐  ┌─────────────────────┐      
    │   Worker Node 1     │  │   Worker Node 2     │      
    │     (CPU/GPU)       │  │     (CPU/GPU)       │      
    ├─────────────────────┤  ├─────────────────────┤      
    │ - Inference Engine  │  │ - Inference Engine  │      
    │ - Model Loader      │  │ - Model Loader      │      
    │ - Thread Pool       │  │ - Thread Pool       │      
    │ - GPU Manager       │  │ - GPU Manager       │      
    └──────────┬──────────┘  └──────────┬──────────┘      
               │                        │                  
          3. Direct                3. Direct               
          Connection               Connection               
               ↓                        ↓                  
         ┌────────────────────────────────────┐           
         │        Client (Direct Stream)       │           
         └────────────────────────────────────┘           
```

## Communication Flow

### 1. Discovery Phase
1. Client sends request to Manager with model ID and requirements
2. Manager validates API key and permissions
3. Manager selects optimal worker using load balancing algorithm
4. Manager returns worker address and session token

### 2. Inference Phase
1. Client connects directly to assigned Worker
2. Client sends inference request with session token
3. Worker validates token and loads model (if not already loaded)
4. Worker executes inference on CPU/GPU
5. Worker streams results directly back to client

### 3. Monitoring Phase
1. Workers send periodic heartbeats to Manager
2. Workers report metrics (CPU, GPU, memory, latency)
3. Manager aggregates metrics for monitoring
4. Manager updates worker status and availability

## Components

### Manager Node

**Responsibilities:**
- Worker lifecycle management (registration, heartbeats, removal)
- Model registry with versioning
- API key authentication and authorization
- Load balancing and worker assignment
- Health monitoring and metrics aggregation

**Key Classes:**
- `ManagerServer`: Main gRPC server
- `WorkerRegistry`: Tracks active workers
- `ModelRegistry`: Manages model metadata
- `ApiKeyManager`: Handles authentication
- `LoadBalancer`: Assigns workers to clients
- `HealthMonitor`: Monitors system health

**Storage:**
- SQLite database for model metadata and API keys
- File system for model binary storage

### Worker Node

**Responsibilities:**
- Model loading and caching
- Inference execution (CPU/GPU)
- Direct client communication
- Resource management
- Metrics reporting

**Key Classes:**
- `WorkerServer`: Main gRPC server
- `InferenceEngine`: Executes inference using ONNX Runtime
- `ModelLoader`: Lazy loads models with timeout-based unloading
- `ThreadPool`: Manages concurrent requests
- `GPUManager`: Handles GPU memory and allocation
- `ManagerClient`: Communicates with Manager

**Optimization:**
- Lazy model loading (load on first request)
- Automatic unloading after configurable timeout
- GPU memory pooling
- Thread pool for concurrency
- Model caching

### API Gateway

**Responsibilities:**
- REST API interface for HTTP clients
- Request validation and transformation
- WebSocket support for streaming
- CORS handling
- Rate limiting

**Technology:**
- FastAPI (Python)
- Async/await for high concurrency
- Pydantic for validation
- gRPC client for Manager communication

### Web UI

**Responsibilities:**
- Real-time dashboard
- Model management interface
- Worker monitoring
- Inference playground
- Schema visualization

**Technology:**
- React + TypeScript
- Material-UI for components
- React Query for data fetching
- WebSocket for real-time updates
- Recharts for visualization

## Data Flow

### Model Upload
```
CLI/Web UI → API Gateway → Manager → File Storage
                                   └→ Database (metadata)
```

### Inference Request
```
Client → Manager (get worker)
Client → Worker (inference) → ONNX Runtime → GPU/CPU
Worker → Client (results)
```

### Metrics Collection
```
Worker → Manager (heartbeat + metrics)
Manager → Prometheus Exporter
Prometheus → Grafana
```

## Scalability

### Horizontal Scaling

**Workers:**
- Add more workers dynamically
- No downtime required
- Automatic registration with Manager
- Load balancer distributes requests

**Managers (future):**
- Multiple managers with shared state
- Consistent hashing for worker assignment
- Replicated model registry

### Vertical Scaling

**Worker Resources:**
- Increase GPU memory
- Add more CPU cores
- Increase thread pool size

**Manager Resources:**
- Increase database performance
- Add more handler threads

## Performance Optimizations

### Network
- gRPC binary protocol (vs REST/JSON)
- Direct client-worker communication
- Streaming for large payloads
- Connection pooling

### Compute
- GPU acceleration with CUDA
- Batch processing support
- Model optimization (graph optimization, quantization)
- Thread pool for parallelism

### Memory
- Lazy model loading
- Automatic model unloading
- GPU memory pooling
- Model caching

### Latency
- Direct worker communication (no proxy)
- Session affinity for subsequent requests
- Prefetch models on startup
- Asynchronous processing

## Security

### Authentication
- API key-based authentication
- Granular permissions (inference, upload, admin)
- Session tokens for worker communication
- Rate limiting per API key

### TLS/SSL
- Optional TLS for all gRPC connections
- Certificate-based authentication
- Mutual TLS support

### Isolation
- Non-root containers
- Resource limits (CPU, memory, GPU)
- Network isolation
- Separate model storage

## Monitoring

### Metrics
- Request count and latency (p50, p95, p99)
- CPU and memory usage
- GPU utilization and memory
- Model load/unload events
- Error rates

### Health Checks
- Manager health endpoint
- Worker health endpoint
- API Gateway health endpoint
- Model availability checks

### Logging
- Structured logging (JSON)
- Log levels (trace, debug, info, warn, error)
- Log rotation
- Centralized log aggregation (optional)

## Fault Tolerance

### Worker Failures
- Manager detects missed heartbeats
- Manager marks worker as offline
- Clients get reassigned to healthy workers
- Automatic cleanup of dead workers

### Manager Failures (single manager)
- Workers continue serving existing clients
- New clients cannot discover workers
- Recovery: restart manager, workers re-register

### Network Failures
- Automatic retry with exponential backoff
- Connection pooling with health checks
- Graceful degradation

## Deployment

### Docker Compose
- Single-command deployment
- Suitable for development and small deployments
- Services: Manager, Workers, API Gateway, Web UI

### Kubernetes
- Production-grade deployment
- Automatic scaling (HPA)
- Service discovery
- Load balancing
- Rolling updates

## Future Enhancements

### High Availability
- Multiple manager instances
- Distributed model registry
- Leader election (Raft/etcd)

### Advanced Features
- A/B testing support
- Canary deployments
- Model versioning with traffic splitting
- Automatic model retraining triggers

### Optimization
- Dynamic batching
- Model ensembles
- Quantization support
- Knowledge distillation

### Observability
- Distributed tracing (OpenTelemetry)
- Advanced metrics (RED, USE)
- Anomaly detection
- Predictive scaling

## References

- gRPC: https://grpc.io/
- ONNX Runtime: https://onnxruntime.ai/
- Protocol Buffers: https://protobuf.dev/
- FastAPI: https://fastapi.tiangolo.com/
