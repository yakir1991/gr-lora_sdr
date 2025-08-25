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

## Credits
Portions of the modules originated from the
[`gr-lora_sdr`](https://github.com/daniestevez/gr-lora_sdr) project and lora_lite
relies on [liquid-dsp](https://github.com/jgaeddert/liquid-dsp) for digital
signal processing utilities.

## Citation
If lora_lite contributes to your research, please cite the work referenced in
[`CITATION.cff`](../CITATION.cff).

