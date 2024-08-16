cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(libws)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")

find_package(Boost 1.74 REQUIRED)

set(SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/websocket-client.cpp"
)

add_library(ws STATIC ${SOURCES})

target_compile_features(ws
    PUBLIC
    cxx_std_14
)

target_include_directories(ws
    PUBLIC
    src
)

target_link_libraries(ws
    PUBLIC
    Boost::boost
)
