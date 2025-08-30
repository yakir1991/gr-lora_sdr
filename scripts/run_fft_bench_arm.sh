#!/usr/bin/env bash
set -euo pipefail

# Usage: run_fft_bench_arm.sh user@host:/remote/dir [preset]
# Env: ARM_TOOLCHAIN_FILE=/path/to/your/toolchain.cmake
# Builds with CMake preset 'arm-embedded-release', scp's bench + lib to target and runs bench_fft.

if [[ $# -lt 1 ]]; then
  echo "Usage: $0 user@host:/remote/dir [preset]" >&2
  exit 2
fi

TARGET_SPEC="$1"   # e.g., pi@192.168.1.10:/home/pi/lora_bench
PRESET="${2:-arm-embedded-release}"

if [[ -z "${ARM_TOOLCHAIN_FILE:-}" ]]; then
  echo "ARM_TOOLCHAIN_FILE not set; export ARM_TOOLCHAIN_FILE to a valid toolchain file." >&2
  exit 3
fi

echo "[+] Configuring preset: ${PRESET}"
cmake --preset "${PRESET}"
echo "[+] Building: ${PRESET}-build"
cmake --build --preset "${PRESET}-build" -j

BUILDDIR=$(jq -r '.configurePresets[] | select(.name=="'"${PRESET}"'") | .binaryDir' CMakePresets.json)
if [[ -z "${BUILDDIR}" || ! -d "${BUILDDIR}" ]]; then
  echo "Could not resolve build dir from CMakePresets.json" >&2
  exit 4
fi

BIN="${BUILDDIR}/tests/bench_fft"
LIB="${BUILDDIR}/src/liblora_fft.so"
if [[ ! -x "${BIN}" ]]; then echo "Missing bench binary: ${BIN}" >&2; exit 5; fi
if [[ ! -f "${LIB}" ]]; then echo "Missing lib: ${LIB}" >&2; exit 6; fi

TARGET_HOST="${TARGET_SPEC%%:*}"
TARGET_DIR="${TARGET_SPEC#*:}"

echo "[+] Creating remote dir ${TARGET_DIR}"
ssh "${TARGET_HOST}" "mkdir -p '${TARGET_DIR}'"

echo "[+] Copying bench + lib to ${TARGET_SPEC}"
scp "${BIN}" "${LIB}" "${TARGET_SPEC}/"

echo "[+] Running bench on target"
ssh "${TARGET_HOST}" "cd '${TARGET_DIR}' && chmod +x bench_fft && LD_LIBRARY_PATH=. ./bench_fft" | tee bench_arm_fft.out

echo "[+] Done. Results captured to bench_arm_fft.out"

