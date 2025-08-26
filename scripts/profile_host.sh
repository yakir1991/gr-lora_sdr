#!/usr/bin/env bash
set -Eeuo pipefail

RESULTS_DIR="${RESULTS_DIR:-bench_out}"
BUILD_DIR="${BUILD_DIR:-build}"
CSV_PATH="${RESULTS_DIR}/host_profile.csv"

mkdir -p "$RESULTS_DIR"

echo "[build] Release + tests"
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build "$BUILD_DIR" -j"$(nproc)"

echo "[bench] writing CSV -> ${CSV_PATH}"
"./${BUILD_DIR}/tests/bench_lora_chain" "${CSV_PATH}" || {
  echo "[ERROR] bench_lora_chain failed"; exit 1;
}

maybe_run_perf() {
  if ! command -v perf >/dev/null 2>&1; then
    echo "[perf] not installed – skipping"
    return 0
  fi
  set +e
  perf stat -d -d -d -r 3 -- "./${BUILD_DIR}/tests/bench_lora_chain" /dev/null
  rc=$?
  set -e
  if [ "$rc" -ne 0 ]; then
    echo "[perf] no permission/unsupported on runner – skipping (rc=${rc})"
  fi
  return 0
}

maybe_run_perf

# Optional lightweight timing (לא מפיל את ה-job)
if command -v /usr/bin/time >/dev/null 2>&1; then
  echo "[time] collecting wall/CPU/mem (best-effort)"
  /usr/bin/time -v "./${BUILD_DIR}/tests/bench_lora_chain" /dev/null || true
fi

echo "[done] CSV at ${CSV_PATH}"
