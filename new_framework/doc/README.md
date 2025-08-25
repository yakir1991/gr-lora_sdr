# New Framework

## Project Goals
The new framework provides a dedicated space for experimental LoRa SDR extensions. It focuses on modular blocks and test utilities that enable rapid prototyping of alternative physical-layer ideas.

## Build Instructions
1. Configure and build the framework in an isolated directory:
   ```sh
   rm -rf build/new_framework
   mkdir -p build/new_framework
   cd build/new_framework
   cmake ../../new_framework
   cmake --build .
   sudo make install
   ```
2. After installation, the new framework blocks become available to GNU Radio just like the rest of the project.

## Running Tests
From the `build/new_framework` directory, run the framework's test suite:
```sh
ctest --output-on-failure
```

## End-to-End File Example
An automated test verifies that a file can be transmitted through the full LoRa chain.
It uses `data/GRC_default/example_tx_source.txt` as the payload, converts it to LoRa
chips with `lora_tx_chain`, stores the chips to a temporary binary file, and then
decodes them with `lora_rx_chain` to confirm the original text is recovered.

Run the test after building:

```sh
ctest -R test_end_to_end_file --output-on-failure
```

To examine the process manually, execute the test binary directly:

```sh
./tests/test_end_to_end_file
```

The program will create `tx_capture.bin`, perform the round trip, and delete the
temporary file after verifying success.

## Third-Party Libraries
The new framework relies on [liquid-dsp](https://github.com/jgaeddert/liquid-dsp) for digital signal processing utilities. The
library is pulled automatically during configuration using CMake's `FetchContent` mechanism; no manual download is required.

To include `liquid-dsp` in your own modules, add the header and link against the `liquid` target:

```c
#include <liquid/liquid.h>
```

```cmake
target_link_libraries(your_target PRIVATE liquid)
```

## Test Output Processor
The build copies `process_test_output.py` into the build directory. Use it to summarize results from simple text logs:

```sh
python3 process_test_output.py <path/to/test_output.txt>
```

Each log line should contain a test name followed by `PASS` or `FAIL`, and the script prints a summary of the counts.

## Symbol Dump Inspector
Tests such as `test_end_to_end_file` write intermediate complex samples to a
binary file (`tx_capture.bin`).  After building, the utility scripts are copied
to the build directory, allowing quick visualization of these dumps:

```sh
python3 inspect_symbols.py tx_capture.bin -o symbols.png
```

The script loads the `complex64` values and plots the real and imaginary parts
with `matplotlib`.  Omitting `-o` displays the plot interactively instead of
writing an image.

## Citation
If this framework contributes to your research, please cite the work referenced in [`CITATION.cff`](../../CITATION.cff).
