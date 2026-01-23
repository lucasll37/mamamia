#!/bin/bash

# ============================================
# Docker Build Script for Worker
# ============================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
IMAGE_NAME="${IMAGE_NAME:-ml-inference-worker}"
IMAGE_TAG="${IMAGE_TAG:-latest}"
ENABLE_GPU="${ENABLE_GPU:-False}"
ONNX_VERSION="${ONNX_VERSION:-1.23.2}"
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Building ML Inference Worker Docker Image${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "Configuration:"
echo "  Image Name:    ${IMAGE_NAME}"
echo "  Image Tag:     ${IMAGE_TAG}"
echo "  GPU Support:   ${ENABLE_GPU}"
echo "  ONNX Version:  ${ONNX_VERSION}"
echo "  Build Type:    ${CMAKE_BUILD_TYPE}"
echo ""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --gpu)
            ENABLE_GPU="True"
            shift
            ;;
        --cpu)
            ENABLE_GPU="False"
            shift
            ;;
        --tag)
            IMAGE_TAG="$2"
            shift 2
            ;;
        --name)
            IMAGE_NAME="$2"
            shift 2
            ;;
        --onnx-version)
            ONNX_VERSION="$2"
            shift 2
            ;;
        --debug)
            CMAKE_BUILD_TYPE="Debug"
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --gpu                Enable GPU support (default: false)"
            echo "  --cpu                Disable GPU support"
            echo "  --tag TAG            Docker image tag (default: latest)"
            echo "  --name NAME          Docker image name (default: ml-inference-worker)"
            echo "  --onnx-version VER   ONNX Runtime version (default: 1.23.2)"
            echo "  --debug              Build in Debug mode (default: Release)"
            echo "  --help, -h           Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0 --cpu                          # Build CPU version"
            echo "  $0 --gpu --tag v1.0.0            # Build GPU version with tag"
            echo "  $0 --debug                        # Build Debug version"
            echo ""
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Update configuration after parsing
if [ "$ENABLE_GPU" = "True" ]; then
    IMAGE_TAG="${IMAGE_TAG}-gpu"
    RUNTIME_STAGE="runtime-gpu"
else
    IMAGE_TAG="${IMAGE_TAG}-cpu"
    RUNTIME_STAGE="runtime-cpu"
fi

FULL_IMAGE_NAME="${IMAGE_NAME}:${IMAGE_TAG}"

echo -e "${YELLOW}Final Configuration:${NC}"
echo "  Full Image:    ${FULL_IMAGE_NAME}"
echo "  Runtime:       ${RUNTIME_STAGE}"
echo ""

# Check if Dockerfile exists
if [ ! -f "Dockerfile.worker" ]; then
    echo -e "${RED}Error: Dockerfile.worker not found${NC}"
    echo "Please run this script from the project root directory"
    exit 1
fi

# Check if required files exist
echo -e "${BLUE}Checking required files...${NC}"
required_files=(
    "CMakeLists.txt"
    "conanfile.py"
    "proto/common.proto"
    "proto/worker.proto"
    "cpp/worker/src/main.cpp"
)

for file in "${required_files[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "${RED}✗ Missing: $file${NC}"
        exit 1
    fi
    echo -e "${GREEN}✓ Found: $file${NC}"
done
echo ""

# Build Docker image
echo -e "${BLUE}Building Docker image...${NC}"
echo -e "${YELLOW}This may take several minutes on first build...${NC}"
echo ""

DOCKER_BUILDKIT=1 docker build \
    --file Dockerfile.worker \
    --target ${RUNTIME_STAGE} \
    --build-arg ENABLE_GPU=${ENABLE_GPU} \
    --build-arg ONNX_VERSION=${ONNX_VERSION} \
    --build-arg CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
    --tag ${FULL_IMAGE_NAME} \
    --progress=plain \
    .

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}✓ Build Successful!${NC}"
    echo -e "${GREEN}========================================${NC}"
    echo ""
    echo "Image created: ${FULL_IMAGE_NAME}"
    echo ""
    echo "Image details:"
    docker images ${IMAGE_NAME} --format "table {{.Repository}}\t{{.Tag}}\t{{.Size}}\t{{.CreatedAt}}"
    echo ""
    echo -e "${YELLOW}Next steps:${NC}"
    echo ""
    echo "1. Test the image:"
    echo "   docker run --rm ${FULL_IMAGE_NAME} --help"
    echo ""
    echo "2. Run the worker:"
    if [ "$ENABLE_GPU" = "true" ]; then
        echo "   docker run --rm --gpus all -p 50052:50052 ${FULL_IMAGE_NAME}"
    else
        echo "   docker run --rm -p 50052:50052 ${FULL_IMAGE_NAME}"
    fi
    echo ""
    echo "3. Run with custom options:"
    echo "   docker run --rm -p 50052:50052 ${FULL_IMAGE_NAME} \\"
    echo "     --id worker-1 \\"
    echo "     --address 0.0.0.0:50052 \\"
    echo "     --threads 4"
    echo ""
    echo "4. Run with mounted model directory:"
    echo "   docker run --rm -p 50052:50052 \\"
    echo "     -v \$(pwd)/models:/models:ro \\"
    echo "     ${FULL_IMAGE_NAME}"
    echo ""
    
    # Tag as latest if building a versioned image
    if [[ "$IMAGE_TAG" != "latest"* ]]; then
        LATEST_TAG="${IMAGE_NAME}:latest"
        if [ "$ENABLE_GPU" = "true" ]; then
            LATEST_TAG="${IMAGE_NAME}:latest-gpu"
        else
            LATEST_TAG="${IMAGE_NAME}:latest-cpu"
        fi
        
        echo -e "${BLUE}Tagging as ${LATEST_TAG}...${NC}"
        docker tag ${FULL_IMAGE_NAME} ${LATEST_TAG}
        echo -e "${GREEN}✓ Tagged as ${LATEST_TAG}${NC}"
        echo ""
    fi
else
    echo ""
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}✗ Build Failed${NC}"
    echo -e "${RED}========================================${NC}"
    echo ""
    echo "Check the error messages above for details."
    exit 1
fi