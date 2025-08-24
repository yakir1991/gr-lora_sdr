cmake_minimum_required(VERSION 3.12)

project(new_framework LANGUAGES C)

# Enable testing via CTest
include(CTest)
enable_testing()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/process_test_output.py
               ${CMAKE_CURRENT_BINARY_DIR}/process_test_output.py COPYONLY)

add_subdirectory(src)
add_subdirectory(tests)
