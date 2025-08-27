#!/usr/bin/env bash
set -Eeuo pipefail

INCLUDE_FIXED="${INCLUDE_FIXED:-0}"
EMBED="${EMBED:-0}"
LOG="${LOG:-0}"

TIME_TAG="$(date +%Y%m%d-%H%M%S)"
RAW_DIR="bench_out/emb-${TIME_TAG}"
MATRIX_DIR="bench_out/emb-matrix-${TIME_TAG}"
SUMMARY="${MATRIX_DIR}/SUMMARY.md"

mkdir -p "$RAW_DIR" "$MATRIX_DIR"

log_opt=$([ "$LOG" -eq 1 ] && echo ON || echo OFF)

# Metadata
cpu=$(grep -m1 'model name' /proc/cpuinfo | sed 's/^.*: //')
cores=$(nproc)
compiler=$(cc --version | head -n1)

{
  echo "# Full Test & Bench Matrix"
  echo ""
  echo "Date: $(date)"
  echo ""
  echo "## System"
  echo "- CPU: ${cpu}"
  echo "- Cores: ${cores}"
  echo "- Compiler: ${compiler}"
  echo ""
  echo "## Build Matrix"
  echo "- LORA_LITE_USE_LIQUID_FFT: OFF, ON"
  if [ "$INCLUDE_FIXED" -eq 1 ]; then
    echo "- LORA_LITE_FIXED_POINT: OFF, ON"
  else
    echo "- LORA_LITE_FIXED_POINT: OFF"
  fi
  echo "- LOG: ${log_opt}"
  echo "- EMBED profile: ${EMBED}"
  echo ""
} > "$SUMMARY"

fft_opts=(OFF ON)
if [ "$INCLUDE_FIXED" -eq 1 ]; then
  fixed_opts=(OFF ON)
else
  fixed_opts=(OFF)
fi

float_present=0

for fixed in "${fixed_opts[@]}"; do
  for fft in "${fft_opts[@]}"; do
    bdir="build_${fft}_${fixed}_${log_opt}"
    cmake_args=(-S . -B "$bdir" -DBUILD_TESTING=ON -DLORA_LITE_USE_LIQUID_FFT="${fft}" -DLORA_LITE_FIXED_POINT="${fixed}" -DLORA_LITE_ENABLE_LOGGING="${log_opt}")
    if [ "$EMBED" -eq 1 ]; then
      cmake_args+=(-C cmake/embedded_profile.cmake -DBUILD_SHARED_LIBS=OFF)
    fi
    echo ""; echo "=== Building (FFT=${fft} FIXED=${fixed} LOG=${log_opt}) ==="
    cmake "${cmake_args[@]}"
    cmake --build "$bdir" -j"$(nproc)"
    ctest --test-dir "$bdir" --output-on-failure
    ctest --test-dir "$bdir" -R nolog --output-on-failure
    csv="${RAW_DIR}/bench_${fft}_${fixed}_${log_opt}.csv"
    "./${bdir}/tests/bench_lora_chain" "$csv"
    if [ "$fixed" = "OFF" ]; then
      if [ -s "$csv" ]; then float_present=1; fi
    fi
  done
  off_csv="${RAW_DIR}/bench_OFF_${fixed}_${log_opt}.csv"
  on_csv="${RAW_DIR}/bench_ON_${fixed}_${log_opt}.csv"
  if [ -s "$off_csv" ] && [ -s "$on_csv" ]; then
    dest_off="${MATRIX_DIR}/bench_OFF_${fixed}_${log_opt}.csv"
    dest_on="${MATRIX_DIR}/bench_ON_${fixed}_${log_opt}.csv"
    cp "$off_csv" "$dest_off"
    cp "$on_csv" "$dest_on"
    tmp_dir="$(mktemp -d)"
    cp "$off_csv" "$tmp_dir/bench_OFF.csv"
    cp "$on_csv" "$tmp_dir/bench_ON.csv"
    {
      echo ""; echo "### FFT OFF vs ON (FIXED=${fixed} LOG=${log_opt})";
      python3 scripts/bench_compare.py "$tmp_dir";
    } >> "$SUMMARY"
    rm -rf "$tmp_dir"
  fi

done

if [ "$float_present" -ne 1 ]; then
  echo "No Float results produced" >&2
  exit 5
fi

echo ""; echo "Matrix directory: ${MATRIX_DIR}"
echo "Summary: ${SUMMARY}"
