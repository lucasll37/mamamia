#!/bin/bash

# ============================================
# Docker Build Script for Client
# ============================================

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
IMAGE_NAME="${IMAGE_NAME:-ml-inference-client}"
IMAGE_TAG="${IMAGE_TAG:-latest}"
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Building ML Inference Client Docker Image${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "Configuration:"
echo "  Image Name:    ${IMAGE_NAME}"
echo "  Image Tag:     ${IMAGE_TAG}"
echo "  Build Type:    ${CMAKE_BUILD_TYPE}"
echo ""

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --tag)
            IMAGE_TAG="$2"
            shift 2
            ;;
        --name)
            IMAGE_NAME="$2"
            shift 2
            ;;
        --debug)
            CMAKE_BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            CMAKE_BUILD_TYPE="Release"
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --tag TAG            Docker image tag (default: latest)"
            echo "  --name NAME          Docker image name (default: ml-inference-client)"
            echo "  --debug              Build in Debug mode"
            echo "  --release            Build in Release mode (default)"
            echo "  --help, -h           Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                                   # Build with defaults"
            echo "  $0 --tag v1.0.0                     # Build with specific tag"
            echo "  $0 --debug                           # Build Debug version"
            echo "  $0 --tag v2.0.0 --name my-client    # Custom name and tag"
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

FULL_IMAGE_NAME="${IMAGE_NAME}:${IMAGE_TAG}"

echo -e "${YELLOW}Final Configuration:${NC}"
echo "  Full Image:    ${FULL_IMAGE_NAME}"
echo "  Build Type:    ${CMAKE_BUILD_TYPE}"
echo ""

# Check if Dockerfile exists
if [ ! -f "./docker/Dockerfile.client" ]; then
    echo -e "${RED}Error: ./docker/Dockerfile.client not found${NC}"
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
    "cpp/client/src/main.cpp"
    "cpp/client/src/inference_client.cpp"
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
    --file ./docker/Dockerfile.client \
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
    echo "2. Run the client (with worker on localhost):"
    echo "   docker run --rm --network host ${FULL_IMAGE_NAME} localhost:50052"
    echo ""
    echo "3. Run with custom server address:"
    echo "   docker run --rm --network host ${FULL_IMAGE_NAME} worker.example.com:50052"
    echo ""
    echo "4. Run in a Docker network:"
    echo "   # First, ensure worker is running in the same network"
    echo "   docker run --rm --network ml-inference-net ${FULL_IMAGE_NAME} worker:50052"
    echo ""
    echo "5. Using Docker Compose:"
    echo "   docker-compose run --rm client worker-cpu:50052"
    echo ""
    
    # Inspect binary
    echo -e "${BLUE}Inspecting binary...${NC}"
    echo ""
    echo "Binary information:"
    docker run --rm ${FULL_IMAGE_NAME} sh -c "ls -lh /app/bin/client_example"
    echo ""
    echo "Library dependencies:"
    docker run --rm ${FULL_IMAGE_NAME} sh -c "ldd /app/bin/client_example" || true
    echo ""
    
    # Tag as latest if building a versioned image
    if [[ "$IMAGE_TAG" != "latest" ]]; then
        LATEST_TAG="${IMAGE_NAME}:latest"
        
        echo -e "${BLUE}Tagging as ${LATEST_TAG}...${NC}"
        docker tag ${FULL_IMAGE_NAME} ${LATEST_TAG}
        echo -e "${GREEN}✓ Tagged as ${LATEST_TAG}${NC}"
        echo ""
    fi
    
    # Show all tags
    echo "All client images:"
    docker images ${IMAGE_NAME}
    echo ""
    
    # Quick test suggestion
    echo -e "${YELLOW}Quick Test:${NC}"
    echo ""
    echo "To test the complete system:"
    echo "  # Terminal 1: Start worker"
    echo "  docker run --rm -p 50052:50052 ml-inference-worker:latest-cpu"
    echo ""
    echo "  # Terminal 2: Run client"
    echo "  docker run --rm --network host ${FULL_IMAGE_NAME} localhost:50052"
    echo ""
else
    echo ""
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}✗ Build Failed${NC}"
    echo -e "${RED}========================================${NC}"
    echo ""
    echo "Check the error messages above for details."
    echo ""
    echo "Common issues:"
    echo "  - Missing source files (check required_files list above)"
    echo "  - Docker daemon not running"
    echo "  - Insufficient disk space"
    echo "  - Network issues downloading dependencies"
    echo ""
    exit 1
fi