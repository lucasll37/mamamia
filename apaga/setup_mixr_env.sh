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
