add_test(NAME hello_world COMMAND hello_world)
set_tests_properties(hello_world PROPERTIES PASS_REGULAR_EXPRESSION "Hello, world!")

add_executable(test_signal_utils test_signal_utils.c)
target_link_libraries(test_signal_utils PRIVATE signal_utils)
add_test(NAME test_signal_utils COMMAND test_signal_utils)
set_tests_properties(test_signal_utils PROPERTIES PASS_REGULAR_EXPRESSION "RMS: 1.0")

add_executable(test_lora_data_source test_lora_data_source.c)
target_link_libraries(test_lora_data_source PRIVATE lora_data_source)
add_test(NAME test_lora_data_source COMMAND test_lora_data_source)
set_tests_properties(test_lora_data_source PROPERTIES PASS_REGULAR_EXPRESSION "Read [0-9]+ bytes successfully")

add_executable(test_lora_add_crc test_lora_add_crc.c)
target_link_libraries(test_lora_add_crc PRIVATE lora_add_crc)
add_test(NAME test_lora_add_crc COMMAND test_lora_add_crc)
set_tests_properties(test_lora_add_crc PROPERTIES PASS_REGULAR_EXPRESSION "All CRC tests passed")
