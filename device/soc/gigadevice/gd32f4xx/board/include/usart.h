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

#include "gd32f470z_eval.h"

#define USART_REC_LEN      4096

typedef struct
{
  uint16_t buf[USART_REC_LEN];
  uint16_t head;
  uint16_t tail;
  uint16_t count;
} usart_group_type;

extern usart_group_type usart_group_struct;

void ota_uart_init(uint32_t baudrate);
uint8_t uart_data_take(void);

#endif
