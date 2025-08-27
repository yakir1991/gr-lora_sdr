function(lora_add_test NAME)
  set(options)
  set(oneValueArgs PASS_REGEX WORKING_DIR)
  set(multiValueArgs SOURCES LIBS INCLUDES DEFINITIONS)
  cmake_parse_arguments(LT "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_executable(${NAME} ${LT_SOURCES})
  target_link_libraries(${NAME} PRIVATE ${LT_LIBS})
  target_include_directories(${NAME} PRIVATE ${LT_INCLUDES})
  target_compile_definitions(${NAME} PRIVATE ${LT_DEFINITIONS} LORA_LITE_ENABLE_LOGGING=1 $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
  add_test(NAME ${NAME} COMMAND ${NAME})
  if(LT_PASS_REGEX)
    set_tests_properties(${NAME} PROPERTIES PASS_REGULAR_EXPRESSION "${LT_PASS_REGEX}")
  endif()
  if(LT_WORKING_DIR)
    set_tests_properties(${NAME} PROPERTIES WORKING_DIRECTORY ${LT_WORKING_DIR})
  endif()

  add_executable(${NAME}_nolog ${LT_SOURCES})
  target_link_libraries(${NAME}_nolog PRIVATE ${LT_LIBS})
  target_include_directories(${NAME}_nolog PRIVATE ${LT_INCLUDES})
  target_compile_options(${NAME}_nolog PRIVATE -U LORA_LITE_ENABLE_LOGGING)
  target_compile_definitions(${NAME}_nolog PRIVATE ${LT_DEFINITIONS} LORA_LITE_ENABLE_LOGGING=0 $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
  add_test(NAME ${NAME}_nolog COMMAND ${NAME}_nolog)
  if(LT_WORKING_DIR)
    set_tests_properties(${NAME}_nolog PROPERTIES WORKING_DIRECTORY ${LT_WORKING_DIR})
  endif()
endfunction()

# Expose project headers to all tests (needed for lora_fft.h, ctx headers, etc.)
include_directories(${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)

add_test(NAME hello_world COMMAND hello_world)
if(LORA_LITE_ENABLE_LOGGING)
  set_tests_properties(hello_world PROPERTIES PASS_REGULAR_EXPRESSION "Hello, world!")
endif()

add_executable(hello_world_nolog ../src/hello_world.c)
target_link_libraries(hello_world_nolog PRIVATE signal_utils)
target_compile_options(hello_world_nolog PRIVATE -U LORA_LITE_ENABLE_LOGGING)
target_compile_definitions(hello_world_nolog PRIVATE LORA_LITE_ENABLE_LOGGING=0 $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
add_test(NAME hello_world_nolog COMMAND hello_world_nolog)

lora_add_test(test_signal_utils
  SOURCES test_signal_utils.c
  LIBS signal_utils
  PASS_REGEX "RMS: 1.0")

lora_add_test(test_lora_io
  SOURCES test_lora_io.c
  LIBS lora_io
  PASS_REGEX "Read [0-9]+ bytes successfully"
  WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})

lora_add_test(test_lora_add_crc
  SOURCES test_lora_add_crc.c
  LIBS lora_add_crc
  PASS_REGEX "All CRC tests passed")

lora_add_test(test_lora_payload_id
  SOURCES test_lora_payload_id.c
  LIBS lora_payload_id
  PASS_REGEX "Payload ID test passed")

lora_add_test(test_lora_whitening
  SOURCES test_lora_whitening.c
  LIBS lora_whitening
  PASS_REGEX "Whitening test passed")

lora_add_test(test_whitening_golden
  SOURCES test_whitening_golden.c
  LIBS lora_whitening
  PASS_REGEX "Whitening golden test passed")

lora_add_test(test_lora_hamming
  SOURCES test_lora_hamming.c
  LIBS lora_hamming
  PASS_REGEX "All Hamming tests passed")

lora_add_test(test_lora_interleaver
  SOURCES test_lora_interleaver.c
  LIBS lora_interleaver
  PASS_REGEX "Interleaver test passed")

lora_add_test(test_interleaver_golden
  SOURCES test_interleaver_golden.c
  LIBS lora_interleaver
  PASS_REGEX "Interleaver golden test passed")

lora_add_test(test_lora_graymap
  SOURCES test_lora_graymap.c
  LIBS lora_graymap
  PASS_REGEX "Gray map test passed")

lora_add_test(test_gray_golden
  SOURCES test_gray_golden.c
  LIBS lora_graymap
  PASS_REGEX "Gray golden test passed")

lora_add_test(test_os_factor_check
  SOURCES test_os_factor_check.c
  LIBS lora_mod lora_fft_demod
  PASS_REGEX "OS factor check test passed")

lora_add_test(test_lora_mod_fft
  SOURCES test_lora_mod_fft.c
  LIBS lora_mod lora_fft_demod $<$<NOT:$<BOOL:${LORA_LITE_FIXED_POINT}>>:m>
  PASS_REGEX "FFT demod regression test passed")

lora_add_test(test_lora_frame_sync
  SOURCES test_lora_frame_sync.c
  LIBS lora_frame_sync
  PASS_REGEX "Frame sync test passed")

lora_add_test(test_lora_header
  SOURCES test_lora_header.c
  LIBS lora_header
  PASS_REGEX "All header tests passed")

lora_add_test(test_lora_chain
  SOURCES test_lora_chain.c
  LIBS lora_tx_chain lora_rx_chain
  PASS_REGEX "Payload recovered successfully")

lora_add_test(test_lora_chain_edge_cases
  SOURCES test_lora_chain_edge_cases.c
  LIBS lora_tx_chain lora_rx_chain
  PASS_REGEX "Edge case tests passed")

lora_add_test(embedded_loopback
  SOURCES embedded_loopback.c
  LIBS lora_tx_chain lora_rx_chain
  DEFINITIONS LORA_LITE_FIXED_POINT
  PASS_REGEX "Embedded loopback test passed")

lora_add_test(test_end_to_end_file
  SOURCES test_end_to_end_file.c
  LIBS lora_tx_chain lora_rx_chain lora_io
  PASS_REGEX "End-to-end file test passed"
  WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})

lora_add_test(test_ber_snr
  SOURCES test_ber_snr.c
  LIBS lora_tx_chain lora_rx_chain
  WORKING_DIR ${CMAKE_SOURCE_DIR})

lora_add_test(test_tx_rx_compare
  SOURCES test_tx_rx_compare.c
  LIBS lora_io
  WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})

lora_add_test(test_full_chain_compare
  SOURCES test_full_chain_compare.c
  LIBS lora_io
  WORKING_DIR ${CMAKE_SOURCE_DIR})

lora_add_test(test_fixed_float_float
  SOURCES test_fixed_float_equiv.c ../src/lora_mod.c ../src/lora_fft_demod.c ../src/lora_utils.c ../src/lora_fft.c ../src/q15_to_cf.c
  LIBS m
  DEFINITIONS LORA_LITE_FIXED_POINT)

lora_add_test(test_fixed_float_fixed
  SOURCES test_fixed_float_equiv.c ../src/lora_mod.c ../src/lora_fft_demod.c ../src/lora_utils.c ../src/lora_fft.c ../src/q15_to_cf.c
  LIBS m
  DEFINITIONS LORA_LITE_FIXED_POINT)

add_test(NAME run_fixed_float_float
  COMMAND test_fixed_float_float ${CMAKE_CURRENT_BINARY_DIR}/float.bin)
set_tests_properties(run_fixed_float_float PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME run_fixed_float_fixed
  COMMAND test_fixed_float_fixed ${CMAKE_CURRENT_BINARY_DIR}/fixed.bin)
set_tests_properties(run_fixed_float_fixed PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME compare_fixed_float
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compare_fixed_float.sh
          ${CMAKE_CURRENT_BINARY_DIR}/float.bin
          ${CMAKE_CURRENT_BINARY_DIR}/fixed.bin)
set_tests_properties(compare_fixed_float PROPERTIES
  DEPENDS "run_fixed_float_float;run_fixed_float_fixed"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME run_fixed_float_float_nolog
  COMMAND test_fixed_float_float_nolog ${CMAKE_CURRENT_BINARY_DIR}/float_nolog.bin)
set_tests_properties(run_fixed_float_float_nolog PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME run_fixed_float_fixed_nolog
  COMMAND test_fixed_float_fixed_nolog ${CMAKE_CURRENT_BINARY_DIR}/fixed_nolog.bin)
set_tests_properties(run_fixed_float_fixed_nolog PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME compare_fixed_float_nolog
  COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compare_fixed_float.sh
          ${CMAKE_CURRENT_BINARY_DIR}/float_nolog.bin
          ${CMAKE_CURRENT_BINARY_DIR}/fixed_nolog.bin)
set_tests_properties(compare_fixed_float_nolog PROPERTIES
  DEPENDS "run_fixed_float_float_nolog;run_fixed_float_fixed_nolog"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(bench_lora_chain benchmarks/bench_lora_chain.c)
target_link_libraries(bench_lora_chain PRIVATE lora_tx_chain lora_rx_chain)
target_compile_definitions(bench_lora_chain PRIVATE LORA_LITE_ENABLE_LOGGING=0 $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
target_compile_options(bench_lora_chain PRIVATE -O3 -DNDEBUG)
add_test(NAME bench_lora_chain COMMAND bench_lora_chain bench_results.csv)
set_tests_properties(bench_lora_chain PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Micro-benchmark: FFT demod (float and fixed variants)
add_executable(bench_demod benchmarks/bench_demod.c)
target_link_libraries(bench_demod PRIVATE lora_fft_demod)
target_compile_definitions(bench_demod PRIVATE LORA_LITE_ENABLE_LOGGING=0 $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
target_compile_options(bench_demod PRIVATE -O3 -DNDEBUG)
add_test(NAME bench_demod COMMAND bench_demod)

add_executable(bench_demod_fixed benchmarks/bench_demod.c)
target_link_libraries(bench_demod_fixed PRIVATE lora_fft_demod)
target_compile_definitions(bench_demod_fixed PRIVATE LORA_LITE_ENABLE_LOGGING=0 LORA_LITE_FIXED_POINT)
target_compile_options(bench_demod_fixed PRIVATE -O3 -DNDEBUG)
add_test(NAME bench_demod_fixed COMMAND bench_demod_fixed)

# Correctness: minimal FFT demod sanity
add_executable(test_fft_demod_correctness test_fft_demod_correctness.c)
target_link_libraries(test_fft_demod_correctness PRIVATE lora_fft_demod)
target_compile_definitions(test_fft_demod_correctness PRIVATE LORA_LITE_ENABLE_LOGGING=1 $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
add_test(NAME test_fft_demod_correctness COMMAND test_fft_demod_correctness)

# Micro-benchmark: Whitening throughput
add_executable(bench_whitening benchmarks/bench_whitening.c)
target_link_libraries(bench_whitening PRIVATE lora_whitening)
target_compile_definitions(bench_whitening PRIVATE LORA_LITE_ENABLE_LOGGING=0 $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
target_compile_options(bench_whitening PRIVATE -O3 -DNDEBUG)
add_test(NAME bench_whitening COMMAND bench_whitening)

# Micro-benchmark: Demod with CFO
add_executable(bench_demod_cfo benchmarks/bench_demod_cfo.c)
target_link_libraries(bench_demod_cfo PRIVATE lora_fft_demod)
target_compile_definitions(bench_demod_cfo PRIVATE LORA_LITE_ENABLE_LOGGING=0 $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
target_compile_options(bench_demod_cfo PRIVATE -O3 -DNDEBUG)
add_test(NAME bench_demod_cfo COMMAND bench_demod_cfo)

# Micro-benchmark: Interleaver
add_executable(bench_interleaver benchmarks/bench_interleaver.c)
target_link_libraries(bench_interleaver PRIVATE lora_interleaver)
target_compile_definitions(bench_interleaver PRIVATE LORA_LITE_ENABLE_LOGGING=0)
target_compile_options(bench_interleaver PRIVATE -O3 -DNDEBUG)
add_test(NAME bench_interleaver COMMAND bench_interleaver)

# Micro-benchmark: CRC
add_executable(bench_crc benchmarks/bench_crc.c)
target_link_libraries(bench_crc PRIVATE lora_add_crc)
target_compile_definitions(bench_crc PRIVATE LORA_LITE_ENABLE_LOGGING=0)
target_compile_options(bench_crc PRIVATE -O3 -DNDEBUG)
add_test(NAME bench_crc COMMAND bench_crc)

# Micro-benchmark: Modulation (upchirp generation)
add_executable(bench_mod benchmarks/bench_mod.c)
target_link_libraries(bench_mod PRIVATE lora_mod)
target_compile_definitions(bench_mod PRIVATE LORA_LITE_ENABLE_LOGGING=0 $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
target_compile_options(bench_mod PRIVATE -O3 -DNDEBUG)
add_test(NAME bench_mod COMMAND bench_mod)

# Micro-benchmark: FFT (all SFs)
add_executable(bench_fft benchmarks/bench_fft.c)
target_link_libraries(bench_fft PRIVATE lora_fft)
target_compile_definitions(bench_fft PRIVATE LORA_LITE_ENABLE_LOGGING=0)
target_compile_options(bench_fft PRIVATE -O3 -DNDEBUG)
add_test(NAME bench_fft COMMAND bench_fft)

# End-to-end embedded throughput guard (optional)
add_test(NAME pps_guard COMMAND /usr/bin/env python3 ${CMAKE_CURRENT_SOURCE_DIR}/pps_guard.py)
set_tests_properties(pps_guard PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

# Micro-benchmark: q15_to_cf conversion
if(LORA_LITE_FIXED_POINT)
  add_executable(bench_q15_to_cf benchmarks/bench_q15_to_cf.c)
  target_link_libraries(bench_q15_to_cf PRIVATE lora_fft)
  target_compile_definitions(bench_q15_to_cf PRIVATE LORA_LITE_ENABLE_LOGGING=0 LORA_LITE_FIXED_POINT)
  target_compile_options(bench_q15_to_cf PRIVATE -O3 -DNDEBUG)
  add_test(NAME bench_q15_to_cf COMMAND bench_q15_to_cf)
endif()
