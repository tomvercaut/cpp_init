include(FetchContent)

FetchContent_Declare(
        ext_cmake_helpers
        GIT_REPOSITORY https://github.com/tomvercaut/cmake_helpers.git
        GIT_TAG main
)
FetchContent_MakeAvailable(ext_cmake_helpers)
