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

#include <pico_freertos_shell/commands.h>

#include "pfs_io.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// assume maximum number of arguments
#define PFS_MAX_ARGC (PFS_MAX_INPUT_SIZE / 5)

typedef struct {
    pfs_command_handler_t handler;
    int argc;
    char **argv;
} pfs_cmd_args_t;

void pfs_set_commands(const pfs_command_t *commands, size_t number_of_commands);
size_t pfs_get_number_of_commands(void);
const pfs_command_t *pfs_get_commands(void);
int pfs_handle_shell_input(const char *input);
bool pfs_input_has_only_whitespaces(const char *input);
int pfs_custom_tokenizer(char *input,
                         char *argv[],
                         int max_argc,
                         int *out_argc);
const pfs_command_t *
pfs_get_core_commands_sorted_by_name_dynamic(size_t *out_number_of_commands);
const pfs_command_t *
pfs_get_subcommands_sorted_by_name_dynamic(const pfs_command_t *subcommands,
                                           size_t number_of_subcommands);
int pfs_command_has_subcommands(const pfs_command_t *command);
const pfs_command_t *pfs_find_command_by_name(const pfs_command_t *subcommands,
                                              const char *name,
                                              size_t number_of_subcommands);

#ifdef __cplusplus
}
#endif // __cplusplus
