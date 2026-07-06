# Unit tests (GoogleTest). Add new unit-test sources to this single executable.
find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
include(GoogleTest)

add_executable(unit_tests
    unit/json_test.cpp
)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main fmt::fmt nlohmann_json::nlohmann_json)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
vanillapdf_cli_configure_msvc_runtime(unit_tests)
gtest_discover_tests(unit_tests)

# CLI integration tests using CTest

# Helpers
add_test(NAME cli_version COMMAND vanillapdf-cli --version)
add_test(NAME cli_help COMMAND vanillapdf-cli --help)

# Informational flags must fire standalone (no subcommand) and print their content.
# The regex guards against the callback silently not running (e.g. if it were
# evaluated after the require_subcommand check instead of on parse).
add_test(NAME cli_copyright COMMAND vanillapdf-cli --copyright)
set_tests_properties(cli_copyright PROPERTIES PASS_REGULAR_EXPRESSION "Apache License")

add_test(NAME cli_about COMMAND vanillapdf-cli --about)
set_tests_properties(cli_about PROPERTIES PASS_REGULAR_EXPRESSION "github.com/sponsors/vanillapdf")

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
add_test(NAME cli_pages_help COMMAND vanillapdf-cli pages --help)
add_test(NAME cli_pages_delete_help COMMAND vanillapdf-cli pages delete --help)
add_test(NAME cli_pages_reorder_help COMMAND vanillapdf-cli pages reorder --help)
add_test(NAME cli_pages_insert_help COMMAND vanillapdf-cli pages insert --help)
add_test(NAME cli_rotate_help COMMAND vanillapdf-cli rotate --help)
add_test(NAME cli_compress_help COMMAND vanillapdf-cli compress --help)
add_test(NAME cli_encrypt_help COMMAND vanillapdf-cli encrypt --help)
add_test(NAME cli_sign_help COMMAND vanillapdf-cli sign --help)

# Nested subcommand help
add_test(NAME cli_encrypt_add_help COMMAND vanillapdf-cli encrypt add --help)
add_test(NAME cli_encrypt_remove_help COMMAND vanillapdf-cli encrypt remove --help)
add_test(NAME cli_sign_add_help COMMAND vanillapdf-cli sign add --help)
add_test(NAME cli_sign_list_help COMMAND vanillapdf-cli sign list --help)
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

# sign list on a freshly-signed document. Guards against a regression where a
# signature lacking optional fields (Name/Reason/...) aborted the whole listing.
add_test(NAME cli_sign_list_setup
    COMMAND vanillapdf-cli sign add "${SIGN_TEST_PDF}"
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
    COMMAND vanillapdf-cli sign add "${SIGN_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_sign_meta.pdf" -k "${SIGN_TEST_KEY}"
            --name "Jane Smith" --reason "Approved" --location "London")
set_tests_properties(cli_sign_add_metadata_setup PROPERTIES FIXTURES_SETUP SIGNED_META)

add_test(NAME cli_sign_add_metadata_roundtrip
    COMMAND vanillapdf-cli sign list "${CMAKE_BINARY_DIR}/test_sign_meta.pdf")
set_tests_properties(cli_sign_add_metadata_roundtrip PROPERTIES
    FIXTURES_REQUIRED SIGNED_META
    PASS_REGULAR_EXPRESSION "Name: Jane Smith")

# Fixture-based encrypt tests
set(ENCRYPT_TEST_PDF "${CMAKE_CURRENT_LIST_DIR}/fixtures/minimal.pdf")

# Invalid key length for the chosen algorithm is rejected before any crypto work
add_test(NAME cli_encrypt_add_invalid_bits
    COMMAND vanillapdf-cli encrypt add "${ENCRYPT_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_badbits.pdf" -p pw --bits 40)
set_tests_properties(cli_encrypt_add_invalid_bits PROPERTIES WILL_FAIL TRUE)

# Unknown permission token is rejected by the parser
add_test(NAME cli_encrypt_add_invalid_permission
    COMMAND vanillapdf-cli encrypt add "${ENCRYPT_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_badperm.pdf" -p pw --allow bogus)
set_tests_properties(cli_encrypt_add_invalid_permission PROPERTIES WILL_FAIL TRUE)

# AES-256 (default) round trip: encrypt, then decrypt with the same password.
# FIXTURES_SETUP/REQUIRED guarantees the decrypt runs after the encrypt.
add_test(NAME cli_encrypt_add_aes256
    COMMAND vanillapdf-cli encrypt add "${ENCRYPT_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_aes256.pdf" -p secret)
set_tests_properties(cli_encrypt_add_aes256 PROPERTIES FIXTURES_SETUP ENCRYPTED_AES256)

add_test(NAME cli_encrypt_remove_aes256
    COMMAND vanillapdf-cli encrypt remove "${CMAKE_BINARY_DIR}/test_encrypt_aes256.pdf"
            -o "${CMAKE_BINARY_DIR}/test_decrypt_aes256.pdf" -p secret)
set_tests_properties(cli_encrypt_remove_aes256 PROPERTIES FIXTURES_REQUIRED ENCRYPTED_AES256)

# Wrong password must fail
add_test(NAME cli_encrypt_remove_wrong_password
    COMMAND vanillapdf-cli encrypt remove "${CMAKE_BINARY_DIR}/test_encrypt_aes256.pdf"
            -o "${CMAKE_BINARY_DIR}/test_decrypt_wrong.pdf" -p wrongpw)
set_tests_properties(cli_encrypt_remove_wrong_password PROPERTIES
    FIXTURES_REQUIRED ENCRYPTED_AES256 WILL_FAIL TRUE)

# AES-128 and RC4 variants
add_test(NAME cli_encrypt_add_aes128
    COMMAND vanillapdf-cli encrypt add "${ENCRYPT_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_aes128.pdf" -p pw --bits 128)

add_test(NAME cli_encrypt_add_rc4
    COMMAND vanillapdf-cli encrypt add "${ENCRYPT_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_rc4.pdf" -p pw --algorithm rc4)

# Access permissions
add_test(NAME cli_encrypt_add_permissions
    COMMAND vanillapdf-cli encrypt add "${ENCRYPT_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_perm.pdf" -p pw --allow print,copy)

# Fixture-based info tests
set(INFO_TEST_PDF "${CMAKE_CURRENT_LIST_DIR}/fixtures/minimal.pdf")

add_test(NAME cli_info_minimal COMMAND vanillapdf-cli info "${INFO_TEST_PDF}")
set_tests_properties(cli_info_minimal PROPERTIES PASS_REGULAR_EXPRESSION "Pages: 1")

add_test(NAME cli_info_minimal_json COMMAND vanillapdf-cli info "${INFO_TEST_PDF}" --json)
set_tests_properties(cli_info_minimal_json PROPERTIES PASS_REGULAR_EXPRESSION "\"pages\": 1")

# Fixture-based rotate test (regression for 1-based page indexing, #31)
set(ROTATE_TEST_PDF "${CMAKE_CURRENT_LIST_DIR}/fixtures/minimal.pdf")

add_test(NAME cli_rotate_all
    COMMAND vanillapdf-cli rotate "${ROTATE_TEST_PDF}" -a 90 -o "${CMAKE_BINARY_DIR}/test_rotate.pdf")
set_tests_properties(cli_rotate_all PROPERTIES FIXTURES_SETUP ROTATE_ALL)
add_test(NAME cli_rotate_all_count COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_rotate.pdf")
set_tests_properties(cli_rotate_all_count PROPERTIES
    FIXTURES_REQUIRED ROTATE_ALL PASS_REGULAR_EXPRESSION "Pages: 1")

# Fixture-based pages tests (3-page fixture)
set(PAGES_TEST_PDF "${CMAKE_CURRENT_LIST_DIR}/fixtures/three_pages.pdf")
set(MINIMAL_TEST_PDF "${CMAKE_CURRENT_LIST_DIR}/fixtures/minimal.pdf")

add_test(NAME cli_pages_no_subcommand COMMAND vanillapdf-cli pages)
set_tests_properties(cli_pages_no_subcommand PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_pages_delete_no_args COMMAND vanillapdf-cli pages delete)
set_tests_properties(cli_pages_delete_no_args PROPERTIES WILL_FAIL TRUE)

# delete: 3 pages, remove page 2 -> 2 pages
add_test(NAME cli_pages_delete
    COMMAND vanillapdf-cli pages delete "${PAGES_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_pages_del.pdf" -p 2)
set_tests_properties(cli_pages_delete PROPERTIES FIXTURES_SETUP PAGES_DEL)
add_test(NAME cli_pages_delete_count
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_pages_del.pdf")
set_tests_properties(cli_pages_delete_count PROPERTIES
    FIXTURES_REQUIRED PAGES_DEL PASS_REGULAR_EXPRESSION "Pages: 2")

# deleting every page must fail
add_test(NAME cli_pages_delete_all
    COMMAND vanillapdf-cli pages delete "${PAGES_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_pages_delall.pdf" -p 1 2 3)
set_tests_properties(cli_pages_delete_all PROPERTIES WILL_FAIL TRUE)

# reorder: full permutation preserves the page count
add_test(NAME cli_pages_reorder
    COMMAND vanillapdf-cli pages reorder "${PAGES_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_pages_reo.pdf" --order 3,1,2)
set_tests_properties(cli_pages_reorder PROPERTIES FIXTURES_SETUP PAGES_REO)
add_test(NAME cli_pages_reorder_count
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_pages_reo.pdf")
set_tests_properties(cli_pages_reorder_count PROPERTIES
    FIXTURES_REQUIRED PAGES_REO PASS_REGULAR_EXPRESSION "Pages: 3")

# reorder with a non-permutation must fail
add_test(NAME cli_pages_reorder_invalid
    COMMAND vanillapdf-cli pages reorder "${PAGES_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_pages_reobad.pdf" --order 1,2)
set_tests_properties(cli_pages_reorder_invalid PROPERTIES WILL_FAIL TRUE)

# insert: 3 pages + 1 inserted page -> 4 pages
add_test(NAME cli_pages_insert
    COMMAND vanillapdf-cli pages insert "${PAGES_TEST_PDF}"
            -o "${CMAKE_BINARY_DIR}/test_pages_ins.pdf" --from "${MINIMAL_TEST_PDF}" --at 2)
set_tests_properties(cli_pages_insert PROPERTIES FIXTURES_SETUP PAGES_INS)
add_test(NAME cli_pages_insert_count
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_pages_ins.pdf")
set_tests_properties(cli_pages_insert_count PROPERTIES
    FIXTURES_REQUIRED PAGES_INS PASS_REGULAR_EXPRESSION "Pages: 4")

# validate
add_test(NAME cli_validate_no_args COMMAND vanillapdf-cli validate)
set_tests_properties(cli_validate_no_args PROPERTIES WILL_FAIL TRUE)

# Fixture-based validate tests
set(VALIDATE_TEST_PDF "${CMAKE_CURRENT_LIST_DIR}/fixtures/minimal.pdf")

add_test(NAME cli_validate_minimal COMMAND vanillapdf-cli validate "${VALIDATE_TEST_PDF}")
add_test(NAME cli_validate_minimal_json COMMAND vanillapdf-cli validate "${VALIDATE_TEST_PDF}" --json)
add_test(NAME cli_validate_minimal_strict COMMAND vanillapdf-cli validate "${VALIDATE_TEST_PDF}" --strict)
