#!/bin/bash

# ML Inference System - Build Script com Conan

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Configuration
BUILD_DIR="${BUILD_DIR:-build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
NUM_JOBS="${NUM_JOBS:-$(nproc)}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}ML Inference - Build com Conan${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Check if Conan is installed
if ! command -v conan &> /dev/null; then
    echo -e "${RED}Error: Conan não está instalado!${NC}"
    echo "Instale com: pip3 install conan"
    exit 1
fi

echo -e "${GREEN}✓ Conan encontrado: $(conan --version)${NC}"
echo ""

# Configure Conan profile (only once)
if [ ! -f ~/.conan2/profiles/default ]; then
    echo -e "${YELLOW}Detectando perfil Conan...${NC}"
    conan profile detect --force
fi

echo -e "${YELLOW}Configuration:${NC}"
echo "  Build Directory: ${BUILD_DIR}"
echo "  Build Type: ${BUILD_TYPE}"
echo "  Parallel Jobs: ${NUM_JOBS}"
echo ""

# Clean previous build
if [ -d "${BUILD_DIR}" ]; then
    echo -e "${YELLOW}Limpando build anterior...${NC}"
    rm -rf "${BUILD_DIR}"
fi

# Create build directory
echo -e "${BLUE}Criando diretório de build...${NC}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Install dependencies with Conan
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Instalando dependências com Conan...${NC}"
echo -e "${BLUE}========================================${NC}"

conan install .. \
    --output-folder=. \
    --build=missing \
    -s build_type=${BUILD_TYPE}

# Configure with CMake
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Configurando com CMake...${NC}"
echo -e "${BLUE}========================================${NC}"

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DBUILD_WORKER=ON \
    -DBUILD_CLIENT=ON

# Build
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Compilando...${NC}"
echo -e "${BLUE}========================================${NC}"

cmake --build . -j${NUM_JOBS}

cd ..

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}✓ Build concluído com sucesso!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "${YELLOW}Executáveis:${NC}"
echo "  Worker:  ${BUILD_DIR}/bin/worker"
echo "  Client:  ${BUILD_DIR}/bin/client_example"
echo ""
echo -e "${YELLOW}Para executar:${NC}"
echo "  1. Terminal 1: ./${BUILD_DIR}/bin/worker"
echo "  2. Terminal 2: ./${BUILD_DIR}/bin/client_example"
echo ""