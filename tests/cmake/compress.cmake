# compress command

add_test(NAME cli_compress_help COMMAND vanillapdf-cli compress --help)

add_test(NAME cli_compress_no_args COMMAND vanillapdf-cli compress)
set_tests_properties(cli_compress_no_args PROPERTIES WILL_FAIL TRUE)
