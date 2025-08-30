#!/usr/bin/env bash
set -euo pipefail

# Build a local embedded-profile (host) and run bench_lora_chain, then
# append its CSV output to bench_emb.csv for tracking.

BUILD_DIR="build-emb-chain"

echo "[+] Configuring ${BUILD_DIR}"
cmake -S . -B "${BUILD_DIR}" \
  -DLORA_LITE_BENCHMARK=ON \
  -DLORA_LITE_USE_LIQUID_FFT=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS_RELEASE="-Os -flto -ffunction-sections -fdata-sections" \
  -DCMAKE_EXE_LINKER_FLAGS_RELEASE="-Wl,--gc-sections"

echo "[+] Building"
cmake --build "${BUILD_DIR}" -j

echo "[+] Running bench_lora_chain"
pushd "${BUILD_DIR}/tests" >/dev/null
./bench_lora_chain bench_results.csv
popd >/dev/null

CSV_OUT="${BUILD_DIR}/tests/bench_results.csv"
if [[ ! -f "${CSV_OUT}" ]]; then
  echo "bench_results.csv not found" >&2
  exit 3
fi

echo "[+] Appending results to bench_emb.csv"
cat "${CSV_OUT}" >> bench_emb.csv
echo "[+] Done. Appended:" && tail -n +1 "${CSV_OUT}"

