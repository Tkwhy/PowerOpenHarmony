/**
  **************************************************************************
  * @file     flash.c
  * @version  v2.0.5
  * @date     2021-12-17
  * @brief    flash program
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

#include "flash.h"
#include "ota_uart.h"
#include "app_log.h"
bool s_flash_security_status = false;
uint8_t ota_upgrade_flag[] = {0x41,0x54,0x4B,0x38};      //升级标志值，在bootloader中check

/**
  * @brief  disable flash security.
  * @param  none
  * @retval none
  */
void security_disable(void)
{
    uint32_t sys_security = sys_security_enable_status_check();
    if(sys_security)
    {
        s_flash_security_status = hal_flash_get_security();
        hal_flash_set_security(false);
    }
}

/**
  * @brief  recovery flash security.
  * @param  none
  * @retval none
  */
void security_state_recovery(void)
{
    uint32_t sys_security = sys_security_enable_status_check();
    if(sys_security)
    {
        hal_flash_set_security(s_flash_security_status);
    }
}

/**
  * @brief  flash erase/program operation.
  * @note   follow 4kb operation of ont time
  * @param  none
  * @retval none
  */
error_status_t flash_4kb_write(uint32_t write_addr, uint8_t *pbuffer)
{
    uint16_t index, write_data, read_data;  
    security_disable();
    hal_flash_erase(write_addr, 4096);
    int ret =hal_flash_write(write_addr , pbuffer, 4096);
    if(ret != 4096)
    {
        return ERROR;
    }
    security_state_recovery();
    return SUCCESS;
}

/**
  * @brief  check flash upgrade flag.
  * @param  none
  * @retval none
  */
flag_status_t flash_upgrade_flag_read(void)
{
    uint8_t flag_buf[sizeof(ota_upgrade_flag)];
    hal_flash_read(OTA_UPGRADE_FLAG_ADDR, flag_buf, sizeof(flag_buf));
    if(flag_buf[0] == ota_upgrade_flag[0] && flag_buf[1] == ota_upgrade_flag[1] && flag_buf[2] == ota_upgrade_flag[2] && flag_buf[3] == ota_upgrade_flag[3])
        return SET;
    else
        return RESET;
}

/**
  * @brief  write flash upgrade flag.
  * @param  none
  * @retval none
  */
void write_upgrade_flag(void)
{
    uint8_t buf[sizeof(ota_upgrade_flag)];
    APP_LOG_INFO("write flash upgrade flag.");
    security_disable();
    hal_flash_erase(OTA_UPGRADE_FLAG_ADDR, sizeof(ota_upgrade_flag));
    hal_flash_write(OTA_UPGRADE_FLAG_ADDR, ota_upgrade_flag, sizeof(ota_upgrade_flag));
    hal_flash_read(OTA_UPGRADE_FLAG_ADDR, buf, sizeof(buf));
    for (int i = 0; i < sizeof(buf); i++)
    {
      printf("%x",buf[i]);
    }
    
    security_state_recovery();
}
