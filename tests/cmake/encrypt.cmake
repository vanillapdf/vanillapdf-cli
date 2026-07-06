# encrypt command

add_test(NAME cli_encrypt_help COMMAND vanillapdf-cli encrypt --help)
add_test(NAME cli_encrypt_add_help COMMAND vanillapdf-cli encrypt add --help)
add_test(NAME cli_encrypt_remove_help COMMAND vanillapdf-cli encrypt remove --help)

add_test(NAME cli_encrypt_no_subcommand COMMAND vanillapdf-cli encrypt)
set_tests_properties(cli_encrypt_no_subcommand PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_encrypt_add_no_args COMMAND vanillapdf-cli encrypt add)
set_tests_properties(cli_encrypt_add_no_args PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_encrypt_remove_no_args COMMAND vanillapdf-cli encrypt remove)
set_tests_properties(cli_encrypt_remove_no_args PROPERTIES WILL_FAIL TRUE)

# Invalid key length for the chosen algorithm is rejected before any crypto work
add_test(NAME cli_encrypt_add_invalid_bits
    COMMAND vanillapdf-cli encrypt add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_badbits.pdf" -p pw --bits 40)
set_tests_properties(cli_encrypt_add_invalid_bits PROPERTIES WILL_FAIL TRUE)

# Unknown permission token is rejected by the parser
add_test(NAME cli_encrypt_add_invalid_permission
    COMMAND vanillapdf-cli encrypt add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_badperm.pdf" -p pw --allow bogus)
set_tests_properties(cli_encrypt_add_invalid_permission PROPERTIES WILL_FAIL TRUE)

# AES-256 (default) round trip: encrypt, then decrypt with the same password.
# FIXTURES_SETUP/REQUIRED guarantees the decrypt runs after the encrypt.
add_test(NAME cli_encrypt_add_aes256
    COMMAND vanillapdf-cli encrypt add "${CLI_FIXTURES}/minimal.pdf"
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
    COMMAND vanillapdf-cli encrypt add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_aes128.pdf" -p pw --bits 128)

add_test(NAME cli_encrypt_add_rc4
    COMMAND vanillapdf-cli encrypt add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_rc4.pdf" -p pw --algorithm rc4)

# Access permissions
add_test(NAME cli_encrypt_add_permissions
    COMMAND vanillapdf-cli encrypt add "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_encrypt_perm.pdf" -p pw --allow print,copy)
