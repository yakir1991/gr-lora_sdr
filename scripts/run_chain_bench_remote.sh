#!/usr/bin/env bash
set -euo pipefail

# Usage: run_chain_bench_remote.sh PRESET user@host:/remote/dir [tag]
# Builds with a CMake preset, ships bench_lora_chain and dependent .so files
# to the remote target, runs it, fetches CSV results, and appends to bench_emb.csv.

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

BUILDDIR=$(jq -r '.configurePresets[] | select(.name=="'"${PRESET}"'") | .binaryDir' CMakePresets.json)
if [[ -z "${BUILDDIR}" || ! -d "${BUILDDIR}" ]]; then
  echo "Could not resolve build dir from CMakePresets.json" >&2
  exit 4
fi

BIN="${BUILDDIR}/tests/bench_lora_chain"
if [[ ! -x "${BIN}" ]]; then echo "Missing bench binary: ${BIN}" >&2; exit 5; fi

# Collect shared libs needed at runtime from build tree
LIBDIR="${BUILDDIR}/src"
DEPS=(
  "${LIBDIR}/liblora_fft.so"
  "${LIBDIR}/liblora_utils.so"
  "${LIBDIR}/libsignal_utils.so"
  "${LIBDIR}/liblora_tx_chain.so"
  "${LIBDIR}/liblora_rx_chain.so"
  "${LIBDIR}/liblora_io.so"
  "${LIBDIR}/liblora_add_crc.so"
  "${LIBDIR}/liblora_payload_id.so"
  "${LIBDIR}/liblora_whitening.so"
  "${LIBDIR}/liblora_hamming.so"
  "${LIBDIR}/liblora_interleaver.so"
  "${LIBDIR}/liblora_graymap.so"
  "${LIBDIR}/liblora_header.so"
  "${LIBDIR}/liblora_frame_sync.so"
)

TARGET_HOST="${TARGET_SPEC%%:*}"
TARGET_DIR="${TARGET_SPEC#*:}"
ssh "${TARGET_HOST}" "mkdir -p '${TARGET_DIR}'"

echo "[+] Copying bench + libs to ${TARGET_SPEC}"
scp "${BIN}" ${DEPS[@]} "${TARGET_SPEC}/"

echo "[+] Running chain bench on target"
ssh "${TARGET_HOST}" "cd '${TARGET_DIR}' && chmod +x bench_lora_chain && LD_LIBRARY_PATH=. ./bench_lora_chain bench_results.csv"

LOCAL_CSV="bench_chain_${TAG}.csv"
scp "${TARGET_HOST}:${TARGET_DIR}/bench_results.csv" "${LOCAL_CSV}"

echo "[+] Appending chain PPS results to bench_emb.csv"
cat "${LOCAL_CSV}" >> bench_emb.csv
echo "[+] Done. Appended:" && tail -n +1 "${LOCAL_CSV}"

