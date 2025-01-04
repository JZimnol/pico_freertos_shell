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

cmake_minimum_required(VERSION 3.13)

add_library(pico_freertos_shell_lib)

########################################
# CMake options
########################################
set(PFS_MSG_QUEUE_SIZE 30
    CACHE STRING "Maximum number of messages held on a heap before being printed by the shell task")
set(PFS_MAX_INPUT_SIZE 64
    CACHE STRING "Maximum number of characters in an input command")
option(PFS_WITH_COMMAND_HISTORY
    "Enable command history (handle up and down arrows)" ON)
set(PFS_COMMAND_HISTORY_SIZE 10
    CACHE STRING "Maximum number of commands stored in history")
set(PFS_TERMINAL_TYPE "VT100"
    CACHE STRING "Escape sequences to use for terminal")
set(PFS_TASK_PRIORITY 0
    CACHE STRING "Priority of the shell tasks")
option(PFS_WITH_TESTS
    "Enable tests" OFF)

########################################
# Sources and includes
########################################
if(PFS_WITH_TESTS)
    target_compile_definitions(pico_freertos_shell_lib PUBLIC
                               PFS_WITH_TESTS)
    target_sources(pico_freertos_shell_lib PUBLIC
                   src/pfs_commands.c
                   src/pfs_handle_shell_input.c
                   src/pfs_io.c
                   src/pfs_autocompletion.c)
    target_include_directories(pico_freertos_shell_lib PUBLIC
                               ${CMAKE_CURRENT_SOURCE_DIR}/include
                               ${CMAKE_CURRENT_SOURCE_DIR}/src)

    set(PFS_WITH_COMMAND_HISTORY OFF)
    set(PFS_MAX_INPUT_SIZE 64)

    add_subdirectory(tests)
else()
    target_sources(pico_freertos_shell_lib PUBLIC
                   src/pfs_core.c
                   src/pfs_commands.c)
    target_sources(pico_freertos_shell_lib PRIVATE
                   src/pfs_handle_shell_input.c
                   src/pfs_escape_sequences.c
                   src/pfs_io.c
                   src/pfs_cmd_queue.c
                   src/pfs_autocompletion.c)
    target_include_directories(pico_freertos_shell_lib PUBLIC
                               ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(pico_freertos_shell_lib PUBLIC
                          pico_stdlib
                          pico_printf
                          pico_stdio)
endif()

########################################
# Macros
########################################
macro(pico_freertos_shell_link_freertos FREERTOS_TARGET)
    target_link_libraries(pico_freertos_shell_lib PUBLIC ${FREERTOS_TARGET})
endmacro()

########################################
# Compile definitions
########################################
target_compile_definitions(pico_freertos_shell_lib PUBLIC
                           PFS_MSG_QUEUE_SIZE=${PFS_MSG_QUEUE_SIZE})
target_compile_definitions(pico_freertos_shell_lib PUBLIC
                           PFS_MAX_INPUT_SIZE=${PFS_MAX_INPUT_SIZE})

if (PFS_WITH_COMMAND_HISTORY)
    target_compile_definitions(pico_freertos_shell_lib PRIVATE
                               PFS_WITH_COMMAND_HISTORY
                               PFS_COMMAND_HISTORY_SIZE=${PFS_COMMAND_HISTORY_SIZE})
    target_sources(pico_freertos_shell_lib PRIVATE
                   src/pfs_cmd_history.c)
endif()

if (PFS_TERMINAL_TYPE STREQUAL "VT100")
    target_compile_definitions(pico_freertos_shell_lib PUBLIC
                               PFS_TERMINAL_TYPE_VT100)
    if (NOT PFS_WITH_TESTS)
        target_sources(pico_freertos_shell_lib PRIVATE
                       src/pfs_vt100.c)
    endif()
else()
    message(FATAL_ERROR "Unsupported terminal type: ${PFS_TERMINAL_TYPE}. Supported are: VT100.")
endif()

target_compile_definitions(pico_freertos_shell_lib PUBLIC
                           PFS_TASK_PRIORITY=${PFS_TASK_PRIORITY})
