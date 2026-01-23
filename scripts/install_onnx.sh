#!/bin/bash

# Script to download and install ONNX Runtime
# Supports CPU and GPU versions

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
ONNX_VERSION="${ONNX_VERSION:-1.23.2}" 
INSTALL_DIR="${INSTALL_DIR:-/opt/onnxruntime}"
ENABLE_GPU="${ENABLE_GPU:-false}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ONNX Runtime Installation${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "Configuration:"
echo "  Version:     ${ONNX_VERSION}"
echo "  Install Dir: ${INSTALL_DIR}"
echo "  GPU Support: ${ENABLE_GPU}"
echo ""

# Detect OS and architecture
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

echo "Detected system:"
echo "  OS:   ${OS}"
echo "  Arch: ${ARCH}"
echo ""

# Determine download URL
if [ "${ENABLE_GPU}" = "true" ]; then
    if [ "${OS}" = "linux" ] && [ "${ARCH}" = "x86_64" ]; then
        DOWNLOAD_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-x64-gpu-${ONNX_VERSION}.tgz"
        PACKAGE_NAME="onnxruntime-linux-x64-gpu-${ONNX_VERSION}"
    else
        echo -e "${RED}GPU version not available for ${OS}-${ARCH}${NC}"
        exit 1
    fi
else
    if [ "${OS}" = "linux" ]; then
        if [ "${ARCH}" = "x86_64" ]; then
            DOWNLOAD_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-x64-${ONNX_VERSION}.tgz"
            PACKAGE_NAME="onnxruntime-linux-x64-${ONNX_VERSION}"
        elif [ "${ARCH}" = "aarch64" ]; then
            DOWNLOAD_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-linux-aarch64-${ONNX_VERSION}.tgz"
            PACKAGE_NAME="onnxruntime-linux-aarch64-${ONNX_VERSION}"
        else
            echo -e "${RED}Unsupported architecture: ${ARCH}${NC}"
            exit 1
        fi
    elif [ "${OS}" = "darwin" ]; then
        if [ "${ARCH}" = "x86_64" ]; then
            DOWNLOAD_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-osx-x86_64-${ONNX_VERSION}.tgz"
            PACKAGE_NAME="onnxruntime-osx-x86_64-${ONNX_VERSION}"
        elif [ "${ARCH}" = "arm64" ]; then
            DOWNLOAD_URL="https://github.com/microsoft/onnxruntime/releases/download/v${ONNX_VERSION}/onnxruntime-osx-arm64-${ONNX_VERSION}.tgz"
            PACKAGE_NAME="onnxruntime-osx-arm64-${ONNX_VERSION}"
        else
            echo -e "${RED}Unsupported architecture: ${ARCH}${NC}"
            exit 1
        fi
    else
        echo -e "${RED}Unsupported OS: ${OS}${NC}"
        exit 1
    fi
fi

echo "Download URL: ${DOWNLOAD_URL}"
echo ""

# Check if already installed
if [ -d "${INSTALL_DIR}" ]; then
    echo -e "${YELLOW}ONNX Runtime already installed at ${INSTALL_DIR}${NC}"
    read -p "Reinstall? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Installation cancelled"
        exit 0
    fi
    echo -e "${YELLOW}Removing existing installation...${NC}"
    sudo rm -rf "${INSTALL_DIR}"
fi

# Create temporary directory
TMP_DIR=$(mktemp -d)
cd "${TMP_DIR}"

echo -e "${BLUE}Downloading ONNX Runtime...${NC}"
wget -q --show-progress "${DOWNLOAD_URL}"

echo ""
echo -e "${BLUE}Extracting...${NC}"
tar -xzf "${PACKAGE_NAME}.tgz"

echo -e "${BLUE}Installing to ${INSTALL_DIR}...${NC}"
sudo mkdir -p "${INSTALL_DIR}"
sudo cp -r "${PACKAGE_NAME}"/* "${INSTALL_DIR}/"

# Update library cache (Linux only)
if [ "${OS}" = "linux" ]; then
    echo -e "${BLUE}Updating library cache...${NC}"
    echo "${INSTALL_DIR}/lib" | sudo tee /etc/ld.so.conf.d/onnxruntime.conf > /dev/null
    sudo ldconfig
fi

# Clean up
cd - > /dev/null
rm -rf "${TMP_DIR}"

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}✓ ONNX Runtime installed successfully${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo "Installation details:"
echo "  Location: ${INSTALL_DIR}"
echo "  Include:  ${INSTALL_DIR}/include"
echo "  Library:  ${INSTALL_DIR}/lib"
echo ""
echo "To build with ONNX Runtime:"
echo "  export ONNXRUNTIME_ROOT_DIR=${INSTALL_DIR}"
echo "  cd build && cmake -DONNXRUNTIME_ROOT_DIR=${INSTALL_DIR} .."
echo ""
echo "Or add to your build script:"
echo "  cmake -DONNXRUNTIME_ROOT_DIR=${INSTALL_DIR} .."
echo ""

# Verify installation
echo -e "${BLUE}Verifying installation...${NC}"
if [ -f "${INSTALL_DIR}/include/onnxruntime_cxx_api.h" ]; then
    echo -e "${GREEN}✓ Headers found${NC}"
else
    echo -e "${RED}✗ Headers not found${NC}"
fi

if [ -f "${INSTALL_DIR}/lib/libonnxruntime.so" ] || [ -f "${INSTALL_DIR}/lib/libonnxruntime.dylib" ]; then
    echo -e "${GREEN}✓ Library found${NC}"
else
    echo -e "${RED}✗ Library not found${NC}"
fi

echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "  1. Rebuild your project: make rebuild"
echo "  2. Run worker: ./build/bin/worker"
echo ""