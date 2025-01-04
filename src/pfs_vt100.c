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

#include "pfs_escape_sequences.h"
#include "pfs_io.h"
#include "pfs_utils.h"

#ifdef PFS_TERMINAL_TYPE_VT100
static char m_buffer[sizeof("[[<v>;<h>H")];

static const pfs_escape_sequence_t const ESCAPE_SEQUENCES[] = {
        {.str = "[D", .handler = pfs_esc_seq_arrow_left},
        {.str = "[C", .handler = pfs_esc_seq_arrow_right},
#ifdef PFS_WITH_COMMAND_HISTORY
        {.str = "[B", .handler = pfs_esc_seq_arrow_down},
        {.str = "[A", .handler = pfs_esc_seq_arrow_up},
#endif // PFS_WITH_COMMAND_HISTORY
        {.str = "[3~", .handler = pfs_esc_seq_delete},
        {.str = "[1~", .handler = pfs_esc_seq_home},
        {.str = "[H", .handler = pfs_esc_seq_home},
        {.str = "OF", .handler = pfs_esc_seq_end},
        {.str = "[F", .handler = pfs_esc_seq_end},
};

static int find_longest_escape_sequence(void) {
    static int m_longest_escape_sequence = -1;
    if (m_longest_escape_sequence >= 0) {
        return m_longest_escape_sequence;
    }
    int longest = 0;
    for (size_t i = 0; i < PFS_ARRAY_SIZE(ESCAPE_SEQUENCES); i++) {
        int len = strlen(ESCAPE_SEQUENCES[i].str);
        if (len > longest) {
            longest = len;
        }
    }

    return longest;
}

void pfs_handle_esacpe_sequence(pfs_input_buffer_t *input_buffer) {
    memset(m_buffer, 0, sizeof(m_buffer));
    int longest = find_longest_escape_sequence();

    for (int i = 0; i < longest; i++) {
        // assume the whole sequence is sent at once (should be...)
        char c = getchar_timeout_us(1);
        if (c < 0 || c > 127) {
            break;
        }
        m_buffer[i] = c;
    }

    bool found = false;

    size_t input_len = strlen(m_buffer);

    for (size_t i = 0; i < PFS_ARRAY_SIZE(ESCAPE_SEQUENCES); i++) {
        size_t len = strlen(ESCAPE_SEQUENCES[i].str);
        bool differ = false;
        for (size_t j = 0; j < len; j++) {
            if (m_buffer[j] != ESCAPE_SEQUENCES[i].str[j]) {
                differ = true;
                break;
            }
        }
        if (differ) {
            continue;
        }
        found = true;
        ESCAPE_SEQUENCES[i].handler(input_buffer);
        for (size_t j = strlen(ESCAPE_SEQUENCES[i].str); j < input_len; j++) {
            pfs_io_handle_input_char(m_buffer[j]);
        }
        break;
    }

    if (found) {
        return;
    }

    if (input_len == 0) {
        return;
    }

    pfs_io_handle_input_char('\\');
    pfs_io_handle_input_char('0');
    pfs_io_handle_input_char('3');
    pfs_io_handle_input_char('3');
    for (size_t i = 0; i < input_len; i++) {
        pfs_io_handle_input_char(m_buffer[i]);
    }
}

#endif // PFS_TERMINAL_TYPE_VT100
