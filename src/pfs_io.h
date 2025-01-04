/*
 * Copyright (c) 2025 Jakub Zimnol
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <inttypes.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef PFS_TERMINAL_TYPE_VT100
#include "pfs_vt100.h"
#else // PFS_TERMINAL_TYPE_VT100
#error "Unknown terminal type defined"
#endif // PFS_TERMINAL_TYPE_VT100

#ifndef PFS_IO_MOVE_LEFT
#error "PFS_IO_MOVE_LEFT escape sequence not defined, please define it in specified terminal header"
#endif // PFS_IO_MOVE_LEFT

#ifndef PFS_IO_MOVE_RIGHT
#error "PFS_IO_MOVE_RIGHT escape sequence not defined, please define it in specified terminal header"
#endif // PFS_IO_MOVE_RIGHT

#ifndef PFS_IO_ERASE_LINE_RIGHT
#error "PFS_IO_ERASE_LINE_RIGHT escape sequence not defined, please define it in specified terminal header"
#endif // PFS_IO_ERASE_LINE_RIGHT

#ifndef PFS_IO_CLEAR_LINE
#error "PFS_IO_CLEAR_LINE escape sequence not defined, please define it in specified terminal header"
#endif // PFS_IO_CLEAR_LINE

#ifndef PFS_IO_PROMPT_COLOR
#define PFS_IO_PROMPT_COLOR
#endif // PFS_IO_PROMPT_COLOR

#ifndef PFS_IO_INF_COLOR
#define PFS_IO_INF_COLOR
#endif // PFS_IO_INF_COLOR

#ifndef PFS_IO_WRN_COLOR
#define PFS_IO_WRN_COLOR
#endif // PFS_IO_WRN_COLOR

#ifndef PFS_IO_ERR_COLOR
#define PFS_IO_ERR_COLOR
#endif // PFS_IO_ERR_COLOR

#ifndef PFS_IO_COLOR_RESET
#define PFS_IO_COLOR_RESET
#endif // PFS_IO_COLOR_RESET

#ifndef PFS_IO_BOLD_ON
#define PFS_IO_BOLD_ON
#endif // PFS_IO_BOLD_ON

#ifndef PFS_IO_BOLD_OFF
#define PFS_IO_BOLD_OFF
#endif // PFS_IO_BOLD_OFF

#define PFS_IO_SHELL_PROMPT            \
    PFS_IO_PROMPT_COLOR PFS_IO_BOLD_ON \
            "shell:~$ " PFS_IO_BOLD_OFF PFS_IO_COLOR_RESET

// useful in unit tests
#define PFS_IO_SHELL_MESSAGE_STR "[SHELL] "
#define PFS_IO_SHELL_MESSAGE_INF_BEGIN                                       \
    PFS_IO_INF_COLOR PFS_IO_BOLD_ON PFS_IO_SHELL_MESSAGE_STR PFS_IO_BOLD_OFF \
            PFS_IO_COLOR_RESET
#define PFS_IO_SHELL_MESSAGE_WRN_BEGIN                                       \
    PFS_IO_WRN_COLOR PFS_IO_BOLD_ON PFS_IO_SHELL_MESSAGE_STR PFS_IO_BOLD_OFF \
            PFS_IO_COLOR_RESET
#define PFS_IO_SHELL_MESSAGE_ERR_BEGIN                                       \
    PFS_IO_ERR_COLOR PFS_IO_BOLD_ON PFS_IO_SHELL_MESSAGE_STR PFS_IO_BOLD_OFF \
            PFS_IO_COLOR_RESET

#define PFS_IO_CHAR_CTRL_C 0x03
#define PFS_IO_CHAR_CTRL_D 0x04
#define PFS_IO_CHAR_BACKSPACE 0x08
#define PFS_IO_CHAR_TAB 0x09
#define PFS_IO_CHAR_ENTER 0x0D
#define PFS_IO_CHAR_ESACPE 0x1B
#define PFS_IO_CHAR_CTRL_H 0x7f

#define PFS_IO_TAB "    "

typedef struct {
    char buffer[PFS_MAX_INPUT_SIZE];
    uint16_t len;
    uint16_t cursor;
} pfs_input_buffer_t;

void pfs_io_handle_input_char(char c);
void __attribute__((format(printf, 1, 2)))
pfs_io_printf_immediately(const char *format, ...);
void pfs_io_vprintf_immediately(const char *format, va_list va);
void pfs_io_puts_immediately(const char *s);
void pfs_io_putchar_immediately(char c);

void pfs_io_remove_shell_prompt(void);
void pfs_io_restore_shell_prompt(void);

#define PFS_SHELL_LOG(Level, ...) \
    pfs_io_printf_immediately(PFS_IO_SHELL_MESSAGE_##Level##_BEGIN __VA_ARGS__)

#ifdef __cplusplus
}
#endif // __cplusplus
