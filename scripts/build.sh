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
CONAN_PROFILE="${CONAN_PROFILE:-ml-inference}"
CONAN_REMOTE="${CONAN_REMOTE:-conancenter}"

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

# ========================================
# ETAPAS INICIAIS DO CONAN
# ========================================

# 1. Criar perfil do projeto se não existir
if [ ! -f ~/.conan2/profiles/${CONAN_PROFILE} ]; then
    echo -e "${YELLOW}Criando perfil '${CONAN_PROFILE}' para o projeto...${NC}"
    
    # Detectar perfil base se não existir
    if [ ! -f ~/.conan2/profiles/default ]; then
        conan profile detect --force
    fi
    
    # Copiar default para o perfil do projeto
    cp ~/.conan2/profiles/default ~/.conan2/profiles/${CONAN_PROFILE}
    
    echo -e "${GREEN}✓ Perfil '${CONAN_PROFILE}' criado${NC}"
else
    echo -e "${GREEN}✓ Perfil '${CONAN_PROFILE}' encontrado${NC}"
fi

# 2. Mostrar perfil que será usado
echo ""
echo -e "${BLUE}Perfil Conan em uso: ${CONAN_PROFILE}${NC}"
conan profile show -pr ${CONAN_PROFILE}

# 3. Configurar para usar apenas o registry do Conan (ConanCenter)
echo ""
echo -e "${YELLOW}Verificando remotes do Conan...${NC}"
conan remote list

# Garantir que o remote existe
if ! conan remote list | grep -q "${CONAN_REMOTE}"; then
    echo -e "${YELLOW}Adicionando remote '${CONAN_REMOTE}'...${NC}"
    case ${CONAN_REMOTE} in
        conancenter)
            conan remote add conancenter https://center.conan.io
            ;;
        # Adicione outros remotes se necessário
        *)
            echo -e "${RED}Remote '${CONAN_REMOTE}' não reconhecido${NC}"
            exit 1
            ;;
    esac
fi

# 4. Verificar se conanfile existe
if [ ! -f "conanfile.txt" ] && [ ! -f "conanfile.py" ]; then
    echo -e "${RED}Error: conanfile.txt ou conanfile.py não encontrado!${NC}"
    exit 1
fi

if [ -f "conanfile.txt" ]; then
    echo -e "${GREEN}✓ conanfile.txt encontrado${NC}"
else
    echo -e "${GREEN}✓ conanfile.py encontrado${NC}"
fi

echo ""
echo -e "${YELLOW}Configuration:${NC}"
echo "  Build Directory: ${BUILD_DIR}"
echo "  Build Type: ${BUILD_TYPE}"
echo "  Conan Profile: ${CONAN_PROFILE}"
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

# ========================================
# INSTALAÇÃO DE DEPENDÊNCIAS
# ========================================
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Instalando dependências com Conan...${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "${YELLOW}Usando perfil: ${CONAN_PROFILE}${NC}"
echo -e "${YELLOW}Isso pode levar alguns minutos na primeira execução...${NC}"
echo ""

conan install .. \
    --profile=${CONAN_PROFILE} \
    --remote=${CONAN_REMOTE} \
    --output-folder=. \
    --build=missing \
    -s build_type=${BUILD_TYPE}


echo ""
echo -e "${GREEN}✓ Dependências instaladas${NC}"

# ========================================
# CONFIGURAÇÃO CMAKE
# ========================================
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Configurando com CMake...${NC}"
echo -e "${BLUE}========================================${NC}"

cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DBUILD_WORKER=ON \
    -DBUILD_CLIENT=ON

echo ""
echo -e "${GREEN}✓ Configuração CMake concluída${NC}"

# ========================================
# COMPILAÇÃO
# ========================================
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Compilando (${NUM_JOBS} jobs paralelos)...${NC}"
echo -e "${BLUE}========================================${NC}"

cmake --build . -j${NUM_JOBS}

# cd ..

# ========================================
# SUCESSO
# ========================================
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
echo -e "${BLUE}Dicas:${NC}"
echo "  • Rebuild rápido: BUILD_TYPE=Debug ./build.sh"
echo "  • Usar outro perfil: CONAN_PROFILE=outro-perfil ./build.sh"
echo "  • Editar perfil: conan profile show ${CONAN_PROFILE}"
echo ""