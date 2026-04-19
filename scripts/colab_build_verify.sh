#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="${1:-$(pwd)}"
BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build-colab}"
CONFIG="${CONFIG:-Release}"
SIZE="${SIZE:-64}"
STEPS="${STEPS:-50}"
SEED="${SEED:-12345}"
ALIVE_PROB="${ALIVE_PROB:-0.3}"
BLOCK_X="${BLOCK_X:-8}"
BLOCK_Y="${BLOCK_Y:-8}"
BLOCK_Z="${BLOCK_Z:-8}"

version_ge() {
  [ "$(printf '%s\n' "$2" "$1" | sort -V | head -n1)" = "$2" ]
}

ensure_python_package() {
  local package="$1"
  python3 -m pip install -q --user "$package"
  export PATH="${HOME}/.local/bin:${PATH}"
}

if ! command -v g++ >/dev/null 2>&1; then
  apt-get update
  apt-get install -y build-essential
fi

if ! command -v cmake >/dev/null 2>&1; then
  ensure_python_package cmake
fi

CMAKE_VERSION="$(cmake --version | head -n1 | awk '{print $3}')"
if ! version_ge "${CMAKE_VERSION}" "3.24"; then
  ensure_python_package cmake
  CMAKE_VERSION="$(cmake --version | head -n1 | awk '{print $3}')"
fi

if ! command -v nvidia-smi >/dev/null 2>&1; then
  echo "nvidia-smi not found. In Colab, switch to a GPU runtime first."
  exit 1
fi

if ! command -v nvcc >/dev/null 2>&1; then
  echo "nvcc not found. This runtime does not expose the CUDA toolkit compiler."
  exit 1
fi

echo "=== Toolchain ==="
nvidia-smi
nvcc --version
cmake --version
g++ --version | head -n1

cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${CONFIG}" \
  -DCUDA_LAB_ENABLE_CUDA=ON \
  -DCMAKE_CUDA_ARCHITECTURES=native

cmake --build "${BUILD_DIR}" --config "${CONFIG}" -j"$(nproc)"

SEQ_BIN="$(find "${BUILD_DIR}" -type f -name ca_seq | head -n1)"
CUDA_BIN="$(find "${BUILD_DIR}" -type f -name ca_cuda | head -n1)"

if [ -z "${SEQ_BIN}" ] || [ -z "${CUDA_BIN}" ]; then
  echo "Failed to locate built executables in ${BUILD_DIR}."
  exit 1
fi

echo
echo "=== Sequential Run ==="
"${SEQ_BIN}" \
  --size "${SIZE}" \
  --steps "${STEPS}" \
  --seed "${SEED}" \
  --alive-prob "${ALIVE_PROB}"

echo
echo "=== CUDA Run With Verification ==="
"${CUDA_BIN}" \
  --size "${SIZE}" \
  --steps "${STEPS}" \
  --seed "${SEED}" \
  --alive-prob "${ALIVE_PROB}" \
  --block-x "${BLOCK_X}" \
  --block-y "${BLOCK_Y}" \
  --block-z "${BLOCK_Z}" \
  --verify
