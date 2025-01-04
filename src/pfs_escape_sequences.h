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

#include "pfs_io.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void (*pfs_escape_sequence_handler_t)(pfs_input_buffer_t *input_buffer);

typedef struct {
    const char *str;
    pfs_escape_sequence_handler_t handler;
} pfs_escape_sequence_t;

void pfs_handle_esacpe_sequence(pfs_input_buffer_t *input_buffer);

#ifdef PFS_WITH_COMMAND_HISTORY
void pfs_esc_seq_arrow_up(pfs_input_buffer_t *input_buffer);
void pfs_esc_seq_arrow_down(pfs_input_buffer_t *input_buffer);
#endif // PFS_WITH_COMMAND_HISTORY
void pfs_esc_seq_arrow_left(pfs_input_buffer_t *input_buffer);
void pfs_esc_seq_arrow_right(pfs_input_buffer_t *input_buffer);
void pfs_esc_seq_delete(pfs_input_buffer_t *input_buffer);
void pfs_esc_seq_home(pfs_input_buffer_t *input_buffer);
void pfs_esc_seq_end(pfs_input_buffer_t *input_buffer);

#ifdef __cplusplus
}
#endif // __cplusplus
