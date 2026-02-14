# CLI integration tests using CTest

# Helpers
add_test(NAME cli_version COMMAND vanillapdf-cli --version)
add_test(NAME cli_help COMMAND vanillapdf-cli --help)

# Missing required args should fail
add_test(NAME cli_no_args COMMAND vanillapdf-cli)
set_tests_properties(cli_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_unknown_subcommand COMMAND vanillapdf-cli nonexistent)
set_tests_properties(cli_unknown_subcommand PROPERTIES WILL_FAIL TRUE)

# Subcommand help
add_test(NAME cli_info_help COMMAND vanillapdf-cli info --help)
add_test(NAME cli_merge_help COMMAND vanillapdf-cli merge --help)
add_test(NAME cli_sign_help COMMAND vanillapdf-cli sign --help)
add_test(NAME cli_verify_help COMMAND vanillapdf-cli verify --help)
add_test(NAME cli_encrypt_help COMMAND vanillapdf-cli encrypt --help)
add_test(NAME cli_decrypt_help COMMAND vanillapdf-cli decrypt --help)
add_test(NAME cli_extract_help COMMAND vanillapdf-cli extract --help)

# Missing required arguments should fail
add_test(NAME cli_info_no_input COMMAND vanillapdf-cli info)
set_tests_properties(cli_info_no_input PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_merge_no_args COMMAND vanillapdf-cli merge)
set_tests_properties(cli_merge_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_sign_no_args COMMAND vanillapdf-cli sign)
set_tests_properties(cli_sign_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_verify_no_args COMMAND vanillapdf-cli verify)
set_tests_properties(cli_verify_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_encrypt_no_args COMMAND vanillapdf-cli encrypt)
set_tests_properties(cli_encrypt_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_decrypt_no_args COMMAND vanillapdf-cli decrypt)
set_tests_properties(cli_decrypt_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_extract_no_args COMMAND vanillapdf-cli extract)
set_tests_properties(cli_extract_no_args PROPERTIES WILL_FAIL TRUE)
