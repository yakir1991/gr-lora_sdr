#!/usr/bin/env bash
set -Eeuo pipefail

BUILD_TYPE="${BUILD_TYPE:-Release}"
TIME_TAG="$(date +%Y%m%d-%H%M%S)"
OUT_DIR="${OUT_DIR:-bench_out/$TIME_TAG}"

need_liquid_dev() {
  if ! dpkg -s libliquid-dev >/dev/null 2>&1; then
    echo "[hint] sudo apt update && sudo apt install -y libliquid-dev" >&2
  fi
}

run_one() {
  local use_liq="$1"
  local bdir="build-${use_liq,,}"
  local csv="${OUT_DIR}/bench_${use_liq}.csv"

  echo ""
  echo "=== Building (${BUILD_TYPE}, LORA_LITE_USE_LIQUID_FFT=${use_liq}) -> ${bdir} ==="
  cmake -S . -B "$bdir" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DBUILD_TESTING=ON \
    -DLORA_LITE_USE_LIQUID_FFT="${use_liq}"
  cmake --build "$bdir" -j"$(nproc)"

  mkdir -p "${OUT_DIR}"
  echo "--- Running bench_lora_chain -> ${csv}"
  "./${bdir}/tests/bench_lora_chain" "${csv}"

  local pps
  pps="$(awk -F, 'NR==2 {print $3}' "${csv}" 2>/dev/null || true)"
  echo "RESULT: USE_LIQUID=${use_liq} packets_per_sec=${pps}"
}

mkdir -p "${OUT_DIR}"
run_one "OFF"
run_one "ON"

echo ""
echo "[done] Baselines saved:"
ls -1 "${OUT_DIR}"/bench_*.csv
