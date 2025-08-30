#!/usr/bin/env bash
set -euo pipefail

# Usage: run_fft_bench_remote.sh PRESET user@host:/remote/dir [tag]
# Builds with a CMake preset, ships bench + lib to remote, runs it, brings back
# results, and appends FFT metrics to bench_emb.csv with an identifying tag.

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

BIN="${BUILDDIR}/tests/bench_fft"
LIB="${BUILDDIR}/src/liblora_fft.so"
if [[ ! -x "${BIN}" ]]; then echo "Missing bench binary: ${BIN}" >&2; exit 5; fi
if [[ ! -f "${LIB}" ]]; then echo "Missing lib: ${LIB}" >&2; exit 6; fi

TARGET_HOST="${TARGET_SPEC%%:*}"
TARGET_DIR="${TARGET_SPEC#*:}"
ssh "${TARGET_HOST}" "mkdir -p '${TARGET_DIR}'"
scp "${BIN}" "${LIB}" "${TARGET_SPEC}/"

REMOTE_OUT="${TARGET_DIR}/bench_fft.out"
echo "[+] Running bench on target -> ${REMOTE_OUT}"
ssh "${TARGET_HOST}" "cd '${TARGET_DIR}' && chmod +x bench_fft && LD_LIBRARY_PATH=. ./bench_fft > bench_fft.out"

LOCAL_OUT="bench_fft_${TAG}.out"
scp "${TARGET_HOST}:${REMOTE_OUT}" "${LOCAL_OUT}"
echo "[+] Results copied to ${LOCAL_OUT}"

echo "[+] Appending FFT metrics to bench_emb.csv with tag: ${TAG}"
{
  echo "fft_target,${TAG}"
  # Capture fft_backend and all fft_us_per_exec lines
  awk -F',' '/^fft_backend/ {print $0} /^fft_us_per_exec/ {print $0}' "${LOCAL_OUT}"
} >> bench_emb.csv

echo "[+] Done. Appended the following:" && tail -n +1 "${LOCAL_OUT}" | awk -F',' '/^fft_backend|^fft_us_per_exec/ {print $0}'

