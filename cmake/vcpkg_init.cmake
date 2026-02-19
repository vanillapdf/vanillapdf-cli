# vanillapdf-cli vcpkg initialization
# Mirrors the pattern used by the vanillapdf native library.

if(NOT VANILLAPDF_CLI_INTERNAL_VCPKG)
    message(STATUS "vanillapdf-cli internal vcpkg disabled via VANILLAPDF_CLI_INTERNAL_VCPKG=OFF")
    return()
endif()

message(STATUS "vanillapdf-cli managing dependencies via internal vcpkg")

# Auto-bootstrap vcpkg via FetchContent when no toolchain file is provided.
# This enables a zero-configuration out-of-the-box build for developers.
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    include(FetchContent)
    FetchContent_Declare(vcpkg
        GIT_REPOSITORY https://github.com/Microsoft/vcpkg.git
        GIT_TAG        2025.12.12
        SOURCE_DIR     "${CMAKE_BINARY_DIR}/vcpkg"
    )
    FetchContent_MakeAvailable(vcpkg)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_BINARY_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
    message(STATUS "Using bootstrapped vcpkg: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# Build the vcpkg feature list based on which dependencies are external.
# Features not listed here will not be installed by vcpkg.
set(VANILLAPDF_CLI_VCPKG_FEATURE_LIST "")

if(NOT VANILLAPDF_CLI_EXTERNAL_CLI11)
    list(APPEND VANILLAPDF_CLI_VCPKG_FEATURE_LIST "cli11")
endif()

# When vanillapdf is built from source via FetchContent (VANILLAPDF_INTERNAL_VCPKG=OFF
# is set on the sub-project), its transitive dependencies must be provided by the
# parent project's vcpkg. Only include them when not using an external vanillapdf.
if(NOT VANILLAPDF_CLI_EXTERNAL_VANILLAPDF)
    list(APPEND VANILLAPDF_CLI_VCPKG_FEATURE_LIST "encryption")
    list(APPEND VANILLAPDF_CLI_VCPKG_FEATURE_LIST "jpeg")
    list(APPEND VANILLAPDF_CLI_VCPKG_FEATURE_LIST "jpeg2000")
    list(APPEND VANILLAPDF_CLI_VCPKG_FEATURE_LIST "zlib")
    list(APPEND VANILLAPDF_CLI_VCPKG_FEATURE_LIST "spdlog")
    list(APPEND VANILLAPDF_CLI_VCPKG_FEATURE_LIST "fmt")
    list(APPEND VANILLAPDF_CLI_VCPKG_FEATURE_LIST "nlohmann-json")
endif()

list(JOIN VANILLAPDF_CLI_VCPKG_FEATURE_LIST ";" VANILLAPDF_CLI_VCPKG_FEATURE_STRING)
set(VCPKG_MANIFEST_FEATURES "${VANILLAPDF_CLI_VCPKG_FEATURE_STRING}" CACHE STRING "" FORCE)
message(STATUS "vcpkg manifest features: ${VCPKG_MANIFEST_FEATURES}")
