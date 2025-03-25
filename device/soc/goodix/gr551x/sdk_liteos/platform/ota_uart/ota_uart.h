/**
  **************************************************************************
  * @file     usart.h
  * @version  v2.0.5
  * @date     2021-12-17
  * @brief    usart header file
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

#ifndef __USART_H
#define __USART_H

#include "app_uart.h"

#define USART_REC_LEN      8192

#define OTA_UART_ID                     APP_UART_ID_1
#define OTA_UART_TX_IO_TYPE             APP_IO_TYPE_NORMAL
#define OTA_UART_RX_IO_TYPE             APP_IO_TYPE_NORMAL
#define OTA_UART_TX_PIN                 APP_IO_PIN_9
#define OTA_UART_RX_PIN                 APP_IO_PIN_8
#define OTA_UART_TX_PINMUX              APP_IO_MUX_3
#define OTA_UART_RX_PINMUX              APP_IO_MUX_3
#define OTA_UART_TX_PULL                APP_IO_PULLUP
#define OTA_UART_RX_PULL                APP_IO_PULLUP
/**
  * @brief  usart group type
  */
typedef struct
{
  uint16_t buf[USART_REC_LEN];
  uint16_t head;
  uint16_t tail;
  uint16_t count;
} usart_group_type;

extern usart_group_type usart_group_struct;

void ota_uart_init(uint32_t baudrate);
uint8_t uart_data_send(int8_t *p_data, uint16_t size);
//uint8_t uart_data_take(uint8_t *str);

#endif
