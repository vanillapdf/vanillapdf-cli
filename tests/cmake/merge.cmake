# merge command

add_test(NAME cli_merge_help COMMAND vanillapdf-cli merge --help)

add_test(NAME cli_merge_no_args COMMAND vanillapdf-cli merge)
set_tests_properties(cli_merge_no_args PROPERTIES WILL_FAIL TRUE)

# Fixture-based merge tests (regression for the spurious blank page, #32)
add_test(NAME cli_merge_two
    COMMAND vanillapdf-cli merge "${CLI_FIXTURES}/minimal.pdf" "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_merge_two.pdf")
set_tests_properties(cli_merge_two PROPERTIES FIXTURES_SETUP MERGE_TWO)
add_test(NAME cli_merge_two_count COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_merge_two.pdf")
set_tests_properties(cli_merge_two_count PROPERTIES
    FIXTURES_REQUIRED MERGE_TWO PASS_REGULAR_EXPRESSION "Pages: 2")

add_test(NAME cli_merge_one
    COMMAND vanillapdf-cli merge "${CLI_FIXTURES}/minimal.pdf" -o "${CMAKE_BINARY_DIR}/test_merge_one.pdf")
set_tests_properties(cli_merge_one PROPERTIES FIXTURES_SETUP MERGE_ONE)
add_test(NAME cli_merge_one_count COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_merge_one.pdf")
set_tests_properties(cli_merge_one_count PROPERTIES
    FIXTURES_REQUIRED MERGE_ONE PASS_REGULAR_EXPRESSION "Pages: 1")
