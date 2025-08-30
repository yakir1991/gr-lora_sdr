# Whitening Block — lora_whiten/lora_dewhiten

Block-level tests and micro-benchmarks for the whitening/dewhitening routines.
The implementation uses SIMD (SSE2/NEON) for 16-byte chunks, then 8-byte and
a scalar tail. We added `restrict` qualifiers to help compilers optimize.

## What we verify
- Invertibility: `dewhiten(whiten(x)) == x` for various lengths/patterns
- Compatibility: matches the golden whitening sequence (spot-check)
- Throughput (MB/s) with different buffer sizes

## How to run

```bash
# Unit test
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build -j
ctest --test-dir build -R test_lora_whitening_block -V

# Bench (requires -DLORA_LITE_BENCHMARK=ON)
cmake -S . -B build-bench -DLORA_LITE_BENCHMARK=ON
cmake --build build-bench -j
./build-bench/blocks/lora_whitening/bench_lora_whitening
```

## Expected output
- Test prints `Whitening block test passed` and CSV with mismatch=0.
- Bench prints CSV metrics with sizes and MB/s.

## Before/After
Baseline bench (host x86_64) ~310–315 MB/s; after adding `restrict` and light
unrolling in the SSE2/64-bit paths:

```
metric,value (before)
whiten_MBps_len256,22721.63
whiten_MBps_len1024,31081.78
whiten_MBps_len8192,36024.23

metric,value (after)
whiten_MBps_len256,23151.51
whiten_MBps_len1024,34722.43
whiten_MBps_len8192,37135.51
```

Gains are modest but consistent for larger buffers on this host; on embedded,
the reduced loop overhead can help even without SIMD.
