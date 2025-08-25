# Example toolchain file for cross-compiling LoRa Lite with arm-none-eabi

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar CACHE FILEPATH "")

# Adjust flags as needed for your target MCU
set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-m4 -mthumb")
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cortex-m4 -mthumb")

# Avoid executing target binaries on the host during tests of the compiler
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

