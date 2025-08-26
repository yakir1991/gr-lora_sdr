#!/usr/bin/env bash
set -Eeuo pipefail
BUILD_TYPE="${BUILD_TYPE:-Release}"
DURATION_SEC="${DURATION_SEC:-15}"
INTERVAL_MS="${INTERVAL_MS:-50}"
TIME_TAG="${TIME_TAG:-$(date +%Y%m%d-%H%M%S)}"
OUT_ROOT="${OUT_ROOT:-power_out/$TIME_TAG}"
EXTRA_CMAKE_ARGS="${EXTRA_CMAKE_ARGS:-}"

run_cfg() {
  local cfg="$1" # OFF / ON
  local bdir="build-power-${cfg,,}"
  local dir="${OUT_ROOT}/${cfg}"
  mkdir -p "${dir}"

  echo "=== Building (${BUILD_TYPE}, LORA_LITE_USE_LIQUID_FFT=${cfg}) -> ${bdir}"
  cmake -S . -B "$bdir" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DBUILD_TESTING=OFF \
    -DLORA_LITE_USE_LIQUID_FFT="${cfg}" \
    ${EXTRA_CMAKE_ARGS}
  cmake --build "$bdir" -j"$(nproc)"

  echo "--- Warm-up (1x)"
  "./${bdir}/tests/bench_lora_chain" /dev/null >/dev/null 2>&1 || true

  echo "--- Sampling power for ${DURATION_SEC}s"
  # Run a tight loop of bench to keep the CPU busy during sampling window
  # Collect stdout to parse pps across iterations
  timeout "${DURATION_SEC}s" bash -lc '
    i=0
    while true; do
      i=$((i+1))
      "./'"${bdir}"'/tests/bench_lora_chain" /dev/null
    done
  ' | tee "${dir}/bench_stdout.log" &

  sampler_pid=""
  set +e
  ./scripts/energy/sample_energy.py --out "${dir}/energy.csv" --interval-ms "${INTERVAL_MS}" -- -- sleep "${DURATION_SEC}" &
  sampler_pid=$!
  wait $sampler_pid
  set -e

  ./scripts/energy/power_reduce.py "${dir}/energy.csv" "${dir}/bench_stdout.log" "${dir}/summary.csv"
}

mkdir -p "${OUT_ROOT}"
chmod +x scripts/energy/sample_energy.py scripts/energy/power_reduce.py scripts/energy/energy_compare.py

run_cfg "OFF"
run_cfg "ON"

echo ""
./scripts/energy/energy_compare.py "${OUT_ROOT}"
echo "[done] Power results in ${OUT_ROOT}"
find "${OUT_ROOT}" -type f -maxdepth 2 -print
