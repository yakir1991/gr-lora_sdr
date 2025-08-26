# lora_lite

See [doc/README.md](doc/README.md) for full project documentation.

The original GNU Radio-based implementation is preserved in [legacy_gr_lora_sdr/](legacy_gr_lora_sdr/) and remains unmodified for reference.

## Benchmark sweep

Run `./scripts/sweep_bench.sh` to build and execute the benchmark across multiple
spreading factors, coding rates, LDRO settings, fixed-point modes, and logging options.
Results accumulate in `results/host_sweep.csv` with columns:

```
sf,cr,ldro,fixed,logging,cycles,bytes_allocated,packets_per_sec
```

Analyze the CSV to compare performance across configurations.

## AArch64 QEMU benchmark

Prerequisites:

- `aarch64-linux-gnu-gcc` cross-compiler
- `qemu-aarch64` user emulator

Build and run the benchmark under QEMU:

```sh
./scripts/run_qemu_aarch64.sh
```

The script saves metrics to `results/arm_qemu.csv`. Compare them with host
measurements using:

```sh
python scripts/analyze_bench.py results/host.csv results/arm_qemu.csv
```

## Benchmark Matrix

Two helper scripts allow running performance benchmarks and comparing results
between builds with `LORA_LITE_USE_LIQUID_FFT=ON` and `OFF`.

### Run Matrix

```bash
./scripts/bench_matrix.sh
```

This will create two builds (`build-off`, `build-on`), run
`bench_lora_chain`, and save CSV outputs under `bench_out/<timestamp>/`.

### Compare Results

```bash
./scripts/bench_compare.py bench_out/<timestamp>
```

This prints a table comparing `packets_per_sec` between OFF and ON,
highlighting the delta and percentage ratio.

Example output:

```
=== LoRa Lite Benchmark Comparison ===
Folder: bench_out/20250826-142030

Config                 packets_per_sec
----------------------------------
FFT=OFF                       18516.393
FFT=ON                        19234.872
----------------------------------
Δ (ON-OFF)                      718.479
Ratio (ON/OFF)                  103.88%
```

## Embedded Optimization & FFT Matrix

This project includes:
- Full test suite via `ctest` (functional + golden + end-to-end).
- FFT matrix benchmarking (KISS vs Liquid-DSP).
- CSV outputs and comparison helpers.
- Optional embedded compile profile (-Os, LTO, GC-sections, fixed-point).
- CI workflow that runs everything and uploads artifacts.

### Run all tests
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build -V
```

### Embedded profile (optional)
```bash
cmake -S . -B build-emb -C cmake/embedded_profile.cmake -DBUILD_TESTING=ON
cmake --build build-emb -j"$(nproc)"
```

### FFT Matrix Benchmark (KISS vs Liquid)
```bash
./scripts/bench_matrix.sh
# outputs under bench_out/<timestamp>/bench_OFF.csv and bench_ON.csv
./scripts/bench_compare.py bench_out/<timestamp>
```

### Performance Guard (catch regressions)
- Default thresholds live in `bench/targets.json`.
- You can override per-run:
```bash
MIN_PPS_OFF=12000 MIN_PPS_ON=12000 MIN_RATIO=0.97 \
  ./scripts/bench_guard.py bench_out/<timestamp>
```

### CI
A workflow at `.github/workflows/embedded-bench.yml` runs:
1. Full `ctest`.
2. FFT matrix benchmarks (OFF/ON).
3. Comparison & guard.
4. Uploads artifacts (CSV files) for inspection.

### Notes
- To use Liquid-DSP locally: `sudo apt install -y libliquid-dev`.
- For embedded targets, prefer the `embedded_profile.cmake` preset, and enable `-DLORA_LITE_FIXED_POINT=ON` when possible.
