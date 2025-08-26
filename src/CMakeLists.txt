list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_library(signal_utils signal_utils.c)
target_include_directories(signal_utils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)
target_link_libraries(signal_utils PUBLIC m)

add_library(lora_utils lora_utils.c)
target_include_directories(lora_utils PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)
target_link_libraries(lora_utils PUBLIC m)
if(LORA_LITE_FIXED_POINT)
  target_link_libraries(lora_utils PUBLIC liquid)
endif()


add_library(lora_io lora_io.c)
target_include_directories(lora_io PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)

add_library(lora_add_crc lora_add_crc.c)
target_include_directories(lora_add_crc PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)

add_library(lora_payload_id lora_payload_id.c)
target_include_directories(lora_payload_id PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)

add_library(lora_whitening lora_whitening.c)
target_include_directories(lora_whitening PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)
target_link_libraries(lora_whitening PUBLIC lora_utils)

add_library(lora_hamming lora_hamming.c)
target_include_directories(lora_hamming PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)

add_library(lora_interleaver lora_interleaver.c)
target_include_directories(lora_interleaver PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)

add_library(lora_graymap lora_graymap.c)
target_include_directories(lora_graymap PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)

add_library(lora_mod lora_mod.c)
target_include_directories(lora_mod PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)
target_link_libraries(lora_mod PUBLIC lora_utils m)

# Path to legacy sources used by the FFT demodulator
set(LEGACY_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../legacy_gr_lora_sdr/lib)

if(LORA_LITE_USE_LIQUID_FFT)
  find_package(Liquid REQUIRED)
  add_library(lora_fft_demod lora_fft_demod.c lora_fft_demod_ctx.c)
  target_include_directories(lora_fft_demod PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/lora_lite>)
  target_link_libraries(lora_fft_demod PUBLIC lora_utils m)
  target_link_libraries(lora_fft_demod PRIVATE Liquid::liquid)
  target_compile_definitions(lora_fft_demod PUBLIC LORA_LITE_LIQUID_FFT LORA_LITE_USE_LIQUID_FFT)
else()
  add_library(lora_fft_demod lora_fft_demod.c lora_fft_demod_ctx.c ${LEGACY_LIB_DIR}/kiss_fft.c)
  target_include_directories(lora_fft_demod PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${LEGACY_LIB_DIR}>
    $<INSTALL_INTERFACE:include/lora_lite>)
  target_link_libraries(lora_fft_demod PUBLIC lora_utils m)
endif()

add_library(lora_header lora_header.c)
target_include_directories(lora_header PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)

add_library(lora_frame_sync lora_frame_sync.c)
target_include_directories(lora_frame_sync PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)

add_executable(hello_world hello_world.c)
target_link_libraries(hello_world PRIVATE signal_utils)

add_library(lora_tx_chain lora_tx_chain.c)
target_include_directories(lora_tx_chain PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)
target_link_libraries(lora_tx_chain PUBLIC lora_add_crc lora_whitening lora_mod m)

add_library(lora_rx_chain lora_rx_chain.c)
target_include_directories(lora_rx_chain PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/lora_lite>)
target_link_libraries(lora_rx_chain PUBLIC lora_add_crc lora_whitening lora_fft_demod m)

add_executable(lora_chain_runner lora_chain_runner.c)
target_link_libraries(lora_chain_runner PRIVATE lora_tx_chain lora_rx_chain lora_io m)

set(LORA_LITE_LIBS
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
  lora_tx_chain
  lora_rx_chain
)
set(LORA_LITE_EXES
  hello_world
  lora_chain_runner
)
set(LORA_LITE_TARGETS ${LORA_LITE_LIBS} ${LORA_LITE_EXES})
foreach(t ${LORA_LITE_TARGETS})
  target_compile_definitions(${t} PRIVATE
    $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>
    LORA_LITE_ENABLE_LOGGING=$<IF:$<BOOL:${LORA_LITE_ENABLE_LOGGING}>,1,0>)
endforeach()

install(TARGETS ${LORA_LITE_LIBS}
        EXPORT lora_liteTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
        DESTINATION include/lora_lite
        FILES_MATCHING PATTERN "*.h")

install(EXPORT lora_liteTargets
        FILE lora_liteTargets.cmake
        NAMESPACE lora_lite::
        DESTINATION lib/cmake/lora_lite)
