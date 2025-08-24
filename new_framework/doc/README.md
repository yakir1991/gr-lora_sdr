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

## Citation
If this framework contributes to your research, please cite the work referenced in [`CITATION.cff`](../../CITATION.cff).
