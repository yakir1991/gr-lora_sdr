# Testing, Benchmarking, and CI

## Run all tests
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j"$(nproc)"
ctest --test-dir build -V
```

### Troubleshooting (Liquid-DSP)
On some distros `libliquid-dev` installs headers/libs but no `liquid-dsp.pc`.
Our CMake includes a fallback that detects `/usr/include/liquid/liquid.h`
and `/usr/lib/x86_64-linux-gnu/libliquid.so` and creates `Liquid::liquid`.

If detection still fails, you can override explicitly:
```bash
cmake -S . -B build \
  -DLORA_LITE_USE_LIQUID_FFT=ON -DBUILD_TESTING=ON \
  -DLiquid_INCLUDE_DIR=/usr/include \
  -DLiquid_LIBRARY=/usr/lib/x86_64-linux-gnu/libliquid.so
```
Or set `PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig` if you do have a `.pc`.

Historically, builds that omitted `lora_fft` from the link line produced runtime errors like `undefined symbol: q15_to_cf`. The library is now linked explicitly to avoid this issue.

### End-to-End file test
```bash
ctest --test-dir build -R test_end_to_end_file --output-on-failure
```

## Basic benchmark
Enable the benchmark targets and run the TX→RX chain.

```bash
cmake -S . -B build -DLORA_LITE_BENCHMARK=ON
cmake --build build
ctest --test-dir build -R bench_lora_chain
./build/tests/bench_lora_chain results/bench_results.csv
python scripts/analyze_bench.py results/bench_results.csv
```

To benchmark on a microcontroller, cross-compile as described in [SETUP.md](SETUP.md) and run the generated `bench_lora_chain` on hardware. Collect the CSV output and analyze it with `analyze_bench.py` to compare against host results.

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

## Profiling
Collect performance counters and heap usage on the host:

```bash
./scripts/profile_host.sh
```

The script requires `perf` and `valgrind` and writes results to `results/profile_perf.txt` and `results/profile_massif.txt`.

## Embedded Optimization & FFT Matrix

This project includes:
- Full test suite via `ctest` (functional + golden + end-to-end).
- FFT matrix benchmarking (KISS vs Liquid-DSP).
- CSV outputs and comparison helpers.
- Optional embedded compile profile (-Os, LTO, GC-sections, fixed-point).
- CI workflow that runs everything and uploads artifacts.

### Embedded profile (optional)
Build the project using the embedded profile preset described in [SETUP.md](SETUP.md). After building, run:

```bash
./build-emb/tests/bench_lora_chain bench_emb.csv
```

To favor throughput over code size, rebuild with `-DLORA_LITE_EMB_THROUGHPUT=ON`. For maximum packets-per-second, combine fixed-point and throughput flags and run:

```bash
./build-emb-o3/tests/bench_lora_chain bench_fixed_o3.csv
```

### FFT Matrix Benchmark (KISS vs Liquid)
```bash
./scripts/bench_matrix.sh
# outputs under bench_out/<timestamp>/bench_OFF.csv and bench_ON.csv
./scripts/bench_compare.py bench_out/<timestamp>
```

### Performance Guard (catch regressions)
- Default thresholds live in `bench/targets.embedded.json`.
- When the benchmark is built with fixed-point (`LORA_LITE_FIXED_POINT=ON`),
  the guard automatically loads `bench/targets.fixed.embedded.json`:

```json
{ "min_pps_off": 7300.0, "min_pps_on": 8000.0, "min_ratio_on_over_off": 1.08 }
```

Run the guard on a results folder:

```bash
./scripts/bench_guard.py bench_out/<timestamp>
# fixed-point example
LORA_LITE_FIXED_POINT=ON ./scripts/bench_guard.py bench_out/<timestamp>
```

- You can also override thresholds per-run:

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
`scripts/profile_host.sh` runs benchmark + CSV; `perf` metrics are collected on a best-effort basis in restricted environments.

### Notes
- To use Liquid-DSP locally: `sudo apt install -y libliquid-dev`.
- For embedded targets, prefer the `embedded_profile.cmake` preset, enable
  `-DLORA_LITE_FIXED_POINT=ON` when possible, and use
  `-DLORA_LITE_EMB_THROUGHPUT=ON` to favor throughput over code size.

### Embedded profile: Liquid FFT default & baseline
After building with the embedded profile (see [SETUP.md](SETUP.md)):
```bash
./build-emb/tests/bench_lora_chain bench_emb.csv
```
**Observed on embedded profile:** Liquid FFT is ~10–11% faster than KISS
(example: `ON ≈ 9.83k pps` vs `OFF ≈ 8.88k pps`).

To benchmark both FFT modes and compare:
```bash
./scripts/bench_matrix.sh
LATEST_DIR=$(ls -1d bench_out/* | tail -n1)
./scripts/bench_compare.py "$LATEST_DIR"
```

Guard thresholds for embedded live in `bench/targets.embedded.json`:
```json
{ "min_pps_off": 8500.0, "min_pps_on": 9600.0, "min_ratio_on_over_off": 1.06 }
```
For fixed-point builds (`LORA_LITE_FIXED_POINT=ON`), the guard uses
`bench/targets.fixed.embedded.json`:
```json
{ "min_pps_off": 7300.0, "min_pps_on": 8000.0, "min_ratio_on_over_off": 1.08 }
```
Run the guard on a results folder:
```bash
./scripts/bench_guard.py "$LATEST_DIR"
# fixed-point:
LORA_LITE_FIXED_POINT=ON ./scripts/bench_guard.py "$LATEST_DIR"
```

#### CI matrix: host vs embedded
The workflow `.github/workflows/embedded-bench.yml` runs both host and embedded profiles:
- Host: build, tests, bench, compare (no guard).
- Embedded: build with `-C cmake/embedded_profile.cmake`, bench, compare, and **guard** (automatically loads `bench/targets.fixed.embedded.json` when `LORA_LITE_FIXED_POINT=ON`).

#### ASAN profile (CI-only by default)
A third CI profile `embedded-asan` builds with AddressSanitizer (no LTO, `RelWithDebInfo`), runs `ctest -V`, and performs a short smoke-run of `bench_lora_chain` to exercise hot paths. Results are uploaded under `bench-results-embedded-asan`.
Local run:
```bash
cmake -S . -B build-asan -C cmake/embedded_profile.cmake -C cmake/asan_profile.cmake -DBUILD_TESTING=ON
cmake --build build-asan -j"$(nproc)"
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 LSAN_OPTIONS="suppressions=$(pwd)/asan/lsan.supp" ctest --test-dir build-asan -V
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 ./build-asan/tests/bench_lora_chain /dev/null || true
```

#### UBSAN profile (CI-only by default)
The `embedded-ubsan` CI profile builds with UndefinedBehaviorSanitizer (no LTO, `RelWithDebInfo`), runs `ctest -V`, and a short smoke-run of `bench_lora_chain`.
Local run:
```bash
cmake -S . -B build-ubsan -C cmake/embedded_profile.cmake -C cmake/ubsan_profile.cmake -DBUILD_TESTING=ON
cmake --build build-ubsan -j"$(nproc)"
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 ctest --test-dir build-ubsan -V
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 ./build-ubsan/tests/bench_lora_chain /dev/null || true
```

### ARM Power/Energy sampling
Measure average power, energy and energy-per-packet while running the LoRa bench.

**Prereqs (ARM Linux):**
- Kernel exposes power sensors under `/sys/class/power_supply/*/{power_now,voltage_now,current_now}` (µW/µV/µA), or under `/sys/class/hwmon/hwmon*/power*_input` (µW).
- If auto-detect fails, set:
  - `POWER_FILE=/sys/.../power_now` **or**
  - `VOLTAGE_FILE=/sys/.../voltage_now` and `CURRENT_FILE=/sys/.../current_now`

**Local run (OFF/ON, 20s):**
```bash
chmod +x scripts/energy/run_power_matrix.sh
DURATION_SEC=20 ./scripts/energy/run_power_matrix.sh
# Results under power_out/<timestamp>/{OFF,ON}/
./scripts/energy/energy_compare.py power_out/<timestamp>
```

**Files produced**
- `energy.csv` – samples: `time_ms,power_mw`
- `bench_stdout.log` – raw stdout lines with `packets_per_sec=...`
- `summary.csv` – aggregated: `avg_power_mw,energy_j,duration_s,pps_avg,energy_per_packet_mJ`

**CI (self-hosted ARM64)**
- Add a self-hosted runner with labels `self-hosted, linux, ARM64`.
- Run workflow: `.github/workflows/arm-power.yml`
- Artifacts include raw CSVs + summaries.

**Tip:** For stable numbers, pin CPU freq/governor to `performance` and disable background services. Use longer `DURATION_SEC` on noisy boards.

How to use (briefly)
On the ARM:

sudo apt-get install -y python3
export POWER_FILE=/sys/class/power_supply/*/power_now   
DURATION_SEC=20 ./scripts/energy/run_power_matrix.sh


For comparison:

./scripts/energy/energy_compare.py power_out/<timestamp>


In CI: run arm-power on self-hosted ARM64.
