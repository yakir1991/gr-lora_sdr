# lora_lite

lora_lite is a sandbox for experimenting with LoRa® software-defined radio components. It exposes small, self-contained C modules and corresponding tests so researchers can prototype alternative physical-layer ideas without impacting the rest of the project. All runtime state resides in per-instance contexts with no mutable global data, allowing multiple threads to run independently.

## Key Features
- **Standalone C implementation** – signal-processing blocks are written in portable C with no dependency on GNU Radio.
- **Modular block structure** – each component is an independent module with its own tests.
- **Simplified build system** – uses only CMake and CTest for quick setup and easy CI integration.
- **Dual math paths** – builds support floating-point or Q15 fixed-point demodulation; set
  `-DLORA_LITE_FIXED_POINT=ON` to use the fixed-point variant.

The original GNU Radio-based implementation is preserved in [legacy_gr_lora_sdr/](legacy_gr_lora_sdr/) and remains unmodified for reference.

For installation and cross-compilation instructions see [SETUP.md](SETUP.md). Testing and benchmarking steps live in [TESTING.md](TESTING.md). Additional documentation can be found in [doc/README.md](doc/README.md). A detailed log of performance experiments and optimization roadmap is available in [doc/EXPERIMENTS.md](doc/EXPERIMENTS.md).

## Repository Layout
- `lora_lite/` – core modular library and tests
- `legacy_gr_lora_sdr/` – archived GNU Radio implementation kept for reference
