add_library(signal_utils signal_utils.c)
target_include_directories(signal_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(signal_utils PUBLIC m)

add_library(lora_data_source lora_data_source.c)
target_include_directories(lora_data_source PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_add_crc lora_add_crc.c)
target_include_directories(lora_add_crc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_whitening lora_whitening.c)
target_include_directories(lora_whitening PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_hamming lora_hamming.c)
target_include_directories(lora_hamming PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hello_world hello_world.c)
target_link_libraries(hello_world PRIVATE signal_utils)
