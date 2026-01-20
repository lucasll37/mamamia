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
