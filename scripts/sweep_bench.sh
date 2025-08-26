#!/usr/bin/env bash
set -e

out_dir="results"
mkdir -p "$out_dir"
out_csv="$out_dir/host_sweep.csv"

if [ ! -f "$out_csv" ]; then
  echo "sf,cr,ldro,fixed,logging,cycles,bytes_allocated,packets_per_sec" > "$out_csv"
fi

if grep -qE '#\s*error.*requires\s+LORA_LITE_FIXED_POINT' "src/lora_rx_chain.c"; then
  echo "[sweep] compile-time guard requires fixed-point; skipping float builds"
  FIXED_MODES=(1)
else
  FIXED_MODES=(0 1)
fi

for sf in 7 9 12; do
  for cr in 1 4; do
    for ldro in 0 1; do
      for fixed in "${FIXED_MODES[@]}"; do
        for logging in 0 1; do
          build="build_sf${sf}_cr${cr}_ldro${ldro}_fix${fixed}_log${logging}"
          cmake -S . -B "$build" \
            -DLORA_LITE_BENCHMARK=ON \
            -DLORA_LITE_SF=${sf} \
            -DLORA_LITE_CR=${cr} \
            -DLORA_LITE_LDRO=${ldro} \
            -DLORA_LITE_FIXED_POINT="$([ "$fixed" -eq 1 ] && echo ON || echo OFF)" \
            -DLORA_LITE_ENABLE_LOGGING="$([ "$logging" -eq 1 ] && echo ON || echo OFF)"
          cmake --build "$build" >/dev/null
          ctest --test-dir "$build" -R bench_lora_chain >/dev/null
          metrics=$(tail -n 1 "$build/tests/bench_results.csv")
          echo "$sf,$cr,$ldro,$fixed,$logging,$metrics" >> "$out_csv"
        done
      done
    done
  done
done
