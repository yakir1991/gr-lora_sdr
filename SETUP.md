# Setup

## Prerequisites
- CMake ≥ 3.12
- A C compiler such as GCC
- GNU Make
- Python 3 (for utility scripts)

## Host Build
From the repository root:

```sh
cmake -S . -B build
cmake --build build
```

(Optional) install:

```sh
sudo cmake --install build
```

## Embedded and Cross-compiling
Use the provided toolchain file to target an ARM Cortex-M class device:

```sh
cmake -S . -B build-arm \
  -DCMAKE_TOOLCHAIN_FILE=../toolchain-arm-none-eabi.cmake \
  -DLORA_LITE_FIXED_POINT=ON \
  -DLORA_LITE_ENABLE_LOGGING=OFF \
  -DBUILD_SHARED_LIBS=OFF
cmake --build build-arm
```

### Memory Considerations
- Enable `LORA_LITE_FIXED_POINT` to avoid floating-point operations.
- Disable logging via `LORA_LITE_ENABLE_LOGGING` to save flash and RAM.
- Build static libraries (`BUILD_SHARED_LIBS=OFF`) to simplify bare-metal deployment.
- Only include the modules you need to keep the binary size minimal.

### Optional Features
- `LORA_LITE_STATIC` – produce static library artifacts.
- `USE_SYSTEM_LIQUID_DSP` – link against a prebuilt `liquid-dsp` instead of fetching it.
- `ENABLE_SANITIZERS` – add AddressSanitizer and UndefinedBehaviorSanitizer when building non-`Release` configurations:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_SANITIZERS=ON
```

### Embedded Profile Preset
Optimize for embedded targets using the preset configuration:

```sh
cmake -S . -B build-emb -C cmake/embedded_profile.cmake
cmake --build build-emb
```

To favor throughput over code size, enable:

```sh
cmake -S . -B build-emb -C cmake/embedded_profile.cmake \
  -DLORA_LITE_EMB_THROUGHPUT=ON
```

For maximum packets-per-second, combine fixed-point and throughput flags:

```sh
cmake -S . -B build-emb-o3 -C cmake/embedded_profile.cmake \
  -DLORA_LITE_FIXED_POINT=ON \
  -DLORA_LITE_EMB_THROUGHPUT=ON
cmake --build build-emb-o3
```

