#!/usr/bin/env bash
set -Eeuo pipefail

BUILD_TYPE="${BUILD_TYPE:-Release}"
TIME_TAG="$(date +%Y%m%d-%H%M%S)"
OUT_DIR="${OUT_DIR:-bench_out/$TIME_TAG}"

run_one() {
  local use_liq="$1"             # ON / OFF
  local bdir="build-chain-${use_liq,,}"
  local csv="${OUT_DIR}/bench_chain_${use_liq}.csv"

  cmake -S . -B "$bdir" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DBUILD_TESTING=ON -DLORA_LITE_BENCHMARK=ON -DLORA_LITE_USE_LIQUID_FFT="${use_liq}"
  cmake --build "$bdir" -j"$(nproc)"
  mkdir -p "${OUT_DIR}"
  "./${bdir}/tests/bench_lora_chain" "${csv}" >/dev/null
  echo "saved ${csv}"
}

mkdir -p "${OUT_DIR}"
run_one OFF
run_one ON || true

echo "files under ${OUT_DIR}:"
ls -1 "${OUT_DIR}"/bench_chain_*.csv

