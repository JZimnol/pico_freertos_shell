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

#include <string.h>

#include <unity.h>

#include <test_utils.h>

#include <pico_freertos_shell/commands.h>

#include <pfs_handle_shell_input.h>
#include <pfs_io.h>
#include <pfs_utils.h>

void setUp(void) {
    utils_reset_out_string_immediately_buffer();
    pfs_reset_commands();
}

void tearDown(void) {}

static void dummy_cmd_handler(int argc, char **argv) {
    (void) argc;
    (void) argv;
}

static const pfs_command_t subcommand[] = {
        PFS_COMMAND_INITIALIZER(longestsubcommand,
                                "longestsubcommand description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
};

static const pfs_command_t subcommands0[] = {
        PFS_COMMAND_INITIALIZER(subcommand00,
                                "subcommand00 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
        PFS_COMMAND_INITIALIZER(subcommand01,
                                "subcommand01 description",
                                PFS_SUBCOMMANDS(subcommand,
                                                PFS_ARRAY_SIZE(subcommand))),
};

static const pfs_command_t subcommands1[] = {
        PFS_COMMAND_INITIALIZER(subcommand10,
                                "subcommand10 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
        PFS_COMMAND_INITIALIZER(subcommand11,
                                "subcommand11 description",
                                PFS_SUBCOMMANDS(subcommands0,
                                                PFS_ARRAY_SIZE(subcommands0))),
};

static const pfs_command_t multiplecommands[] = {
        PFS_COMMAND_INITIALIZER(command,
                                "command0 description",
                                PFS_COMMAND_HANDLER(dummy_cmd_handler)),
        PFS_COMMAND_INITIALIZER(command1,
                                "command1 description",
                                PFS_SUBCOMMANDS(subcommands1,
                                                PFS_ARRAY_SIZE(subcommands1))),
};

#define TEST_PREPARE(BeginningString)                       \
    pfs_io_handle_input_char(PFS_IO_CHAR_CTRL_C);           \
    pfs_io_handle_input_char(PFS_IO_CHAR_ENTER);            \
    utils_reset_out_string_immediately_buffer();            \
    memset(beginning, 0, sizeof(beginning));                \
    strncpy(beginning, BeginningString, sizeof(beginning)); \
    for (size_t i = 0; i < PFS_MAX_INPUT_SIZE - 1; i++) {   \
        pfs_io_handle_input_char(beginning[i]);             \
    }                                                       \
    utils_reset_out_string_immediately_buffer();            \
    pfs_io_handle_input_char(PFS_IO_CHAR_TAB);

#define TEST_ASSERT_OUTPUT_EQUAL(...)              \
    memset(output, 0, sizeof(output));             \
    snprintf(output, sizeof(output), __VA_ARGS__); \
    TEST_ASSERT_EQUAL_STRING(output, utils_get_out_string_immediately_buffer());

#define TEST_ASSERT_OUTPUT_EMPTY() \
    TEST_ASSERT_EQUAL_STRING("", utils_get_out_string_immediately_buffer());

void HandleTabInputCharacter(void) {
    TEST_ASSERT_EQUAL_INT(0, pfs_commands_register(multiplecommands, 2));

    char beginning[PFS_MAX_INPUT_SIZE];
    memset(beginning, 0, sizeof(beginning));
    char output[1024];
    memset(output, 0, sizeof(output));

    // clang-format off
    TEST_PREPARE("");
    TEST_ASSERT_OUTPUT_EQUAL(
        "\n"
        PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_BOLD_ON "command" PFS_IO_TAB "command1" PFS_IO_TAB "help" PFS_IO_TAB "helptree" PFS_IO_TAB PFS_IO_BOLD_OFF "\n"
        PFS_IO_SHELL_PROMPT "%s", beginning);

    TEST_PREPARE("helpt");
    TEST_ASSERT_OUTPUT_EQUAL("ree ");

    TEST_PREPARE("hel");
    TEST_ASSERT_OUTPUT_EQUAL("p");

    TEST_PREPARE("help");
    TEST_ASSERT_OUTPUT_EQUAL(
        "\n"
        PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_BOLD_ON "help" PFS_IO_TAB "helptree" PFS_IO_TAB PFS_IO_BOLD_OFF "\n"
        PFS_IO_SHELL_PROMPT "%s", beginning);

    TEST_PREPARE("command");
    TEST_ASSERT_OUTPUT_EQUAL(
        "\n"
        PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_BOLD_ON "command" PFS_IO_TAB "command1" PFS_IO_TAB PFS_IO_BOLD_OFF "\n"
        PFS_IO_SHELL_PROMPT "%s", beginning);

    TEST_PREPARE("command ");
    TEST_ASSERT_OUTPUT_EMPTY();

    TEST_PREPARE("command1 ");
    TEST_ASSERT_OUTPUT_EQUAL("subcommand1");

    TEST_PREPARE("command1 s");
    TEST_ASSERT_OUTPUT_EQUAL("ubcommand1");

    TEST_PREPARE("command1 subcommand1");
    TEST_ASSERT_OUTPUT_EQUAL(
        "\n"
        PFS_IO_SHELL_MESSAGE_INF_BEGIN PFS_IO_BOLD_ON "subcommand10" PFS_IO_TAB "subcommand11" PFS_IO_TAB PFS_IO_BOLD_OFF "\n"
        PFS_IO_SHELL_PROMPT "%s", beginning);

    TEST_PREPARE("command1 subcommand11 subcommand00");
    TEST_ASSERT_OUTPUT_EQUAL(" ");

    TEST_PREPARE("command1 subcommand11 subcommand00 ");
    TEST_ASSERT_OUTPUT_EMPTY();

    TEST_PREPARE("command1 subcommand11 subcommand01 ");
    TEST_ASSERT_OUTPUT_EQUAL("longestsubcommand ");

    TEST_PREPARE("command1 subcommand11 nonexisting ");
    TEST_ASSERT_OUTPUT_EMPTY();

    TEST_PREPARE("command1 subcommand11 nonexisting");
    TEST_ASSERT_OUTPUT_EMPTY();

    TEST_PREPARE("nonexisting");
    TEST_ASSERT_OUTPUT_EMPTY();

    TEST_PREPARE("nonexisting command");
    TEST_ASSERT_OUTPUT_EMPTY();
    // clang-format on
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(HandleTabInputCharacter);

    return UNITY_END();
}
