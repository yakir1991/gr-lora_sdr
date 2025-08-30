# CRC Block — lora_add_crc (Embedded Focus)

This block tracks the CRC-16-CCITT implementation used by the RX/TX chains
and documents focused tests + micro-benchmarks. The main change was replacing
a lazy-initialized table with a precomputed constant table, removing runtime
init/checks and making the code thread-safe.

## What changed
- Moved the 256-entry CRC-16 table to a compile-time constant (`src/lora_crc_table.h`).
- Removed static mutable state (lazy init) from `lora_add_crc.c`.
- Kept semantics identical to legacy: init=0x0000, no reflect, and legacy post-XOR
  with the trailing two bytes.

## How to run

```bash
# Unit test for the block
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build -j
ctest --test-dir build -R test_lora_add_crc_block -V

# Micro-benchmark (requires -DLORA_LITE_BENCHMARK=ON)
cmake -S . -B build-bench -DLORA_LITE_BENCHMARK=ON
cmake --build build-bench -j
./build-bench/blocks/lora_add_crc/bench_lora_add_crc
```

## Expected output
- Unit test prints `CRC block test passed` when all vectors match a slow reference.
- Bench prints CSV metrics:

```
metric,value
crc_MBps,XX.YY
last_out,a b c d
```

## Results (host x86_64, GCC 11.4, Release)

Before: table initialized at runtime (lazy), benchmark ~64–65 MB/s.
After:  constant table, benchmark ~64–65 MB/s (within noise), but no lazy init
or branches in hot path and no shared mutable state.

Conclusion: functionality unchanged, small structural improvement beneficial for
embedded/threaded builds.

