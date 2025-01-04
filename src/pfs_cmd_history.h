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

#include <stdbool.h>
#include <stddef.h>

#include "pfs_io.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef PFS_WITH_COMMAND_HISTORY

typedef struct pfs_cmd_history {
    pfs_input_buffer_t *data;
    size_t head;
    size_t capacity;
    size_t occupancy;
} pfs_cmd_history_t;

int pfs_cmd_history_append(pfs_cmd_history_t *cmd_buff,
                           pfs_input_buffer_t *value);
int pfs_cmd_history_get(pfs_cmd_history_t *cmd_buff,
                        size_t offset,
                        pfs_input_buffer_t *out_value);
int pfs_cmd_history_update_newest(pfs_cmd_history_t *cmd_buff,
                                  pfs_input_buffer_t *value);

static inline bool pfs_cmd_history_is_full(pfs_cmd_history_t *cmd_buff) {
    return cmd_buff ? cmd_buff->capacity == cmd_buff->occupancy : true;
}

static inline bool pfs_cmd_history_is_empty(pfs_cmd_history_t *cmd_buff) {
    return cmd_buff ? cmd_buff->occupancy == 0 : true;
}

int pfs_cmd_history_reset(pfs_cmd_history_t *cmd_buff);
size_t *pfs_cmd_history_get_offset_ptr(void);
pfs_cmd_history_t *pfs_cmd_history_get_buff_ptr(void);

#endif // PFS_WITH_COMMAND_HISTORY

#ifdef __cplusplus
}
#endif // __cplusplus
