#!/bin/bash
# Arquivo setenv.sh local para build do MIXR

# Define MIXR_ROOT como o diretório do projeto
export MIXR_ROOT="/home/lucas/Projects/mamamia/cpp/mixr_dev/mixr"

# Define onde as dependências 3rd party serão instaladas
export MIXR_3RD_PARTY_ROOT="/home/lucas/Projects/mamamia/cpp/mixr_dev/mixr-3rdparty"

# Adiciona ao PATH
export PATH="${MIXR_3RD_PARTY_ROOT}/bin:${PATH}"

# Adiciona ao LD_LIBRARY_PATH
export LD_LIBRARY_PATH="${MIXR_3RD_PARTY_ROOT}/lib:${LD_LIBRARY_PATH}"

# PKG_CONFIG_PATH
export PKG_CONFIG_PATH="${MIXR_3RD_PARTY_ROOT}/lib/pkgconfig:${PKG_CONFIG_PATH}"

# Flags de compilação
export CFLAGS="-I${MIXR_3RD_PARTY_ROOT}/include"
export CXXFLAGS="-I${MIXR_3RD_PARTY_ROOT}/include"
export LDFLAGS="-L${MIXR_3RD_PARTY_ROOT}/lib"

echo "Ambiente MIXR configurado:"
echo "  MIXR_ROOT: ${MIXR_ROOT}"
echo "  MIXR_3RD_PARTY_ROOT: ${MIXR_3RD_PARTY_ROOT}"
