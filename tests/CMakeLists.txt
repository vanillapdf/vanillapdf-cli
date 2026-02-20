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
add_test(NAME cli_validate_help COMMAND vanillapdf-cli validate --help)
add_test(NAME cli_info_help COMMAND vanillapdf-cli info --help)
add_test(NAME cli_merge_help COMMAND vanillapdf-cli merge --help)
add_test(NAME cli_split_help COMMAND vanillapdf-cli split --help)
add_test(NAME cli_rotate_help COMMAND vanillapdf-cli rotate --help)
add_test(NAME cli_compress_help COMMAND vanillapdf-cli compress --help)
add_test(NAME cli_encrypt_help COMMAND vanillapdf-cli encrypt --help)
add_test(NAME cli_sign_help COMMAND vanillapdf-cli sign --help)

# Nested subcommand help
add_test(NAME cli_encrypt_add_help COMMAND vanillapdf-cli encrypt add --help)
add_test(NAME cli_encrypt_remove_help COMMAND vanillapdf-cli encrypt remove --help)
add_test(NAME cli_sign_add_help COMMAND vanillapdf-cli sign add --help)
add_test(NAME cli_sign_list_help COMMAND vanillapdf-cli sign list --help)
add_test(NAME cli_sign_remove_help COMMAND vanillapdf-cli sign remove --help)
add_test(NAME cli_sign_verify_help COMMAND vanillapdf-cli sign verify --help)

# Missing required arguments should fail
add_test(NAME cli_info_no_input COMMAND vanillapdf-cli info)
set_tests_properties(cli_info_no_input PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_merge_no_args COMMAND vanillapdf-cli merge)
set_tests_properties(cli_merge_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_split_no_args COMMAND vanillapdf-cli split)
set_tests_properties(cli_split_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_rotate_no_args COMMAND vanillapdf-cli rotate)
set_tests_properties(cli_rotate_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_compress_no_args COMMAND vanillapdf-cli compress)
set_tests_properties(cli_compress_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_encrypt_no_subcommand COMMAND vanillapdf-cli encrypt)
set_tests_properties(cli_encrypt_no_subcommand PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_encrypt_add_no_args COMMAND vanillapdf-cli encrypt add)
set_tests_properties(cli_encrypt_add_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_encrypt_remove_no_args COMMAND vanillapdf-cli encrypt remove)
set_tests_properties(cli_encrypt_remove_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_sign_no_subcommand COMMAND vanillapdf-cli sign)
set_tests_properties(cli_sign_no_subcommand PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_sign_add_no_args COMMAND vanillapdf-cli sign add)
set_tests_properties(cli_sign_add_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_sign_list_no_args COMMAND vanillapdf-cli sign list)
set_tests_properties(cli_sign_list_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_sign_remove_no_args COMMAND vanillapdf-cli sign remove)
set_tests_properties(cli_sign_remove_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_sign_verify_no_args COMMAND vanillapdf-cli sign verify)
set_tests_properties(cli_sign_verify_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_validate_no_args COMMAND vanillapdf-cli validate)
set_tests_properties(cli_validate_no_args PROPERTIES WILL_FAIL TRUE)

# Fixture-based validate tests
set(VALIDATE_TEST_PDF "${CMAKE_CURRENT_LIST_DIR}/fixtures/minimal.pdf")

add_test(NAME cli_validate_minimal COMMAND vanillapdf-cli validate "${VALIDATE_TEST_PDF}")
add_test(NAME cli_validate_minimal_json COMMAND vanillapdf-cli validate "${VALIDATE_TEST_PDF}" --json)
add_test(NAME cli_validate_minimal_strict COMMAND vanillapdf-cli validate "${VALIDATE_TEST_PDF}" --strict)
