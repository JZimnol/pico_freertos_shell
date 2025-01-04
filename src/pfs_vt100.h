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

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifdef PFS_TERMINAL_TYPE_VT100
#define PFS_VT100_BOLD_ON "\x1B[1m"
#define PFS_VT100_BOLD_OFF "\x1B[0m"
#define PFS_VT100_CLEAR_LINE "\x1B[2K\r"
#define PFS_VT100_MOVE_LEFT "\033[D"
#define PFS_VT100_MOVE_RIGHT "\033[C"
#define PFS_VT100_ERASE_LINE_RIGHT "\033[0K"

#define PFS_VT100_COLOR_RESET "\033[0m"
#define PFS_VT100_COLOR_BLACK "\033[30m"
#define PFS_VT100_COLOR_RED "\033[31m"
#define PFS_VT100_COLOR_GREEN "\033[32m"
#define PFS_VT100_COLOR_YELLOW "\033[33m"
#define PFS_VT100_COLOR_BLUE "\033[34m"
#define PFS_VT100_COLOR_MAGENTA "\033[35m"
#define PFS_VT100_COLOR_CYAN "\033[36m"
#define PFS_VT100_COLOR_WHITE "\033[37m"

#define PFS_VT100_COLOR_BRIGHT_BLACK "\033[90m"
#define PFS_VT100_COLOR_BRIGHT_RED "\033[91m"
#define PFS_VT100_COLOR_BRIGHT_GREEN "\033[92m"
#define PFS_VT100_COLOR_BRIGHT_YELLOW "\033[93m"
#define PFS_VT100_COLOR_BRIGHT_BLUE "\033[94m"
#define PFS_VT100_COLOR_BRIGHT_MAGENTA "\033[95m"
#define PFS_VT100_COLOR_BRIGHT_CYAN "\033[96m"
#define PFS_VT100_COLOR_BRIGHT_WHITE "\033[97m"

#define PFS_IO_PROMPT_COLOR PFS_VT100_COLOR_BRIGHT_GREEN
#define PFS_IO_INF_COLOR PFS_VT100_COLOR_BRIGHT_BLUE
#define PFS_IO_WRN_COLOR PFS_VT100_COLOR_BRIGHT_YELLOW
#define PFS_IO_ERR_COLOR PFS_VT100_COLOR_BRIGHT_RED
#define PFS_IO_COLOR_RESET PFS_VT100_COLOR_RESET
#define PFS_IO_BOLD_ON PFS_VT100_BOLD_ON
#define PFS_IO_BOLD_OFF PFS_VT100_BOLD_OFF
#define PFS_IO_MOVE_LEFT PFS_VT100_MOVE_LEFT
#define PFS_IO_MOVE_RIGHT PFS_VT100_MOVE_RIGHT
#define PFS_IO_ERASE_LINE_RIGHT PFS_VT100_ERASE_LINE_RIGHT
#define PFS_IO_CLEAR_LINE PFS_VT100_CLEAR_LINE

#endif // PFS_TERMINAL_TYPE_VT100

#ifdef __cplusplus
}
#endif // __cplusplus
