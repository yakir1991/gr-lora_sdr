#!/bin/sh
set -e

build_dir="build-aarch64"
out_dir="results"

cmake -S . -B "$build_dir" \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
  -DLORA_LITE_BENCHMARK=ON \
  -DLORA_LITE_USE_LIQUID_FFT=OFF \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir"

(cd "$build_dir/tests" && qemu-aarch64 -L /usr/aarch64-linux-gnu ./bench_lora_chain)

mkdir -p "$out_dir"
cp "$build_dir/tests/bench_results.csv" "$out_dir/arm_qemu.csv"
