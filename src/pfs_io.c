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

#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico_freertos_shell/init.h>

#include "pfs_autocompletion.h"
#include "pfs_cmd_queue.h"
#include "pfs_escape_sequences.h"
#include "pfs_handle_shell_input.h"
#include "pfs_io.h"
#include "pfs_utils.h"

#ifdef PFS_WITH_COMMAND_HISTORY
#include "pfs_cmd_history.h"
#endif // PFS_WITH_COMMAND_HISTORY

static pfs_input_buffer_t m_input_buffer;

void pfs_io_remove_shell_prompt(void) {
    pfs_io_puts_immediately(PFS_IO_CLEAR_LINE);
}

void pfs_io_restore_shell_prompt(void) {
    pfs_io_puts_immediately(PFS_IO_SHELL_PROMPT);
    pfs_io_puts_immediately(m_input_buffer.buffer);
    size_t difference = m_input_buffer.len - m_input_buffer.cursor;
    for (size_t i = 0; i < difference; i++) {
        pfs_io_puts_immediately(PFS_IO_MOVE_LEFT);
    }
}

static void reset_input_buffer(void) {
    memset(m_input_buffer.buffer, 0, sizeof(m_input_buffer.buffer));
    m_input_buffer.len = 0;
    m_input_buffer.cursor = 0;
}

static void handle_char_ctrl_c(void) {
    pfs_io_puts_immediately("^C\n");
    reset_input_buffer();
    pfs_io_restore_shell_prompt();
}

void PFS_REAL(pfs_io_putchar_immediately)(char c) {
    putchar(c);
}

static void handle_char_backspace(void) {
    if (m_input_buffer.cursor == 0) {
        return;
    }

    m_input_buffer.cursor--;
    for (size_t i = m_input_buffer.cursor; i < m_input_buffer.len; i++) {
        m_input_buffer.buffer[i] = m_input_buffer.buffer[i + 1];
    }
    m_input_buffer.len--;
    pfs_io_remove_shell_prompt();
    pfs_io_restore_shell_prompt();
}

static void handle_char_enter(void) {
    if (pfs_cmd_queue_is_in_handler()) {
        pfs_io_remove_shell_prompt();
        PFS_SHELL_LOG(WRN, "command handler is being executed\n");
        pfs_io_restore_shell_prompt();
        return;
    }
    m_input_buffer.buffer[m_input_buffer.len] = '\0';
    if (!pfs_input_has_only_whitespaces(m_input_buffer.buffer)) {
        pfs_io_putchar_immediately('\n');
    }
    m_input_buffer.cursor = m_input_buffer.len;
#ifdef PFS_WITH_COMMAND_HISTORY
    if (!pfs_input_has_only_whitespaces(m_input_buffer.buffer)) {
        (void) pfs_cmd_history_append(pfs_cmd_history_get_buff_ptr(),
                                      &m_input_buffer);
        size_t *offset = pfs_cmd_history_get_offset_ptr();
        *offset = 0;
    }
#endif // PFS_WITH_COMMAND_HISTORY
    pfs_handle_shell_input(m_input_buffer.buffer);
    if (pfs_input_has_only_whitespaces(m_input_buffer.buffer)) {
        pfs_io_putchar_immediately('\n');
    }
    reset_input_buffer();
    pfs_io_restore_shell_prompt();
}

static void handle_other_characters(char c) {
    if (!isprint(c)) {
        return;
    }
    if (m_input_buffer.len >= PFS_MAX_INPUT_SIZE - 1) {
        return;
    }

    if (m_input_buffer.cursor != m_input_buffer.len) {
        pfs_io_puts_immediately(PFS_IO_ERASE_LINE_RIGHT);
        for (size_t i = m_input_buffer.len; i > m_input_buffer.cursor; i--) {
            m_input_buffer.buffer[i] = m_input_buffer.buffer[i - 1];
        }
    }
    m_input_buffer.buffer[m_input_buffer.cursor] = c;
    m_input_buffer.len++;
    m_input_buffer.cursor++;
    pfs_io_putchar_immediately(c);
    if (m_input_buffer.cursor != m_input_buffer.len) {
        for (size_t i = m_input_buffer.cursor; i < m_input_buffer.len; i++) {
            pfs_io_putchar_immediately(m_input_buffer.buffer[i]);
        }
        size_t difference = m_input_buffer.len - m_input_buffer.cursor;
        for (size_t i = 0; i < difference; i++) {
            pfs_io_puts_immediately(PFS_IO_MOVE_LEFT);
        }
    }
}

void pfs_io_handle_input_char(char c) {
    switch (c) {
    case PFS_IO_CHAR_CTRL_C:
        handle_char_ctrl_c();
        break;
    case PFS_IO_CHAR_CTRL_H:
    case PFS_IO_CHAR_BACKSPACE:
        handle_char_backspace();
        break;
    case PFS_IO_CHAR_ENTER:
    case PFS_IO_CHAR_CTRL_D:
        handle_char_enter();
        break;
    case PFS_IO_CHAR_TAB:
        pfs_autocompletion(&m_input_buffer);
        break;
    case PFS_IO_CHAR_ESACPE:
        pfs_handle_esacpe_sequence(&m_input_buffer);
        break;
    default:
        handle_other_characters(c);
        break;
    }
}

void pfs_io_vprintf_immediately(const char *format, va_list va) {
    // dunno about this buffer size
    const size_t BUFFER_SIZE = 2048;
    char out_buffer[BUFFER_SIZE];
    memset(out_buffer, 0, sizeof(out_buffer));
    int ret = vsnprintf(out_buffer, BUFFER_SIZE, format, va);
    pfs_io_puts_immediately(out_buffer);
}

void __attribute__((format(printf, 1, 2)))
pfs_io_printf_immediately(const char *format, ...) {
    va_list va;
    va_start(va, format);
    pfs_io_vprintf_immediately(format, va);
    va_end(va);
}

void PFS_REAL(pfs_io_puts_immediately)(const char *s) {
    while (*s) {
        putchar(*s);
        s++;
    }
}
