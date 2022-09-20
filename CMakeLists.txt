cmake_minimum_required(VERSION 3.24)

project(cpp_init
        LANGUAGES CXX
        VERSION 0.0.1
        )

include(cmake/cmake_helpers.cmake)

add_app(
        APP_NAME cpp_init
        APP_CMAKE_NAMESPACE ci
        APP_OUTPUT_NAME cpp_init
        APP_VERSION ${PROJECT_VERSION}
        APP_PRIVATE_INCLUDE_DIR
            cpp_init
        APP_PRIVATE_SOURCES
            src/generator.cpp
            src/interactive.cpp
            src/main.cpp
#        APP_PRIVATE_LIBRARIES
)

include(cmake/cpack_config.cmake)