#!/usr/bin/env bash
set -euo pipefail

# Usage: run_fft_q15_remote.sh PRESET user@host:/remote/dir [tag]
# Builds bench_fft_q15 with the given CMake preset (expects ARM toolchain/CMSIS
# in the preset), ships the bench + required libs to the remote target, runs it,
# fetches the output, and appends key metrics to bench_emb.csv.

if [[ $# -lt 2 ]]; then
  echo "Usage: $0 PRESET user@host:/remote/dir [tag]" >&2
  exit 2
fi

PRESET="$1"
TARGET_SPEC="$2"
TAG="${3:-$PRESET}"

echo "[+] Configuring preset: ${PRESET}"
cmake --preset "${PRESET}"
echo "[+] Building: ${PRESET}-build"
cmake --build --preset "${PRESET}-build" -j

# Resolve build dir from presets
BUILDDIR=$(jq -r '.configurePresets[] | select(.name=="'"${PRESET}"'") | .binaryDir' CMakePresets.json)
if [[ -z "${BUILDDIR}" || ! -d "${BUILDDIR}" ]]; then
  echo "Could not resolve build dir from CMakePresets.json" >&2
  exit 4
fi

BIN="${BUILDDIR}/tests/bench_fft_q15"
LIB="${BUILDDIR}/src/liblora_fft.so"
if [[ ! -x "${BIN}" ]]; then echo "Missing bench binary: ${BIN}" >&2; exit 5; fi
if [[ ! -f "${LIB}" ]]; then echo "Missing lib: ${LIB}" >&2; exit 6; fi

TARGET_HOST="${TARGET_SPEC%%:*}"
TARGET_DIR="${TARGET_SPEC#*:}"
ssh "${TARGET_HOST}" "mkdir -p '${TARGET_DIR}'"
scp "${BIN}" "${LIB}" "${TARGET_SPEC}/"

REMOTE_OUT="${TARGET_DIR}/bench_fft_q15.out"
echo "[+] Running bench on target -> ${REMOTE_OUT}"
ssh "${TARGET_HOST}" "cd '${TARGET_DIR}' && chmod +x bench_fft_q15 && LD_LIBRARY_PATH=. ./bench_fft_q15 > bench_fft_q15.out"

LOCAL_OUT="bench_fft_q15_${TAG}.out"
scp "${TARGET_HOST}:${REMOTE_OUT}" "${LOCAL_OUT}"
echo "[+] Results copied to ${LOCAL_OUT}"

echo "[+] Appending Q15 FFT metrics to bench_emb.csv with tag: ${TAG}"
{
  echo "fft_q15_target,${TAG}"
  awk -F',' '/^q15_backend/ {print $0} /^q15_us_per_exec/ {print $0} /^float_us_per_exec/ {print $0} /^q15_vs_float_/ {print $0}' "${LOCAL_OUT}"
} >> bench_emb.csv

echo "[+] Done. Appended the following:"
awk -F',' '/^q15_backend|^q15_us_per_exec|^float_us_per_exec|^q15_vs_float_/ {print $0}' "${LOCAL_OUT}"

