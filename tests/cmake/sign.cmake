# sign command

add_test(NAME cli_sign_help COMMAND vanillapdf-cli sign --help)
add_test(NAME cli_sign_add_help COMMAND vanillapdf-cli sign add --help)
add_test(NAME cli_sign_list_help COMMAND vanillapdf-cli sign list --help)
add_test(NAME cli_sign_verify_help COMMAND vanillapdf-cli sign verify --help)

add_test(NAME cli_sign_no_subcommand COMMAND vanillapdf-cli sign)
set_tests_properties(cli_sign_no_subcommand PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_sign_add_no_args COMMAND vanillapdf-cli sign add)
set_tests_properties(cli_sign_add_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_sign_list_no_args COMMAND vanillapdf-cli sign list)
set_tests_properties(cli_sign_list_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_sign_verify_no_args COMMAND vanillapdf-cli sign verify)
set_tests_properties(cli_sign_verify_no_args PROPERTIES WILL_FAIL TRUE)

set(SIGN_TEST_KEY "${CLI_FIXTURES}/test.p12")

# Invalid digest value is rejected before the document is opened
add_test(NAME cli_sign_add_invalid_digest
    COMMAND vanillapdf-cli sign add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_sign_invalid_digest.pdf"
            -k "${SIGN_TEST_KEY}" --digest bogus)
set_tests_properties(cli_sign_add_invalid_digest PROPERTIES WILL_FAIL TRUE)

# Each supported digest should produce a signed output
add_test(NAME cli_sign_add_sha256
    COMMAND vanillapdf-cli sign add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_sign_sha256.pdf"
            -k "${SIGN_TEST_KEY}")

add_test(NAME cli_sign_add_sha512
    COMMAND vanillapdf-cli sign add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_sign_sha512.pdf"
            -k "${SIGN_TEST_KEY}" --digest sha512)

add_test(NAME cli_sign_add_sha384
    COMMAND vanillapdf-cli sign add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_sign_sha384.pdf"
            -k "${SIGN_TEST_KEY}" --digest sha384)

add_test(NAME cli_sign_add_sha1
    COMMAND vanillapdf-cli sign add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_sign_sha1.pdf"
            -k "${SIGN_TEST_KEY}" --digest sha1)

# sign list on a freshly-signed document. Guards against a regression where a
# signature lacking optional fields (Name/Reason/...) aborted the whole listing.
add_test(NAME cli_sign_list_setup
    COMMAND vanillapdf-cli sign add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_sign_list.pdf" -k "${SIGN_TEST_KEY}")
set_tests_properties(cli_sign_list_setup PROPERTIES FIXTURES_SETUP SIGNED_FOR_LIST)

add_test(NAME cli_sign_list_text
    COMMAND vanillapdf-cli sign list "${CMAKE_BINARY_DIR}/test_sign_list.pdf")
set_tests_properties(cli_sign_list_text PROPERTIES
    FIXTURES_REQUIRED SIGNED_FOR_LIST
    PASS_REGULAR_EXPRESSION "Signatures: 1")

add_test(NAME cli_sign_list_json
    COMMAND vanillapdf-cli sign list "${CMAKE_BINARY_DIR}/test_sign_list.pdf" --json)
set_tests_properties(cli_sign_list_json PROPERTIES
    FIXTURES_REQUIRED SIGNED_FOR_LIST
    PASS_REGULAR_EXPRESSION "\"signatures\"")

# sign add with metadata, then confirm sign list reads it back
add_test(NAME cli_sign_add_metadata_setup
    COMMAND vanillapdf-cli sign add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_sign_meta.pdf" -k "${SIGN_TEST_KEY}"
            --name "Jane Smith" --reason "Approved" --location "London")
set_tests_properties(cli_sign_add_metadata_setup PROPERTIES FIXTURES_SETUP SIGNED_META)

add_test(NAME cli_sign_add_metadata_roundtrip
    COMMAND vanillapdf-cli sign list "${CMAKE_BINARY_DIR}/test_sign_meta.pdf")
set_tests_properties(cli_sign_add_metadata_roundtrip PROPERTIES
    FIXTURES_REQUIRED SIGNED_META
    PASS_REGULAR_EXPRESSION "Name: Jane Smith")
