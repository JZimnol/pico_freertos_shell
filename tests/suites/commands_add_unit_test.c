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

#include <unity.h>

#include <test_utils.h>

#include <pico_freertos_shell/commands.h>

#include <pfs_handle_shell_input.h>
#include <pfs_utils.h>

static void dummy_cmd_handler(int argc, char **argv) {
    (void) argc;
    (void) argv;
}

static const pfs_command_t subcommands_20_OK[] = {
        PFS_COMMAND_INITIALIZER(subcommand200,
                                "subcommand200 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
        PFS_COMMAND_INITIALIZER(subcommand201,
                                "subcommand201 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
};

static const pfs_command_t subcommands_21_OK[] = {
        PFS_COMMAND_INITIALIZER(subcommand210,
                                "subcommand210 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
        PFS_COMMAND_INITIALIZER(subcommand211,
                                "subcommand211 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
};

static const pfs_command_t subcommands_10_OK[] = {
        PFS_COMMAND_INITIALIZER(
                subcommand100,
                "subcommand100 description",
                PFS_SUBCOMMANDS(subcommands_20_OK,
                                PFS_ARRAY_SIZE(subcommands_20_OK))),
        PFS_COMMAND_INITIALIZER(
                subcommand101,
                "subcommand101 description",
                PFS_SUBCOMMANDS(subcommands_21_OK,
                                PFS_ARRAY_SIZE(subcommands_21_OK))),
};

static const pfs_command_t subcommands_11_NOT_OK[] = {
        PFS_COMMAND_INITIALIZER(
                subcommand110,
                "",
                PFS_SUBCOMMANDS(subcommands_20_OK,
                                PFS_ARRAY_SIZE(subcommands_20_OK))),
        PFS_COMMAND_INITIALIZER(
                ,
                "subcommand111 description",
                PFS_SUBCOMMANDS(subcommands_21_OK,
                                PFS_ARRAY_SIZE(subcommands_21_OK))),
};

static const pfs_command_t subcommands_00_OK[] = {
        PFS_COMMAND_INITIALIZER(
                subcommand000,
                "subcommand000 description",
                PFS_SUBCOMMANDS(subcommands_10_OK,
                                PFS_ARRAY_SIZE(subcommands_10_OK))),
        PFS_COMMAND_INITIALIZER(subcommand001,
                                "subcommand001 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
};

static const pfs_command_t subcommands_01_NOT_OK[] = {
        PFS_COMMAND_INITIALIZER(
                subcommand010,
                "subcommand010 description",
                PFS_SUBCOMMANDS(subcommands_10_OK,
                                PFS_ARRAY_SIZE(subcommands_10_OK))),
        PFS_COMMAND_INITIALIZER(subcommand010,
                                "subcommand010 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
};

static const pfs_command_t commands_0_OK[] = {
        PFS_COMMAND_INITIALIZER(command00,
                                "command00 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
};

static const pfs_command_t commands_1_OK[] = {
        PFS_COMMAND_INITIALIZER(command10,
                                "command10 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
        PFS_COMMAND_INITIALIZER(
                command11,
                "command11 description",
                PFS_SUBCOMMANDS(subcommands_00_OK,
                                PFS_ARRAY_SIZE(subcommands_00_OK))),
        PFS_COMMAND_INITIALIZER(
                command12,
                "command12 description",
                PFS_SUBCOMMANDS(subcommands_00_OK,
                                PFS_ARRAY_SIZE(subcommands_00_OK))),
};

static const pfs_command_t commands_2_NOT_OK[] = {
        PFS_COMMAND_INITIALIZER(command20,
                                "command20 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
        PFS_COMMAND_INITIALIZER(
                command21,
                "command21 description",
                PFS_SUBCOMMANDS(subcommands_01_NOT_OK,
                                PFS_ARRAY_SIZE(subcommands_01_NOT_OK))),
        PFS_COMMAND_INITIALIZER(
                command22,
                "command22 description",
                PFS_SUBCOMMANDS(subcommands_11_NOT_OK,
                                PFS_ARRAY_SIZE(subcommands_11_NOT_OK))),
};

static const pfs_command_t commands_3_NOT_OK[] = {
        PFS_COMMAND_INITIALIZER(command30,
                                "command30 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
        PFS_COMMAND_INITIALIZER(
                command30,
                "command30 description",
                PFS_SUBCOMMANDS(subcommands_01_NOT_OK,
                                PFS_ARRAY_SIZE(subcommands_01_NOT_OK))),
};

static const pfs_command_t commands_4_OK[] = {
        PFS_COMMAND_INITIALIZER(
                command40,
                "command40 description",
                PFS_SUBCOMMANDS(subcommands_00_OK,
                                PFS_ARRAY_SIZE(subcommands_00_OK))),
};

static const pfs_command_t commands_5_NOT_OK[] = {
        PFS_COMMAND_INITIALIZER(command50,
                                "command50 description",
                                dummy_cmd_handler,
                                subcommands_00_OK,
                                PFS_ARRAY_SIZE(subcommands_00_OK)),
};

void setUp(void) {
    pfs_reset_commands();
}

void tearDown(void) {}

void AddZeroCommands(void) {
    const pfs_command_t commands;
    TEST_ASSERT_EQUAL_INT(1, pfs_commands_register(NULL, 0));
    TEST_ASSERT_EQUAL_INT(1, pfs_commands_register(NULL, 5));
    TEST_ASSERT_EQUAL_INT(1, pfs_commands_register(commands_0_OK, 0));
}

void AddOneCommand(void) {
    TEST_ASSERT_EQUAL_INT(1, pfs_commands_register(commands_5_NOT_OK, 1));
    TEST_ASSERT_EQUAL_INT(0, pfs_commands_register(commands_0_OK, 1));
    TEST_ASSERT_EQUAL_INT(-1, pfs_commands_register(commands_0_OK, 1));
    setUp();
    TEST_ASSERT_EQUAL_INT(0, pfs_commands_register(commands_4_OK, 1));
}

void AddFewCommands(void) {
    TEST_ASSERT_EQUAL_INT(0, pfs_commands_register(commands_1_OK, 3));
    TEST_ASSERT_EQUAL_INT(-1, pfs_commands_register(commands_1_OK, 3));
    setUp();
    TEST_ASSERT_EQUAL_INT(1, pfs_commands_register(commands_2_NOT_OK, 3));
    TEST_ASSERT_EQUAL_INT(1, pfs_commands_register(commands_2_NOT_OK, 3));
    setUp();
    TEST_ASSERT_EQUAL_INT(1, pfs_commands_register(commands_3_NOT_OK, 2));
    TEST_ASSERT_EQUAL_INT(1, pfs_commands_register(commands_3_NOT_OK, 2));
    TEST_ASSERT_EQUAL_INT(0, pfs_commands_register(commands_1_OK, 3));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(AddZeroCommands);
    RUN_TEST(AddOneCommand);
    RUN_TEST(AddFewCommands);

    return UNITY_END();
}
