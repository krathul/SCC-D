cmake_minimum_required(VERSION 3.5)
project(SCC-D)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -g -Wextra -Wconversion -pedantic -fopenmp")

file(MAKE_DIRECTORY
    ${CMAKE_SOURCE_DIR}/test
    ${CMAKE_SOURCE_DIR}/log
)

find_package(OpenMP REQUIRED)

add_executable(StaticSCC
    scc_main.cpp
    src/graph.cpp
    src/static/static.cpp
    src/static/scc_serial.cpp
)
target_compile_definitions(StaticSCC PUBLIC STATIC_MODE=1)
target_include_directories(StaticSCC PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/static
)
target_link_libraries(StaticSCC gomp)

add_executable(DynamicSCC
    scc_main.cpp
    src/graph.cpp
    src/static/static.cpp
    src/static/scc_serial.cpp
    src/dynamic/dynamic.cpp)
target_include_directories(DynamicSCC PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/static
    ${CMAKE_SOURCE_DIR}/src/dynamic
)
target_link_libraries(DynamicSCC gomp)
