cmake_minimum_required(VERSION 3.12)

project(new_framework LANGUAGES C)

include(FetchContent)

# Fetch the liquid-dsp library for DSP utilities
FetchContent_Declare(
  liquid_dsp
  GIT_REPOSITORY https://github.com/jgaeddert/liquid-dsp.git
  GIT_TAG v1.3.2
  GIT_SHALLOW TRUE
)

# Disable optional components to keep the build lightweight
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_AUTOTESTS OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BUILD_SANDBOX OFF CACHE BOOL "" FORCE)
set(BUILD_DOC OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(liquid_dsp)

# Enable testing via CTest
include(CTest)
enable_testing()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/process_test_output.py
               ${CMAKE_CURRENT_BINARY_DIR}/process_test_output.py COPYONLY)

add_subdirectory(src)
add_subdirectory(tests)
