/**
  **************************************************************************
  * @file     ota.h
  * @version  v2.0.5
  * @date     2021-12-17
  * @brief    ota header file
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

#ifndef __OTA_DEMO_H__
#define __OTA_DEMO_H__

#include "gr55xx.h"

/* app starting address */
#define APP_START_ADDR                   0x01011000
#define TEMPLATE_START_ADDR              0x01080000

/* the previous sector of app starting address is ota upgrade flag */
#define OTA_UPGRADE_FLAG_ADDR            (APP_START_ADDR - 0x1000)

/* ota set up the flag */
#define OTA_UPGRADE_FLAG         0x41544B38

//#define READ_BUF_LEN             2048
#define READ_BUF_LEN             4096


/**
  * @brief  cmd data group type
  */
typedef struct
{
    uint8_t cmd_head;
    uint8_t cmd_addr[4];
    uint8_t cmd_buf[READ_BUF_LEN];
    uint8_t cmd_check;
} cmd_data_group_type;

typedef struct
{
    uint8_t RX_HEAD_55;
    uint8_t RX_HEAD_AA;
    uint8_t RX_VER;
    uint8_t RX_CMD;
    uint8_t RX_LEN_H;
    uint8_t RX_LEN_L;
    uint8_t DATA_1;
    uint8_t DATA_2;
    uint8_t DATA_3;
    uint8_t DATA_4;
    uint8_t RX_SUM;
    uint32_t RX_ALL_SUM;
} ota_cmd_data_group_type;

void ota_init(void);
void ota_demo_task(void);

#endif
