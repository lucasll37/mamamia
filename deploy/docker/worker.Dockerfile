# Worker Node Dockerfile
# Multi-stage build with optional GPU support

# Build stage
FROM nvidia/cuda:12.0.0-devel-ubuntu22.04 AS builder

ARG ENABLE_GPU=true

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    pkg-config \
    libssl-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Install gRPC and protobuf
RUN apt-get update && apt-get install -y \
    libgrpc++-dev \
    libprotobuf-dev \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

# Install other C++ dependencies
RUN apt-get update && apt-get install -y \
    libspdlog-dev \
    libyaml-cpp-dev \
    && rm -rf /var/lib/apt/lists/*

# Install ONNX Runtime
WORKDIR /tmp
RUN if [ "$ENABLE_GPU" = "true" ]; then \
        wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-gpu-1.16.3.tgz && \
        tar -xzf onnxruntime-linux-x64-gpu-1.16.3.tgz && \
        cp -r onnxruntime-linux-x64-gpu-1.16.3/include/* /usr/local/include/ && \
        cp -r onnxruntime-linux-x64-gpu-1.16.3/lib/* /usr/local/lib/; \
    else \
        wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz && \
        tar -xzf onnxruntime-linux-x64-1.16.3.tgz && \
        cp -r onnxruntime-linux-x64-1.16.3/include/* /usr/local/include/ && \
        cp -r onnxruntime-linux-x64-1.16.3/lib/* /usr/local/lib/; \
    fi && \
    ldconfig

# Set working directory
WORKDIR /build

# Copy source code
COPY proto/ ./proto/
COPY cpp/ ./cpp/
COPY cmake/ ./cmake/

# Generate protobuf files
RUN mkdir -p /build/generated && \
    cd proto && \
    protoc --cpp_out=/build/generated \
           --grpc_out=/build/generated \
           --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
           *.proto

# Build Worker
RUN mkdir -p cpp/build && cd cpp/build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_MANAGER=OFF \
          -DBUILD_WORKER=ON \
          -DBUILD_CLIENT=OFF \
          -DENABLE_GPU=${ENABLE_GPU} \
          .. && \
    make -j$(nproc)

# Runtime stage
FROM nvidia/cuda:12.0.0-runtime-ubuntu22.04

ARG ENABLE_GPU=true

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libgrpc++1.45 \
    libprotobuf23 \
    libspdlog1 \
    libyaml-cpp0.7 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install ONNX Runtime (runtime only)
WORKDIR /tmp
RUN if [ "$ENABLE_GPU" = "true" ]; then \
        apt-get update && apt-get install -y wget && \
        wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-gpu-1.16.3.tgz && \
        tar -xzf onnxruntime-linux-x64-gpu-1.16.3.tgz && \
        cp -r onnxruntime-linux-x64-gpu-1.16.3/lib/* /usr/local/lib/ && \
        rm -rf onnxruntime-linux-x64-gpu-1.16.3*; \
    else \
        apt-get update && apt-get install -y wget && \
        wget https://github.com/microsoft/onnxruntime/releases/download/v1.16.3/onnxruntime-linux-x64-1.16.3.tgz && \
        tar -xzf onnxruntime-linux-x64-1.16.3.tgz && \
        cp -r onnxruntime-linux-x64-1.16.3/lib/* /usr/local/lib/ && \
        rm -rf onnxruntime-linux-x64-1.16.3*; \
    fi && \
    ldconfig && \
    apt-get remove -y wget && \
    apt-get autoremove -y && \
    rm -rf /var/lib/apt/lists/*

# Create app directory
RUN mkdir -p /app/bin /app/data /models /models_cache /etc/ml-inference

# Copy binary from builder
COPY --from=builder /build/cpp/build/bin/worker /app/bin/

# Copy default configuration
COPY configs/worker_config.yaml /etc/ml-inference/worker_config.yaml

# Set working directory
WORKDIR /app

# Create non-root user
RUN useradd -m -u 1000 -s /bin/bash mluser && \
    chown -R mluser:mluser /app /models /models_cache /etc/ml-inference
USER mluser

# Expose ports
EXPOSE 50052

# Environment variables
ENV ENABLE_GPU=${ENABLE_GPU}
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=10s --retries=3 \
    CMD /app/bin/worker --health-check || exit 1

# Set entrypoint
ENTRYPOINT ["/app/bin/worker"]
CMD ["--config", "/etc/ml-inference/worker_config.yaml"]
