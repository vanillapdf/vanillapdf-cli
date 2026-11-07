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

# Page-range syntax (3-page fixture)
add_test(NAME cli_split_range
    COMMAND vanillapdf-cli split "${CLI_FIXTURES}/three_pages.pdf" -o "${CMAKE_BINARY_DIR}/test_split_range.pdf" -p 2-last)
set_tests_properties(cli_split_range PROPERTIES FIXTURES_SETUP SPLIT_RANGE)
add_test(NAME cli_split_range_count COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_split_range.pdf")
set_tests_properties(cli_split_range_count PROPERTIES
    FIXTURES_REQUIRED SPLIT_RANGE PASS_REGULAR_EXPRESSION "Pages: 2")

add_test(NAME cli_split_odd
    COMMAND vanillapdf-cli split "${CLI_FIXTURES}/three_pages.pdf" -o "${CMAKE_BINARY_DIR}/test_split_odd.pdf" -p odd)
set_tests_properties(cli_split_odd PROPERTIES FIXTURES_SETUP SPLIT_ODD)
add_test(NAME cli_split_odd_count COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_split_odd.pdf")
set_tests_properties(cli_split_odd_count PROPERTIES
    FIXTURES_REQUIRED SPLIT_ODD PASS_REGULAR_EXPRESSION "Pages: 2")

add_test(NAME cli_split_out_of_range
    COMMAND vanillapdf-cli split "${CLI_FIXTURES}/three_pages.pdf" -o "${CMAKE_BINARY_DIR}/test_split_oor.pdf" -p 2-5)
set_tests_properties(cli_split_out_of_range PROPERTIES
    PASS_REGULAR_EXPRESSION "out of range \\(document has 3 pages\\)")

add_test(NAME cli_split_malformed
    COMMAND vanillapdf-cli split "${CLI_FIXTURES}/three_pages.pdf" -o "${CMAKE_BINARY_DIR}/test_split_bad.pdf" -p "1,,2")
set_tests_properties(cli_split_malformed PROPERTIES WILL_FAIL TRUE)
