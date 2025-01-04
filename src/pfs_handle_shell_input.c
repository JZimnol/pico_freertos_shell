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

#include <pico_freertos_shell/commands.h>

#include "pfs_cmd_queue.h"
#include "pfs_handle_shell_input.h"
#include "pfs_io.h"

static const pfs_command_t *m_commands = NULL;
static size_t m_number_of_commands = 0;

#ifdef PFS_WITH_TESTS
void pfs_reset_commands(void) {
    m_number_of_commands = 0;
    m_commands = NULL;
}
#endif // PFS_WITH_TESTS

static const pfs_command_t HELP_COMMAND = {
        .description = {.name = "help",
                        .help = "display help message for a specified command"},
        .handler = NULL,
        .subcommands = NULL,
        .number_of_subcommands = 0,
};

static const pfs_command_t HELPTREE_COMMAND = {
        .description =
                {.name = "helptree",
                 .help = "display help message tree for a specified command"},
        .handler = NULL,
        .subcommands = NULL,
        .number_of_subcommands = 0,
};

static const pfs_command_t m_aditional_commands[] = {HELP_COMMAND,
                                                     HELPTREE_COMMAND};

#define PFS_ADDITIONAL_COMMANDS_SIZE \
    (sizeof(m_aditional_commands) / sizeof(pfs_command_t))

void pfs_set_commands(const pfs_command_t *commands,
                      size_t number_of_commands) {
    m_commands = commands;
    m_number_of_commands = number_of_commands + PFS_ADDITIONAL_COMMANDS_SIZE;
}

size_t pfs_get_number_of_commands(void) {
    if (m_number_of_commands > 0) {
        return m_number_of_commands;
    }
    return PFS_ADDITIONAL_COMMANDS_SIZE;
}

const pfs_command_t *pfs_get_commands(void) {
    // does not include HELP_COMMAND and HELPTREE_COMMAND
    return m_commands;
}

const pfs_command_t *pfs_get_aditional_commands(size_t *out_len) {
    if (out_len != NULL) {
        *out_len = PFS_ADDITIONAL_COMMANDS_SIZE;
    }
    return m_aditional_commands;
}

#define RET_UNCLOSED_QUOTE -1
#define RET_NO_SPACE_AFTER_QUOTE -2
#define RET_INTERNAL_BUFFER_TOO_SMALL -3

int pfs_custom_tokenizer(char *input,
                         char *argv[],
                         int max_argc,
                         int *out_argc) {
    *out_argc = 0;
    char *current = input;
    char *token_start = NULL;
    bool in_quotes = false;

    while (*current != '\0') {
        switch (in_quotes) {
        case true:
            if (*current != '"') {
                break;
            }
            if (*(current + 1) != ' ' && *(current + 1) != '\0') {
                return RET_NO_SPACE_AFTER_QUOTE;
            }
            *current = '\0';
            if (token_start != current) {
                argv[*out_argc] = token_start;
                (*out_argc)++;
            }
            in_quotes = false;
            token_start = NULL;
            break;
        case false:
            switch (*current) {
            case ' ':
                if (token_start == NULL) {
                    break;
                }
                *current = '\0';
                argv[*out_argc] = token_start;
                (*out_argc)++;
                token_start = NULL;
                break;
            case '"':
                if (*(current + 1) != '\0') {
                    token_start = current + 1;
                    in_quotes = true;
                } else {
                    return RET_UNCLOSED_QUOTE;
                }
                break;
            default:
                if (token_start == NULL) {
                    token_start = current;
                }
                break;
            }
        }
        if (*out_argc >= max_argc) {
            return RET_INTERNAL_BUFFER_TOO_SMALL;
        }
        current++;
    }

    if (in_quotes) {
        return RET_UNCLOSED_QUOTE;
    }

    if (token_start != NULL) {
        argv[*out_argc] = token_start;
        (*out_argc)++;
    }

    return 0;
}

static int sort_commands_by_name(const void *a, const void *b) {
    const pfs_command_t *command_a = (const pfs_command_t *) a;
    const pfs_command_t *command_b = (const pfs_command_t *) b;

    return strcmp(command_a->description.name, command_b->description.name);
}

const pfs_command_t *
pfs_get_subcommands_sorted_by_name_dynamic(const pfs_command_t *subcommands,
                                           size_t number_of_subcommands) {
    pfs_command_t *sorted_subcommands =
            malloc(number_of_subcommands * sizeof(pfs_command_t));
    if (sorted_subcommands == NULL) {
        PFS_SHELL_LOG(ERR,
                      "*** failed to allocate memory for sorted subcommands\n");
        return NULL;
    }

    memcpy(sorted_subcommands, subcommands,
           number_of_subcommands * sizeof(pfs_command_t));
    qsort(sorted_subcommands, number_of_subcommands, sizeof(pfs_command_t),
          sort_commands_by_name);

    return sorted_subcommands;
}

const pfs_command_t *
pfs_get_core_commands_sorted_by_name_dynamic(size_t *out_number_of_commands) {
    pfs_command_t *sorted_commands =
            malloc((pfs_get_number_of_commands()) * sizeof(pfs_command_t));
    if (sorted_commands == NULL) {
        PFS_SHELL_LOG(ERR,
                      "*** failed to allocate memory for sorted commands\n");
        return NULL;
    }

    if (m_commands) {
        memcpy(sorted_commands, m_commands,
               (pfs_get_number_of_commands() - PFS_ADDITIONAL_COMMANDS_SIZE)
                       * sizeof(pfs_command_t));
    }
    memcpy(sorted_commands + (pfs_get_number_of_commands() - 1), &HELP_COMMAND,
           sizeof(pfs_command_t));
    memcpy(sorted_commands + (pfs_get_number_of_commands() - 2),
           &HELPTREE_COMMAND, sizeof(pfs_command_t));

    qsort(sorted_commands, pfs_get_number_of_commands(), sizeof(pfs_command_t),
          sort_commands_by_name);

    if (out_number_of_commands != NULL) {
        *out_number_of_commands = pfs_get_number_of_commands();
    }

    return sorted_commands;
}

int pfs_command_has_subcommands(const pfs_command_t *command) {
    if (command == NULL) {
        return 0;
    }
    return command->number_of_subcommands > 0;
}

const pfs_command_t *pfs_find_command_by_name(const pfs_command_t *commands,
                                              const char *name,
                                              size_t number_of_commands) {
    if (commands == m_commands && name != NULL) {
        if (strcmp(name, "help") == 0) {
            return &HELP_COMMAND;
        } else if (strcmp(name, "helptree") == 0) {
            return &HELPTREE_COMMAND;
        }
        number_of_commands -= PFS_ADDITIONAL_COMMANDS_SIZE;
    }

    if (commands == NULL || name == NULL) {
        return NULL;
    }

    const pfs_command_t *curr_command = commands;
    for (size_t i = 0; i < number_of_commands; i++) {
        if (strcmp(curr_command->description.name, name) == 0) {
            return curr_command;
        }
        curr_command++;
    }

    return NULL;
}

static void print_tabs(size_t level) {
    for (size_t i = 0; i < level; i++) {
        pfs_io_puts_immediately(PFS_IO_TAB);
    }
}

static int handle_helptree_command(const pfs_command_t *subcommand, int level) {
    if (subcommand == NULL) {
        return 0;
    }

    if (!pfs_command_has_subcommands(subcommand)) {
        return 0;
    }

    const pfs_command_t *sorted_subcommands =
            pfs_get_subcommands_sorted_by_name_dynamic(
                    subcommand->subcommands, subcommand->number_of_subcommands);
    if (sorted_subcommands == NULL) {
        return 1;
    }
    for (size_t i = 0; i < subcommand->number_of_subcommands; i++) {
        PFS_SHELL_LOG(INF, "");
        print_tabs(level);
        pfs_io_printf_immediately(PFS_IO_BOLD_ON "%s" PFS_IO_BOLD_OFF " - %s\n",
                                  sorted_subcommands[i].description.name,
                                  sorted_subcommands[i].description.help);
        if (handle_helptree_command(&sorted_subcommands[i], level + 1)) {
            free((void *) sorted_subcommands);
            return 1;
        }
    }
    free((void *) sorted_subcommands);

    return 0;
}

static int handle_help_command(char *argv[], int argc, bool helptree) {
    if (argc == 0) {
        const pfs_command_t *sorted_commands =
                pfs_get_core_commands_sorted_by_name_dynamic(NULL);
        if (sorted_commands == NULL) {
            return 1;
        }

        PFS_SHELL_LOG(INF, "Available commands:\n");
        for (size_t i = 0; i < pfs_get_number_of_commands(); i++) {
            PFS_SHELL_LOG(INF,
                          PFS_IO_TAB PFS_IO_BOLD_ON "%s" PFS_IO_BOLD_OFF
                                                    " - %s\n",
                          sorted_commands[i].description.name,
                          sorted_commands[i].description.help);
            if (helptree) {
                if (handle_helptree_command(
                            (const pfs_command_t *) &sorted_commands[i], 2)) {
                    free((void *) sorted_commands);
                    return 1;
                }
            }
        }

        free((void *) sorted_commands);
        return 0;
    }

    char message_chain[PFS_MAX_INPUT_SIZE];
    memset(message_chain, 0, sizeof(message_chain));
    snprintf(message_chain, sizeof(message_chain), "%s", argv[0]);
    const pfs_command_t *command =
            pfs_find_command_by_name(m_commands, argv[0], m_number_of_commands);
    if (command == NULL) {
        PFS_SHELL_LOG(WRN, "%s: command not found\n", message_chain);
        PFS_SHELL_LOG(WRN, "type 'help' for a list of available commands\n");
        return 1;
    }

    size_t iterator = 1;
    while (iterator < argc) {
        if (!pfs_command_has_subcommands(command)) {
            PFS_SHELL_LOG(WRN, "no help entry for '%s %s'\n", message_chain,
                          argv[iterator]);
            return 1;
        }
        command = pfs_find_command_by_name(command->subcommands, argv[iterator],
                                           command->number_of_subcommands);
        if (command == NULL) {
            PFS_SHELL_LOG(WRN, "%s %s: command not found'\n", message_chain,
                          argv[iterator]);
            PFS_SHELL_LOG(
                    WRN, "type 'help %s' for a list of available subcommands\n",
                    message_chain);
            return 1;
        }
        strcat(message_chain, " ");
        strcat(message_chain, argv[iterator]);
        iterator++;
    }

    PFS_SHELL_LOG(INF, "Description:\n");
    PFS_SHELL_LOG(INF, PFS_IO_TAB "%s\n", command->description.help);
    PFS_SHELL_LOG(INF, "Avalable subcommands:\n");
    if (pfs_command_has_subcommands(command)) {
        if (helptree) {
            if (handle_helptree_command(command, 1)) {
                return 1;
            }
            return 0;
        }
        const pfs_command_t *sorted_subcommands =
                pfs_get_subcommands_sorted_by_name_dynamic(
                        command->subcommands, command->number_of_subcommands);
        for (size_t i = 0; i < command->number_of_subcommands; i++) {
            PFS_SHELL_LOG(INF,
                          PFS_IO_TAB PFS_IO_BOLD_ON "%s" PFS_IO_BOLD_OFF
                                                    " - %s\n",
                          sorted_subcommands[i].description.name,
                          sorted_subcommands[i].description.help);
        }
        free((void *) sorted_subcommands);
    } else {
        PFS_SHELL_LOG(INF, PFS_IO_TAB "no subcommands available for '%s'\n",
                      message_chain);
    }

    return 0;
}

static pfs_command_handler_t
find_command_handler(char *argv[], int argc, size_t *out_level) {
    if (!out_level) {
        return NULL;
    }
    *out_level = 0;

    if (argv == NULL || argc <= 0) {
        return NULL;
    }

    const pfs_command_t *curr_command =
            pfs_find_command_by_name(pfs_get_commands(), argv[0],
                                     pfs_get_number_of_commands());
    if (curr_command == NULL) {
        PFS_SHELL_LOG(WRN, "%s: command not found\n", argv[0]);
        PFS_SHELL_LOG(WRN, "type 'help' for a list of available commands\n");
        return NULL;
    }

    (*out_level)++;

    if (argc == 1) {
        if (pfs_command_has_subcommands(curr_command)) {
            PFS_SHELL_LOG(WRN, "incomplete command: '%s'\n", argv[0]);
            PFS_SHELL_LOG(
                    WRN, "type 'help %s' for a list of available subcommands\n",
                    argv[0]);
            return NULL;
        }
    }

    if (!pfs_command_has_subcommands(curr_command)) {
        return curr_command->handler;
    }

    const pfs_command_t *curr_subcommand =
            pfs_find_command_by_name(curr_command->subcommands, argv[1],
                                     curr_command->number_of_subcommands);
    if (curr_subcommand == NULL) {
        PFS_SHELL_LOG(WRN, "%s: subcommand not found\n", argv[1]);
        PFS_SHELL_LOG(WRN,
                      "type 'help %s' for a list of available subcommands\n",
                      argv[0]);
        return NULL;
    }

    (*out_level)++;

    char message_chain[256];
    memset(message_chain, 0, sizeof(message_chain));
    snprintf(message_chain, sizeof(message_chain), "%s %s", argv[0], argv[1]);

    size_t iterator = 2;
    while (iterator < argc) {
        if (!pfs_command_has_subcommands(curr_subcommand)) {
            break;
        }

        curr_subcommand = pfs_find_command_by_name(
                curr_subcommand->subcommands, argv[iterator],
                curr_subcommand->number_of_subcommands);
        if (curr_subcommand == NULL) {
            PFS_SHELL_LOG(WRN, "%s: subcommand not found\n", argv[iterator]);
            PFS_SHELL_LOG(
                    WRN, "type 'help %s' for a list of available subcommands\n",
                    message_chain);
            return NULL;
        }
        (*out_level)++;
        strcat(message_chain, " ");
        strcat(message_chain, argv[iterator]);
        iterator++;
    }

    if (pfs_command_has_subcommands(curr_subcommand)) {
        PFS_SHELL_LOG(WRN, "incomplete command: '%s'\n", message_chain);
        PFS_SHELL_LOG(WRN,
                      "type 'help %s' for a list of available subcommands\n",
                      message_chain);
        return NULL;
    }

    return curr_subcommand->handler;
}

bool pfs_input_has_only_whitespaces(const char *input) {
    if (input == NULL) {
        return true;
    }

    while (*input != '\0') {
        if (!isspace(*input)) {
            return false;
        }
        input++;
    }

    return true;
}

static const char *error_to_string(int error) {
    switch (error) {
    case RET_UNCLOSED_QUOTE:
        return "unclosed quote";
    case RET_NO_SPACE_AFTER_QUOTE:
        return "no space after quote";
    case RET_INTERNAL_BUFFER_TOO_SMALL:
        return "internal buffer too small";
    default:
        return "unknown error";
    }
}

char m_buffer[PFS_MAX_INPUT_SIZE];
char *m_argv[PFS_MAX_ARGC];

int pfs_handle_shell_input(const char *input) {
    if (input == NULL || strlen(input) >= PFS_MAX_INPUT_SIZE) {
        // unlikely, but handle it anyway
        return 1;
    }

    if (pfs_input_has_only_whitespaces(input)) {
        return 0;
    }

    int argc = 0;

    strncpy(m_buffer, input, PFS_MAX_INPUT_SIZE);
    m_buffer[PFS_MAX_INPUT_SIZE - 1] = '\0';

    int ret = pfs_custom_tokenizer(m_buffer, m_argv, PFS_MAX_ARGC, &argc);
    if (ret != 0) {
        PFS_SHELL_LOG(ERR, "invalid input: %s\n", error_to_string(ret));
        return 1;
    }

    if (argc == 0) {
        return 0;
    }

    if (strcmp(m_argv[0], "help") == 0) {
        return handle_help_command(m_argv + 1, argc - 1, false);
    }

    if (strcmp(m_argv[0], "helptree") == 0) {
        return handle_help_command(m_argv + 1, argc - 1, true);
    }

    size_t level = 0;
    pfs_command_handler_t handler = find_command_handler(m_argv, argc, &level);
    if (handler == NULL) {
        return 1;
    }

    pfs_cmd_args_t cmd_args = {
            .handler = handler, .argc = argc - level, .argv = m_argv + level};

    if (pfs_cmd_queue_add(&cmd_args)) {
        PFS_SHELL_LOG(ERR, "failed to add item to cmd_handler_queue\n");
        return 1;
    }

    return 0;
}
