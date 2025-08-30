# Signal Utils Block — RMS (Embedded Focus)

This block contains focused tests and micro-benchmarks for the RMS utilities
used across the project, optimized with embedded/float targets in mind.

## What we measure
- Numerical stability of `rms(double)` vs a naive reference on wide-dynamic data
- New `rmsf(float)` for embedded: stability vs. speed tradeoff
- Micro-benchmark of calls/sec for both variants

## How to run

```bash
# Build tests and run the block test
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build -j
ctest --test-dir build -R test_signal_utils_block -V

# Micro-benchmark (requires -DLORA_LITE_BENCHMARK=ON)
cmake -S . -B build-bench -DLORA_LITE_BENCHMARK=ON
cmake --build build-bench -j
./build-bench/blocks/signal_utils/bench_signal_utils
```

## Interpreting results
- The unit test prints max relative error vs a long-double reference; thresholds are kept tight to ensure stability.
- The bench prints `metric,value` CSV with `rms_us_per_call_double` and `rms_us_per_call_float`.
  - On embedded or with `-mfpu=...` enabled, expect `rmsf` to be faster while staying within acceptable error bounds.

## Notes
- Both implementations use Neumaier-compensated summation to reduce error accumulation without heavy overhead.
- For further gains on specific MCUs, consider enabling FPU flags and LTO, and pinning the core during measurement.

## Recent Results (host x86_64, GCC 11.4, Release)

```
metric,value (before)
rms_us_per_call_double,3.327
rms_us_per_call_float,3.329

metric,value (after SIMD)
rms_us_per_call_double,3.291
rms_us_per_call_float,0.185
```

- Float path (`rmsf`) improved ~18× with SSE2 vectorization.
- Double path unchanged (uses scalar compensated sum as reference).

