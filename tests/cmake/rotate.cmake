# rotate command

add_test(NAME cli_rotate_help COMMAND vanillapdf-cli rotate --help)

add_test(NAME cli_rotate_no_args COMMAND vanillapdf-cli rotate)
set_tests_properties(cli_rotate_no_args PROPERTIES WILL_FAIL TRUE)

# Fixture-based rotate test (regression for 1-based page indexing, #31)
add_test(NAME cli_rotate_all
    COMMAND vanillapdf-cli rotate "${CLI_FIXTURES}/minimal.pdf" -a 90 -o "${CMAKE_BINARY_DIR}/test_rotate.pdf")
set_tests_properties(cli_rotate_all PROPERTIES FIXTURES_SETUP ROTATE_ALL)
add_test(NAME cli_rotate_all_count COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_rotate.pdf")
set_tests_properties(cli_rotate_all_count PROPERTIES
    FIXTURES_REQUIRED ROTATE_ALL PASS_REGULAR_EXPRESSION "Pages: 1")

# Page-range syntax: the overlap in "1-2,2" rotates page 2 once (3-page fixture)
add_test(NAME cli_rotate_range
    COMMAND vanillapdf-cli rotate "${CLI_FIXTURES}/three_pages.pdf" -a 90 -p "1-2,2"
            -o "${CMAKE_BINARY_DIR}/test_rotate_range.pdf")
set_tests_properties(cli_rotate_range PROPERTIES PASS_REGULAR_EXPRESSION "Rotated 2 pages")

add_test(NAME cli_rotate_out_of_range
    COMMAND vanillapdf-cli rotate "${CLI_FIXTURES}/three_pages.pdf" -a 90 -p last,7
            -o "${CMAKE_BINARY_DIR}/test_rotate_oor.pdf")
set_tests_properties(cli_rotate_out_of_range PROPERTIES WILL_FAIL TRUE)
