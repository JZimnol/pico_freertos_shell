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

void setUp(void) {
    utils_reset_out_string_immediately_buffer();
    pfs_reset_commands();
}

void tearDown(void) {}

static int single_command_counter = 0;
static void HandleInputSingleCommandCmdHandler(int argc, char **argv) {
    switch (single_command_counter) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        TEST_ASSERT_EQUAL_INT(0, argc);
        break;
    case 5:
        TEST_ASSERT_EQUAL_INT(1, argc);
        TEST_ASSERT_EQUAL_STRING("arg1", argv[0]);
        break;
    case 6:
        TEST_ASSERT_EQUAL_INT(4, argc);
        TEST_ASSERT_EQUAL_STRING("singlecommand", argv[0]);
        TEST_ASSERT_EQUAL_STRING("arg1", argv[1]);
        TEST_ASSERT_EQUAL_STRING("arg2", argv[2]);
        TEST_ASSERT_EQUAL_STRING("arg3", argv[3]);
        break;
    case 7:
        TEST_ASSERT_EQUAL_INT(2, argc);
        TEST_ASSERT_EQUAL_STRING("arg1", argv[0]);
        TEST_ASSERT_EQUAL_STRING("arg2", argv[1]);
        break;
    case 8:
        TEST_ASSERT_EQUAL_INT(1, argc);
        TEST_ASSERT_EQUAL_STRING("arg1 ", argv[0]);
        break;
    case 9:
        TEST_ASSERT_EQUAL_INT(1, argc);
        TEST_ASSERT_EQUAL_STRING("arg1 arg2   arg3", argv[0]);
        break;
    default:
        TEST_FAIL();
        break;
    }
}

static const pfs_command_t singlecommand[] = {
        PFS_COMMAND_INITIALIZER(singlecommand,
                                "singlecommand description",
                                PFS_COMMAND_HANDLER(
                                        HandleInputSingleCommandCmdHandler)),
};

void HandleInputSingleCommand(void) {
    TEST_ASSERT_EQUAL_INT(0, pfs_commands_register(singlecommand, 1));
    const char *valid_inputs[] = {
            "singlecommand",                               // 0
            " singlecommand",                              // 1
            "singlecommand ",                              // 2
            " singlecommand ",                             // 3
            "singlecommand  ",                             // 4
            "singlecommand arg1",                          // 5
            "singlecommand singlecommand arg1 arg2 arg3 ", // 6
            "singlecommand \"arg1\" \"\" arg2",            // 7
            "singlecommand \"arg1 \"",                     // 8
            "singlecommand \"arg1 arg2   arg3\"",          // 9
    };

    for (single_command_counter = 0;
         single_command_counter < PFS_ARRAY_SIZE(valid_inputs);
         single_command_counter++) {
        TEST_ASSERT_EQUAL_INT(0, pfs_handle_shell_input(
                                         valid_inputs[single_command_counter]));
    }

    const char *invalid_inputs[] = {
            "single command arg1 arg2",               // 0
            "singlecommand \"arg1\"arg2 arg3",        // 1
            "singlecommand \"arg1\"\"arg2\"\"arg3\"", // 2
            "singlecommand \"arg1",                   // 3
            "singlecommand arg1\"",                   // 4
    };

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[0]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN
            "single: command not found\n" PFS_IO_SHELL_MESSAGE_WRN_BEGIN
            "type 'help' for a list of available commands\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[1]));
    TEST_ASSERT_EQUAL_STRING(PFS_IO_SHELL_MESSAGE_ERR_BEGIN
                             "invalid input: no space after quote\n",
                             utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[2]));
    TEST_ASSERT_EQUAL_STRING(PFS_IO_SHELL_MESSAGE_ERR_BEGIN
                             "invalid input: no space after quote\n",
                             utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[3]));
    TEST_ASSERT_EQUAL_STRING(PFS_IO_SHELL_MESSAGE_ERR_BEGIN
                             "invalid input: unclosed quote\n",
                             utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[4]));
    TEST_ASSERT_EQUAL_STRING(PFS_IO_SHELL_MESSAGE_ERR_BEGIN
                             "invalid input: unclosed quote\n",
                             utils_get_out_string_immediately_buffer());
};

static int multiple_command_counter = 0;

static void command0_cmd_handler(int argc, char **argv) {
    switch (multiple_command_counter) {
    case 0:
        TEST_ASSERT_EQUAL_INT(0, argc);
        break;
    case 1:
        TEST_ASSERT_EQUAL_INT(2, argc);
        TEST_ASSERT_EQUAL_STRING("arg1", argv[0]);
        TEST_ASSERT_EQUAL_STRING("arg2", argv[1]);
        break;
    case 2:
        TEST_ASSERT_EQUAL_INT(1, argc);
        TEST_ASSERT_EQUAL_STRING("subcommand10", argv[0]);
        break;
    case 3:
        TEST_ASSERT_EQUAL_INT(1, argc);
        TEST_ASSERT_EQUAL_STRING("command1", argv[0]);
        break;
    default:
        TEST_FAIL();
        break;
    }
}

static void command1_subcommand10_cmd_handler(int argc, char **argv) {
    switch (multiple_command_counter) {
    case 4:
        TEST_ASSERT_EQUAL_INT(0, argc);
        break;
    case 5:
        TEST_ASSERT_EQUAL_INT(1, argc);
        TEST_ASSERT_EQUAL_STRING("arg1", argv[0]);
        break;
    case 6:
        TEST_ASSERT_EQUAL_INT(1, argc);
        TEST_ASSERT_EQUAL_STRING("subcommand11", argv[0]);
        break;
    default:
        TEST_FAIL();
        break;
    }
}

static void command1_subcommand11_subcommand00_cmd_handler(int argc,
                                                           char **argv) {
    switch (multiple_command_counter) {
    case 7:
        TEST_ASSERT_EQUAL_INT(0, argc);
        break;
    case 8:
        TEST_ASSERT_EQUAL_INT(2, argc);
        TEST_ASSERT_EQUAL_STRING("arg1", argv[0]);
        TEST_ASSERT_EQUAL_STRING("arg2", argv[1]);
        break;
    default:
        TEST_FAIL();
        break;
    }
}

static void
command1_subcommand11_subcommand01_subcommand_cmd_handler(int argc,
                                                          char **argv) {
    switch (multiple_command_counter) {
    case 9:
        TEST_ASSERT_EQUAL_INT(0, argc);
        break;
    case 10:
        TEST_ASSERT_EQUAL_INT(2, argc);
        TEST_ASSERT_EQUAL_STRING("arg1", argv[0]);
        TEST_ASSERT_EQUAL_STRING("arg2", argv[1]);
        break;
    default:
        TEST_FAIL();
        break;
    }
}

static const pfs_command_t subcommand[] = {
        PFS_COMMAND_INITIALIZER(
                subcommand,
                "subcommand description",
                PFS_COMMAND_HANDLER(
                        command1_subcommand11_subcommand01_subcommand_cmd_handler)),
};

static const pfs_command_t subcommands0[] = {
        PFS_COMMAND_INITIALIZER(
                subcommand00,
                "subcommand00 description",
                PFS_COMMAND_HANDLER(
                        command1_subcommand11_subcommand00_cmd_handler)),
        PFS_COMMAND_INITIALIZER(subcommand01,
                                "subcommand01 description",
                                PFS_SUBCOMMANDS(subcommand,
                                                PFS_ARRAY_SIZE(subcommand))),
};

static const pfs_command_t subcommands1[] = {
        PFS_COMMAND_INITIALIZER(subcommand10,
                                "subcommand10 description",
                                PFS_COMMAND_HANDLER(
                                        command1_subcommand10_cmd_handler)),
        PFS_COMMAND_INITIALIZER(subcommand11,
                                "subcommand11 description",
                                PFS_SUBCOMMANDS(subcommands0,
                                                PFS_ARRAY_SIZE(subcommands0))),
};

static const pfs_command_t multiplecommands[] = {
        PFS_COMMAND_INITIALIZER(command0,
                                "command0 description",
                                PFS_COMMAND_HANDLER(command0_cmd_handler)),
        PFS_COMMAND_INITIALIZER(command1,
                                "command1 description",
                                PFS_SUBCOMMANDS(subcommands1,
                                                PFS_ARRAY_SIZE(subcommands1))),
};

void HandleInputMultipleCommand(void) {
    TEST_ASSERT_EQUAL_INT(0, pfs_commands_register(multiplecommands, 2));

    const char *valid_inputs[] = {
            "command0",                                                // 0
            "command0 arg1 arg2",                                      // 1
            "command0 subcommand10",                                   // 2
            "command0 command1",                                       // 3
            "command1 subcommand10",                                   // 4
            "command1 subcommand10 arg1",                              // 5
            "command1 subcommand10 subcommand11",                      // 6
            "command1 subcommand11 subcommand00",                      // 7
            "command1 subcommand11 subcommand00 arg1 arg2",            // 8
            "command1 subcommand11 subcommand01 subcommand",           // 9
            "command1 subcommand11 subcommand01 subcommand arg1 arg2", // 10
    };
    for (multiple_command_counter = 0;
         multiple_command_counter < PFS_ARRAY_SIZE(valid_inputs);
         multiple_command_counter++) {
        TEST_ASSERT_EQUAL_INT(0,
                              pfs_handle_shell_input(
                                      valid_inputs[multiple_command_counter]));
    }

    const char *invalid_inputs[] = {
            "command1 subcommand12",                   // 0
            "command1 subcommand12 arg1",              // 1
            "command1 subcommand11",                   // 2
            "command1 subcommand11 subcommand02 arg1", // 3
            "command1 subcommand11 subcommand01 arg1", // 4
    };

    // clang-format off
    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[0]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "subcommand12: subcommand not found\n"
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "type 'help command1' for a list of available subcommands\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[1]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "subcommand12: subcommand not found\n"
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "type 'help command1' for a list of available subcommands\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[2]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "incomplete command: 'command1 subcommand11'\n"
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "type 'help command1 subcommand11' for a list of available subcommands\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[3]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "subcommand02: subcommand not found\n"
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "type 'help command1 subcommand11' for a list of available subcommands\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[4]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "arg1: subcommand not found\n"
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "type 'help command1 subcommand11 subcommand01' for a list of available subcommands\n",
            utils_get_out_string_immediately_buffer());
};

void HandleHelpCommand(void) {
    TEST_ASSERT_EQUAL_INT(0, pfs_commands_register(multiplecommands, 2));

    const char *valid_inputs[] = {
            "help",          // 0
            "help command0", // 1
            "help command1", // 2
            "help command1 subcommand11", // 3
    };

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(0, pfs_handle_shell_input(valid_inputs[0]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Available commands:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "command0" PFS_IO_BOLD_OFF " - command0 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "command1" PFS_IO_BOLD_OFF " - command1 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "help" PFS_IO_BOLD_OFF " - display help message for a specified command\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "helptree" PFS_IO_BOLD_OFF " - display help message tree for a specified command\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(0, pfs_handle_shell_input(valid_inputs[1]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Description:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB "command0 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Avalable subcommands:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB "no subcommands available for 'command0'\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(0, pfs_handle_shell_input(valid_inputs[2]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Description:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB "command1 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Avalable subcommands:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "subcommand10" PFS_IO_BOLD_OFF " - subcommand10 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "subcommand11" PFS_IO_BOLD_OFF " - subcommand11 description\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(0, pfs_handle_shell_input(valid_inputs[3]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Description:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB "subcommand11 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Avalable subcommands:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "subcommand00" PFS_IO_BOLD_OFF " - subcommand00 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "subcommand01" PFS_IO_BOLD_OFF " - subcommand01 description\n",
            utils_get_out_string_immediately_buffer());

    const char *invalid_inputs[] = {
            "help command2", // 0
    };

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[0]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "command2: command not found\n"
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "type 'help' for a list of available commands\n",
            utils_get_out_string_immediately_buffer());
};

void HandleHeltreepCommand(void) {
    TEST_ASSERT_EQUAL_INT(0, pfs_commands_register(multiplecommands, 2));

    const char *valid_inputs[] = {
            "helptree",          // 0
            "helptree command0", // 1
            "helptree command1", // 2
            "helptree command1 subcommand11", // 3
    };

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(0, pfs_handle_shell_input(valid_inputs[0]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Available commands:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "command0" PFS_IO_BOLD_OFF " - command0 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "command1" PFS_IO_BOLD_OFF " - command1 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_TAB PFS_IO_BOLD_ON "subcommand10" PFS_IO_BOLD_OFF " - subcommand10 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_TAB PFS_IO_BOLD_ON "subcommand11" PFS_IO_BOLD_OFF " - subcommand11 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_TAB PFS_IO_TAB PFS_IO_BOLD_ON "subcommand00" PFS_IO_BOLD_OFF " - subcommand00 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_TAB PFS_IO_TAB PFS_IO_BOLD_ON "subcommand01" PFS_IO_BOLD_OFF " - subcommand01 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_TAB PFS_IO_TAB PFS_IO_TAB PFS_IO_BOLD_ON "subcommand" PFS_IO_BOLD_OFF " - subcommand description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "help" PFS_IO_BOLD_OFF " - display help message for a specified command\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "helptree" PFS_IO_BOLD_OFF " - display help message tree for a specified command\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(0, pfs_handle_shell_input(valid_inputs[1]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Description:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB "command0 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Avalable subcommands:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB "no subcommands available for 'command0'\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(0, pfs_handle_shell_input(valid_inputs[2]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Description:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB "command1 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Avalable subcommands:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "subcommand10" PFS_IO_BOLD_OFF " - subcommand10 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "subcommand11" PFS_IO_BOLD_OFF " - subcommand11 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_TAB PFS_IO_BOLD_ON "subcommand00" PFS_IO_BOLD_OFF " - subcommand00 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_TAB PFS_IO_BOLD_ON "subcommand01" PFS_IO_BOLD_OFF " - subcommand01 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_TAB PFS_IO_TAB PFS_IO_BOLD_ON "subcommand" PFS_IO_BOLD_OFF " - subcommand description\n",
            utils_get_out_string_immediately_buffer());

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(0, pfs_handle_shell_input(valid_inputs[3]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Description:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB "subcommand11 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN "Avalable subcommands:\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "subcommand00" PFS_IO_BOLD_OFF " - subcommand00 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_BOLD_ON "subcommand01" PFS_IO_BOLD_OFF " - subcommand01 description\n"
            PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_TAB PFS_IO_TAB PFS_IO_BOLD_ON "subcommand" PFS_IO_BOLD_OFF " - subcommand description\n",
            utils_get_out_string_immediately_buffer());

    const char *invalid_inputs[] = {
            "helptree command2", // 0
    };

    utils_reset_out_string_immediately_buffer();
    TEST_ASSERT_EQUAL_INT(1, pfs_handle_shell_input(invalid_inputs[0]));
    TEST_ASSERT_EQUAL_STRING(
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "command2: command not found\n"
            PFS_IO_SHELL_MESSAGE_WRN_BEGIN "type 'help' for a list of available commands\n",
            utils_get_out_string_immediately_buffer());
}
// clang-format on

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(HandleInputSingleCommand);
    RUN_TEST(HandleInputMultipleCommand);
    RUN_TEST(HandleHelpCommand);
    RUN_TEST(HandleHeltreepCommand);

    return UNITY_END();
}
