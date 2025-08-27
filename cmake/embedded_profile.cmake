# Embedded-friendly defaults: -Os, LTO, GC sections, extra warnings
# Pass -DLORA_LITE_EMB_THROUGHPUT=ON to favor throughput over size.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Os -ffunction-sections -fdata-sections -flto -Wall -Wextra -Wpedantic -Wconversion -Wshadow")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
# Favor fixed point for embedded builds (caller may override)
set(LORA_LITE_FIXED_POINT ON CACHE BOOL "Use fixed-point math for embedded" FORCE)
# Prefer internal FFT on embedded builds (Liquid tends to be slower with fixed/Q15)
set(LORA_LITE_USE_LIQUID_FFT OFF CACHE BOOL "Use Liquid FFT by default on embedded profile" FORCE)
