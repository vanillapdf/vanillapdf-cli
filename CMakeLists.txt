cmake_minimum_required(VERSION 3.20)

# Dependency management options — declared before project() so that
# vcpkg_init.cmake can configure the toolchain before compiler detection.
#
# VANILLAPDF_CLI_INTERNAL_VCPKG: When ON (default), vcpkg provides cli11 and
#   vanillapdf's transitive dependencies. If CMAKE_TOOLCHAIN_FILE is not set,
#   vcpkg is bootstrapped automatically. Set to OFF when all dependencies are
#   provided by the system (deb/rpm/brew/snap).
option(VANILLAPDF_CLI_INTERNAL_VCPKG "Enable internal vcpkg dependency management" ON)

# VANILLAPDF_CLI_EXTERNAL_VANILLAPDF: When OFF (default), vanillapdf is built
#   from source via FetchContent at the version in VANILLAPDF_GIT_TAG.
#   Set to ON to use a pre-installed vanillapdf (system packages, brew, etc.).
option(VANILLAPDF_CLI_EXTERNAL_VANILLAPDF "Use vanillapdf from the system or a package manager" OFF)

# VANILLAPDF_CLI_EXTERNAL_CLI11: When OFF (default), CLI11 is provided by vcpkg.
#   Set to ON to use CLI11 from the system or a package manager.
option(VANILLAPDF_CLI_EXTERNAL_CLI11 "Use CLI11 from the system or a package manager" OFF)

include(cmake/vcpkg_init.cmake)

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

# vanillapdf
if(VANILLAPDF_CLI_EXTERNAL_VANILLAPDF)
    find_package(vanillapdf REQUIRED)
else()
    set(VANILLAPDF_GIT_TAG "main" CACHE STRING
        "vanillapdf git tag/branch to build from source via FetchContent")
    include(FetchContent)
    FetchContent_Declare(vanillapdf
        GIT_REPOSITORY https://github.com/vanillapdf/vanillapdf.git
        GIT_TAG        ${VANILLAPDF_GIT_TAG}
        GIT_SUBMODULES ""
    )
    set(VANILLAPDF_INTERNAL_VCPKG OFF)
    FetchContent_MakeAvailable(vanillapdf)
endif()

# CLI11
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
