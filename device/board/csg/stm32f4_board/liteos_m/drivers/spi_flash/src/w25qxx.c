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

#include "w25qxx.h"

#define W25x_PageSize              256
#define W25x_PerWritePageSize      256
#define W25X_WriteEnable           0x06
#define W25X_WriteDisable          0x04
#define W25X_ReadStatusReg         0x05
#define W25X_WriteStatusReg        0x01
#define W25X_ReadData              0x03
#define W25X_FastReadData          0x0B
#define W25X_FastReadDual          0x3B
#define W25X_PageProgram           0x02
#define W25X_BlockErase            0xD8
#define W25X_SectorErase           0x20
#define W25X_ChipErase             0xC7
#define W25X_PowerDown             0xB9
#define W25X_ReleasePowerDown      0xAB
#define W25X_DeviceID              0xAB
#define W25X_ManufactDeviceID      0x90
#define W25X_JedecDeviceID         0x9F

#define WIP_Flag                   0x01

#define Dummy_Byte                 0xFF

#ifdef LOSCFG_DRIVERS_HDF_PLATFORM_SPI
DevHandle spiHandle = NULL;
HDF_STATUS W25Qxx_InitSpiFlash(uint32_t busNum, uint32_t csNum)
{
    struct SpiDevInfo spiDevinfo;
    spiDevinfo.busNum = busNum;
    spiDevinfo.csNum = csNum;
    spiHandle = SpiOpen(&spiDevinfo);
    if (spiHandle == NULL) {
        HDF_LOGE("%s: SpiOpen failed", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success(%x)", __func__, spiHandle);

    return HDF_SUCCESS;
}

HDF_STATUS W25Qxx_DeInitSpiFlash(void)
{
    if (spiHandle != NULL) {
        SpiClose(spiHandle);
        HDF_LOGI("%s: success(%x)", __func__, spiHandle);
        spiHandle = NULL;
        return HDF_SUCCESS;
    }
    HDF_LOGE("%s: SpiClose failed", __func__);
    return HDF_FAILURE;
}


DevHandle W25x_GetSpiHandle(void)
{
    return spiHandle;
}


void W25x_BulkErase(void)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return;
    }
    W25x_WriteEnable();
    uint8_t wbuf[1] = {W25X_ChipErase};
    uint8_t rbuf[1] = {0};
    struct SpiMsg msg = {0};
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 0;
    msg.delayUs = 0;
    int32_t ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
        return;
    }
    W25x_WaitForWriteEnd();
    return;
}

void W25x_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return;
    }
    W25x_WriteEnable();
    uint8_t wbuf[4] = {W25X_PageProgram, (WriteAddr & 0xff0000) >> 16, (WriteAddr & 0xff00) >> 8, (WriteAddr & 0xff)};
    uint8_t rbuf[4] = {0};
    uint8_t *rbuf1 = NULL;
    int32_t ret = 0;

    struct SpiMsg msg = {0};
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 1;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }

    if (NumByteToWrite > W25x_PerWritePageSize) {
            NumByteToWrite = W25x_PerWritePageSize;
            HDF_LOGE("Err: W25x_PageWrite too large!\n");
    }

    rbuf1 = (uint8_t*)OsalMemAlloc(NumByteToWrite);
    if (rbuf1 == NULL) {
        HDF_LOGE("malloc failed\n");
        return;
    }
    memset_s(rbuf1, NumByteToWrite, 0, NumByteToWrite);
    msg.wbuf = pBuffer;
    msg.rbuf = rbuf1;
    msg.len = NumByteToWrite;
    msg.keepCs = 0;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }
    W25x_WaitForWriteEnd();
    OsalMemFree(rbuf1);
    return;
}

void W25x_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    Addr = WriteAddr % W25x_PageSize;
    count = W25x_PageSize - Addr;
    NumOfPage =  NumByteToWrite / W25x_PageSize;
    NumOfSingle = NumByteToWrite % W25x_PageSize;

    if (Addr == 0) {
        if (NumOfPage == 0) {
            W25x_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
        } else {
            while (NumOfPage--) {
                W25x_PageWrite(pBuffer, WriteAddr, W25x_PageSize);
                WriteAddr +=  W25x_PageSize;
                pBuffer += W25x_PageSize;
            }
            W25x_PageWrite(pBuffer, WriteAddr, NumOfSingle);
        }
    } else {
        if (NumOfPage == 0) {
            if (NumOfSingle > count) {
                temp = NumOfSingle - count;

                W25x_PageWrite(pBuffer, WriteAddr, count);
                WriteAddr +=  count;
                pBuffer += count;

                W25x_PageWrite(pBuffer, WriteAddr, temp);
            } else {
                W25x_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
            }
        } else {
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / W25x_PageSize;
            NumOfSingle = NumByteToWrite % W25x_PageSize;

            W25x_PageWrite(pBuffer, WriteAddr, count);
            WriteAddr +=  count;
            pBuffer += count;

            while (NumOfPage--) {
                W25x_PageWrite(pBuffer, WriteAddr, W25x_PageSize);
                WriteAddr +=  W25x_PageSize;
                pBuffer += W25x_PageSize;
            }

            if (NumOfSingle != 0) {
                W25x_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }
}

void W25x_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return;
    }

    int32_t ret = 0;

    uint8_t wbuf[4] = {W25X_ReadData, (ReadAddr & 0xFF0000) >> 16, (ReadAddr& 0xFF00) >> 8, ReadAddr & 0xFF};
    uint8_t rbuf[4] = {0};
    struct SpiMsg msg = {0};
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 1;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
        return;
    }
    uint8_t *wbuf1 = (uint8_t*)OsalMemAlloc(NumByteToRead);
    if (wbuf1 == NULL) {
        HDF_LOGE("malloc failed\n");
        return;
    }
    memset_s(wbuf1, NumByteToRead, 0xff, NumByteToRead);
    msg.wbuf = wbuf1;
    msg.rbuf = pBuffer;
    msg.len = NumByteToRead;
    msg.keepCs = 0;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
        return;
    }

    OsalMemFree(wbuf1);

    return;
}

uint32_t W25x_ReadID(void)
{
    int32_t ret = 0;
    uint16_t flashId = 0;
    uint8_t rbuff1[4] = { 0 };
    uint8_t wbuff1[4] = { W25X_JedecDeviceID, Dummy_Byte, Dummy_Byte, Dummy_Byte };
    struct SpiMsg msg1 = {0};
    msg1.wbuf = wbuff1;
    msg1.rbuf = rbuff1;
    msg1.len = sizeof(wbuff1);
    msg1.keepCs = 0;
    msg1.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg1, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    } else {
        flashId = msg1.rbuf[2]<<8 | msg1.rbuf[3];
    }

    return flashId;
}

uint32_t W25x_ReadDeviceID(void)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return 0;
    }
    struct SpiMsg msg;
    uint16_t deviceId = 0;
    uint8_t rbuff[5] = { 0 };
    uint8_t wbuff[5] = { W25X_DeviceID, Dummy_Byte, Dummy_Byte, Dummy_Byte, Dummy_Byte };
    int32_t ret = 0;
    msg.wbuf = wbuff;
    msg.rbuf = rbuff;
    msg.len = sizeof(wbuff);
    msg.keepCs = 0;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    } else {
        deviceId = rbuff[4];
    }

    return deviceId;
}

void W25x_StartReadSequence(uint32_t ReadAddr)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return;
    }
    struct SpiMsg msg;
    uint8_t rbuff[4] = { 0 };
    uint8_t wbuff[4] = { W25X_ReadData, (ReadAddr & 0xff0000) >> 16, (ReadAddr & 0xff00) >> 8, ReadAddr & 0xff };
    int32_t ret = 0;
    msg.wbuf = wbuff;
    msg.rbuf = rbuff;
    msg.len = sizeof(wbuff);
    msg.keepCs = 0;
    msg.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }
}

void W25x_WriteEnable(void)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return;
    }

    uint8_t wbuf[1] = {W25X_WriteEnable};
    uint8_t rbuf[1] = {0};
    struct SpiMsg msg = {0};
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 0;
    msg.delayUs = 0;
    int32_t ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }

    return;
}

void W25x_WaitForWriteEnd(void)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return;
    }

    uint8_t FLASH_Status = 0;

    /* Send "Read Status Register" instruction */
    uint8_t wbuf[1] = {W25X_ReadStatusReg};
    uint8_t wbuf1[1] = {0xff};
    uint8_t rbuf[1] = {0};
    struct SpiMsg msg = {0};
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 1;
    msg.delayUs = 0;
    int32_t ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }

    /* Loop as long as the memory is busy with a write cycle */
    do {
        msg.wbuf = wbuf1;
        msg.rbuf = rbuf;
        msg.len = sizeof(wbuf1);
        msg.keepCs = 1;
        msg.delayUs = 0;

        ret = SpiTransfer(spiHandle, &msg, 1);
        if (ret != 0) {
            HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
        }
        FLASH_Status = rbuf[0];
    }
    while ((FLASH_Status & WIP_Flag) == 1); /* Write in progress */

    msg.wbuf = wbuf1;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf1);
    msg.keepCs = 0;
    msg.delayUs = 0;

    ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }

    return;
}



void W25x_PowerDown(void)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return;
    }

    uint8_t wbuf[1] = {W25X_PowerDown};
    uint8_t rbuf[1] = {0};
    struct SpiMsg msg = {0};
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 0;
    msg.delayUs = 0;
    int32_t ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }

    return;
}

void W25QXX_Erase_Sector(uint32_t SectorAddr)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return;
    }
    W25x_WriteEnable();
    W25x_WaitForWriteEnd();
    uint8_t wbuf[4] = {0x20, (SectorAddr & 0xff0000) >> 16, (SectorAddr & 0xff00) >> 8, (SectorAddr & 0xff)};
    uint8_t rbuf[4] = {0};
    struct SpiMsg msg = {0};
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 0;
    msg.delayUs = 0;
    int32_t ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
        return;
    }
    W25x_WaitForWriteEnd();
}

void W25x_WAKEUP(void)
{
    if (spiHandle == NULL) {
        HDF_LOGE("spi flash haven't been inited\n");
        return;
    }

    uint8_t wbuf[1] = {W25X_ReleasePowerDown};
    uint8_t rbuf[1] = {0};
    struct SpiMsg msg = {0};
    msg.wbuf = wbuf;
    msg.rbuf = rbuf;
    msg.len = sizeof(wbuf);
    msg.keepCs = 0;
    msg.delayUs = 0;
    int32_t ret = SpiTransfer(spiHandle, &msg, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }

    return;
}
#endif
