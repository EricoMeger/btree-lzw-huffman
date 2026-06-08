#!/usr/bin/env bash
#
# Esse script:
#   1. Cria o diretório workspace/ ao lado do repositório treectl
#   2. Move o treectl para dentro dele
#   3. Clona o repositório do compressor
#   4. Copia o CMakeLists.txt raiz para o workspace
#
# Uso:
#   ./setup.sh [--workspace <dir>]
#
# Exemplos:
#   ./setup.sh
#   ./setup.sh --workspace ~/projetos/trabalho2

set -euo pipefail

COMPRESSOR_REPO="https://github.com/Eroshla/huffman-lzw-compressor/"
WORKSPACE_NAME="trabalho2-workspace"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --workspace)
            WORKSPACE_NAME="$2"; shift 2 ;;
        --help)
            grep '^#' "$0" | sed 's/^# \?//'
            exit 0 ;;
        *)
            echo "Argumento desconhecido: $1"; exit 1 ;;
    esac
done

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TREECTL_NAME="$(basename "$SCRIPT_DIR")"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"
WORKSPACE="$PARENT_DIR/$WORKSPACE_NAME"

echo " ==== treectl workspace setup ===="
echo ""
echo "  treectl:    $SCRIPT_DIR"
echo "  workspace:  $WORKSPACE"
echo "  compressor: $COMPRESSOR_REPO"
echo ""

if [[ -d "$WORKSPACE" ]]; then
    echo " Workspace já existe em $WORKSPACE"
    read -rp "   Deseja recriar? Isso apagará o diretório atual. [s/N] " confirm
    if [[ "$confirm" =~ ^[Ss]$ ]]; then
        rm -rf "$WORKSPACE"
    else
        echo "Abortando."
        exit 0
    fi
fi

mkdir -p "$WORKSPACE"
echo " Workspace criado em $WORKSPACE"

cp -r "$SCRIPT_DIR" "$WORKSPACE/treectl"
echo " treectl copiado para $WORKSPACE/treectl"

echo ""
echo "Clonando compressor de $COMPRESSOR_REPO ..."
if git clone "$COMPRESSOR_REPO" "$WORKSPACE/huffman-lzw-compressor"; then
    echo " Compressor clonado em $WORKSPACE/huffman-lzw-compressor"
else
    echo " Falha ao clonar o compressor."
    exit 1
fi

cat > "$WORKSPACE/CMakeLists.txt" << 'CMAKE_EOF'
cmake_minimum_required(VERSION 3.16)
project(trabalho2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(TREECTL_USE_COMPRESSION
    "Linka a lib do compressor no treectl (habilita save/load com compressão)" ON)

add_subdirectory(huffman-lzw-compressor)

add_subdirectory(treectl)
CMAKE_EOF

echo " CMakeLists.txt raiz criado"

echo ""
echo " ==== Setup concluído! Para compilar: ===="
echo ""
echo "  cd $WORKSPACE"
echo "  cmake -B build"
echo "  cmake --build build"
echo ""
echo "  Binários gerados em:"
echo "    build/treectl/treectl"
echo "    build/huffman-lzw-compressor/huffman-lzw-compressor"
echo ""
echo "  Para compilar sem integração de compressão:"
echo "    cmake -B build -DTREECTL_USE_COMPRESSION=OFF"
echo ""
