#!/bin/sh
set -e

BUILD_DIR="$(pwd)/build"
RESULTS_DIR="$(pwd)/results"
CSV="$RESULTS_DIR/bench.csv"

cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLORA_LITE_BENCHMARK=ON
cmake --build "$BUILD_DIR"

mkdir -p "$RESULTS_DIR"
echo "CWD: $(pwd)"
echo "RESULTS_DIR: $RESULTS_DIR"

"$BUILD_DIR/tests/bench_lora_chain" "$CSV" || {
  code=$?
  echo "bench_lora_chain failed with exit code $code" >&2
  exit $code
}

if [ ! -s "$CSV" ]; then
  echo "bench_lora_chain did not produce CSV at $CSV" >&2
  exit 1
fi

perf stat -e cycles,instructions,branches,branch-misses,cache-misses -- "$BUILD_DIR/tests/bench_lora_chain" "$CSV" 2> "$RESULTS_DIR/profile_perf.txt" || {
  code=$?
  echo "perf stat: bench_lora_chain failed with exit code $code" >&2
  exit $code
}
valgrind --tool=massif --massif-out-file="$RESULTS_DIR/profile_massif.txt" "$BUILD_DIR/tests/bench_lora_chain" "$CSV" >/dev/null 2>&1 || {
  code=$?
  echo "valgrind: bench_lora_chain failed with exit code $code" >&2
  exit $code
}
