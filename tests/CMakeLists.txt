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

lora_add_test(test_lora_hamming
  SOURCES test_lora_hamming.c
  LIBS lora_hamming
  PASS_REGEX "All Hamming tests passed")

lora_add_test(test_lora_interleaver
  SOURCES test_lora_interleaver.c
  LIBS lora_interleaver
  PASS_REGEX "Interleaver test passed")

lora_add_test(test_lora_graymap
  SOURCES test_lora_graymap.c
  LIBS lora_graymap
  PASS_REGEX "Gray map test passed")

lora_add_test(test_lora_mod_fft
  SOURCES test_lora_mod_fft.c
  LIBS lora_mod lora_fft_demod $<$<NOT:$<BOOL:${LORA_LITE_FIXED_POINT}>>:m>
  PASS_REGEX "Mod/demod round-trip passed")

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

lora_add_test(test_tx_rx_compare
  SOURCES test_tx_rx_compare.c
  LIBS lora_io
  WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})

lora_add_test(test_full_chain_compare
  SOURCES test_full_chain_compare.c
  LIBS lora_io
  WORKING_DIR ${CMAKE_SOURCE_DIR})

lora_add_test(test_fixed_float_float
  SOURCES test_fixed_float_equiv.c ../src/lora_mod.c ../src/lora_fft_demod.c ../src/lora_utils.c ../legacy_gr_lora_sdr/lib/kiss_fft.c
  LIBS m
  INCLUDES ../src ../legacy_gr_lora_sdr/lib ${CMAKE_CURRENT_SOURCE_DIR})

lora_add_test(test_fixed_float_fixed
  SOURCES test_fixed_float_equiv.c ../src/lora_mod.c ../src/lora_fft_demod.c ../src/lora_utils.c ../legacy_gr_lora_sdr/lib/kiss_fft.c
  LIBS m
  DEFINITIONS LORA_LITE_FIXED_POINT
  INCLUDES ../src ../legacy_gr_lora_sdr/lib ${CMAKE_CURRENT_SOURCE_DIR})

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
