# info command

add_test(NAME cli_info_help COMMAND vanillapdf-cli info --help)

add_test(NAME cli_info_no_input COMMAND vanillapdf-cli info)
set_tests_properties(cli_info_no_input PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_info_minimal COMMAND vanillapdf-cli info "${CLI_FIXTURES}/minimal.pdf")
set_tests_properties(cli_info_minimal PROPERTIES PASS_REGULAR_EXPRESSION "Pages: 1")

add_test(NAME cli_info_minimal_json COMMAND vanillapdf-cli info "${CLI_FIXTURES}/minimal.pdf" --json)
set_tests_properties(cli_info_minimal_json PROPERTIES PASS_REGULAR_EXPRESSION "\"pages\": 1")
