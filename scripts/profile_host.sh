#!/bin/sh
set -e

build_dir="build"
out_dir="results"

cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DLORA_LITE_BENCHMARK=ON
cmake --build "$build_dir"

mkdir -p "$out_dir"
perf stat -e cycles,instructions,branches,branch-misses,cache-misses -- "./$build_dir/tests/bench_lora_chain" 2> "$out_dir/profile_perf.txt"
valgrind --tool=massif --massif-out-file="$out_dir/profile_massif.txt" "./$build_dir/tests/bench_lora_chain" >/dev/null 2>&1
