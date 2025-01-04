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

#include "pfs_handle_shell_input.h"
#include "pfs_io.h"

bool _pfs_is_initialized(void);

static int name_is_valid(const char *name) {
    size_t len = strlen(name);

    if (len == 0) {
        return 0;
    }

    for (size_t i = 0; i < len; i++) {
        if (!isalnum(name[i])) {
            PFS_SHELL_LOG(ERR, "invalid character '%c' in name '%s'\n", name[i],
                          name);
            return 0;
        }
    }

    return 1;
}

static int description_is_valid(const pfs_command_description_t *description) {
    if (!description) {
        return 0;
    }

    if (description->name == NULL || description->help == NULL) {
        return 0;
    }

    if (!name_is_valid(description->name)) {
        return 0;
    }

    return 1;
}

static int pointers_are_valid(const pfs_command_t *command) {
    if (command == NULL) {
        return 0;
    }

    if (command->handler == NULL
        && (command->subcommands == NULL
            || command->number_of_subcommands == 0)) {
        return 0;
    }

    if (command->handler != NULL
        && (command->subcommands != NULL
            || command->number_of_subcommands != 0)) {
        return 0;
    }

    return 1;
}

static int commands_have_the_same_name(const pfs_command_t *c1,
                                       const pfs_command_t *c2) {
    return strcmp(c1->description.name, c2->description.name) == 0;
}

static int commands_are_valid(const pfs_command_t *commands,
                              size_t number_of_commands) {
    if (commands == NULL && number_of_commands == 0) {
        return 1;
    }

    if (commands == NULL && number_of_commands != 0) {
        PFS_SHELL_LOG(ERR,
                      "commands are NULL but number of commands is not 0\n");
        return 0;
    }

    for (size_t i = 0; i < number_of_commands; i++) {
        if (!pointers_are_valid(&commands[i])) {
            PFS_SHELL_LOG(ERR, "command %p has invalid pointers\n",
                          &commands[i]);
            return 0;
        }

        if (!description_is_valid(&commands[i].description)) {
            PFS_SHELL_LOG(ERR, "command %p has invalid description\n",
                          &commands[i]);
            return 0;
        }

        for (int j = i - 1; j >= 0; j--) {
            if (commands_have_the_same_name(&commands[i], &commands[j])) {
                PFS_SHELL_LOG(ERR, "commands %p and %p have the same name\n",
                              &commands[i], &commands[j]);
                return 0;
            }
        }
        if (!commands_are_valid(commands[i].subcommands,
                                commands[i].number_of_subcommands)) {
            return 0;
        }
    }

    return 1;
}

int pfs_commands_register(const pfs_command_t commands[],
                          size_t number_of_commands) {
    puts("");
    if (pfs_get_commands() != NULL) {
        PFS_SHELL_LOG(ERR, "commands already registered\n");
        return -1;
    }

    if (_pfs_is_initialized()) {
        PFS_SHELL_LOG(ERR, "shell already initialized\n");
        return -1;
    }

    if (commands == NULL || number_of_commands == 0) {
        PFS_SHELL_LOG(ERR, "commands are NULL or number of commands is 0\n");
        return 1;
    }

    if (!commands_are_valid(commands, number_of_commands)) {
        return 1;
    }

    pfs_set_commands(commands, number_of_commands);

    return 0;
}
