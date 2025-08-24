add_library(signal_utils signal_utils.c)
target_include_directories(signal_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(signal_utils PUBLIC m)

add_library(lora_utils lora_utils.c)
target_include_directories(lora_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lora_utils PUBLIC m)

add_library(lora_data_source lora_data_source.c)
target_include_directories(lora_data_source PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_add_crc lora_add_crc.c)
target_include_directories(lora_add_crc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_payload_id lora_payload_id.c)
target_include_directories(lora_payload_id PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_whitening lora_whitening.c)
target_include_directories(lora_whitening PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lora_whitening PUBLIC lora_utils)

add_library(lora_hamming lora_hamming.c)
target_include_directories(lora_hamming PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_interleaver lora_interleaver.c)
target_include_directories(lora_interleaver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_graymap lora_graymap.c)
target_include_directories(lora_graymap PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_mod lora_mod.c)
target_include_directories(lora_mod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lora_mod PUBLIC m lora_utils)

add_library(lora_fft_demod lora_fft_demod.c ../../lib/kiss_fft.c)
target_include_directories(lora_fft_demod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ../../lib)
target_link_libraries(lora_fft_demod PUBLIC m lora_utils)

add_library(lora_header lora_header.c)
target_include_directories(lora_header PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_frame_sync lora_frame_sync.c)
target_include_directories(lora_frame_sync PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hello_world hello_world.c)
target_link_libraries(hello_world PRIVATE signal_utils)

add_library(lora_tx_chain lora_tx_chain.c)
target_include_directories(lora_tx_chain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lora_tx_chain PUBLIC lora_data_source lora_add_crc lora_whitening lora_mod m)

add_library(lora_rx_chain lora_rx_chain.c)
target_include_directories(lora_rx_chain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lora_rx_chain PUBLIC lora_add_crc lora_whitening lora_fft_demod m)
