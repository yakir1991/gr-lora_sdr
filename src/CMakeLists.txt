add_library(signal_utils signal_utils.c)
target_include_directories(signal_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(signal_utils PUBLIC m)

add_library(lora_utils lora_utils.c)
target_include_directories(lora_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lora_utils PUBLIC m)
if(LORA_LITE_FIXED_POINT)
  target_link_libraries(lora_utils PUBLIC liquid)
endif()


add_library(lora_io lora_io.c)
target_include_directories(lora_io PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

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

# Path to legacy sources used by the FFT demodulator
set(LEGACY_LIB_DIR ../legacy_gr_lora_sdr/lib)

add_library(lora_fft_demod lora_fft_demod.c ${LEGACY_LIB_DIR}/kiss_fft.c)
target_include_directories(lora_fft_demod PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${LEGACY_LIB_DIR})
target_link_libraries(lora_fft_demod PUBLIC m lora_utils)

add_library(lora_header lora_header.c)
target_include_directories(lora_header PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(lora_frame_sync lora_frame_sync.c)
target_include_directories(lora_frame_sync PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hello_world hello_world.c)
target_link_libraries(hello_world PRIVATE signal_utils)

add_library(lora_tx_chain lora_tx_chain.c)
target_include_directories(lora_tx_chain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lora_tx_chain PUBLIC lora_add_crc lora_whitening lora_mod m)

add_library(lora_rx_chain lora_rx_chain.c)
target_include_directories(lora_rx_chain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lora_rx_chain PUBLIC lora_add_crc lora_whitening lora_fft_demod m)

add_executable(lora_chain_runner lora_chain_runner.c)
target_link_libraries(lora_chain_runner PRIVATE lora_tx_chain lora_rx_chain lora_io m)

set(LORA_LITE_TARGETS
  signal_utils
  lora_utils
  lora_io
  lora_add_crc
  lora_payload_id
  lora_whitening
  lora_hamming
  lora_interleaver
  lora_graymap
  lora_mod
  lora_fft_demod
  lora_header
  lora_frame_sync
  hello_world
  lora_tx_chain
  lora_rx_chain
  lora_chain_runner
)
foreach(t ${LORA_LITE_TARGETS})
  target_compile_definitions(${t} PUBLIC $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
endforeach()
