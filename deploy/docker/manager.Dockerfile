# Manager Node Dockerfile
# Multi-stage build for optimized production image

# Build stage
FROM ubuntu:22.04 AS builder

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

# Build Manager
RUN mkdir -p cpp/build && cd cpp/build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_MANAGER=ON \
          -DBUILD_WORKER=OFF \
          -DBUILD_CLIENT=OFF \
          -DENABLE_GPU=OFF \
          .. && \
    make -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libgrpc++1.45 \
    libprotobuf23 \
    libspdlog1 \
    libyaml-cpp0.7 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
RUN mkdir -p /app/bin /app/data /etc/ml-inference

# Copy binary from builder
COPY --from=builder /build/cpp/build/bin/manager /app/bin/

# Copy default configuration
COPY configs/manager_config.yaml /etc/ml-inference/manager_config.yaml

# Set working directory
WORKDIR /app

# Create non-root user
RUN useradd -m -u 1000 -s /bin/bash mluser && \
    chown -R mluser:mluser /app /etc/ml-inference
USER mluser

# Expose ports
EXPOSE 50051
EXPOSE 9090

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD /app/bin/manager --health-check || exit 1

# Set entrypoint
ENTRYPOINT ["/app/bin/manager"]
CMD ["--config", "/etc/ml-inference/manager_config.yaml"]
