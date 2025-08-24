cmake_minimum_required(VERSION 3.12)

project(new_framework LANGUAGES C)

# Enable testing via CTest
include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
