cmake_minimum_required(VERSION 3.20)

# Dependency management options — declared before project() so that
# vcpkg_init.cmake can configure the toolchain before compiler detection.
#
# VANILLAPDF_CLI_INTERNAL_VCPKG: When ON (default), vcpkg provides cli11 and
#   vanillapdf's transitive dependencies. If CMAKE_TOOLCHAIN_FILE is not set,
#   vcpkg is bootstrapped automatically. Set to OFF when all dependencies are
#   provided by the system (deb/rpm/brew/snap).
option(VANILLAPDF_CLI_INTERNAL_VCPKG "Enable internal vcpkg dependency management" ON)

# VANILLAPDF_CLI_EXTERNAL_CLI11: When OFF (default), CLI11 is provided by vcpkg.
#   Set to ON to use CLI11 from the system or a package manager.
option(VANILLAPDF_CLI_EXTERNAL_CLI11 "Use CLI11 from the system or a package manager" OFF)

# VANILLAPDF_CLI_EXTERNAL_FMT: When OFF (default), fmt is provided by vcpkg.
#   Set to ON to use fmt from the system or a package manager.
option(VANILLAPDF_CLI_EXTERNAL_FMT "Use fmt from the system or a package manager" OFF)

# Off by default: a plain source/package build needs neither the tests nor
# their GoogleTest dependency. CI and the dev presets turn it on. Declared
# before vcpkg_init so GoogleTest is only requested from vcpkg when enabled.
option(VANILLAPDF_CLI_ENABLE_TESTS "Build tests" OFF)

include(cmake/vcpkg_init.cmake)

project(vanillapdf-cli
    VERSION 0.1.0
    DESCRIPTION "Command-line toolkit for PDF processing, validation and digital signatures"
    HOMEPAGE_URL "https://github.com/vanillapdf/vanillapdf-cli"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Match a target's MSVC runtime (CRT) to the dependencies. vanillapdf configures
# its own targets from VANILLAPDF_USE_STATIC_CRT; apply the same choice to our
# targets so a static build (/MT) does not mismatch the static-CRT dependencies
# with LNK2038. Mirrors vanillapdf's vanillapdf_configure_msvc_runtime().
function(vanillapdf_cli_configure_msvc_runtime target)
    if(MSVC AND VANILLAPDF_USE_STATIC_CRT)
        set_target_properties(${target} PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
    endif()
endfunction()

# vanillapdf
include(cmake/vanillapdf.cmake)

# CLI11
find_package(CLI11 REQUIRED)

# fmt
find_package(fmt CONFIG REQUIRED)

# Executable
add_executable(vanillapdf-cli
    src/main.cpp
    src/commands/commands.h
    src/commands/info.cpp
    src/commands/metadata.cpp
    src/commands/merge.cpp
    src/commands/split.cpp
    src/commands/pages.cpp
    src/commands/rotate.cpp
    src/commands/compress.cpp
    src/commands/encrypt.cpp
    src/commands/sign.cpp
    src/commands/validate.cpp
    src/utils/encoding.h
    src/utils/encoding.cpp
    src/utils/error.h
    src/utils/error.cpp
    src/utils/handle_guard.h
    src/utils/json.h
    src/utils/json.cpp
    src/utils/page_range.h
    src/utils/page_range.cpp
)

target_link_libraries(vanillapdf-cli PRIVATE
    vanillapdf::vanillapdf
    CLI11::CLI11
    fmt::fmt
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

vanillapdf_cli_configure_msvc_runtime(vanillapdf-cli)

# On Windows with a shared vanillapdf build, copy the DLL and its runtime
# dependencies (OpenSSL, zlib, etc.) alongside the executable so tests and
# direct invocation work without adding the vanillapdf output directory to PATH.
# Not needed for static builds (BUILD_SHARED_LIBS=OFF) or system installs.
if(WIN32 AND NOT VANILLAPDF_CLI_EXTERNAL_VANILLAPDF AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET vanillapdf-cli POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_FILE_DIR:vanillapdf::vanillapdf>"
            "$<TARGET_FILE_DIR:vanillapdf-cli>"
        COMMENT "Copying vanillapdf runtime dependencies"
    )
endif()

# Install
include(GNUInstallDirs)

install(TARGETS vanillapdf-cli
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES LICENSE NOTICE.md README.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

# Packaging
set(CPACK_PACKAGE_NAME "vanillapdf")
set(CPACK_PACKAGE_VENDOR "VanillaPDF")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

# Emit a <archive>.sha256 next to each package (replaces manual sha256sum).
set(CPACK_PACKAGE_CHECKSUM SHA256)

# DEB
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libvanillapdf (>= 0.1.0), libfmt10")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

# RPM
set(CPACK_RPM_PACKAGE_REQUIRES "vanillapdf >= 0.1.0, fmt")
set(CPACK_RPM_PACKAGE_LICENSE "Apache-2.0")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Text")

include(CPack)

# Tests
if(VANILLAPDF_CLI_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
