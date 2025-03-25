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
#include "spi_if.h"
#include "hdf_log.h"

#ifdef LOSCFG_DRIVERS_HDF_PLATFORM_SPI
static DevHandle spiHandle = NULL;

/**
 * @brief    初始化SPI FLASH的IO口
 *
 * @param   void
 *
 * @return  void
 */
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

/**
 * @brief    读取W25QXX的状态寄存器，W25QXX一共有3个状态寄存器
 *             状态寄存器1：
 *             BIT7  6   5   4   3   2   1   0
 *             SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 *             SPR:默认0,状态寄存器保护位,配合WP使用
 *             TB,BP2,BP1,BP0:FLASH区域写保护设置
 *             WEL:写使能锁定
 *             BUSY:忙标记位(1,忙;0,空闲)
 *             默认:0x00
 *             状态寄存器2：
 *             BIT7  6   5   4   3   2   1   0
 *             SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
 *             状态寄存器3：
 *             BIT7      6    5    4   3   2   1   0
 *             HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
 *
 * @param   regno    状态寄存器号，范:1~3
 *
 * @return  uint8_t        状态寄存器值
 */
uint8_t W25QXX_ReadSR(uint8_t *pu8Status)
{
    int32_t ret = 0;
    uint8_t wbuff1[9] = { 0x05, 0, 0, 0, 0, 0, 0, 8, 1};

    struct SpiMsg msg1 = {0};
    msg1.wbuf = wbuff1;
    msg1.rbuf = pu8Status;
    msg1.len = 1;
    msg1.keepCs = 0;
    msg1.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg1, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }
    return ret;
}

/**
 * @brief    W25QXX写使能,将S1寄存器的WEL置位
 *
 * @param   void
 *
 * @return  void
 */
static int32_t W25QXX_Write_Enable(void)
{
    int32_t ret = 0;
    //instruction address dummycycles addressnode addresssize datamode
    uint8_t wbuff1[9] = { 0x06, 0, 0, 0, 0, 0, 0, 8, 0 };
    struct SpiMsg msg1 = {0};
    msg1.wbuf = wbuff1;
    msg1.len = 0;
    msg1.keepCs = 0;
    msg1.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg1, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }

    return ret;
}

static int32_t W25QXX_Write_Disable(void)
{
    int32_t ret = 0;
    //instruction address dummycycles addressnode addresssize datamode
    uint8_t wbuff1[9] = { 0x04, 0, 0, 0, 0, 0, 0, 8, 0 };
    struct SpiMsg msg1 = {0};
    msg1.wbuf = wbuff1;
    msg1.len = 0;
    msg1.keepCs = 0;
    msg1.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg1, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    }

    return ret;
}

/**
 * @brief    读取W25QXX芯片ID
 *
 * @param   void
 *
 * @return  uint8_t    返回值如下
 *                 0XEF13,表示芯片型号为W25Q80
 *                 0XEF14,表示芯片型号为W25Q16
 *                 0XEF15,表示芯片型号为W25Q32
 *                 0XEF16,表示芯片型号为W25Q64
 *                 0XEF17,表示芯片型号为W25Q128
 *                 0XEF18,表示芯片型号为W25Q256
 */
uint16_t W25QXX_ReadID(void)
{
    int32_t ret = 0;
    uint8_t rbuff1[2] = { 0 };
    uint16_t deviceid;
    
    uint8_t wbuff1[9] = { 0x90, 0, 0, 0, 0, 0, 1, 24, 1 };
    struct SpiMsg msg1 = {0};
    msg1.wbuf = wbuff1;
    msg1.rbuf = rbuff1;
    msg1.len = sizeof(rbuff1);
    msg1.keepCs = 0;
    msg1.delayUs = 0;
    ret = SpiTransfer(spiHandle, &msg1, 1);
    if (ret != 0) {
        HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
    } else {
        deviceid = (msg1.rbuf[0]<<8)|msg1.rbuf[1];
    }

    HDF_LOGI("%s: deviceid %x\n", __func__, deviceid);
    return deviceid;
}

/**
 * @brief    读取SPI FLASH数据
 *
 * @param   pBuffer            数据存储区
 * @param   ReadAddr        开始读取的地址(最大32bit)
 * @param   NumByteToRead    要读取的字节数(最大65535)
 *
 * @return  void
 */
int32_t W25QXX_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
    int32_t ret = -1;
    if((ReadAddr + NumByteToRead) > W25QXX_MAX_ADDR) {
        HDF_LOGE("SpiTransfer: failed, (ReadAddr + NumByteToRead) > W25Q128_MAX_ADDR\n");
        return -1;
    }

    if ((pBuffer != NULL) && (NumByteToRead != 0UL)) {
        //instruction address dummycycles addressnode addresssize datamode
        uint8_t wbuff1[9] = { 0x0b, 0, 0, 0, 0, 8, 1, 24, 1 };
        wbuff1[0] = W25X_FastReadData;
        wbuff1[1] = ReadAddr & 0xff;
        wbuff1[2] = (ReadAddr >> 8) & 0xff;
        wbuff1[3] = (ReadAddr >> 16) & 0xff;
        wbuff1[4] = (ReadAddr >> 24) & 0xff;
        struct SpiMsg msg1 = {0};
        msg1.wbuf = wbuff1;
        msg1.rbuf = pBuffer;
        msg1.len = NumByteToRead;
        msg1.keepCs = 0;
        msg1.delayUs = 0;
        ret = SpiTransfer(spiHandle, &msg1, 1);
        if (ret != 0) {
            HDF_LOGE("SpiTransfer: failed, ret %d\n", ret);
        }
    }

    return ret;
}

/**
 * @brief    在指定地址开始写入最大256字节的数据
 *
 * @param   pBuffer            数据存储区
 * @param   WriteAddr        开始写入的地址(最大32bit)
 * @param   NumByteToWrite    要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
 *
 * @return  void
 */
int32_t W25QXX_Write_Page(const uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
    int32_t i32Ret = -1;
    uint8_t wbuff1[9] = { 0x02, 0, 0, 0, 0, 0, 1, 24, 1 };
    wbuff1[0] = W25X_PageProgram;
    wbuff1[1] = WriteAddr & 0xff;
    wbuff1[2] = (WriteAddr >> 8) & 0xff;
    wbuff1[3] = (WriteAddr >> 16) & 0xff;
    wbuff1[4] = (WriteAddr >> 24) & 0xff;
    struct SpiMsg msg1 = {0};
    msg1.wbuf = wbuff1;
    msg1.keepCs = 0;
    msg1.delayUs = 0;

    if ((pBuffer != NULL) && (NumByteToWrite != 0UL)) {
        i32Ret = W25QXX_Write_Enable();
        if (i32Ret == 0) {
            i32Ret = SpiTransfer(spiHandle, &msg1, 1);
            if (i32Ret != 0) {
                HDF_LOGE("SpiTransfer: failed, i32Ret %d\n", i32Ret);
            }
            i32Ret = SpiWrite(spiHandle, pBuffer, NumByteToWrite);
        }
        if (i32Ret == 0) {
            i32Ret = W25QXX_Wait_Busy();
        }
    }

    return i32Ret;
}

/**
 * @brief    无检验写SPI FLASH
 *             必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
 *             具有自动换页功能
 *             在指定地址开始写入指定长度的数据,但是要确保地址不越界!
 *
 * @param   pBuffer            数据存储区
 * @param   WriteAddr        开始写入的地址(最大32bit)
 * @param   NumByteToWrite    要写入的字节数(最大65535)
 *
 * @return  void
 */
int32_t W25QXX_Write_NoCheck(const uint8_t* pBuffer, uint32_t WriteAddr, uint32_t NumByteToWrite)
{
    uint32_t u32TempSize;
    uint32_t u32AddrOfst = 0UL;
    int32_t i32Ret = -1;

    if ((WriteAddr + NumByteToWrite) > W25QXX_MAX_ADDR) {
        HDF_LOGE("W25QXX_Write_NoCheck: failed, (WriteAddr + NumByteToWrite) > W25QXX_MAX_ADDR\n");
        return -1;
    }

    if ((pBuffer != NULL) && (NumByteToWrite > 0UL)) {
        while (NumByteToWrite != 0UL) {
            if (NumByteToWrite >= W25QXX_PAGE_SIZE) {
                u32TempSize = W25QXX_PAGE_SIZE;
            } else {
                u32TempSize = NumByteToWrite;
            }

            i32Ret = W25QXX_Write_Page((const uint8_t *)&pBuffer[u32AddrOfst], WriteAddr, u32TempSize);
            if (i32Ret != 0) {
                break;
            }
            NumByteToWrite -= u32TempSize;
            u32AddrOfst += u32TempSize;
            WriteAddr  += u32TempSize;
        }
    }

    return 0;
}

/**
 * @brief    写SPI FLASH
 *             在指定地址开始写入指定长度的数据
 *             该函数带擦除操作!
 *
 * @param   pBuffer            数据存储区
 * @param   WriteAddr        开始写入的地址(最大32bit)
 * @param   NumByteToWrite    要写入的字节数(最大65535)
 *
 * @return  void
 */
uint8_t W25QXX_BUFFER[4096];
void W25QXX_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t * W25QXX_BUF;
    W25QXX_BUF = W25QXX_BUFFER;
    secpos = WriteAddr / 4096; // 扇区地址
    secoff = WriteAddr % 4096; // 在扇区内的偏移
    secremain = 4096 - secoff; // 扇区剩余空间大小

    //HDF_LOGI("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite); // 测试用
    if (NumByteToWrite <= secremain) secremain = NumByteToWrite; // 不大于4096个字节

    while(1) {
        W25QXX_Read(W25QXX_BUF, secpos * 4096, 4096); // 读出整个扇区的内容

        for(i = 0; i < secremain; i++) { // 校验数据
            if (W25QXX_BUF[secoff + i] != 0XFF) break; // 需要擦除
        }

        if (i < secremain) { // 需要擦除
            W25QXX_Erase_Sector(secpos); // 擦除这个扇区
            for(i = 0; i < secremain; i++) { // 复制
                W25QXX_BUF[i + secoff] = pBuffer[i];
            }
            W25QXX_Write_NoCheck(W25QXX_BUF, secpos * 4096, 4096); // 写入整个扇区
        } else {
            W25QXX_Write_NoCheck(pBuffer, WriteAddr, secremain); // 写已经擦除了的,直接写入扇区剩余区间.
        }

        if (NumByteToWrite == secremain) break; // 写入结束了
        else { // 写入未结束
            secpos++; // 扇区地址增1
            secoff = 0; // 偏移位置为0
            pBuffer += secremain; // 指针偏移
            WriteAddr += secremain; // 写地址偏移
            NumByteToWrite -= secremain; // 字节数递减

            if (NumByteToWrite > 4096) secremain = 4096; // 下一个扇区还是写不完
            else secremain = NumByteToWrite; // 下一个扇区可以写完了
        }
    };
}

/**
 * @brief    擦除一个扇区，擦除一个扇区的最少时间:150ms
 *
 * @param   Dst_Addr    扇区地址 根据实际容量设置
 *
 * @return  void
 */
int32_t W25QXX_Erase_Sector(uint32_t Dst_Addr)
{
    if (Dst_Addr >= W25QXX_MAX_ADDR) {
        return -1;
    }

    int32_t i32Ret = -1;
    uint8_t wbuff1[9] = { 0x20, 0, 0, 0, 0, 0, 1, 24, 0 };
    wbuff1[1] = Dst_Addr & 0xff;
    wbuff1[2] = (Dst_Addr >> 8) & 0xff;
    wbuff1[3] = (Dst_Addr >> 16) & 0xff;
    wbuff1[4] = (Dst_Addr >> 24) & 0xff;
    struct SpiMsg msg1 = {0};
    msg1.wbuf = wbuff1;
    msg1.keepCs = 0;
    msg1.delayUs = 0;
    
    i32Ret = W25QXX_Write_Enable();
    if (i32Ret == 0) {
        i32Ret = W25QXX_Wait_Busy();
    }
    if (i32Ret == 0) {
        i32Ret = SpiTransfer(spiHandle, &msg1, 1);
    }
    if (i32Ret == 0) {
        i32Ret = W25QXX_Wait_Busy();
    }
    if (i32Ret == 0) {
        i32Ret = W25QXX_Write_Disable();
    }

    return i32Ret;
}

/**
 * @brief    等待空闲
 *
 * @param   void
 *
 * @return  void
 */
int32_t W25QXX_Wait_Busy(void)
{
    uint8_t u8Status;
    int32_t i32Ret = -1;
    volatile uint32_t u32Timecount = W25QXX_TIMEOUT;

    while (u32Timecount-- != 0UL) {
        i32Ret = W25QXX_ReadSR(&u8Status);
        if ((i32Ret == 0) && ((u8Status & W25QXX_FLAG_BUSY) == 0U)) {
            break;
        }
    }

    return i32Ret;
}
#endif
