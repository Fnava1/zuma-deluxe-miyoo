#!/bin/bash
# Cross-compile Zuma Deluxe for Miyoo Mini / OnionOS using Docker toolchain
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=========================================================="
echo " Building Zuma Deluxe for Miyoo Mini (Cortex-A7 NEON)"
echo " Toolchain container: aemiii91/miyoomini-toolchain:latest"
echo "=========================================================="

docker run --rm -v "$REPO_ROOT:/root/workspace" aemiii91/miyoomini-toolchain:latest /bin/bash -c "
  cd /root/workspace && \
  rm -rf build-miyoo && \
  mkdir -p build-miyoo && \
  cd build-miyoo && \
  cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/miyoomini.toolchain.cmake -DCMAKE_BUILD_TYPE=Release .. && \
  make -j\$(nproc)
"

if [ -f "$REPO_ROOT/build-miyoo/source/CircleShoot/Zuma" ]; then
    echo ""
    echo "=========================================================="
    echo " BUILD SUCCESSFUL!"
    echo " Binary generated: build-miyoo/source/CircleShoot/Zuma"
    echo "=========================================================="
    mkdir -p "$REPO_ROOT/bin"
    cp "$REPO_ROOT/build-miyoo/source/CircleShoot/Zuma" "$REPO_ROOT/bin/Zuma"
    echo " Copied to: bin/Zuma"
else
    echo "Error: Binary not found after compilation."
    exit 1
fi
