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

#include <stddef.h>
#include <string.h>

#include "pfs_cmd_history.h"
#include "pfs_io.h"
#include "pfs_utils.h"

#ifdef PFS_WITH_COMMAND_HISTORY

static pfs_input_buffer_t m_cmd_history_buffers[PFS_COMMAND_HISTORY_SIZE + 1];
static pfs_cmd_history_t m_cmd_history_buff = {
        .data = m_cmd_history_buffers,
        .head = 0,
        .capacity = PFS_ARRAY_SIZE(m_cmd_history_buffers),
        .occupancy = 0,
};
static size_t m_cmd_history_offset;

size_t *pfs_cmd_history_get_offset_ptr(void) {
    return &m_cmd_history_offset;
}

pfs_cmd_history_t *pfs_cmd_history_get_buff_ptr(void) {
    return &m_cmd_history_buff;
}

int pfs_cmd_history_append(pfs_cmd_history_t *cmd_buff,
                           pfs_input_buffer_t *value) {
    if (!cmd_buff) {
        return 1;
    }

    size_t index = (cmd_buff->head + cmd_buff->occupancy) % cmd_buff->capacity;

    memcpy(&cmd_buff->data[index], value, sizeof(pfs_input_buffer_t));
    if (pfs_cmd_history_is_full(cmd_buff)) {
        cmd_buff->head++;
        cmd_buff->head %= cmd_buff->capacity;
    } else {
        cmd_buff->occupancy++;
    }
    return 0;
}

int pfs_cmd_history_get(pfs_cmd_history_t *cmd_buff,
                        size_t offset,
                        pfs_input_buffer_t *out_value) {
    if (!(cmd_buff && out_value)) {
        return 1;
    }
    if (pfs_cmd_history_is_empty(cmd_buff)) {
        return 1;
    }

    if (!pfs_cmd_history_is_full(cmd_buff)) {
        if (cmd_buff->occupancy < offset) {
            return 1;
        }
    } else {
        if (cmd_buff->occupancy <= offset) {
            return 1;
        }
    }

    size_t index = (cmd_buff->head + cmd_buff->occupancy - offset)
                   % cmd_buff->capacity;

    // get by copy so that the original data is not modified
    memcpy(out_value, &cmd_buff->data[index], sizeof(pfs_input_buffer_t));

    return 0;
}

int pfs_cmd_history_update_newest(pfs_cmd_history_t *cmd_buff,
                                  pfs_input_buffer_t *value) {
    if (!cmd_buff) {
        return 1;
    }

    size_t index = (cmd_buff->head + cmd_buff->occupancy) % cmd_buff->capacity;

    memcpy(&cmd_buff->data[index], value, sizeof(pfs_input_buffer_t));

    return 0;
}

int pfs_cmd_history_reset(pfs_cmd_history_t *cmd_buff) {
    if (!cmd_buff) {
        return 1;
    }
    cmd_buff->head = 0;
    cmd_buff->occupancy = 0;

    return 0;
}

#endif // PFS_WITH_COMMAND_HISTORY
