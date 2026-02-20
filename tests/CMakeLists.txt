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

# Fixture-based sign add tests
set(SIGN_TEST_PDF "${CMAKE_CURRENT_LIST_DIR}/fixtures/minimal.pdf")
set(SIGN_TEST_KEY "${CMAKE_CURRENT_LIST_DIR}/fixtures/test.p12")

# Invalid digest value is rejected before the document is opened
add_test(NAME cli_sign_add_invalid_digest
    COMMAND vanillapdf-cli sign add "${SIGN_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_sign_invalid_digest.pdf"
            -k "${SIGN_TEST_KEY}" --digest bogus)
set_tests_properties(cli_sign_add_invalid_digest PROPERTIES WILL_FAIL TRUE)

# Each supported digest should produce a signed output
add_test(NAME cli_sign_add_sha256
    COMMAND vanillapdf-cli sign add "${SIGN_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_sign_sha256.pdf"
            -k "${SIGN_TEST_KEY}")

add_test(NAME cli_sign_add_sha512
    COMMAND vanillapdf-cli sign add "${SIGN_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_sign_sha512.pdf"
            -k "${SIGN_TEST_KEY}" --digest sha512)

add_test(NAME cli_sign_add_sha384
    COMMAND vanillapdf-cli sign add "${SIGN_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_sign_sha384.pdf"
            -k "${SIGN_TEST_KEY}" --digest sha384)

add_test(NAME cli_sign_add_sha1
    COMMAND vanillapdf-cli sign add "${SIGN_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_sign_sha1.pdf"
            -k "${SIGN_TEST_KEY}" --digest sha1)
