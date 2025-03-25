/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <string.h>
#include "hdf_log.h"
#include "osal_mem.h"
#include "uart_if.h"

#define HDF_LOG_TAG hello_uart_dispatch
#define UART_PORT   4

int main(void)
{
    const char *info = " HELLO UART! ";
    char rxbuf[10] = {0};
    int ret = 0;
    char len = 0;

    DevHandle handle = UartOpen(UART_PORT);
    if (handle == NULL) {
        HDF_LOGE("Failed to open uart %d", UART_PORT);
        return HDF_FAILURE;
    }

    ret = UartWrite(handle, (uint8_t *)info, strlen(info));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Failed to send data to uart");
    }

    //不清楚其他串口怎样接，连接后调用此接口读取
    len = 10;
    ret = UartRead(handle, rxbuf, len); //len为读取指定的长度，ret表示接收到的实际长度
    if (ret != 0) {
        len = ret;
        for (int i = 0; i< len; i++) {
            printf("%x", rxbuf[i]);
        }
        printf("\n");
    }
    UartClose(handle);
    return HDF_SUCCESS;
}