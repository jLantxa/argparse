#
# MIT License
#
# Copyright (c) 2023 Javier Lancha Vázquez
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

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
