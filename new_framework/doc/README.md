# New Framework

## Introduction
The new framework is a sandbox for experimenting with LoRa® software-defined radio
components. It exposes small, self-contained C modules and corresponding tests so
researchers can prototype alternative physical-layer ideas without impacting the
rest of the project. Typical use cases include trying new modulation schemes,
evaluating signal processing utilities, or validating end‑to‑end transmission
concepts.

## Installation and Build
### Prerequisites
- CMake ≥ 3.12
- A C compiler such as GCC
- GNU Make
- Python 3 (for utility scripts)

### Steps
1. Configure the project:
   ```sh
   cmake -S new_framework -B build/new_framework
   ```
2. Compile the sources:
   ```sh
   cmake --build build/new_framework
   ```
3. Run the test suite:
   ```sh
   ctest --test-dir build/new_framework --output-on-failure
   ```
4. (Optional) install the modules into your system:
   ```sh
   sudo cmake --install build/new_framework
   ```

## Example Usage
### Hello World
After building, execute the sample program:
```sh
./build/new_framework/hello_world
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
./build/new_framework/tests/test_end_to_end_file
```

### GNURadio Comparison
To compare the C framework with the GNURadio reference flowgraph, generate both outputs and run the comparison test:

1. Build the reference with GNURadio:
```sh
PYTHONPATH=build/python LD_LIBRARY_PATH=build/lib python examples/tx_rx_simulation.py > gnuradio_out.bin
```
2. Produce the framework output:
```sh
./build/new_framework/tests/test_end_to_end_file > framework_out.bin
```
3. Execute the comparison test:
```sh
ctest -R test_tx_rx_compare --test-dir build/new_framework --output-on-failure
```

### Utility Scripts
The build copies helper scripts next to the binaries:

- `process_test_output.py` — summarize `PASS`/`FAIL` logs
- `inspect_symbols.py` — plot complex sample dumps such as `tx_capture.bin`

## Credits
Portions of the modules originated from the
[`gr-lora_sdr`](https://github.com/daniestevez/gr-lora_sdr) project and the
framework relies on [liquid-dsp](https://github.com/jgaeddert/liquid-dsp) for
digital signal processing utilities.

## Citation
If this framework contributes to your research, please cite the work referenced
in [`CITATION.cff`](../../CITATION.cff).

