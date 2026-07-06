# validate command

add_test(NAME cli_validate_help COMMAND vanillapdf-cli validate --help)

add_test(NAME cli_validate_no_args COMMAND vanillapdf-cli validate)
set_tests_properties(cli_validate_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_validate_minimal COMMAND vanillapdf-cli validate "${CLI_FIXTURES}/minimal.pdf")
add_test(NAME cli_validate_minimal_json COMMAND vanillapdf-cli validate "${CLI_FIXTURES}/minimal.pdf" --json)
add_test(NAME cli_validate_minimal_strict COMMAND vanillapdf-cli validate "${CLI_FIXTURES}/minimal.pdf" --strict)
