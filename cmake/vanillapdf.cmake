# vanillapdf dependency.
#
# VANILLAPDF_CLI_EXTERNAL_VANILLAPDF=ON  — use a pre-installed vanillapdf
#   (system packages, brew, etc.). find_package must succeed.
# VANILLAPDF_CLI_EXTERNAL_VANILLAPDF=OFF — build vanillapdf from source via
#   FetchContent at VANILLAPDF_GIT_TAG. Update GIT_TAG when bumping the version.
option(VANILLAPDF_CLI_EXTERNAL_VANILLAPDF
    "Use vanillapdf from the system or a package manager" OFF)

set(VANILLAPDF_GIT_TAG "v2.3.0-beta.1" CACHE STRING
    "vanillapdf git tag/branch to build from source via FetchContent")

if(VANILLAPDF_CLI_EXTERNAL_VANILLAPDF)
    find_package(vanillapdf REQUIRED)
else()
    # Use the parent project's vcpkg for vanillapdf's transitive dependencies.
    set(VANILLAPDF_INTERNAL_VCPKG OFF)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "Build with -fPIC" FORCE)

    include(FetchContent)
    FetchContent_Declare(vanillapdf
        GIT_REPOSITORY https://github.com/vanillapdf/vanillapdf.git
        GIT_TAG        ${VANILLAPDF_GIT_TAG}
        GIT_SUBMODULES ""
    )
    FetchContent_MakeAvailable(vanillapdf)
endif()
