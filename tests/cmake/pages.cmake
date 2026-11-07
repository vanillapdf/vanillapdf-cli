# pages command (uses the 3-page fixture)

add_test(NAME cli_pages_help COMMAND vanillapdf-cli pages --help)
add_test(NAME cli_pages_delete_help COMMAND vanillapdf-cli pages delete --help)
add_test(NAME cli_pages_insert_help COMMAND vanillapdf-cli pages insert --help)

add_test(NAME cli_pages_no_subcommand COMMAND vanillapdf-cli pages)
set_tests_properties(cli_pages_no_subcommand PROPERTIES WILL_FAIL TRUE)

add_test(NAME cli_pages_delete_no_args COMMAND vanillapdf-cli pages delete)
set_tests_properties(cli_pages_delete_no_args PROPERTIES WILL_FAIL TRUE)

# delete: 3 pages, remove page 2 -> 2 pages
add_test(NAME cli_pages_delete
    COMMAND vanillapdf-cli pages delete "${CLI_FIXTURES}/three_pages.pdf"
            -o "${CMAKE_BINARY_DIR}/test_pages_del.pdf" -p 2)
set_tests_properties(cli_pages_delete PROPERTIES FIXTURES_SETUP PAGES_DEL)
add_test(NAME cli_pages_delete_count
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_pages_del.pdf")
set_tests_properties(cli_pages_delete_count PROPERTIES
    FIXTURES_REQUIRED PAGES_DEL PASS_REGULAR_EXPRESSION "Pages: 2")

# deleting every page must fail (matched on the message so a parse error can't pass it)
add_test(NAME cli_pages_delete_all
    COMMAND vanillapdf-cli pages delete "${CLI_FIXTURES}/three_pages.pdf"
            -o "${CMAKE_BINARY_DIR}/test_pages_delall.pdf" -p first-last)
set_tests_properties(cli_pages_delete_all PROPERTIES
    PASS_REGULAR_EXPRESSION "Cannot delete all pages")

# delete with an exclusion: "!2" deletes every page except page 2 -> 1 page
add_test(NAME cli_pages_delete_except
    COMMAND vanillapdf-cli pages delete "${CLI_FIXTURES}/three_pages.pdf"
            -o "${CMAKE_BINARY_DIR}/test_pages_delexc.pdf" -p "!2")
set_tests_properties(cli_pages_delete_except PROPERTIES FIXTURES_SETUP PAGES_DELEXC)
add_test(NAME cli_pages_delete_except_count
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_pages_delexc.pdf")
set_tests_properties(cli_pages_delete_except_count PROPERTIES
    FIXTURES_REQUIRED PAGES_DELEXC PASS_REGULAR_EXPRESSION "Pages: 1")

# a page past the end is an error, not a silent skip
add_test(NAME cli_pages_delete_out_of_range
    COMMAND vanillapdf-cli pages delete "${CLI_FIXTURES}/three_pages.pdf"
            -o "${CMAKE_BINARY_DIR}/test_pages_deloor.pdf" -p 4)
set_tests_properties(cli_pages_delete_out_of_range PROPERTIES WILL_FAIL TRUE)

# insert: 3 pages + 1 inserted page -> 4 pages
add_test(NAME cli_pages_insert
    COMMAND vanillapdf-cli pages insert "${CLI_FIXTURES}/three_pages.pdf"
            -o "${CMAKE_BINARY_DIR}/test_pages_ins.pdf" --from "${CLI_FIXTURES}/minimal.pdf" --at 2)
set_tests_properties(cli_pages_insert PROPERTIES FIXTURES_SETUP PAGES_INS)
add_test(NAME cli_pages_insert_count
    COMMAND vanillapdf-cli info "${CMAKE_BINARY_DIR}/test_pages_ins.pdf")
set_tests_properties(cli_pages_insert_count PROPERTIES
    FIXTURES_REQUIRED PAGES_INS PASS_REGULAR_EXPRESSION "Pages: 4")
