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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pico/stdlib.h>

#include "pfs_cmd_history.h"
#include "pfs_escape_sequences.h"
#include "pfs_io.h"

#ifdef PFS_WITH_COMMAND_HISTORY
void pfs_esc_seq_arrow_up(pfs_input_buffer_t *input_buffer) {
    size_t *offset = pfs_cmd_history_get_offset_ptr();
    pfs_cmd_history_t *cmd_buff = pfs_cmd_history_get_buff_ptr();
    if (*offset == 0) {
        (void) pfs_cmd_history_update_newest(cmd_buff, input_buffer);
    }
    if (!pfs_cmd_history_get(cmd_buff, (*offset) + 1, input_buffer)) {
        (*offset)++;
        pfs_io_remove_shell_prompt();
        pfs_io_restore_shell_prompt();
    }
}

void pfs_esc_seq_arrow_down(pfs_input_buffer_t *input_buffer) {
    size_t *offset = pfs_cmd_history_get_offset_ptr();
    pfs_cmd_history_t *cmd_buff = pfs_cmd_history_get_buff_ptr();
    if (*offset == 0) {
        return;
    }
    if (!pfs_cmd_history_get(cmd_buff, (*offset) - 1, input_buffer)) {
        (*offset)--;
        pfs_io_remove_shell_prompt();
        pfs_io_restore_shell_prompt();
    }
}
#endif // PFS_WITH_COMMAND_HISTORY

void pfs_esc_seq_arrow_left(pfs_input_buffer_t *input_buffer) {
    if (input_buffer->cursor == 0) {
        return;
    }
    input_buffer->cursor--;
    pfs_io_puts_immediately(PFS_IO_MOVE_LEFT);
}

void pfs_esc_seq_arrow_right(pfs_input_buffer_t *input_buffer) {
    if (input_buffer->cursor == input_buffer->len) {
        return;
    }
    input_buffer->cursor++;
    pfs_io_puts_immediately(PFS_IO_MOVE_RIGHT);
}

void pfs_esc_seq_delete(pfs_input_buffer_t *input_buffer) {
    if (input_buffer->cursor == input_buffer->len) {
        return;
    }
    for (size_t i = input_buffer->cursor; i < input_buffer->len; i++) {
        input_buffer->buffer[i] = input_buffer->buffer[i + 1];
    }
    input_buffer->len--;
    pfs_io_remove_shell_prompt();
    pfs_io_restore_shell_prompt();
}

void pfs_esc_seq_home(pfs_input_buffer_t *input_buffer) {
    for (size_t i = 0; i < input_buffer->cursor; i++) {
        pfs_io_puts_immediately(PFS_IO_MOVE_LEFT);
    }
    input_buffer->cursor = 0;
}

void pfs_esc_seq_end(pfs_input_buffer_t *input_buffer) {
    for (size_t i = input_buffer->cursor; i < input_buffer->len; i++) {
        pfs_io_putchar_immediately(input_buffer->buffer[i]);
    }
    input_buffer->cursor = input_buffer->len;
}
