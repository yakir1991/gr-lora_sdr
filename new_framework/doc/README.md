# New Framework

## Project Goals
The new framework provides a dedicated space for experimental LoRa SDR extensions. It focuses on modular blocks and test utilities that enable rapid prototyping of alternative physical-layer ideas.

## Build Instructions
1. From the repository root, configure a build directory:
   ```sh
   mkdir build && cd build
   cmake .. -DENABLE_NEW_FRAMEWORK=ON
   make
   sudo make install
   ```
2. After installation, the new framework blocks become available to GNU Radio just like the rest of the project.

## Test Output Processor
The build copies `process_test_output.py` into the build directory. Use it to summarize results from simple text logs:

```sh
python3 process_test_output.py <path/to/test_output.txt>
```

Each log line should contain a test name followed by `PASS` or `FAIL`, and the script prints a summary of the counts.

## Citation
If this framework contributes to your research, please cite the work referenced in [`CITATION.cff`](../../CITATION.cff).
