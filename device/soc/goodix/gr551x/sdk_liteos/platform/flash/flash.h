/**
  **************************************************************************
  * @file     flash.h
  * @version  v2.0.5
  * @date     2021-12-17
  * @brief    flash header file
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
  
#ifndef __FLASH_H__
#define __FLASH_H__

#include "gr55xx.h"
#include "hal_flash.h"
#include "stdbool.h"

#define APP_START_ADDR                   0x01011000
#define OTA_UPGRADE_FLAG_ADDR            (APP_START_ADDR - 0x1000)  //存储升级标志地址

void security_disable(void);
void security_state_recovery(void);
error_status_t flash_4kb_write(uint32_t write_addr, uint8_t *pbuffer);
flag_status_t flash_upgrade_flag_read(void);
void write_upgrade_flag(void);

#endif
