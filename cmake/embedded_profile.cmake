# Embedded-friendly defaults: -Os, LTO, GC sections, extra warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Os -ffunction-sections -fdata-sections -flto -Wall -Wextra -Wpedantic -Wconversion -Wshadow")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
# Favor fixed point for embedded builds (caller may override)
set(LORA_LITE_FIXED_POINT ON CACHE BOOL "Use fixed-point math for embedded" FORCE)
# Favor Liquid FFT by default for embedded builds
set(LORA_LITE_USE_LIQUID_FFT ON CACHE BOOL "Use Liquid FFT by default on embedded profile" FORCE)

