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
#include <stdio.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Command handler function type.
 *
 * @param argc Number of arguments.
 * @param argv Array of null-terminated arguments.
 *
 * @note Arguments do not include the command name. For example, if the command
 *       is `wifi set` and the user enters `wifi set ssid "my ssid"`, argc will
 *       be 2 and argv will be ["ssid", "my ssid"].
 */
typedef void (*pfs_command_handler_t)(int argc, char **argv);

typedef struct {
    const char *name;
    const char *help;
} pfs_command_description_t;

/**
 * @brief Command structure. Note that this structure can be used both for
 *        commands and subcommands.
 *
 * @note This structure is not meant to be initialized directly. Use the
 *       `PFS_COMMAND_INITIALIZER` macro instead.
 */
typedef struct pfs_command {
    const pfs_command_description_t description;
    const size_t number_of_subcommands;
    const struct pfs_command *subcommands;
    pfs_command_handler_t handler;
 } pfs_command_t;

/**
 * @brief Command handler initialization macro. Use this macro to initialize a
 *        command handler if the command does not have subcommands.
 *
 * @param Handler Command handler function. Must not be NULL.
 */
#define PFS_COMMAND_HANDLER(Handler) Handler, NULL, 0

/**
 * @brief Subcommands initialization macro. Use this macro to attach subcommands
 *        to a command.
 *
 * @param Subcommands Array of subcommands. Must not be NULL.
 * @param Size        Number of subcommands.
 */
#define PFS_SUBCOMMANDS(Subcommands, Size) NULL, Subcommands, Size

#define _PFS_COMMAND_DEFINE(Name, Help, Handler, Subcommands, Number)     \
    {                                                                     \
        .description = {.name = #Name, .help = Help}, .handler = Handler, \
        .subcommands = Subcommands, .number_of_subcommands = Number       \
    }

/**
 * @brief Command initializer macro. Use this macro to initialize a command or
 *        a subcommand.
 *
 * @param Name Command name. Must not be a string and can't be empty.
 * @param Help Command help message. Must be a string and can't be empty.
 * @param ...  Command handler or subcommands. If the command has subcommands
 *             (doesn't have a handler), the `PFS_SUBCOMMANDS` macro must be
 *             used. If the command has a handler (does not have subcommands),
 *             the `PFS_COMMAND_HANDLER` macro must be used. If no macro is
 *             used, user should make sure that the command does not have both
 *             handler and subcommands.
 */
#define PFS_COMMAND_INITIALIZER(Name, Help, ...) \
    _PFS_COMMAND_DEFINE(Name, Help, __VA_ARGS__)

/**
 * @brief Registers commands in the shell.
 *
 * @param commands           Array of commands to register. Must not be NULL.
 * @param number_of_commands Number of commands in the array.
 *
 * @return 0 on success,
 *         non-zero on failure.
 *         An error message will be printed to the standard output (if any).
 *
 * @note This function must be called before the shell is initialized using
 *       `pfs_init()`, otherwise will return an error.
 */
int pfs_commands_register(const pfs_command_t commands[],
                          size_t number_of_commands);

#ifdef __cplusplus
}
#endif // __cplusplus
