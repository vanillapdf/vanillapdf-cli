# metadata command

add_test(NAME cli_metadata_help COMMAND vanillapdf-cli metadata --help)
add_test(NAME cli_metadata_set_help COMMAND vanillapdf-cli metadata set --help)

add_test(NAME cli_metadata_no_subcommand COMMAND vanillapdf-cli metadata)
set_tests_properties(cli_metadata_no_subcommand PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_metadata_set_no_args COMMAND vanillapdf-cli metadata set)
set_tests_properties(cli_metadata_set_no_args PROPERTIES WILL_FAIL TRUE)

# Set fields on a document that has no Info dictionary (exercises the create path).
add_test(NAME cli_metadata_set
    COMMAND vanillapdf-cli metadata set "${CLI_FIXTURES}/minimal.pdf"
            -o "${CMAKE_BINARY_DIR}/test_metadata_set.pdf"
            --title "Test Title" --author "Test Author")
set_tests_properties(cli_metadata_set PROPERTIES FIXTURES_SETUP METADATA_SET)

add_test(NAME cli_metadata_set_title
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_metadata_set.pdf")
set_tests_properties(cli_metadata_set_title PROPERTIES
    FIXTURES_REQUIRED METADATA_SET PASS_REGULAR_EXPRESSION "Title: Test Title")

add_test(NAME cli_metadata_set_author
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_metadata_set.pdf")
set_tests_properties(cli_metadata_set_author PROPERTIES
    FIXTURES_REQUIRED METADATA_SET PASS_REGULAR_EXPRESSION "Author: Test Author")

# Producer is always stamped by the tool, regardless of user options.
add_test(NAME cli_metadata_set_producer
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_metadata_set.pdf")
set_tests_properties(cli_metadata_set_producer PROPERTIES
    FIXTURES_REQUIRED METADATA_SET PASS_REGULAR_EXPRESSION "Producer: vanillapdf-cli")

# Setting a field to an empty string removes it; untouched fields are preserved.
add_test(NAME cli_metadata_remove_title
    COMMAND vanillapdf-cli metadata set "${CMAKE_BINARY_DIR}/test_metadata_set.pdf"
            -o "${CMAKE_BINARY_DIR}/test_metadata_removed.pdf" --title "")
set_tests_properties(cli_metadata_remove_title PROPERTIES
    FIXTURES_REQUIRED METADATA_SET FIXTURES_SETUP METADATA_REMOVED)

add_test(NAME cli_metadata_removed_title_gone
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_metadata_removed.pdf")
set_tests_properties(cli_metadata_removed_title_gone PROPERTIES
    FIXTURES_REQUIRED METADATA_REMOVED PASS_REGULAR_EXPRESSION "Title: N/A")

add_test(NAME cli_metadata_removed_author_kept
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_metadata_removed.pdf")
set_tests_properties(cli_metadata_removed_author_kept PROPERTIES
    FIXTURES_REQUIRED METADATA_REMOVED PASS_REGULAR_EXPRESSION "Author: Test Author")

# Encrypted round trip: set metadata on an encrypted document (reusing the AES-256
# fixture, password "secret"), then decrypt and read the value back. Confirms the
# inserted strings are re-encrypted on save.
add_test(NAME cli_metadata_set_encrypted
    COMMAND vanillapdf-cli metadata set "${CMAKE_BINARY_DIR}/test_encrypt_aes256.pdf"
            -o "${CMAKE_BINARY_DIR}/test_metadata_enc.pdf" -p secret --title "Enc Title")
set_tests_properties(cli_metadata_set_encrypted PROPERTIES
    FIXTURES_REQUIRED ENCRYPTED_AES256 FIXTURES_SETUP METADATA_ENC)

add_test(NAME cli_metadata_encrypted_decrypt
    COMMAND vanillapdf-cli encrypt remove "${CMAKE_BINARY_DIR}/test_metadata_enc.pdf"
            -o "${CMAKE_BINARY_DIR}/test_metadata_enc_dec.pdf" -p secret)
set_tests_properties(cli_metadata_encrypted_decrypt PROPERTIES
    FIXTURES_REQUIRED METADATA_ENC FIXTURES_SETUP METADATA_ENC_DEC)

add_test(NAME cli_metadata_encrypted_title
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_metadata_enc_dec.pdf")
set_tests_properties(cli_metadata_encrypted_title PROPERTIES
    FIXTURES_REQUIRED METADATA_ENC_DEC PASS_REGULAR_EXPRESSION "Title: Enc Title")

# Wrong password on an encrypted document must fail before writing.
add_test(NAME cli_metadata_set_encrypted_wrong_password
    COMMAND vanillapdf-cli metadata set "${CMAKE_BINARY_DIR}/test_encrypt_aes256.pdf"
            -o "${CMAKE_BINARY_DIR}/test_metadata_enc_wrong.pdf" -p wrongpw --title "Nope")
set_tests_properties(cli_metadata_set_encrypted_wrong_password PROPERTIES
    FIXTURES_REQUIRED ENCRYPTED_AES256 WILL_FAIL TRUE)
