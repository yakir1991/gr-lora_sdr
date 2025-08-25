cmake_minimum_required(VERSION 3.12)

# Build lora_lite as the primary project
project(lora_lite LANGUAGES C)

option(LORA_LITE_FIXED_POINT "Use fixed-point math in LoRa Lite" OFF)
option(USE_SYSTEM_LIQUID_DSP "Use system or submodule liquid-dsp instead of FetchContent" OFF)
option(BUILD_SHARED_LIBS "Build libraries as shared" ON)
option(LORA_LITE_STATIC "Build static library artifacts" OFF)

if(LORA_LITE_STATIC)
  set(BUILD_SHARED_LIBS OFF)
endif()

option(LORA_LITE_ENABLE_LOGGING "Enable logging output" ON)

if(USE_SYSTEM_LIQUID_DSP)
  # Prefer an in-tree submodule if present; fall back to a system search.
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/liquid-dsp/CMakeLists.txt)
    add_subdirectory(liquid-dsp)
  else()
    find_package(liquid-dsp REQUIRED)
  endif()
else()
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
endif()

# Enable testing via CTest
include(CTest)
enable_testing()

# Copy utility Python scripts into the build directory so they can be
# executed without referring back to the source tree.
foreach(script process_test_output.py inspect_symbols.py)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/${script}
                 ${CMAKE_CURRENT_BINARY_DIR}/${script} COPYONLY)
endforeach()

add_subdirectory(src)
add_subdirectory(tests)
