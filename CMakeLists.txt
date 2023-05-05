cmake_minimum_required(VERSION 3.0)

project(argparse)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -Werror -Wall -Wextra -Wpedantic -Wconversion")

set(INCLUDE include)
set(SRC src)
set(TEST test)

include_directories(${INCLUDE})

set(
    LIB_SOURCES
    ${SRC}/argparse.cpp
)

add_library(argparse_static STATIC ${LIB_SOURCES})
add_library(argparse_shared SHARED ${LIB_SOURCES})
set_target_properties(argparse_static PROPERTIES OUTPUT_NAME argparse)
set_target_properties(argparse_shared PROPERTIES OUTPUT_NAME argparse)

set(
    TEST_SOURCES
    ${LIB_SOURCES}
    ${TEST}/test.cpp
)

add_executable(test ${TEST_SOURCES})
target_link_libraries(test -lgtest -lgtest_main)
