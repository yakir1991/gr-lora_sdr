add_test(NAME hello_world COMMAND hello_world)
set_tests_properties(hello_world PROPERTIES PASS_REGULAR_EXPRESSION "Hello, world!")

add_executable(test_signal_utils test_signal_utils.c)
target_link_libraries(test_signal_utils PRIVATE signal_utils)
add_test(NAME test_signal_utils COMMAND test_signal_utils)
set_tests_properties(test_signal_utils PROPERTIES PASS_REGULAR_EXPRESSION "RMS: 1.0")

add_executable(test_lora_io test_lora_io.c)
target_link_libraries(test_lora_io PRIVATE lora_io)
add_test(NAME test_lora_io COMMAND test_lora_io)
set_tests_properties(test_lora_io PROPERTIES PASS_REGULAR_EXPRESSION "Read [0-9]+ bytes successfully" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(test_lora_add_crc test_lora_add_crc.c)
target_link_libraries(test_lora_add_crc PRIVATE lora_add_crc)
add_test(NAME test_lora_add_crc COMMAND test_lora_add_crc)
set_tests_properties(test_lora_add_crc PROPERTIES PASS_REGULAR_EXPRESSION "All CRC tests passed")

add_executable(test_lora_payload_id test_lora_payload_id.c)
target_link_libraries(test_lora_payload_id PRIVATE lora_payload_id)
add_test(NAME test_lora_payload_id COMMAND test_lora_payload_id)
set_tests_properties(test_lora_payload_id PROPERTIES PASS_REGULAR_EXPRESSION "Payload ID test passed")

add_executable(test_lora_whitening test_lora_whitening.c)
target_link_libraries(test_lora_whitening PRIVATE lora_whitening)
add_test(NAME test_lora_whitening COMMAND test_lora_whitening)
set_tests_properties(test_lora_whitening PROPERTIES PASS_REGULAR_EXPRESSION "Whitening test passed")

add_executable(test_lora_hamming test_lora_hamming.c)
target_link_libraries(test_lora_hamming PRIVATE lora_hamming)
add_test(NAME test_lora_hamming COMMAND test_lora_hamming)
set_tests_properties(test_lora_hamming PROPERTIES PASS_REGULAR_EXPRESSION "All Hamming tests passed")

add_executable(test_lora_interleaver test_lora_interleaver.c)
target_link_libraries(test_lora_interleaver PRIVATE lora_interleaver)
add_test(NAME test_lora_interleaver COMMAND test_lora_interleaver)
set_tests_properties(test_lora_interleaver PROPERTIES PASS_REGULAR_EXPRESSION "Interleaver test passed")

add_executable(test_lora_graymap test_lora_graymap.c)
target_link_libraries(test_lora_graymap PRIVATE lora_graymap)
add_test(NAME test_lora_graymap COMMAND test_lora_graymap)
set_tests_properties(test_lora_graymap PROPERTIES PASS_REGULAR_EXPRESSION "Gray map test passed")

add_executable(test_lora_mod_fft test_lora_mod_fft.c)
target_link_libraries(test_lora_mod_fft PRIVATE
    lora_mod lora_fft_demod
    $<$<NOT:$<BOOL:${LORA_LITE_FIXED_POINT}>>:m>)
add_test(NAME test_lora_mod_fft COMMAND test_lora_mod_fft)
set_tests_properties(test_lora_mod_fft PROPERTIES PASS_REGULAR_EXPRESSION "Mod/demod round-trip passed")

add_executable(test_lora_frame_sync test_lora_frame_sync.c)
target_link_libraries(test_lora_frame_sync PRIVATE lora_frame_sync)
add_test(NAME test_lora_frame_sync COMMAND test_lora_frame_sync)
set_tests_properties(test_lora_frame_sync PROPERTIES PASS_REGULAR_EXPRESSION "Frame sync test passed")
add_executable(test_lora_header test_lora_header.c)
target_link_libraries(test_lora_header PRIVATE lora_header)
add_test(NAME test_lora_header COMMAND test_lora_header)
set_tests_properties(test_lora_header PROPERTIES PASS_REGULAR_EXPRESSION "All header tests passed")

add_executable(test_lora_chain test_lora_chain.c)
target_link_libraries(test_lora_chain PRIVATE lora_tx_chain lora_rx_chain)
add_test(NAME test_lora_chain COMMAND test_lora_chain)
set_tests_properties(test_lora_chain PROPERTIES PASS_REGULAR_EXPRESSION "Payload recovered successfully")

add_executable(test_end_to_end_file test_end_to_end_file.c)
target_link_libraries(test_end_to_end_file PRIVATE lora_tx_chain lora_rx_chain lora_io)
  add_test(NAME test_end_to_end_file COMMAND test_end_to_end_file)
  set_tests_properties(test_end_to_end_file PROPERTIES PASS_REGULAR_EXPRESSION "End-to-end file test passed" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_executable(test_tx_rx_compare test_tx_rx_compare.c)
target_link_libraries(test_tx_rx_compare PRIVATE lora_io)
add_test(NAME test_tx_rx_compare COMMAND test_tx_rx_compare)
set_tests_properties(test_tx_rx_compare PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_executable(test_full_chain_compare test_full_chain_compare.c)
target_link_libraries(test_full_chain_compare PRIVATE lora_io)
add_test(NAME full_chain_compare COMMAND test_full_chain_compare)
set_tests_properties(full_chain_compare PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(test_fixed_float_float
  test_fixed_float_equiv.c
  ../src/lora_mod.c
  ../src/lora_fft_demod.c
  ../src/lora_utils.c
  ../legacy_gr_lora_sdr/lib/kiss_fft.c)
target_include_directories(test_fixed_float_float PRIVATE ../src ../legacy_gr_lora_sdr/lib ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_fixed_float_float PRIVATE m)

add_executable(test_fixed_float_fixed
  test_fixed_float_equiv.c
  ../src/lora_mod.c
  ../src/lora_fft_demod.c
  ../src/lora_utils.c
  ../legacy_gr_lora_sdr/lib/kiss_fft.c)
target_include_directories(test_fixed_float_fixed PRIVATE ../src ../legacy_gr_lora_sdr/lib ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(test_fixed_float_fixed PRIVATE LORA_LITE_FIXED_POINT)
target_link_libraries(test_fixed_float_fixed PRIVATE m)

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

set(LORA_LITE_TEST_TARGETS
  test_signal_utils
  test_lora_io
  test_lora_add_crc
  test_lora_payload_id
  test_lora_whitening
  test_lora_hamming
  test_lora_interleaver
  test_lora_graymap
  test_lora_mod_fft
  test_lora_frame_sync
  test_lora_header
  test_lora_chain
  test_end_to_end_file
  test_tx_rx_compare
  test_full_chain_compare
)
foreach(t ${LORA_LITE_TEST_TARGETS})
  target_compile_definitions(${t} PUBLIC $<$<BOOL:${LORA_LITE_FIXED_POINT}>:LORA_LITE_FIXED_POINT>)
endforeach()
