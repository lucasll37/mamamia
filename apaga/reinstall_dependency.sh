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
