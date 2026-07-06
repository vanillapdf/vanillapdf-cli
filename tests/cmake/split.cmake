# split command

add_test(NAME cli_split_help COMMAND vanillapdf-cli split --help)

add_test(NAME cli_split_no_args COMMAND vanillapdf-cli split)
set_tests_properties(cli_split_no_args PROPERTIES WILL_FAIL TRUE)

# Fixture-based split tests (regression for 1-based page indexing, #30)
add_test(NAME cli_split_all
    COMMAND vanillapdf-cli split "${CLI_FIXTURES}/minimal.pdf" -o "${CMAKE_BINARY_DIR}/test_split_all.pdf")
set_tests_properties(cli_split_all PROPERTIES FIXTURES_SETUP SPLIT_ALL)
add_test(NAME cli_split_all_count COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_split_all.pdf")
set_tests_properties(cli_split_all_count PROPERTIES
    FIXTURES_REQUIRED SPLIT_ALL PASS_REGULAR_EXPRESSION "Pages: 1")

add_test(NAME cli_split_page1
    COMMAND vanillapdf-cli split "${CLI_FIXTURES}/minimal.pdf" -o "${CMAKE_BINARY_DIR}/test_split_p1.pdf" -p 1)
set_tests_properties(cli_split_page1 PROPERTIES FIXTURES_SETUP SPLIT_P1)
add_test(NAME cli_split_page1_count COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_split_p1.pdf")
set_tests_properties(cli_split_page1_count PROPERTIES
    FIXTURES_REQUIRED SPLIT_P1 PASS_REGULAR_EXPRESSION "Pages: 1")
