/*
 * Copyright (c) 2022 Hihope Co., Ltd.
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

#ifndef __BSP_SPIFLASH_H__
#define __BSP_SPIFLASH_H__

#ifdef LOSCFG_DRIVERS_HDF_PLATFORM_SPI
#include "stdio.h"
#include "stdint.h"
#include "hdf_log.h"
#include "spi_if.h"

#define W25Q128_ERASE_GRAN              4096
#define W25QXX_MAX_ADDR                  (0x1000000UL)
#define W25QXX_TIMEOUT                          (100000UL)
#define W25QXX_PAGE_SIZE                 (256UL)
#define W25QXX_FLAG_BUSY                        (1UL << 0U)
// 指令表
#define W25X_WriteEnable        0x06 
#define W25X_WriteDisable        0x04 
#define W25X_ReadStatusReg1        0x05 
#define W25X_ReadStatusReg2        0x35 
#define W25X_ReadStatusReg3        0x15 
#define W25X_WriteStatusReg1    0x01 
#define W25X_WriteStatusReg2    0x31 
#define W25X_WriteStatusReg3    0x11 
#define W25X_ReadData            0x03 
#define W25X_FastReadData        0x0B 
#define W25X_FastReadDual        0x3B 
#define W25X_PageProgram        0x02 
#define W25X_BlockErase            0xD8 
#define W25X_SectorErase        0x20 
#define W25X_ChipErase            0xC7 
#define W25X_PowerDown            0xB9 
#define W25X_ReleasePowerDown    0xAB 
#define W25X_DeviceID            0xAB 
#define W25X_ManufactDeviceID    0x90 
#define W25X_JedecDeviceID        0x9F 
#define W25X_Enable4ByteAddr    0xB7
#define W25X_Exit4ByteAddr      0xE9
#define W25X_SetReadParam        0xC0 
#define W25X_EnterQPIMode       0x38
#define W25X_ExitQPIMode        0xFF

HDF_STATUS W25Qxx_InitSpiFlash(uint32_t busNum, uint32_t csNum);
HDF_STATUS W25Qxx_DeInitSpiFlash(void);
int32_t W25QXX_Erase_Sector(uint32_t Dst_Addr);
uint8_t W25QXX_ReadSR(uint8_t *pu8Status);
uint16_t W25QXX_ReadID(void);
int32_t W25QXX_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
int32_t W25QXX_Write_Page(const uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
int32_t W25QXX_Write_NoCheck(const uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite);
void W25QXX_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
int32_t W25QXX_Erase_Sector(uint32_t Dst_Addr);
int32_t W25QXX_Wait_Busy(void);
#endif

#endif /* __BSP_SPIFLASH_H__ */
