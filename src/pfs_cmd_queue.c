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

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#include "pfs_cmd_queue.h"
#include "pfs_utils.h"

PFS_STATIC_ASSERT(sizeof(QueueHandle_t) == sizeof(void *),
                  QueueHandleSizeIsNotEqualToVoidPtr);
PFS_STATIC_ASSERT(sizeof(SemaphoreHandle_t) == sizeof(void *),
                  SemaphoreHandleSizeIsNotEqualToVoidPtr);

static QueueHandle_t m_cmd_queue = NULL;
static SemaphoreHandle_t m_cmd_queue_sem = NULL;

void *pfs_cmd_queue_create(void) {
    m_cmd_queue = xQueueCreate(1, sizeof(pfs_cmd_args_t));
    return m_cmd_queue;
}

int pfs_cmd_queue_add(const pfs_cmd_args_t *cmd_args) {
    if (xQueueSend(m_cmd_queue, cmd_args, 0) != pdTRUE) {
        return 1;
    }
    return 0;
}

void *pfs_cmd_queue_create_sem(void) {
    m_cmd_queue_sem = xSemaphoreCreateBinary();
    if (m_cmd_queue_sem) {
        xSemaphoreGive(m_cmd_queue_sem);
    }
    return m_cmd_queue_sem;
}

int pfs_cmd_queue_is_in_handler(void) {
    return uxSemaphoreGetCount(m_cmd_queue_sem) == 0;
}
