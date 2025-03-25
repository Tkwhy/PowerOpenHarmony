/*
 * Copyright (c) 2021 GOODIX.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "gr55xx_sys.h"
#include "scatter_common.h"
#include "flash_scatter_config.h"
#include "user_app.h"
#include "app_callback.h"
#include "device_sys_info.h"

#define BLE_TASK_STACK_SIZE   4096
#define BLE_TASK_PRIO         25
#define MS_1000     1000
#define MS_5000     5000

/**@brief Stack global variables for Bluetooth protocol stack. */
STACK_HEAP_INIT(heaps_table);

static app_callback_t s_app_ble_callback = {
    .app_ble_init_cmp_callback  = ble_init_cmp_callback,
    .app_gap_callbacks          = &app_gap_callbacks,
    .app_gatt_common_callback   = &app_gatt_common_callback,
    .app_gattc_callback         = &app_gattc_callback,
    .app_sec_callback           = &app_sec_callback,
};

static void *BLE_Task(const char *arg)
{
    (void)arg;

    printf("Initialize the BLE stack.\r\n");

    /* init ble stack */
    ble_stack_init(&s_app_ble_callback, &heaps_table);

    while (1) {
        osDelay(MS_1000);
    }
}

void BLE_TaskEntry(void)
{
    osThreadAttr_t attr;

    attr.name       = "BLE_Task";
    attr.attr_bits  = 0U;
    attr.cb_mem     = NULL;
    attr.cb_size    = 0U;
    attr.stack_mem  = NULL;
    attr.stack_size = BLE_TASK_STACK_SIZE;
    attr.priority   = BLE_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)BLE_Task, NULL, &attr) == NULL) {
        printf("[HelloDemo] Failed to create HelloTask!\n");
    }
}
SYS_RUN(BLE_TaskEntry);

#define UART_STACK_SIZE        0x1000
#define UART_TASK_NAME         "uart_test_task"
#define UART_TASK_PRIORITY     20
uint8_t cmd_buf[2048];
static uint8_t rcv_data_flag = 0;
uint32_t cmd_buf_len = 0;

static void* UartTaskEntry(void* arg)
{
    int master_connect_handle;
    int app_master_connect_flag;
    int set_adv_name_flag = 0;
    while (1) {
        cmd_buf_len = uart_data_take(cmd_buf);
        printf("%x\r\n",__func__, cmd_buf[0]);
        if(app_master_connect_flag_get() == 1)
        {
            app_master_connect_get();
            uint32_t error =  gus_tx_data_send(master_connect_handle, cmd_buf, cmd_buf_len);
            printf("gus_tx_data_send eror = %d", error);
        }
        else
        {
            app_get_adv_name(cmd_buf, strlen(cmd_buf)-1);
        }

        osDelay(1000);
    }
}

void uart_task(void)
{
    printf("usart_task\r\n");
    osThreadAttr_t attr;

    attr.name = UART_TASK_NAME;
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = UART_STACK_SIZE;
    attr.priority = UART_TASK_PRIORITY;
    ota_uart_init(115200);

    if (osThreadNew((osThreadFunc_t)UartTaskEntry, NULL, &attr) == NULL) {
        printf("Failed to create thread1!\n");
    }
}
SYS_RUN(uart_task);

#define DEVICE_SYS_INFO_STACK_SIZE 0x1000
#define DEVICE_SYS_INFO_TASK_NAME "device_system_info_get_task"
#define DEVICE_SYS_INFO_TASK_PRIORITY 25

void DeviceSystemInfoGetInit(void)
{
    printf("usart_task\r\n");
    osThreadAttr_t attr;

    attr.name = DEVICE_SYS_INFO_TASK_NAME;
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = DEVICE_SYS_INFO_STACK_SIZE;
    attr.priority = DEVICE_SYS_INFO_TASK_PRIORITY;

    if (osThreadNew((osThreadFunc_t)DeviceSystemInfoGetEntry, NULL, &attr) == NULL) {
        printf("Failed to create thread1!\n");
    }

}
SYS_RUN(DeviceSystemInfoGetInit);