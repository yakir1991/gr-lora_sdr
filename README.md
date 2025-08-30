# lora_lite

lora_lite is a sandbox for experimenting with LoRa® software-defined radio components. It exposes small, self-contained C modules and corresponding tests so researchers can prototype alternative physical-layer ideas without impacting the rest of the project. All runtime state resides in per-instance contexts with no mutable global data, allowing multiple threads to run independently.

## Key Features
- **Standalone C implementation** – signal-processing blocks are written in portable C with no dependency on GNU Radio.
- **Modular block structure** – each component is an independent module with its own tests.
- **Simplified build system** – uses only CMake and CTest for quick setup and easy CI integration.
- **Dual math paths** – builds support floating-point or Q15 fixed-point demodulation; set
  `-DLORA_LITE_FIXED_POINT=ON` to use the fixed-point variant.
- **Pluggable FFT backend** – the built-in FFT (KISS-style) can be replaced by Liquid-DSP
  (and FFTW under the hood) by building with `-DLORA_LITE_USE_LIQUID_FFT=ON`.
  Bench scripts under `scripts/` compare KISS vs Liquid/FFTW.

The original GNU Radio-based implementation is preserved in [legacy_gr_lora_sdr/](legacy_gr_lora_sdr/) and remains unmodified for reference.

For installation and cross-compilation instructions see [SETUP.md](SETUP.md). Testing and benchmarking steps live in [TESTING.md](TESTING.md).
Embedded development notes and guardrails are in [README_EMBEDDED.md](README_EMBEDDED.md) and [doc/EMBEDDED_NOTES.md](doc/EMBEDDED_NOTES.md).
A detailed log of performance experiments is available in [doc/EXPERIMENTS.md](doc/EXPERIMENTS.md).
If this project contributes to academic work, please see [CITATION.cff](CITATION.cff) for citation details.

## Repository Layout
- `src/` – core modular library (FFT, demod, whitening, header, rx/tx chains, frame sync)
- `include/` – public headers used across modules
- `tests/` – unit/integration tests and optional micro-benchmarks (gated by `-DLORA_LITE_BENCHMARK=ON`)
- `doc/` – project documentation and experiments log
- `legacy_gr_lora_sdr/` – archived GNU Radio implementation kept for reference
