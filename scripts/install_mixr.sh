#!/bin/bash

# Script de instalação local do MIXR com download automático
# Instala tudo na pasta do projeto sem modificar variáveis de ambiente do sistema

set -e  # Para em caso de erro

# URLs de download (podem ser sobrescritas via variáveis de ambiente)
MIXR_SOURCE_URL="${MIXR_SOURCE_URL:-https://s3.amazonaws.com/mixr-platform/releases/stable_v18.01/mixr_v18.01.zip}"
MIXR_3RDPARTY_URL="${MIXR_3RDPARTY_URL:-https://s3.amazonaws.com/mixr-platform/releases/stable_v18.01/mixr-3rdpartysrc_v18.12.tgz}"
MIXR_EXAMPLES_URL="${MIXR_EXAMPLES_URL:-https://s3.amazonaws.com/mixr-platform/releases/stable_v18.01/mixr-examples_v18.01.zip}"

PROJECT_DIR="$(pwd)/cpp/mixr_dev"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOCAL_PREFIX="${PROJECT_DIR}/mixr-3rdparty"
MIXR_SRC_DIR="${PROJECT_DIR}/mixr"
DOWNLOADS_DIR="${PROJECT_DIR}/downloads"

echo "========================================="
echo "=== Instalação Local do MIXR ==="
echo "========================================="
echo "Diretório do projeto: ${PROJECT_DIR}"
echo "Diretório do script: ${SCRIPT_DIR}"
echo "Prefixo local (3rd party): ${LOCAL_PREFIX}"
echo ""

# Criar diretórios necessários
mkdir -p "${DOWNLOADS_DIR}"

# Verificar dependências do sistema
echo "=== Verificando Dependências do Sistema ==="
MISSING_DEPS=()

# Verificar comandos essenciais
for cmd in cmake m4 wget tar unzip gcc g++ make autoconf automake libtool pkg-config; do
    if ! command -v $cmd &> /dev/null; then
        MISSING_DEPS+=($cmd)
    fi
done

# Verificar bibliotecas dev
for lib in libftgl-dev libfreetype6-dev freeglut3-dev libfontconfig-dev libexpat-dev libxi-dev; do
    if ! dpkg -l 2>/dev/null | grep -q "^ii  $lib"; then
        MISSING_DEPS+=($lib)
    fi
done

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo ""
    echo "AVISO: As seguintes dependências estão faltando:"
    printf '  - %s\n' "${MISSING_DEPS[@]}"
    echo ""
    echo "Instalando dependências do sistema..."
    sudo apt update
    sudo apt install -y \
        libgdal-dev \
        libftgl-dev \
        libfreetype6-dev \
        freeglut3-dev \
        libfontconfig-dev \
        libexpat-dev \
        libxi-dev \
        build-essential \
        cmake \
        m4 \
        autoconf \
        autogen \
        automake \
        libtool \
        libtool-bin \
        pkg-config \
        wget \
        curl \
        unzip \
        tar
    echo ""
    echo "✓ Dependências instaladas!"
else
    echo "✓ Todas as dependências do sistema estão instaladas!"
fi

# Função para download com verificação
download_file() {
    local url=$1
    local output=$2
    local name=$3
    
    if [ -f "${output}" ]; then
        echo "✓ ${name} já foi baixado"
        return 0
    fi
    
    echo "Baixando ${name}..."
    if command -v wget &> /dev/null; then
        wget -c "${url}" -O "${output}" || {
            echo "ERRO ao baixar ${name}"
            rm -f "${output}"
            exit 1
        }
    elif command -v curl &> /dev/null; then
        curl -L "${url}" -o "${output}" || {
            echo "ERRO ao baixar ${name}"
            rm -f "${output}"
            exit 1
        }
    else
        echo "ERRO: wget ou curl não encontrado"
        exit 1
    fi
    echo "✓ ${name} baixado com sucesso"
}

# Download dos arquivos necessários
echo ""
echo "=== Baixando Arquivos do MIXR ==="

download_file "${MIXR_SOURCE_URL}" "${DOWNLOADS_DIR}/mixr_source.zip" "Código fonte do MIXR"
download_file "${MIXR_3RDPARTY_URL}" "${DOWNLOADS_DIR}/mixr-3rdpartysrc.tgz" "Dependências 3rd party"
download_file "${MIXR_EXAMPLES_URL}" "${DOWNLOADS_DIR}/mixr-examples.zip" "Exemplos do MIXR"

# Extrair código fonte do MIXR
echo ""
echo "=== Extraindo Arquivos ==="
cd "${PROJECT_DIR}"

if [ ! -d "mixr" ]; then
    echo "Extraindo código fonte do MIXR..."
    unzip -q "${DOWNLOADS_DIR}/mixr_source.zip"
    # Normalizar nome do diretório
    if [ -d "mixr_v18.01" ]; then
        mv mixr_v18.01 mixr
    elif [ -d "MIXR" ]; then
        mv MIXR mixr
    fi
    echo "✓ MIXR extraído"
else
    echo "✓ Diretório mixr já existe"
fi

if [ ! -d "mixr-examples" ]; then
    echo "Extraindo exemplos..."
    unzip -q "${DOWNLOADS_DIR}/mixr-examples.zip"
    # Normalizar nome do diretório
    if [ -d "mixr-examples_v18.01" ]; then
        mv mixr-examples_v18.01 mixr-examples
    elif [ -d "examples" ]; then
        mv examples mixr-examples
    fi
    echo "✓ Exemplos extraídos"
else
    echo "✓ Diretório mixr-examples já existe"
fi

if [ ! -d "mixr-3rdpartysrc" ]; then
    echo "Extraindo dependências 3rd party..."
    tar xzf "${DOWNLOADS_DIR}/mixr-3rdpartysrc.tgz"
    echo "✓ Dependências extraídas"
else
    echo "✓ Diretório mixr-3rdpartysrc já existe"
fi

# Configurar ambiente local para build
echo ""
echo "=== Configurando Ambiente de Build ==="

# Criar setenv.sh local que simula o original do MIXR
cat > "${PROJECT_DIR}/setenv.sh" << EOF
#!/bin/bash
# Arquivo setenv.sh local para build do MIXR

# Define MIXR_ROOT como o diretório do projeto
export MIXR_ROOT="${PROJECT_DIR}/mixr"

# Define onde as dependências 3rd party serão instaladas
export MIXR_3RD_PARTY_ROOT="${LOCAL_PREFIX}"

# Adiciona ao PATH
export PATH="\${MIXR_3RD_PARTY_ROOT}/bin:\${PATH}"

# Adiciona ao LD_LIBRARY_PATH
export LD_LIBRARY_PATH="\${MIXR_3RD_PARTY_ROOT}/lib:\${LD_LIBRARY_PATH}"

# PKG_CONFIG_PATH
export PKG_CONFIG_PATH="\${MIXR_3RD_PARTY_ROOT}/lib/pkgconfig:\${PKG_CONFIG_PATH}"

# Flags de compilação
export CFLAGS="-I\${MIXR_3RD_PARTY_ROOT}/include"
export CXXFLAGS="-I\${MIXR_3RD_PARTY_ROOT}/include"
export LDFLAGS="-L\${MIXR_3RD_PARTY_ROOT}/lib"

echo "Ambiente MIXR configurado:"
echo "  MIXR_ROOT: \${MIXR_ROOT}"
echo "  MIXR_3RD_PARTY_ROOT: \${MIXR_3RD_PARTY_ROOT}"
EOF

chmod +x "${PROJECT_DIR}/setenv.sh"

# Source do setenv.sh local
source "${PROJECT_DIR}/setenv.sh"

# Verificar que as variáveis estão corretas
echo ""
echo "=== Verificando Variáveis de Ambiente ==="
echo "MIXR_ROOT = ${MIXR_ROOT}"
echo "MIXR_3RD_PARTY_ROOT = ${MIXR_3RD_PARTY_ROOT}"
echo "PROJECT_DIR = ${PROJECT_DIR}"
echo ""

# Verificar versões das ferramentas necessárias
echo "=== Verificando Versões das Ferramentas ==="
autoconf --version 2>/dev/null | head -n1 && echo "✓ autoconf OK" || echo "✗ autoconf FALTANDO"
automake --version 2>/dev/null | head -n1 && echo "✓ automake OK" || echo "✗ automake FALTANDO"
libtool --version 2>/dev/null | head -n1 && echo "✓ libtool OK" || echo "✗ libtool FALTANDO"
cmake --version 2>/dev/null | head -n1 && echo "✓ cmake OK" || echo "✗ cmake FALTANDO"

# Construir dependências 3rd party
echo ""
echo "========================================="
echo "=== Construindo Dependências 3rd Party ==="
echo "========================================="

cd "${PROJECT_DIR}/mixr-3rdpartysrc"

# Verificar se build_libs.sh existe
if [ ! -f "build_libs.sh" ]; then
    echo "ERRO: build_libs.sh não encontrado em mixr-3rdpartysrc"
    exit 1
fi

# Source do build_libs.sh original
echo "Carregando funções de build_libs.sh..."
source build_libs.sh

# Mostrar ajuda disponível
echo ""
echo "Funções disponíveis do build_libs.sh:"
help
echo ""

# Executar instalação de todas as dependências
echo "Iniciando build de todas as dependências..."
echo "Isso pode levar vários minutos..."
echo ""

# Redirecionar saída para log mas também mostrar no terminal
LOG_FILE="${PROJECT_DIR}/build_3rdparty.log"
echo "Log detalhado será salvo em: ${LOG_FILE}"
echo ""

# Tentar instalar todas as dependências, mas continuar mesmo se alguma falhar
set +e  # Temporariamente desabilita exit on error
install_all 2>&1 | tee "${LOG_FILE}"
BUILD_STATUS=${PIPESTATUS[0]}
set -e  # Reabilita exit on error

echo ""
echo "========================================="
echo "=== Verificando Resultado do Build ==="
echo "========================================="

if [ ${BUILD_STATUS} -eq 0 ]; then
    echo "✓ Dependências 3rd party instaladas com sucesso!"
else
    echo "⚠ AVISO: Algumas dependências podem ter falhado (código: ${BUILD_STATUS})"
fi

echo ""
echo "Verificando bibliotecas instaladas..."
echo ""

# Verificar o que foi instalado
if [ -d "${MIXR_3RD_PARTY_ROOT}" ]; then
    echo "Conteúdo de ${MIXR_3RD_PARTY_ROOT}:"
    ls -la "${MIXR_3RD_PARTY_ROOT}" || echo "  Diretório vazio ou não acessível"
    echo ""
    
    if [ -d "${MIXR_3RD_PARTY_ROOT}/lib" ]; then
        echo "Bibliotecas (.a) instaladas:"
        find "${MIXR_3RD_PARTY_ROOT}/lib" -name "*.a" -exec ls -lh {} \; 2>/dev/null | head -20
        
        NUM_LIBS=$(find "${MIXR_3RD_PARTY_ROOT}/lib" -name "*.a" 2>/dev/null | wc -l)
        echo ""
        echo "Total de bibliotecas .a instaladas: ${NUM_LIBS}"
        
        if [ ${NUM_LIBS} -eq 0 ]; then
            echo ""
            echo "⚠ AVISO: Nenhuma biblioteca .a foi instalada!"
            echo "Verifique o log: ${LOG_FILE}"
        fi
    else
        echo "⚠ AVISO: Diretório lib não existe em ${MIXR_3RD_PARTY_ROOT}"
    fi
    
    if [ -d "${MIXR_3RD_PARTY_ROOT}/include" ]; then
        echo ""
        echo "Headers instalados:"
        ls "${MIXR_3RD_PARTY_ROOT}/include" 2>/dev/null | head -10 || echo "  Nenhum header encontrado"
    fi
else
    echo "⚠ AVISO: Diretório ${MIXR_3RD_PARTY_ROOT} não existe!"
    echo "As dependências não foram instaladas corretamente."
fi

echo ""
echo "========================================="

if [ ${BUILD_STATUS} -ne 0 ]; then
    echo ""
    read -p "Deseja continuar mesmo assim? (s/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Ss]$ ]]; then
        echo "Instalação cancelada."
        echo "Para tentar novamente uma dependência específica:"
        echo "  cd mixr-3rdpartysrc"
        echo "  source ../setenv.sh"
        echo "  source build_libs.sh"
        echo "  install_<nome>  # ex: install_jsbsim, install_osg, etc"
        exit 1
    fi
fi

# Construir bibliotecas MIXR
echo ""
echo "========================================="
echo "=== Construindo Bibliotecas MIXR ==="
echo "========================================="

if [ ! -d "${MIXR_SRC_DIR}/src" ]; then
    echo "ERRO: Diretório ${MIXR_SRC_DIR}/src não encontrado"
    exit 1
fi

cd "${MIXR_SRC_DIR}/src"

# Verificar se makedefs existe
if [ -f "makedefs" ]; then
    echo "Encontrado arquivo makedefs"
    echo ""
    echo "Verificando configuração atual do makedefs:"
    echo "---"
    grep -E "MIXR_ROOT|MIXR_3RD_PARTY_ROOT|FREETYPE2_INC_DIR|JSBSIM_INC_DIR|HLA_INC_DIR" makedefs || echo "Variáveis não encontradas"
    echo "---"
    echo ""
    
    # O makedefs já usa variáveis de ambiente, não precisa modificar
    # Apenas verificar se o freetype2 está no local correto
    if [ -d "/usr/include/freetype2" ]; then
        echo "✓ freetype2 encontrado em /usr/include/freetype2"
    else
        echo "⚠ AVISO: freetype2 não encontrado no caminho padrão"
        # Tentar encontrar freetype2
        FREETYPE_PATH=$(pkg-config --cflags freetype2 2>/dev/null | sed 's/-I//' | awk '{print $1}')
        if [ -n "$FREETYPE_PATH" ]; then
            echo "  Encontrado em: $FREETYPE_PATH"
            echo "  Criando makedefs.local com o caminho correto..."
            
            # Fazer backup
            if [ ! -f "makedefs.orig" ]; then
                cp makedefs makedefs.orig
            fi
            
            # Modificar apenas FREETYPE2_INC_DIR
            sed "s|FREETYPE2_INC_DIR = /usr/include/freetype2|FREETYPE2_INC_DIR = ${FREETYPE_PATH}|g" \
                makedefs.orig > makedefs
        fi
    fi
    
    echo "✓ makedefs verificado"
else
    echo "⚠ AVISO: arquivo makedefs não encontrado em ${MIXR_SRC_DIR}/src"
fi

# Compilar MIXR
echo ""
echo "Compilando MIXR com as variáveis de ambiente:"
echo "  MIXR_ROOT=${MIXR_ROOT}"
echo "  MIXR_3RD_PARTY_ROOT=${MIXR_3RD_PARTY_ROOT}"
echo ""

LOG_MIXR="${PROJECT_DIR}/build_mixr.log"

make -j$(nproc) 2>&1 | tee "${LOG_MIXR}"

if [ ${PIPESTATUS[0]} -eq 0 ]; then
    echo ""
    echo "✓ MIXR compilado com sucesso!"
else
    echo ""
    echo "ERRO: Falha na compilação do MIXR"
    echo "Verifique o log em: ${LOG_MIXR}"
    exit 1
fi

# Instalar MIXR localmente (se o Makefile suportar)
echo ""
echo "Tentando instalar MIXR..."
set +e
make install 2>&1 | tee -a "${LOG_MIXR}"
INSTALL_STATUS=$?
set -e

if [ ${INSTALL_STATUS} -ne 0 ]; then
    echo "⚠ AVISO: 'make install' falhou ou não está disponível"
    echo "As bibliotecas MIXR estão em: ${MIXR_ROOT}/lib"
fi

# Criar script de ambiente para uso futuro
echo ""
echo "=== Criando Scripts de Ambiente ==="

cat > "${PROJECT_DIR}/setup_mixr_env.sh" << 'EOF'
#!/bin/bash
# Script para configurar ambiente MIXR local
# Use: source setup_mixr_env.sh

# Usa o diretório onde o script está localizado como base
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export MIXR_ROOT="${SCRIPT_DIR}/mixr"
export MIXR_3RD_PARTY_ROOT="${SCRIPT_DIR}/mixr-3rdparty"
export PATH="${MIXR_3RD_PARTY_ROOT}/bin:${PATH}"
export LD_LIBRARY_PATH="${MIXR_3RD_PARTY_ROOT}/lib:${MIXR_ROOT}/lib:${LD_LIBRARY_PATH}"
export PKG_CONFIG_PATH="${MIXR_3RD_PARTY_ROOT}/lib/pkgconfig:${PKG_CONFIG_PATH}"

echo "========================================="
echo "Ambiente MIXR configurado!"
echo "========================================="
echo "MIXR_ROOT: ${MIXR_ROOT}"
echo "MIXR_3RD_PARTY_ROOT: ${MIXR_3RD_PARTY_ROOT}"
echo "Bibliotecas MIXR: ${MIXR_ROOT}/lib"
echo "Bibliotecas 3rd party: ${MIXR_3RD_PARTY_ROOT}/lib"
echo "Headers 3rd party: ${MIXR_3RD_PARTY_ROOT}/include"
echo "Exemplos: ${SCRIPT_DIR}/mixr-examples"
echo "========================================="
EOF

chmod +x "${PROJECT_DIR}/setup_mixr_env.sh"

# Criar script auxiliar para reinstalar dependências individuais
cat > "${PROJECT_DIR}/reinstall_dependency.sh" << 'EOF'
#!/bin/bash
# Script para reinstalar uma dependência específica

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ $# -eq 0 ]; then
    echo "Uso: $0 <nome_da_dependencia>"
    echo ""
    echo "Dependências disponíveis:"
    echo "  osg        - OpenSceneGraph"
    echo "  cigi       - CIGI"
    echo "  jsbsim     - JSBSim"
    echo "  protobuf   - Google Protobuf"
    echo "  hla        - OpenRTI (HLA)"
    echo "  zeromq     - ZeroMQ"
    echo "  flex       - Flex"
    echo "  bison      - Bison"
    echo ""
    echo "Exemplo: $0 jsbsim"
    exit 1
fi

cd "${SCRIPT_DIR}/mixr-3rdpartysrc"
source "${SCRIPT_DIR}/setenv.sh"
source build_libs.sh

case "$1" in
    osg)
        install_osg
        ;;
    cigi)
        install_cigi
        ;;
    jsbsim)
        install_jsbsim
        ;;
    protobuf)
        install_protobuf
        ;;
    hla)
        install_hla
        ;;
    zeromq)
        install_zeromq
        ;;
    flex)
        install_flex
        ;;
    bison)
        install_bison
        ;;
    *)
        echo "Dependência desconhecida: $1"
        exit 1
        ;;
esac

echo ""
echo "✓ Dependência $1 instalada!"
echo ""
echo "Bibliotecas instaladas:"
find "${MIXR_3RD_PARTY_ROOT}/lib" -name "*.a" -exec ls -lh {} \;
EOF

chmod +x "${PROJECT_DIR}/reinstall_dependency.sh"

# Criar README detalhado
cat > "${PROJECT_DIR}/INSTALACAO_LOCAL.md" << EOF
# Instalação Local do MIXR

Instalação concluída em: \`${PROJECT_DIR}\`

## Estrutura de Diretórios

\`\`\`
${PROJECT_DIR}/
├── mixr/
│   ├── src/                 # Código fonte MIXR
│   │   └── makedefs         # Configuração de build (usa MIXR_ROOT e MIXR_3RD_PARTY_ROOT)
│   └── lib/                 # Bibliotecas MIXR compiladas
├── mixr-3rdparty/
│   ├── lib/                 # Bibliotecas 3rd party (.a)
│   └── include/             # Headers 3rd party
├── mixr-3rdpartysrc/        # Código fonte das dependências
├── mixr-examples/           # Exemplos do MIXR
├── downloads/               # Arquivos baixados originais
├── setenv.sh                # Configuração de ambiente (uso interno)
├── setup_mixr_env.sh        # Configuração de ambiente (uso do usuário)
├── reinstall_dependency.sh  # Reinstalar dependência específica
└── build_*.log              # Logs de compilação
\`\`\`

## Como o MIXR Encontra as Bibliotecas

O arquivo \`makedefs\` em \`mixr/src/\` usa duas variáveis de ambiente:

- **MIXR_ROOT**: Aponta para o diretório \`mixr/\`
- **MIXR_3RD_PARTY_ROOT**: Aponta para \`mixr-3rdparty/\`

O script \`setup_mixr_env.sh\` configura essas variáveis automaticamente.

## Uso Diário

**Sempre execute antes de compilar ou executar:**

\`\`\`bash
source setup_mixr_env.sh
\`\`\`

## Compilar Exemplos

\`\`\`bash
source setup_mixr_env.sh
cd mixr-examples/testGraphics1
make
./testGraphics1
\`\`\`

## Reinstalar Dependência Específica

\`\`\`bash
./reinstall_dependency.sh jsbsim
\`\`\`

## Verificar Instalação

\`\`\`bash
# Ver variáveis de ambiente
source setup_mixr_env.sh

# Ver bibliotecas 3rd party
ls -lh mixr-3rdparty/lib/*.a

# Ver bibliotecas MIXR
ls -lh mixr/lib/*.a

# Ver headers 3rd party
ls mixr-3rdparty/include/
\`\`\`

## Troubleshooting

### makedefs não encontra as bibliotecas

Certifique-se de ter executado:
\`\`\`bash
source setup_mixr_env.sh
\`\`\`

### Verificar se variáveis estão corretas

\`\`\`bash
echo \$MIXR_ROOT
echo \$MIXR_3RD_PARTY_ROOT
\`\`\`

Devem apontar para:
- MIXR_ROOT: \`${PROJECT_DIR}/mixr\`
- MIXR_3RD_PARTY_ROOT: \`${PROJECT_DIR}/mixr-3rdparty\`
EOF

# Sumário final
echo ""
echo "========================================="
echo "=== ✓ Instalação Concluída! ==="
echo "========================================="
echo ""
echo "📁 Instalação em: ${PROJECT_DIR}/"
echo ""
echo "🔧 Variáveis de ambiente configuradas:"
echo "   MIXR_ROOT = ${MIXR_ROOT}"
echo "   MIXR_3RD_PARTY_ROOT = ${MIXR_3RD_PARTY_ROOT}"
echo ""
echo "📝 Próximos passos:"
echo "   1. source setup_mixr_env.sh"
echo "   2. cd mixr-examples/<exemplo>/"
echo "   3. make"
echo ""
echo "🔧 Para reinstalar dependência:"
echo "   ./reinstall_dependency.sh <nome>"
echo ""
echo "📋 Documentação: INSTALACAO_LOCAL.md"
echo ""
echo "========================================="