# Copyright (c) 2025 Jakub Zimnol
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

set(CMAKE_BUILD_TYPE Debug)

include(CTest)

add_custom_target(check COMMAND make && ${CMAKE_CTEST_COMMAND} || make check_failed)

# unity
add_library(Unity STATIC
            deps/Unity/src/unity.c)
target_include_directories(Unity PUBLIC
                           deps/Unity/src)

# suites
function(pfs_unit_test_add SuiteName)
    add_executable(${SuiteName} ${ARGN} ${CMAKE_CURRENT_SOURCE_DIR}/test_mocks.c)
    target_link_libraries(${SuiteName} PRIVATE
                          Unity
                          pico_freertos_shell_lib)
    target_include_directories(${SuiteName} PRIVATE
                               ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME ${SuiteName}
             COMMAND ${SuiteName})
    target_link_options(pico_freertos_shell_lib INTERFACE
                        "LINKER:--wrap=pfs_io_puts_immediately")
    target_link_options(pico_freertos_shell_lib INTERFACE
                        "LINKER:--wrap=pfs_io_putchar_immediately")
endfunction()

# prepare test suites
file(GLOB_RECURSE TEST_SUITE_FILES RELATIVE
     ${CMAKE_CURRENT_SOURCE_DIR}/suites
     "suites/*_unit_test.c")
set(TEST_SUITE_LIST "")
foreach(SuiteFile ${TEST_SUITE_FILES})
    string(REGEX REPLACE "\.c$" "" SUITE_NAME ${SuiteFile})
    list(APPEND TEST_SUITE_LIST ${SUITE_NAME})
    pfs_unit_test_add(${SUITE_NAME} suites/${SuiteFile})
endforeach()

message(STATUS "Test suites: ${TEST_SUITE_LIST}")
