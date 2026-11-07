# Unit tests (GoogleTest). Add new unit-test sources to this single executable.
find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
include(GoogleTest)

add_executable(unit_tests
    unit/json_test.cpp
    unit/page_range_test.cpp
)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main fmt::fmt nlohmann_json::nlohmann_json)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
vanillapdf_cli_configure_msvc_runtime(unit_tests)
gtest_discover_tests(unit_tests)

# CLI integration tests using CTest.
#
# Per-command tests live in cmake/<command>.cmake and are included below. Shared
# fixtures are referenced through CLI_FIXTURES. Cross-file CTest fixture
# dependencies (e.g. metadata's encrypted tests reuse ENCRYPTED_AES256 from
# encrypt.cmake) are resolved by name, so include order does not matter.
set(CLI_FIXTURES "${CMAKE_CURRENT_LIST_DIR}/fixtures")

# Application-level tests (no subcommand)
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

# Per-command tests
include(${CMAKE_CURRENT_LIST_DIR}/cmake/info.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/metadata.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/merge.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/split.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/pages.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/rotate.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/compress.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/encrypt.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/sign.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/validate.cmake)
