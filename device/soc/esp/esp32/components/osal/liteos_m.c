/*
 * Copyright (c) 2022 Hunan OpenValley Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <stdlib.h>
#include "esp_osal.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "los_config.h"
#include "los_task.h"
#include "los_debug.h"
#include "los_queue.h"
#include "los_memory.h"
#include "esp_osal/event_groups.h"
#include "esp_rom_uart.h"
#include "xtensa_rtos.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <errno.h>
#include "hal/uart_ll.h"

extern UINT64 OsTickCount;
// =================================== 调试相关
extern BOOL g_taskScheduled;
static UINT32 s_LogMuxHandle = 0;
extern UINT16 g_losTaskLock;
static void s_vprintf(const char* fmt, va_list ap)
{
    int len;
    uint8_t taskLock;
    static char buf[2][128 * 3];
    char* pbuf;
    if (xPortInterruptedFromISRContext() || g_losTaskLock || (!g_taskScheduled))
    {
        taskLock = 1;
        pbuf = buf[1];
    }
    else
    {
        taskLock = 0;
        pbuf = buf[0];
    }
    if (!taskLock)
    {
        while (!s_LogMuxHandle)
        {
            LOS_MuxCreate(&s_LogMuxHandle);
            LOS_TaskDelay(1);
        }
        LOS_MuxPend(s_LogMuxHandle, LOS_WAIT_FOREVER);
    }
    len = vsnprintf(pbuf, sizeof(buf[0]) - 4, fmt, ap);
    if (len > 0)
    {
        uint16_t fill_len;
        for (fill_len = uart_ll_get_txfifo_len(&UART0); fill_len < len;)
        {
            if (fill_len)
            {
                uart_ll_write_txfifo(&UART0, (uint8_t*)pbuf, fill_len);
                len -= fill_len;
                pbuf += fill_len;
            }
            if (!taskLock)
                LOS_TaskDelay(1); // LOS_TaskYield();
            fill_len = uart_ll_get_txfifo_len(&UART0);
        }
        if (len > 0)
            uart_ll_write_txfifo(&UART0, (uint8_t*)pbuf, len);
        // while(!uart_ll_is_tx_idle(&UART0))LOS_TaskYield();
    }
    if (!taskLock)
        LOS_MuxPost(s_LogMuxHandle);
}

// 获取当前滴答时间
TickType_t IRAM_ATTR xTaskGetTickCount(void)
{
    return (TickType_t)OsTickCount;
}

// 获取当前滴答时间(中断内)
TickType_t IRAM_ATTR xTaskGetTickCountFromISR(void)
{
    return (TickType_t)OsTickCount;
}

#ifndef MALLOC_CAP_8BIT
#define MALLOC_CAP_8BIT (1 << 2)
void* heap_caps_malloc(size_t size, uint32_t caps);
void heap_caps_free(void* ptr);
void* heap_caps_aligned_alloc(size_t alignment, size_t size, uint32_t caps);
#endif

UINT32 __wrap_OsMemSystemInit(VOID)
{
    return LOS_OK;
}

IRAM_ATTR VOID* __wrap_LOS_MemAlloc(VOID* pool, UINT32 size)
{
    return heap_caps_malloc(size, MALLOC_CAP_8BIT);
}

IRAM_ATTR UINT32 __wrap_LOS_MemFree(VOID* pool, VOID* ptr)
{
    if (!ptr) {
        return LOS_NOK;
    }
    else {
        heap_caps_free(ptr);
        return LOS_OK;
    }
}

IRAM_ATTR VOID* __wrap_LOS_MemAllocAlign(VOID* pool, UINT32 size, UINT32 boundary)
{
    return heap_caps_aligned_alloc(boundary, size, MALLOC_CAP_8BIT);
}
