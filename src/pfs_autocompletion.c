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

#include "pfs_autocompletion.h"
#include "pfs_escape_sequences.h"
#include "pfs_handle_shell_input.h"
#include "pfs_io.h"

static size_t find_smallest_command_name_len(const pfs_command_t *commands,
                                             size_t number_of_commands) {
    if (number_of_commands == 0) {
        return 0;
    }
    size_t smallest_len = strlen(commands[0].description.name);
    for (size_t i = 1; i < number_of_commands; i++) {
        if (strlen(commands[i].description.name) < smallest_len) {
            smallest_len = strlen(commands[i].description.name);
        }
    }

    return smallest_len;
}

static const pfs_command_t *
find_commands_by_substring_dynamic(const pfs_command_t *commands,
                                   size_t number_of_commands,
                                   const char *substring,
                                   size_t *out_len) {
    size_t len = 0;
    for (size_t i = 0; i < number_of_commands; i++) {
        if (strstr(commands[i].description.name, substring)
            == commands[i].description.name) {
            len++;
        }
    }

    if (len == 0) {
        *out_len = 0;
        return NULL;
    }

    pfs_command_t *filtered_commands =
            (pfs_command_t *) malloc(len * sizeof(pfs_command_t));
    if (filtered_commands == NULL) {
        *out_len = 0;
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; i < number_of_commands; i++) {
        if (strstr(commands[i].description.name, substring)
            == commands[i].description.name) {
            memcpy(&filtered_commands[j], &commands[i], sizeof(pfs_command_t));
            j++;
        }
    }

    *out_len = len;
    return filtered_commands;
}

static int input_buffer_ends_with_space(pfs_input_buffer_t *input_buffer) {
    if (input_buffer->len == 0) {
        return 0;
    }

    return input_buffer->buffer[input_buffer->len - 1] == ' ';
}

static void find_commands_common_string(const pfs_command_t *commands,
                                        size_t number_of_commands,
                                        char *out_buffer) {
    memset(out_buffer, 0, PFS_MAX_INPUT_SIZE);
    if (number_of_commands == 0 || commands == NULL) {
        return;
    }
    size_t shortest_substring_len =
            find_smallest_command_name_len(commands, number_of_commands);
    bool should_break = false;
    size_t smallest_len = 0;
    for (size_t i = 0; i < shortest_substring_len; i++) {
        for (size_t j = 0; j < number_of_commands; j++) {
            if (commands[j].description.name[i]
                != commands[0].description.name[i]) {
                should_break = true;
                break;
            }
        }
        if (should_break) {
            break;
        }
        smallest_len++;
    }
    memcpy(out_buffer, commands[0].description.name, smallest_len);
}

static void print_command_name_and_space(const pfs_command_t *command,
                                         int argc,
                                         char *argv[],
                                         bool ends_with_space,
                                         pfs_input_buffer_t *input_buffer) {
    int beginning = (argc == 0 || ends_with_space) ? 0 : strlen(argv[argc - 1]);
    for (size_t i = beginning; i < strlen(command->description.name); i++) {
        pfs_io_handle_input_char(command->description.name[i]);
    }
    pfs_io_handle_input_char(' ');
}

static void print_all_commands(const pfs_command_t *commands,
                               size_t number_of_commands) {
    if (commands == NULL || number_of_commands == 0) {
        return;
    }
    pfs_io_printf_immediately(
            "\n" PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_BOLD_ON);
    for (size_t i = 0; i < number_of_commands; i++) {
        pfs_io_printf_immediately("%s" PFS_IO_TAB,
                                  commands[i].description.name);
    }
    pfs_io_printf_immediately(PFS_IO_BOLD_OFF "\n");
    pfs_io_restore_shell_prompt();
}

static void print_common_string(const char *common_string,
                                int argc,
                                char *argv[],
                                bool ends_with_space,
                                pfs_input_buffer_t *input_buffer) {
    int beginning = (argc == 0 || ends_with_space) ? 0 : strlen(argv[argc - 1]);
    for (size_t i = beginning; i < strlen(common_string); i++) {
        pfs_io_handle_input_char(common_string[i]);
    }
}

static const pfs_command_t *
find_latest_subcommand(const pfs_command_t *commands,
                       size_t number_of_commands,
                       int argc,
                       char *argv[]) {
    if (argc == 0) {
        return commands;
    }
    const pfs_command_t *curr_command =
            pfs_find_command_by_name(commands, argv[0], number_of_commands);
    if (curr_command == NULL) {
        return NULL;
    }
    for (int i = 1; i < argc; i++) {
        if (!pfs_command_has_subcommands(curr_command)) {
            return NULL;
        }
        curr_command =
                pfs_find_command_by_name(curr_command->subcommands, argv[i],
                                         curr_command->number_of_subcommands);
        if (curr_command == NULL) {
            return NULL;
        }
    }

    return curr_command;
}

static void print_commands_or_common_string(const pfs_command_t *commands,
                                            size_t number_of_commands,
                                            int argc,
                                            char *argv[],
                                            bool ends_with_space,
                                            pfs_input_buffer_t *input_buffer) {
    if (commands == NULL || number_of_commands == 0) {
        return;
    }

    char common_string[PFS_MAX_INPUT_SIZE] = {0};

    if (number_of_commands == 1) {
        print_command_name_and_space(&commands[0], argc, argv, ends_with_space,
                                     input_buffer);
        return;
    }
    find_commands_common_string(commands, number_of_commands, common_string);
    if (strlen(common_string) == 0
        || (strlen(common_string) == strlen(argv[argc - 1])
            && !ends_with_space)) {
        print_all_commands(commands, number_of_commands);
        return;
    }
    print_common_string(common_string, argc, argv, ends_with_space,
                        input_buffer);
}

void pfs_autocompletion(pfs_input_buffer_t *input_buffer) {
    if (input_buffer->len != input_buffer->cursor) {
        return;
    }

    char *argv[PFS_MAX_ARGC];
    int argc = 0;
    char buffer[PFS_MAX_INPUT_SIZE];
    strncpy(buffer, input_buffer->buffer, PFS_MAX_INPUT_SIZE);
    buffer[PFS_MAX_INPUT_SIZE - 1] = '\0';

    if (pfs_custom_tokenizer(buffer, argv, PFS_MAX_ARGC, &argc)) {
        return;
    }

    size_t number_of_core_commands;
    const pfs_command_t *core_commands =
            pfs_get_core_commands_sorted_by_name_dynamic(
                    &number_of_core_commands);

    size_t number_of_commands = number_of_core_commands;
    const pfs_command_t *commands = core_commands;

    bool ends_with_space = input_buffer_ends_with_space(input_buffer);

    if (argc == 0) {
        print_commands_or_common_string(commands, number_of_commands, argc,
                                        argv, ends_with_space, input_buffer);
        goto on_exit;
    }

    if (ends_with_space) {
        const pfs_command_t *curr_command =
                find_latest_subcommand(commands, number_of_commands, argc,
                                       argv);
        if (curr_command == NULL) {
            goto on_exit;
        }
        if (pfs_command_has_subcommands(curr_command)) {
            commands = curr_command->subcommands;
            number_of_commands = curr_command->number_of_subcommands;
            print_commands_or_common_string(commands, number_of_commands, argc,
                                            argv, ends_with_space,
                                            input_buffer);
        }
        goto on_exit;
    }

    if (argc == 1) {
        commands = find_commands_by_substring_dynamic(
                commands, number_of_commands, argv[0], &number_of_commands);
    } else {
        const pfs_command_t *curr_command =
                find_latest_subcommand(commands, number_of_commands, argc - 1,
                                       argv);
        if (curr_command == NULL) {
            goto on_exit;
        }
        if (!pfs_command_has_subcommands(curr_command)) {
            goto on_exit;
        }
        commands = find_commands_by_substring_dynamic(
                curr_command->subcommands, curr_command->number_of_subcommands,
                argv[argc - 1], &number_of_commands);
    }

    print_commands_or_common_string(commands, number_of_commands, argc, argv,
                                    ends_with_space, input_buffer);
    free((void *) commands);

on_exit:
    free((void *) core_commands);
}
