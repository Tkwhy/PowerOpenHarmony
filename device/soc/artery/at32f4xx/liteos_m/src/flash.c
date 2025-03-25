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
#include "usart.h"

/**
  * @brief  flash erase/program operation.
  * @note   follow 2kb operation of ont time
  * @param  none
  * @retval none
  */
error_status flash_2kb_write(uint32_t write_addr, uint8_t *pbuffer)
{
    uint16_t index, write_data, read_data;  
    flash_unlock();
    flash_sector_erase(write_addr);
    for(index = 0; index < 2048; index += 2)
    {
        write_data = (pbuffer[index+1] << 8) + pbuffer[index];
        flash_halfword_program(write_addr+index, write_data);
        read_data = *(uint16_t*)(write_addr+index);
        if(read_data != write_data)
            return ERROR;
    }
    flash_lock();
    return SUCCESS;
}

/**
  * @brief  check flash upgrade flag.
  * @param  none
  * @retval none
  */
flag_status flash_upgrade_flag_read(void)
{
    if((*(uint32_t*)OTA_UPGRADE_FLAG_ADDR) == OTA_UPGRADE_FLAG)
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
    flash_unlock();
    flash_sector_erase(OTA_UPGRADE_FLAG_ADDR);
    flash_word_program(OTA_UPGRADE_FLAG_ADDR, OTA_UPGRADE_FLAG);
    flash_lock();
}
