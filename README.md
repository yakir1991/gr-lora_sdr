# lora_lite

lora_lite is a sandbox for experimenting with LoRa® software-defined radio components. It exposes small, self-contained C modules and corresponding tests so researchers can prototype alternative physical-layer ideas without impacting the rest of the project. All runtime state resides in per-instance contexts with no mutable global data, allowing multiple threads to run independently.

## Key Features
- **Standalone C implementation** – signal-processing blocks are written in portable C with no dependency on GNU Radio.
- **Modular block structure** – each component is an independent module with its own tests.
- **Simplified build system** – uses only CMake and CTest for quick setup and easy CI integration.

The original GNU Radio-based implementation is preserved in [legacy_gr_lora_sdr/](legacy_gr_lora_sdr/) and remains unmodified for reference.

See [doc/README.md](doc/README.md) for full project documentation and [TESTING.md](TESTING.md) for testing and benchmarking instructions.

## Repository Layout
- `lora_lite/` – core modular library and tests
- `legacy_gr_lora_sdr/` – archived GNU Radio implementation kept for reference

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
