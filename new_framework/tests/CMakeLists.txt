add_test(NAME hello_world COMMAND hello_world)
set_tests_properties(hello_world PROPERTIES PASS_REGULAR_EXPRESSION "Hello, world!")
