#!/bin/bash

# ML Inference System - Build Script
# Builds all components of the system

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-build}"
ENABLE_GPU="${ENABLE_GPU:-ON}"
NUM_JOBS="${NUM_JOBS:-$(nproc)}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ML Inference System - Build Script${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "${YELLOW}Configuration:${NC}"
echo -e "  Build Type: ${BUILD_TYPE}"
echo -e "  Build Directory: ${BUILD_DIR}"
echo -e "  GPU Support: ${ENABLE_GPU}"
echo -e "  Parallel Jobs: ${NUM_JOBS}"
echo ""

# Function to print step
print_step() {
    echo ""
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
}

# Function to print success
print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

# Function to print error
print_error() {
    echo -e "${RED}✗ $1${NC}"
}

# Check prerequisites
print_step "Checking Prerequisites"

command -v cmake >/dev/null 2>&1 || {
    print_error "CMake is required but not installed"
    exit 1
}
print_success "CMake found"

command -v protoc >/dev/null 2>&1 || {
    print_error "protoc is required but not installed"
    exit 1
}
print_success "protoc found"

command -v python3 >/dev/null 2>&1 || {
    print_error "Python 3 is required but not installed"
    exit 1
}
print_success "Python 3 found"

command -v node >/dev/null 2>&1 || {
    print_error "Node.js is required but not installed"
    exit 1
}
print_success "Node.js found"

# Generate Protocol Buffer files
print_step "Generating Protocol Buffer Files"

PROTO_DIR="proto"
mkdir -p "${BUILD_DIR}/generated"

# Generate C++ files
protoc --cpp_out="${BUILD_DIR}/generated" \
       --grpc_out="${BUILD_DIR}/generated" \
       --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) \
       -I"${PROTO_DIR}" \
       "${PROTO_DIR}"/*.proto

print_success "C++ protobuf files generated"

# Generate Python files
python3 -m grpc_tools.protoc \
    -I"${PROTO_DIR}" \
    --python_out=python \
    --grpc_python_out=python \
    "${PROTO_DIR}"/*.proto

print_success "Python protobuf files generated"

# Build C++ components
print_step "Building C++ Components"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
      -DBUILD_MANAGER=ON \
      -DBUILD_WORKER=ON \
      -DBUILD_CLIENT=ON \
      -DENABLE_GPU="${ENABLE_GPU}" \
      ../cpp

make -j"${NUM_JOBS}"

cd ..

print_success "C++ build complete"
echo -e "  Binaries: ${BUILD_DIR}/bin/"
echo -e "  Libraries: ${BUILD_DIR}/lib/"

# Build Python components
print_step "Building Python Components"

cd python

# Create virtual environment if it doesn't exist
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
fi

# Activate virtual environment
source venv/bin/activate

# Install dependencies
pip install --upgrade pip
pip install -r requirements.txt

# Install in development mode
pip install -e .

deactivate
cd ..

print_success "Python build complete"

# Build Web UI
print_step "Building Web UI"

cd web

# Install dependencies
npm install

# Build for production
npm run build

cd ..

print_success "Web UI build complete"
echo -e "  Output: web/dist/"

# Create output summary
print_step "Build Summary"

echo ""
echo -e "${GREEN}✓ Build completed successfully!${NC}"
echo ""
echo -e "${YELLOW}Executables:${NC}"
echo -e "  Manager:  ${BUILD_DIR}/bin/manager"
echo -e "  Worker:   ${BUILD_DIR}/bin/worker"
echo ""
echo -e "${YELLOW}Python:${NC}"
echo -e "  API Gateway: python/venv/bin/ml-inference-api"
echo -e "  CLI:         python/venv/bin/ml-inference"
echo ""
echo -e "${YELLOW}Web UI:${NC}"
echo -e "  Production build: web/dist/"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo -e "  1. Configure: Edit configs/*.yaml files"
echo -e "  2. Run Manager: ${BUILD_DIR}/bin/manager --config configs/manager_config.yaml"
echo -e "  3. Run Worker: ${BUILD_DIR}/bin/worker --config configs/worker_config.yaml"
echo -e "  4. Run API: source python/venv/bin/activate && ml-inference-api"
echo -e "  5. Or use Docker: docker compose up"
echo ""

exit 0
