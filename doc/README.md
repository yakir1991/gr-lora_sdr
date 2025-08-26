# lora_lite

## Introduction
lora_lite is a sandbox for experimenting with LoRa® software-defined radio
components. It exposes small, self-contained C modules and corresponding tests so
researchers can prototype alternative physical-layer ideas without impacting the
rest of the project. Typical use cases include trying new modulation schemes,
evaluating signal processing utilities, or validating end‑to‑end transmission
concepts.

## Differences from gr-lora_sdr

lora_lite evolved from the [gr-lora_sdr](https://github.com/daniestevez/gr-lora_sdr)
project but deliberately diverges in several ways:

- **Standalone C implementation** – all signal-processing blocks are written in
  portable C without a dependency on GNU Radio, reducing external requirements
  and easing integration in small environments.
- **Modular block structure** – each component is an independent module with its
  own tests. This contrasts with gr-lora_sdr’s monolithic GNU Radio flowgraph
  where blocks are primarily exercised within the full pipeline.
- **Simplified build system** – lora_lite uses only CMake and CTest instead of
  GNU Radio’s out‑of‑tree module infrastructure, resulting in a quicker setup
  and easier CI integration.
- **Current limitations** – the focus on self‑contained modules means features
  such as real‑time streaming and hardware radio drivers, present in
  gr-lora_sdr, are not yet exposed.

## Repository Layout

- `lora_lite/` – core modular library and tests
- `legacy_gr_lora_sdr/` – archived GNU Radio implementation kept for reference

Developers seeking the original GNU Radio out-of-tree module can find it unmodified in [`legacy_gr_lora_sdr/`](../legacy_gr_lora_sdr/).

## Installation and Build
### Prerequisites
- CMake ≥ 3.12
- A C compiler such as GCC
- GNU Make
- Python 3 (for utility scripts)

### Steps
From the `lora_lite/` directory:

1. Configure the project:
   ```sh
   cmake -S . -B build
   ```
2. Compile the sources:
   ```sh
   cmake --build build
   ```
3. Run the test suite:
   ```sh
   ctest --test-dir build --output-on-failure
   ```
4. (Optional) install the modules into your system:
   ```sh
   sudo cmake --install build
   ```

## Embedded Platforms

Cross-compiling enables lora_lite on resource‑constrained MCUs. The build relies on
[`liquid-dsp`](https://github.com/jgaeddert/liquid-dsp) for DSP primitives and will
fetch and compile the library for the target automatically.

### Cross-compilation

Use the provided toolchain file to target an ARM Cortex‑M class device:

```sh
cmake -S . -B build-arm \
  -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-none-eabi.cmake \
  -DLORA_LITE_FIXED_POINT=ON \
  -DLORA_LITE_ENABLE_LOGGING=OFF \
  -DBUILD_SHARED_LIBS=OFF
cmake --build build-arm
```

### Memory Considerations

- Enable `LORA_LITE_FIXED_POINT` to avoid floating‑point operations.
- Disable logging via `LORA_LITE_ENABLE_LOGGING` to save flash and RAM.
- Build static libraries (`BUILD_SHARED_LIBS=OFF`) to simplify bare‑metal
  deployment.
- Only include the modules you need to keep the binary size minimal.

### Optional Features

Other CMake switches tailor the build for embedded use:

- `LORA_LITE_STATIC` – produce static library artifacts.
- `USE_SYSTEM_LIQUID_DSP` – link against a prebuilt `liquid-dsp` instead of
  fetching it.

## Example Usage
### Hello World
After building, execute the sample program:
```sh
./build/hello_world
```

### End-to-End File Example
An automated test demonstrates the transmit‑and‑receive chain using `lora_tx_chain`
and `lora_rx_chain`. It converts `data/GRC_default/example_tx_source.txt` into LoRa
chips, stores them in `tx_capture.bin`, then decodes the file to verify the
payload:

```sh
ctest -R test_end_to_end_file --output-on-failure
```

Run the compiled test directly to inspect intermediate files:

```sh
./build/tests/test_end_to_end_file
```

### Embedded Loopback Test
A fixed-point regression test mimics an embedded deployment by using
static buffers and avoiding file I/O. Configure the project with
`-DLORA_LITE_FIXED_POINT=ON`, build, then run:

```sh
ctest -R embedded_loopback --output-on-failure
```

On success the test prints `Embedded loopback test passed`.

### GNURadio Comparison
Two binaries capture end‑to‑end payload recovery through GNU Radio and the
standalone C framework.  Regenerate them and verify equality with the test
suite:

1. Use `grcc` to turn the legacy flowgraph into a Python script and run it with
   deterministic AWGN to create `legacy_gr_lora_sdr/gnuradio_ref.bin`:
   ```sh
   grcc legacy_gr_lora_sdr/examples/tx_rx_simulation.grc -o legacy_gr_lora_sdr
   python legacy_gr_lora_sdr/tx_rx_simulation.py  # produces gnuradio_ref.bin
   ```
2. Build the native executable and run it with the same input and noise seed to
   produce `legacy_gr_lora_sdr/framework_ref.bin`:
   ```sh
   cmake -S . -B build && cmake --build build
   ./build/src/lora_chain_runner \
     legacy_gr_lora_sdr/data/GRC_default/example_tx_source.txt \
     legacy_gr_lora_sdr/framework_ref.bin
   ```
3. Compare the results byte‑for‑byte:
   ```sh
   ctest -R full_chain_compare --test-dir build --output-on-failure
   ```

Both flows rely on seeded noise sources to remain deterministic across
platforms.

### Utility Scripts
The build copies helper scripts next to the binaries:

- `process_test_output.py` — summarize `PASS`/`FAIL` logs
- `inspect_symbols.py` — plot complex sample dumps such as `tx_capture.bin`

## Benchmarking

Build the TX→RX benchmark with:

```sh
cmake -S . -B build -DLORA_LITE_BENCHMARK=ON
cmake --build build
ctest --test-dir build -R bench_lora_chain
```

The run produces `bench_results.csv` in the build tree containing cycle
counts, peak heap usage, and packet throughput. Summarize and compare files
with the helper script:

```sh
python analyze_bench.py build/tests/bench_results.csv
```

To target a microcontroller, add a toolchain file:

```sh
cmake -S . -B build-arm -DLORA_LITE_BENCHMARK=ON \
  -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-none-eabi.cmake \
  -DLORA_LITE_FIXED_POINT=ON -DLORA_LITE_ENABLE_LOGGING=OFF \
  -DBUILD_SHARED_LIBS=OFF
cmake --build build-arm
```

Run the resulting `bench_lora_chain` binary on hardware and copy back its CSV
output. Use `analyze_bench.py --threshold 0.2` on multiple CSV files to flag
host/embedded deviations beyond the chosen tolerance.

### AArch64 QEMU example

Prerequisites:

- `aarch64-linux-gnu-gcc` cross-compiler
- `qemu-aarch64` user emulator

Cross-compile and execute the benchmark under QEMU:

```sh
./scripts/run_qemu_aarch64.sh
```

The command writes `results/arm_qemu.csv`. Compare this file with host results
using:

```sh
python scripts/analyze_bench.py results/host.csv results/arm_qemu.csv
```

## Profiling

The script [`scripts/profile_host.sh`](../scripts/profile_host.sh) builds a
`RelWithDebInfo` host binary and records performance counters and heap usage.
Run it from the repository root:

```sh
./scripts/profile_host.sh
```

The run requires `perf` and `valgrind`.  It creates two files in
`results/`:

- `profile_perf.txt` – CPU cycles, instructions, and branch/cache statistics
  gathered with `perf stat`.
- `profile_massif.txt` – heap snapshots from Valgrind's `massif` tool.  Use
  `ms_print results/profile_massif.txt` to view the peak memory profile.

These artifacts help evaluate runtime behavior and memory consumption on the
host platform.
 
## Callback-based I/O and Logging APIs

`lora_io.h` exposes a callback structure so applications can route I/O through
files, UARTs, or custom transports:

```c
#include "lora_io.h"

lora_io_t io;
FILE *fp = fopen("input.bin", "rb");
lora_io_init_file(&io, fp);
io.read(io.ctx, buffer, sizeof buffer);
```

Logging macros in `lora_log.h` provide lightweight printf‑style diagnostics.
Define `LORA_LITE_ENABLE_LOGGING` at compile time to activate them or override
`LORA_LOG_PRINTF` to redirect output:

```c
#include "lora_log.h"

LORA_LOG_INFO("Received %u bytes", count);
```

## Credits and Licensing
Portions of the modules originated from the
[`gr-lora_sdr`](https://github.com/daniestevez/gr-lora_sdr) project (GPLv3).
The project integrates [`liquid-dsp`](https://github.com/jgaeddert/liquid-dsp),
which is distributed under the MIT license. See [`LICENSE`](../LICENSE) for the
terms governing lora_lite itself.

## Citation
If lora_lite contributes to your research, please cite the work referenced in
[`CITATION.cff`](../CITATION.cff).

