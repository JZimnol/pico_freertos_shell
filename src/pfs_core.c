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

#include <pico/stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

#include <pico_freertos_shell/init.h>

#include "pfs_cmd_queue.h"
#include "pfs_handle_shell_input.h"
#include "pfs_io.h"

static QueueHandle_t m_msg_queue;
static SemaphoreHandle_t m_msg_mutex;

static QueueHandle_t m_cmd_queue;
static SemaphoreHandle_t m_cmd_sem;

static bool m_initialized = false;
static size_t m_dropped_messages;

#define PFS_MAIN_STACK_SIZE (1500U)
static StackType_t m_pfs_main_task_stack[PFS_MAIN_STACK_SIZE];
static StaticTask_t m_psf_main_task_buffer;

#define PFS_CMD_HANDLER_STACK_SIZE (1500U)
static StackType_t m_pfs_cmd_handler_task_stack[PFS_CMD_HANDLER_STACK_SIZE];
static StaticTask_t m_psf_cmd_handler_task_buffer;

static void drop_latest_message(void) {
    char *msg = NULL;
    xQueueReceive(m_msg_queue, &msg, portMAX_DELAY);
    if (msg == NULL) {
        return;
    }
    free(msg);
    m_dropped_messages++;
}

static void handle_dropped_messages(void) {
    if (m_dropped_messages == 0) {
        return;
    }
    pfs_io_remove_shell_prompt();
    pfs_io_printf_immediately(
            PFS_IO_ERR_COLOR PFS_IO_BOLD_ON
            "--- %d messages dropped ---\n" PFS_IO_COLOR_RESET PFS_IO_BOLD_OFF,
            m_dropped_messages);
    m_dropped_messages = 0;
    pfs_io_restore_shell_prompt();
}

bool _pfs_is_initialized(void) {
    // don't care about atomicity here
    return m_initialized;
}

static void pfs_main_task(void *pvParameters) {
    m_initialized = true;
    while (true) {
        handle_dropped_messages();
        bool shell_promt_removed = false;
        while (true) {
            char *msg;
            xSemaphoreTake(m_msg_mutex, portMAX_DELAY);
            BaseType_t ret = xQueueReceive(m_msg_queue, &msg, 0);
            xSemaphoreGive(m_msg_mutex);
            if (ret != pdTRUE) {
                break;
            }
            if (!shell_promt_removed) {
                pfs_io_remove_shell_prompt();
                shell_promt_removed = true;
            }
            pfs_io_puts_immediately(msg);
            free(msg);
        }
        if (shell_promt_removed) {
            pfs_io_restore_shell_prompt();
        }
        while (true) {
            char c = getchar_timeout_us(10);
            if (c < 0 || c > 127) {
                break;
            }
            pfs_io_handle_input_char(c);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

static void pfs_cmd_handler_task(void *pvParameters) {
    while (1) {
        pfs_cmd_args_t cmd_args;
        if (xQueueReceive(m_cmd_queue, &cmd_args, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        if (cmd_args.handler == NULL) {
            continue;
        }
        printf(PFS_IO_SHELL_MESSAGE_INF_BEGIN "entering command handler\n");
        xSemaphoreTake(m_cmd_sem, portMAX_DELAY);
        cmd_args.handler(cmd_args.argc, cmd_args.argv);
        xSemaphoreGive(m_cmd_sem);
        printf(PFS_IO_SHELL_MESSAGE_INF_BEGIN "leaving command handler\n");
    }
}

void pfs_init(void) {
    m_msg_queue = xQueueCreate(PFS_MSG_QUEUE_SIZE, sizeof(void *));
    if (m_msg_queue == NULL) {
        PFS_SHELL_LOG(ERR, "msg_queue initialization failed\n");
        exit(1);
    }
    m_msg_mutex = xSemaphoreCreateMutex();
    if (m_msg_mutex == NULL) {
        PFS_SHELL_LOG(ERR, "msg_mutex initialization failed\n");
        exit(1);
    }
    m_cmd_queue = pfs_cmd_queue_create();
    if (m_cmd_queue == NULL) {
        PFS_SHELL_LOG(ERR, "cmd_queue initialization failed\n");
        exit(1);
    }
    m_cmd_sem = pfs_cmd_queue_create_sem();
    if (m_cmd_sem == NULL) {
        PFS_SHELL_LOG(ERR, "cmd_sem initialization failed\n");
        exit(1);
    }

    (void) xTaskCreateStatic(pfs_cmd_handler_task, "PfsCmdHandlerTask",
                             PFS_CMD_HANDLER_STACK_SIZE, NULL,
                             tskIDLE_PRIORITY + PFS_TASK_PRIORITY,
                             m_pfs_cmd_handler_task_stack,
                             &m_psf_cmd_handler_task_buffer);
    (void) xTaskCreateStatic(pfs_main_task, "PfsMainTask", PFS_MAIN_STACK_SIZE,
                             NULL, tskIDLE_PRIORITY + PFS_TASK_PRIORITY,
                             m_pfs_main_task_stack, &m_psf_main_task_buffer);
}

void _pfs_append_queue(const char *buffer, uint32_t buffer_size) {
    if (buffer_size == 0) {
        return;
    }
    char *msg = malloc(buffer_size + 1);
    if (msg == NULL) {
        PFS_SHELL_LOG(ERR, "failed to allocate memory for message\n");
        return;
    }
    memset(msg, 0, buffer_size + 1);
    memcpy(msg, buffer, buffer_size);

    xSemaphoreTake(m_msg_mutex, portMAX_DELAY);
    if (uxQueueSpacesAvailable(m_msg_queue) == 0) {
        drop_latest_message();
    }
    xQueueSendToBack(m_msg_queue, &msg, portMAX_DELAY);
    xSemaphoreGive(m_msg_mutex);
}
