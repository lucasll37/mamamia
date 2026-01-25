#!/bin/bash
# ============================================
# Script de Instalação do MIXR
# Autor: Lucas
# Data: 2026-01-24
# Descrição: Instala MIXR e dependências em sistemas Ubuntu
# ============================================

set -e  # Para em caso de erro

# ============================================
# CONFIGURAÇÃO DE URLs - EDITE AQUI
# ============================================

# URLs para download (deixe vazio para usar arquivos locais)
MIXR_SOURCE_URL="${MIXR_SOURCE_URL:-https://s3.amazonaws.com/mixr-platform/releases/stable_v18.01/mixr_v18.01.zip}"
MIXR_3RDPARTY_URL="${MIXR_3RDPARTY_URL:-https://s3.amazonaws.com/mixr-platform/releases/stable_v18.01/mixr-3rdpartysrc_v18.12.tgz}"
MIXR_EXAMPLES_URL="${MIXR_EXAMPLES_URL:-https://s3.amazonaws.com/mixr-platform/releases/stable_v18.01/mixr-examples_v18.01.zip}"

# ============================================
# CONFIGURAÇÃO
# ============================================

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Função para mensagens
print_header() {
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "${CYAN}ℹ $1${NC}"
}

# Verifica se está rodando como root para instalação de sistema
check_root() {
    if [ "$EUID" -eq 0 ]; then
        return 0
    else
        return 1
    fi
}

# ============================================
# CONFIGURAÇÃO DE DIRETÓRIOS
# ============================================

# Modo de instalação: "system" ou "user"
INSTALL_MODE="${1:-user}"

# Diretório base (usa diretório atual ou home)
BASE_DIR="${MIXR_BASE_DIR:-$(pwd)}"

# Diretórios
if [ "$INSTALL_MODE" = "system" ]; then
    MIXR_INSTALL_DIR="/usr/local/mixr"
    MIXR_PLATFORM_DIR="/usr/local/mixr-platform"
else
    MIXR_INSTALL_DIR="${BASE_DIR}/cpp/mixr"
    MIXR_PLATFORM_DIR="${BASE_DIR}/cpp/mixr-platform"
fi

# Diretório temporário para downloads
DOWNLOAD_DIR="$HOME/.mixr-installer-tmp"

# Arquivos necessários (locais ou baixados)
MIXR_3RDPARTY_FILE="${MIXR_3RDPARTY_FILE:-mixr-3rdpartysrc.tgz}"
MIXR_SOURCE_FILE="${MIXR_SOURCE_FILE:-mixr_v*.tar.gz}"
MIXR_EXAMPLES_FILE="${MIXR_EXAMPLES_FILE:-mixr-examples.tar.gz}"

# ============================================
# FUNÇÕES AUXILIARES
# ============================================

# Verifica se um comando existe
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Verifica se um arquivo existe
file_exists() {
    [ -f "$1" ]
}

# Resolve caminho absoluto
resolve_path() {
    local path="$1"
    
    # Tenta usar realpath primeiro (mais confiável)
    if command_exists realpath; then
        realpath "$path" 2>/dev/null && return 0
    fi
    
    # Fallback manual
    if [ -d "$path" ]; then
        (cd "$path" && pwd)
    elif [ -f "$path" ]; then
        local dir=$(dirname "$path")
        local file=$(basename "$path")
        echo "$(cd "$dir" && pwd)/$file"
    else
        # Se não existe, retorna como está
        echo "$path"
    fi
}

# Download de arquivo com progress bar
download_file() {
    local url="$1"
    local output="$2"
    local description="$3"
    
    print_info "Baixando $description..."
    print_info "URL: $url"
    
    mkdir -p "$(dirname "$output")"
    
    # Tenta wget primeiro, depois curl
    if command_exists wget; then
        wget --no-check-certificate --show-progress -O "$output" "$url" 2>&1 || {
            print_error "Falha no download com wget"
            return 1
        }
    elif command_exists curl; then
        curl -L -k --progress-bar -o "$output" "$url" || {
            print_error "Falha no download com curl"
            return 1
        }
    else
        print_error "wget ou curl não encontrado. Instale um deles."
        return 1
    fi
    
    # Verifica se o arquivo foi baixado
    if [ ! -f "$output" ] || [ ! -s "$output" ]; then
        print_error "Arquivo baixado está vazio ou não existe"
        return 1
    fi
    
    # Mostra informações do arquivo
    local size=$(du -h "$output" | cut -f1)
    local file_type=$(detect_file_type "$output")
    print_success "Download concluído: $size"
    print_info "Tipo: $file_type"
    
    return 0
}

# Extrai arquivo tar/tgz/zip automaticamente
extract_archive() {
    local file="$1"
    local dest="$2"
    
    print_info "Extraindo $(basename $file) para $dest"
    
    mkdir -p "$dest"
    
    # Detecta tipo de arquivo
    local file_type=$(detect_file_type "$file")
    print_info "Tipo detectado: $file_type"
    
    cd "$dest"
    
    # Extração baseada no tipo
    if echo "$file_type" | grep -qi "gzip compressed"; then
        print_info "Método: tar + gzip"
        tar xzf "$file" || {
            print_warning "Tentando método alternativo..."
            gunzip -c "$file" | tar xf -
        }
    elif echo "$file_type" | grep -qi "bzip2"; then
        print_info "Método: tar + bzip2"
        tar xjf "$file"
    elif echo "$file_type" | grep -qi "xz compressed"; then
        print_info "Método: tar + xz"
        tar xJf "$file"
    elif echo "$file_type" | grep -qi "zip archive"; then
        print_info "Método: unzip"
        unzip -q "$file"
    elif echo "$file_type" | grep -qi "posix tar"; then
        print_info "Método: tar"
        tar xf "$file"
    else
        # Fallback: tenta detectar pela extensão
        print_warning "Tipo não reconhecido, tentando pela extensão..."
        if [[ "$file" == *.tar.gz ]] || [[ "$file" == *.tgz ]]; then
            tar xzf "$file"
        elif [[ "$file" == *.tar.bz2 ]]; then
            tar xjf "$file"
        elif [[ "$file" == *.tar.xz ]]; then
            tar xJf "$file"
        elif [[ "$file" == *.zip ]]; then
            unzip -q "$file"
        else
            print_error "Não foi possível determinar como extrair o arquivo"
            return 1
        fi
    fi
    
    print_success "Extração concluída"
}

# ============================================
# INSTALAÇÃO DE PREREQUISITOS
# ============================================

install_prerequisites() {
    print_header "Instalando Prerequisitos"
    
    if ! command_exists apt; then
        print_error "Este script requer apt (Ubuntu/Debian). Sistema não suportado."
        exit 1
    fi
    
    print_info "Atualizando lista de pacotes..."
    sudo apt update
    
    print_info "Instalando pacotes necessários..."
    
    # Pacotes para gráficos MIXR
    sudo apt install -y \
        libftgl-dev \
        libfreetype6-dev \
        freeglut3-dev \
        libfontconfig-dev
    
    # Pacote para OpenRTI (HLA)
    sudo apt install -y libexpat-dev
    
    # Ferramentas de compilação
    sudo apt install -y \
        build-essential \
        cmake \
        m4
    
    # Ferramentas de download e extração
    sudo apt install -y \
        wget \
        curl \
        unzip \
        file
    
    print_success "Prerequisitos instalados com sucesso!"
}

# ============================================
# DOWNLOAD DE ARQUIVOS
# ============================================

download_files() {
    print_header "Preparando Arquivos"
    
    mkdir -p "$DOWNLOAD_DIR"
    
    local need_download=false
    
    # Verifica se precisa baixar 3rd party
    if [ -n "$MIXR_3RDPARTY_URL" ]; then
        need_download=true
        MIXR_3RDPARTY_FILE="$DOWNLOAD_DIR/mixr-3rdparty.tgz"
        if ! download_file "$MIXR_3RDPARTY_URL" "$MIXR_3RDPARTY_FILE" "MIXR 3rd Party"; then
            print_error "Falha ao baixar dependências de terceiros"
            exit 1
        fi
    else
        # Verifica se arquivo local existe
        if ! file_exists "$MIXR_3RDPARTY_FILE"; then
            print_error "Arquivo não encontrado: $MIXR_3RDPARTY_FILE"
            print_info "Especifique MIXR_3RDPARTY_URL ou coloque o arquivo no diretório atual"
            exit 1
        fi
        print_info "Usando arquivo local: $MIXR_3RDPARTY_FILE"
    fi
    
    # Verifica se precisa baixar MIXR source
    if [ -n "$MIXR_SOURCE_URL" ]; then
        need_download=true
        MIXR_SOURCE_FILE="$DOWNLOAD_DIR/mixr-source.zip"
        if ! download_file "$MIXR_SOURCE_URL" "$MIXR_SOURCE_FILE" "MIXR Source"; then
            print_error "Falha ao baixar código fonte MIXR"
            exit 1
        fi
    else
        # Verifica se arquivo local existe
        MIXR_SOURCE_FOUND=$(ls $MIXR_SOURCE_FILE 2>/dev/null | head -1)
        if [ -z "$MIXR_SOURCE_FOUND" ]; then
            print_warning "Arquivo MIXR source não encontrado: $MIXR_SOURCE_FILE"
            print_info "A instalação continuará sem o código fonte principal"
        else
            MIXR_SOURCE_FILE="$MIXR_SOURCE_FOUND"
            print_info "Usando arquivo local: $MIXR_SOURCE_FILE"
        fi
    fi
    
    # Verifica se precisa baixar exemplos (opcional)
    if [ -n "$MIXR_EXAMPLES_URL" ]; then
        need_download=true
        MIXR_EXAMPLES_FILE="$DOWNLOAD_DIR/mixr-examples.zip"
        if download_file "$MIXR_EXAMPLES_URL" "$MIXR_EXAMPLES_FILE" "MIXR Examples"; then
            print_success "Exemplos baixados"
        else
            print_warning "Falha ao baixar exemplos (continuando sem eles)"
            MIXR_EXAMPLES_FILE=""
        fi
    else
        # Verifica se arquivo local existe
        if file_exists "$MIXR_EXAMPLES_FILE"; then
            print_info "Usando arquivo local: $MIXR_EXAMPLES_FILE"
        else
            print_warning "Arquivo de exemplos não encontrado (opcional)"
            MIXR_EXAMPLES_FILE=""
        fi
    fi
    
    if $need_download; then
        print_success "Todos os downloads concluídos!"
    fi
}

# ============================================
# PREPARAÇÃO DE DIRETÓRIOS
# ============================================

setup_directories() {
    print_header "Preparando Diretórios"
    
    if [ "$INSTALL_MODE" = "system" ]; then
        print_info "Modo de instalação: Sistema (requer sudo)"
        sudo mkdir -p "$MIXR_PLATFORM_DIR"
        sudo chown $USER:$USER "$MIXR_PLATFORM_DIR"
    else
        print_info "Modo de instalação: Usuário"
        mkdir -p "$MIXR_PLATFORM_DIR"
    fi
    
    # Resolve e armazena caminho absoluto
    MIXR_PLATFORM_DIR=$(resolve_path "$MIXR_PLATFORM_DIR")
    
    print_success "Diretórios criados em: $MIXR_PLATFORM_DIR"
}

# ============================================
# EXTRAÇÃO DE ARQUIVOS
# ============================================

extract_files() {
    print_header "Extraindo Arquivos"
    
    # Extrai dependências de terceiros
    if ! file_exists "$MIXR_3RDPARTY_FILE"; then
        print_error "Arquivo não encontrado: $MIXR_3RDPARTY_FILE"
        exit 1
    fi
    
    print_info "Extraindo dependências de terceiros..."
    extract_archive "$MIXR_3RDPARTY_FILE" "$MIXR_PLATFORM_DIR"
    
    # Extrai código fonte MIXR
    if [ -n "$MIXR_SOURCE_FILE" ] && file_exists "$MIXR_SOURCE_FILE"; then
        print_info "Extraindo MIXR source..."
        
        local extract_dir
        if [ "$INSTALL_MODE" = "system" ]; then
            extract_dir="/usr/local"
            sudo mkdir -p "$extract_dir"
            
            # Extrai temporariamente
            local temp_extract="/tmp/mixr-extract-$$"
            mkdir -p "$temp_extract"
            extract_archive "$MIXR_SOURCE_FILE" "$temp_extract"
            
            # Move para destino final
            sudo mv "$temp_extract"/* "$extract_dir/" 2>/dev/null || true
            rm -rf "$temp_extract"
            
            # Cria link simbólico se necessário
            if [ ! -d "$extract_dir/mixr" ]; then
                local mixr_dir=$(sudo find "$extract_dir" -maxdepth 1 -type d -name "mixr*" ! -name "mixr-platform" | head -1)
                if [ -n "$mixr_dir" ]; then
                    sudo ln -s "$mixr_dir" "$extract_dir/mixr"
                    print_info "Link criado: $(basename $mixr_dir) -> mixr"
                fi
            fi
        else
            extract_dir="${BASE_DIR}/cpp"
            mkdir -p "$extract_dir"
            
            # Extrai diretamente
            extract_archive "$MIXR_SOURCE_FILE" "$extract_dir"
            
            # Cria link simbólico se necessário
            if [ ! -d "$extract_dir/mixr" ]; then
                local mixr_dir=$(find "$extract_dir" -maxdepth 1 -type d -name "mixr*" ! -name "mixr-platform" | head -1)
                if [ -n "$mixr_dir" ]; then
                    ln -s "$(basename $mixr_dir)" "$extract_dir/mixr"
                    print_info "Link criado: $(basename $mixr_dir) -> mixr"
                fi
            fi
            
            # Atualiza MIXR_INSTALL_DIR com caminho real
            MIXR_INSTALL_DIR="$extract_dir/mixr"
        fi
    else
        print_warning "Pulando extração do código fonte MIXR"
    fi
    
    # Extrai exemplos (opcional)
    if [ -n "$MIXR_EXAMPLES_FILE" ] && file_exists "$MIXR_EXAMPLES_FILE"; then
        print_info "Extraindo exemplos..."
        local examples_dir="${BASE_DIR}/cpp/mixr-examples"
        mkdir -p "$examples_dir"
        extract_archive "$MIXR_EXAMPLES_FILE" "$examples_dir"
    else
        print_warning "Pulando extração de exemplos"
    fi
    
    print_success "Extração de arquivos concluída!"
}

# ============================================
# COMPILAÇÃO DE DEPENDÊNCIAS
# ============================================

build_dependencies() {
    print_header "Compilando Dependências de Terceiros"
    
    # Verifica diretório com caminho absoluto
    local mixr_3rdparty_dir="$MIXR_PLATFORM_DIR/mixr-3rdpartysrc"
    
    if [ ! -d "$mixr_3rdparty_dir" ]; then
        print_error "Diretório mixr-3rdpartysrc não encontrado!"
        print_info "Procurado em: $mixr_3rdparty_dir"
        print_info "Conteúdo de $MIXR_PLATFORM_DIR:"
        ls -la "$MIXR_PLATFORM_DIR" 2>&1
        exit 1
    fi
    
    print_success "Diretório encontrado: $mixr_3rdparty_dir"
    
    # Define MIXR_ROOT
    if [ "$INSTALL_MODE" = "system" ]; then
        export MIXR_ROOT="/usr/local/mixr"
    else
        export MIXR_ROOT="$(resolve_path "$MIXR_INSTALL_DIR")"
    fi
    
    if [ ! -d "$MIXR_ROOT" ]; then
        print_error "Diretório MIXR não encontrado: $MIXR_ROOT"
        exit 1
    fi
    
    print_info "MIXR_ROOT: $MIXR_ROOT"
    
    cd "$MIXR_ROOT"
    if [ -f "setenv.sh" ]; then
        print_info "Carregando setenv.sh..."
        source setenv.sh
    fi
    
    cd "$mixr_3rdparty_dir"
    
    if [ ! -f "build_libs.sh" ]; then
        print_error "Arquivo build_libs.sh não encontrado!"
        print_info "Conteúdo do diretório:"
        ls -la
        exit 1
    fi
    
    print_info "Carregando build_libs.sh..."
    source build_libs.sh
    
    print_info "Compilando dependências (isso pode demorar)..."
    print_info "Usando $(nproc) cores de CPU"
    install_all
    
    print_success "Dependências compiladas com sucesso!"
}

# ============================================
# COMPILAÇÃO DO MIXR
# ============================================

build_mixr() {
    print_header "Compilando Bibliotecas MIXR"
    
    if [ "$INSTALL_MODE" = "system" ]; then
        export MIXR_ROOT="/usr/local/mixr"
    else
        export MIXR_ROOT="$(resolve_path "$MIXR_INSTALL_DIR")"
    fi
    
    if [ ! -d "$MIXR_ROOT/src" ]; then
        print_error "Diretório src não encontrado em $MIXR_ROOT"
        exit 1
    fi
    
    cd "$MIXR_ROOT"
    if [ -f "setenv.sh" ]; then
        source setenv.sh
    fi
    
    cd src
    
    print_info "Compilando MIXR (usando $(nproc) cores)..."
    make -j$(nproc)
    
    print_info "Instalando MIXR..."
    if [ "$INSTALL_MODE" = "system" ]; then
        sudo make install
    else
        make install
    fi
    
    print_success "MIXR compilado e instalado com sucesso!"
}

# ============================================
# COMPILAÇÃO DE EXEMPLOS
# ============================================

build_examples() {
    print_header "Compilando Exemplos MIXR"
    
    local examples_dir="${BASE_DIR}/cpp/mixr-examples"
    
    if [ ! -d "$examples_dir" ]; then
        print_warning "Diretório de exemplos não encontrado"
        return
    fi
    
    # Procura diretório src
    local src_dir=""
    if [ -d "$examples_dir/src" ]; then
        src_dir="$examples_dir/src"
    else
        src_dir=$(find "$examples_dir" -type d -name "src" 2>/dev/null | head -1)
    fi
    
    if [ -z "$src_dir" ]; then
        print_warning "Diretório src não encontrado em exemplos"
        return
    fi
    
    if [ "$INSTALL_MODE" = "system" ]; then
        export MIXR_ROOT="/usr/local/mixr"
    else
        export MIXR_ROOT="$(resolve_path "$MIXR_INSTALL_DIR")"
    fi
    
    cd "$MIXR_ROOT"
    if [ -f "setenv.sh" ]; then
        source setenv.sh
    fi
    
    cd "$src_dir"
    
    if [ ! -f "Makefile" ]; then
        print_warning "Makefile não encontrado em $src_dir"
        return
    fi
    
    print_info "Compilando exemplos..."
    make -j$(nproc)
    
    print_success "Exemplos compilados com sucesso!"
}

# ============================================
# CONFIGURAÇÃO DO AMBIENTE
# ============================================

setup_environment() {
    print_header "Configurando Ambiente"
    
    if [ "$INSTALL_MODE" = "system" ]; then
        local mixr_root="/usr/local/mixr"
        local mixr_3rdparty="/usr/local/mixr-platform/mixr-3rdparty"
    else
        local mixr_root="$(resolve_path "$MIXR_INSTALL_DIR")"
        local mixr_3rdparty="$(resolve_path "$MIXR_PLATFORM_DIR")/mixr-3rdparty"
    fi
    
    # Adiciona ao .bashrc se ainda não existir
    if ! grep -q "MIXR_ROOT" "$HOME/.bashrc"; then
        print_info "Adicionando variáveis ao .bashrc..."
        cat >> "$HOME/.bashrc" << EOF

# ============================================
# MIXR Environment
# ============================================
export MIXR_ROOT="$mixr_root"
export MIXR_3RDPARTY="$mixr_3rdparty"
export PATH="\$MIXR_ROOT/bin:\$PATH"
export LD_LIBRARY_PATH="\$MIXR_ROOT/lib:\$MIXR_3RDPARTY/lib:\$LD_LIBRARY_PATH"

# Source MIXR setenv if exists
if [ -f "\$MIXR_ROOT/setenv.sh" ]; then
    source "\$MIXR_ROOT/setenv.sh" 2>/dev/null
fi

# Aliases úteis
alias mixr-root="cd \$MIXR_ROOT"
alias mixr-src="cd \$MIXR_ROOT/src"
alias mixr-examples="cd ${BASE_DIR}/cpp/mixr-examples"
alias mixr-platform="cd $mixr_3rdparty/.."
EOF
        print_success "Variáveis adicionadas ao .bashrc"
    else
        print_info "Variáveis MIXR já existem no .bashrc"
    fi
}

# ============================================
# LIMPEZA
# ============================================

cleanup() {
    print_header "Limpeza"
    
    if [ -d "$DOWNLOAD_DIR" ]; then
        print_info "Removendo arquivos temporários..."
        rm -rf "$DOWNLOAD_DIR"
        print_success "Arquivos temporários removidos"
    fi
}

# ============================================
# VERIFICAÇÃO FINAL
# ============================================

verify_installation() {
    print_header "Verificando Instalação"
    
    if [ "$INSTALL_MODE" = "system" ]; then
        local mixr_root="/usr/local/mixr"
        local mixr_3rdparty="/usr/local/mixr-platform/mixr-3rdparty"
    else
        local mixr_root="$(resolve_path "$MIXR_INSTALL_DIR")"
        local mixr_3rdparty="$(resolve_path "$MIXR_PLATFORM_DIR")/mixr-3rdparty"
    fi
    
    echo ""
    
    # Verifica diretórios
    if [ -d "$mixr_root" ]; then
        print_success "MIXR Root: $mixr_root"
    else
        print_error "MIXR Root não encontrado"
    fi
    
    if [ -d "$mixr_root/src" ]; then
        print_success "MIXR Source encontrado"
    else
        print_warning "MIXR Source não encontrado"
    fi
    
    if [ -d "$mixr_3rdparty" ]; then
        print_success "Dependências de terceiros: $mixr_3rdparty"
    else
        print_warning "Dependências de terceiros não encontradas"
    fi
    
    local examples_dir="${BASE_DIR}/cpp/mixr-examples"
    if [ -d "$examples_dir" ]; then
        print_success "Exemplos: $examples_dir"
    else
        print_warning "Exemplos não encontrados"
    fi
    
    echo ""
    
    # Verifica bibliotecas
    if [ -d "$mixr_root/lib" ]; then
        local lib_count=$(find "$mixr_root/lib" -name "*.so" -o -name "*.a" 2>/dev/null | wc -l)
        print_success "Bibliotecas encontradas: $lib_count"
        if [ $lib_count -gt 0 ]; then
            print_info "Primeiras 5 bibliotecas:"
            find "$mixr_root/lib" -name "*.so" -o -name "*.a" 2>/dev/null | head -5 | while read lib; do
                echo "    - $(basename $lib)"
            done
        fi
    else
        print_warning "Diretório lib não encontrado"
    fi
    
    echo ""
}

# ============================================
# FUNÇÃO PRINCIPAL
# ============================================

main() {
    clear
    
    print_header "Instalador MIXR para Ubuntu/Debian"
    echo ""
    print_info "Modo de instalação: $INSTALL_MODE"
    print_info "Diretório base: $BASE_DIR"
    print_info "Diretório de instalação: $MIXR_PLATFORM_DIR"
    echo ""
    
    # Mostra URLs se configuradas
    if [ -n "$MIXR_3RDPARTY_URL" ]; then
        print_info "3rd Party URL: $MIXR_3RDPARTY_URL"
    fi
    if [ -n "$MIXR_SOURCE_URL" ]; then
        print_info "Source URL: $MIXR_SOURCE_URL"
    fi
    if [ -n "$MIXR_EXAMPLES_URL" ]; then
        print_info "Examples URL: $MIXR_EXAMPLES_URL"
    fi
    echo ""
    
    # Verifica modo
    if [ "$INSTALL_MODE" = "system" ] && ! check_root; then
        print_warning "Instalação de sistema requer sudo"
        print_info "O script solicitará senha quando necessário"
    fi
    
    read -p "Deseja continuar? (s/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Ss]$ ]]; then
        print_info "Instalação cancelada"
        exit 0
    fi
    
    # Executa instalação
    install_prerequisites
    download_files
    setup_directories
    extract_files
    build_dependencies
    build_mixr
    # build_examples
    setup_environment
    cleanup
    verify_installation
    
    # Mensagem final
    echo ""
    print_header "Instalação Concluída!"
    echo ""
    print_success "MIXR foi instalado com sucesso!"
    echo ""
    print_info "Para usar o MIXR:"
    echo "  1. Execute: source ~/.bashrc"
    echo "  2. Ou abra um novo terminal"
    echo ""
    print_info "Comandos disponíveis:"
    echo "  mixr-root      - Vai para diretório MIXR"
    echo "  mixr-src       - Vai para código fonte"
    echo "  mixr-examples  - Vai para exemplos"
    echo "  mixr-platform  - Vai para mixr-platform"
    echo ""
    
    print_info "Para recarregar o ambiente agora, execute:"
    echo "  source ~/.bashrc"
    echo ""
}

# ============================================
# AJUDA
# ============================================

show_help() {
    cat << EOF
Uso: $0 [MODO]

MODOS:
  user     - Instala no diretório atual/cpp (padrão)
  system   - Instala como componente de sistema (/usr/local)
  help     - Mostra esta mensagem

VARIÁVEIS DE AMBIENTE:
  MIXR_BASE_DIR       - Diretório base para instalação (padrão: diretório atual)
  MIXR_3RDPARTY_URL   - URL para download de mixr-3rdpartysrc.tgz
  MIXR_SOURCE_URL     - URL para download de mixr_vX.X.zip
  MIXR_EXAMPLES_URL   - URL para download de mixr-examples.zip
  
  MIXR_3RDPARTY_FILE  - Caminho para arquivo local (se não usar URL)
  MIXR_SOURCE_FILE    - Caminho para arquivo local (se não usar URL)
  MIXR_EXAMPLES_FILE  - Caminho para arquivo local (se não usar URL)

EXEMPLOS:

  1. Instalação no diretório atual (download automático):
     $0

  2. Instalação em diretório específico:
     MIXR_BASE_DIR="/opt/myproject" $0

  3. Usando arquivos locais:
     MIXR_3RDPARTY_FILE="/path/to/mixr-3rdpartysrc.tgz" \\
     MIXR_SOURCE_FILE="/path/to/mixr_v18.01.zip" \\
     $0

  4. Instalação de sistema:
     $0 system

ESTRUTURA DE INSTALAÇÃO (modo usuário):
  \$BASE_DIR/cpp/mixr              - Código fonte MIXR
  \$BASE_DIR/cpp/mixr-platform     - Dependências
  \$BASE_DIR/cpp/mixr-examples     - Exemplos

EOF
}

# ============================================
# TRATAMENTO DE INTERRUPÇÃO
# ============================================

trap 'echo ""; print_warning "Instalação interrompida pelo usuário"; cleanup; exit 130' INT TERM

# ============================================
# PONTO DE ENTRADA
# ============================================

if [ "$1" = "help" ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    show_help
    exit 0
fi

# Executa instalação
main