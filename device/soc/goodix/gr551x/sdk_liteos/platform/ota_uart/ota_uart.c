/**
  **************************************************************************
  * @file     usart.c
  * @version  v2.0.5
  * @date     2021-12-17
  * @brief    usart program
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to 
  * download from Artery official website is the copyrighted work of Artery. 
  * Artery authorizes customers to use, copy, and distribute the BSP 
  * software and its related documentation for the purpose of design and 
  * development in conjunction with Artery microcontrollers. Use of the 
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "gr55xx.h"
#include "ota_uart.h"
#include "osal_irq.h"
#include "system_gr55xx.h"
#include "los_task.h"
#include "app_log.h"
#include "cmsis_os2.h"

usart_group_type usart_group_struct;

volatile uint8_t g_tdone = 0;
volatile uint8_t g_rdone = 0;

static bool ota_uart_initialized = false;

static void ota_uart_evt_handler(app_uart_evt_t *p_evt)
{
    switch (p_evt->type)
    {
        case APP_UART_EVT_TX_CPLT:
            g_tdone = 1;
            break;
        case APP_UART_EVT_RX_DATA:
            g_rdone = 1;
            usart_group_struct.count = p_evt->data.size;
            APP_LOG_INFO("%s;data_size=%d\r\n",__func__,  p_evt->data.size);
            break;
        case APP_UART_EVT_ERROR:
            g_tdone = 1;
            g_rdone = 1;
            break;
    
    default:
        break;
    }
}

/**
  * @brief  init ota_uart.
  * @param  none
  * @retval none
  */
void ota_uart_init(uint32_t baudrate)
{
    app_uart_tx_buf_t ota_uart_buffer;
    app_uart_params_t ota_uart_param;

    ota_uart_buffer.tx_buf              = NULL;
    ota_uart_buffer.tx_buf_size         = 0;

    ota_uart_param.id                   = OTA_UART_ID;
    ota_uart_param.init.baud_rate       = baudrate;
    ota_uart_param.init.data_bits       = UART_DATABITS_8;
    ota_uart_param.init.stop_bits       = UART_STOPBITS_1;
    ota_uart_param.init.parity          = UART_PARITY_NONE;
    ota_uart_param.init.hw_flow_ctrl    = UART_HWCONTROL_NONE;
    ota_uart_param.init.rx_timeout_mode = UART_RECEIVER_TIMEOUT_ENABLE;
    ota_uart_param.pin_cfg.rx.type      = OTA_UART_RX_IO_TYPE;
    ota_uart_param.pin_cfg.rx.pin       = OTA_UART_RX_PIN;
    ota_uart_param.pin_cfg.rx.mux       = OTA_UART_RX_PINMUX;
    ota_uart_param.pin_cfg.rx.pull      = OTA_UART_RX_PULL;
    ota_uart_param.pin_cfg.tx.type      = OTA_UART_TX_IO_TYPE;
    ota_uart_param.pin_cfg.tx.pin       = OTA_UART_TX_PIN;
    ota_uart_param.pin_cfg.tx.mux       = OTA_UART_TX_PINMUX;
    ota_uart_param.pin_cfg.tx.pull      = OTA_UART_TX_PULL;
    ota_uart_param.use_mode.type        = APP_UART_TYPE_INTERRUPT;
    app_uart_init(&ota_uart_param, ota_uart_evt_handler, &ota_uart_buffer);
    ota_uart_initialized = true;
    APP_LOG_INFO("init success\r\n");
}
uint8_t uart_data_send(int8_t *p_data, uint16_t size)
{
    hal_status_t err_code =app_uart_transmit_sync(OTA_UART_ID, p_data, size, 5000);
    APP_LOG_INFO("%s:%d,err_code %d\r\n", __func__, __LINE__, err_code);
    return err_code;
}
uint8_t uart_data_take(uint8_t *str)
{
    if (ota_uart_initialized != true) {
        APP_LOG_ERROR("uart is not initialized\r\n");
        return -1;
    }

    g_rdone = 0;
    app_uart_receive_async(OTA_UART_ID, str, USART_REC_LEN);
    APP_LOG_INFO("%s:%d, str: %s\r\n", __func__, __LINE__, str);
    while (g_rdone == 0);
    return  usart_group_struct.count;
}

