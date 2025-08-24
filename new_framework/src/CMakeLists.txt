add_library(signal_utils signal_utils.c)

target_include_directories(signal_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(signal_utils PUBLIC m)

add_executable(hello_world hello_world.c)
target_link_libraries(hello_world PRIVATE signal_utils)
