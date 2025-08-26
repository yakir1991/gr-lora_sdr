#!/bin/sh
set -e

build_dir="build"
out_dir="results"

cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLORA_LITE_BENCHMARK=ON
cmake --build "$build_dir"

mkdir -p "$out_dir"
"./$build_dir/tests/bench_lora_chain" || {
  code=$?
  echo "bench_lora_chain failed with exit code $code" >&2
  exit $code
}
perf stat -e cycles,instructions,branches,branch-misses,cache-misses -- "./$build_dir/tests/bench_lora_chain" 2> "$out_dir/profile_perf.txt" || {
  code=$?
  echo "perf stat: bench_lora_chain failed with exit code $code" >&2
  exit $code
}
valgrind --tool=massif --massif-out-file="$out_dir/profile_massif.txt" "./$build_dir/tests/bench_lora_chain" >/dev/null 2>&1 || {
  code=$?
  echo "valgrind: bench_lora_chain failed with exit code $code" >&2
  exit $code
}
