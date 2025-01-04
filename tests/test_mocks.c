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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <pico_freertos_shell/init.h>

#include <pfs_io.h>
#include <pfs_utils.h>
#include <pfs_handle_shell_input.h>

static char m_buffer[4096];

char *utils_get_out_string_immediately_buffer(void) {
    return m_buffer;
}

void utils_reset_out_string_immediately_buffer(void) {
    memset(m_buffer, 0, sizeof(m_buffer));
}

bool _pfs_is_initialized(void) {
    return false;
}

void PFS_WRAPPED(pfs_io_puts_immediately)(const char *s) {
    strcat(m_buffer, s);
}

void PFS_WRAPPED(pfs_io_putchar_immediately)(char c) {
    char buf[2] = {c, '\0'};
    PFS_WRAPPED(pfs_io_puts_immediately)(buf);
}

void pfs_handle_esacpe_sequence(pfs_input_buffer_t *input_buffer) {
    (void) input_buffer;
}

int pfs_cmd_queue_is_in_handler(void) {
    return 0;
}

int pfs_cmd_queue_add(pfs_cmd_args_t *cmd_args) {
    cmd_args->handler(cmd_args->argc, cmd_args->argv);
    return 0;
}
