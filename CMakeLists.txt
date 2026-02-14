cmake_minimum_required(VERSION 3.20)

project(vanillapdf-cli
    VERSION 0.1.0
    DESCRIPTION "PDF manipulation command-line tool"
    HOMEPAGE_URL "https://github.com/vanillapdf/vanillapdf-cli"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(VANILLAPDF_CLI_ENABLE_TESTS "Build tests" ON)

# Dependencies
find_package(vanillapdf REQUIRED)
find_package(CLI11 REQUIRED)

# Executable
add_executable(vanillapdf-cli
    src/main.cpp
    src/commands/info.cpp
    src/commands/merge.cpp
    src/commands/split.cpp
    src/commands/rotate.cpp
    src/commands/compress.cpp
    src/commands/encrypt.cpp
    src/commands/sign.cpp
    src/commands/attachments.cpp
)

target_link_libraries(vanillapdf-cli PRIVATE
    vanillapdf::vanillapdf
    CLI11::CLI11
)

target_include_directories(vanillapdf-cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(vanillapdf-cli PRIVATE
    VANILLAPDF_CLI_VERSION="${PROJECT_VERSION}"
)

set_target_properties(vanillapdf-cli PROPERTIES
    OUTPUT_NAME vanillapdf-cli
)

# Install
include(GNUInstallDirs)

install(TARGETS vanillapdf-cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Packaging
set(CPACK_PACKAGE_NAME "vanillapdf")
set(CPACK_PACKAGE_VENDOR "VanillaPDF")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

# DEB
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libvanillapdf (>= 0.1.0)")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

# RPM
set(CPACK_RPM_PACKAGE_REQUIRES "vanillapdf >= 0.1.0")
set(CPACK_RPM_PACKAGE_LICENSE "BSD")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Text")

include(CPack)

# Tests
if(VANILLAPDF_CLI_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
