#include <stdio.h>
#include "hc32_ll_efm.h"
#include "read_flash.h"

extern unsigned int _encryption_start;
extern unsigned int _encryption_end;

#define LOSCFG_SYS_FLASH_CHECK_SIZE   ((unsigned int)&_encryption_end - (unsigned int)&_encryption_start)
#define LOSCFG_SYS_FLASH_CHECK_ADDR   (unsigned int)&_encryption_start

uint8_t* ReadFlash(void)
{
    uint32_t readAddr = LOSCFG_SYS_FLASH_CHECK_ADDR; // 起始读取地址
    uint32_t remainingLength = LOSCFG_SYS_FLASH_CHECK_SIZE; // 剩余要读取的数据长度
    uint8_t Flashout[READ_BUFFER_SIZE]; // 存储读取数据的缓冲区
    int32_t ret; // 用于存储返回值

    while(remainingLength > 0)
    {
        // 检查EFM模块是否可用
        if (EFM_GetStatus(EFM_FLAG_RDY) == SET)
        {
            // 计算本次循环中可以读取的最大数据量
            uint32_t chunkSize = (remainingLength > READ_BUFFER_SIZE) ? READ_BUFFER_SIZE : remainingLength; 

            ret = EFM_ReadByte(readAddr, Flashout, READ_BUFFER_SIZE);
            if (ret == LL_OK)
            {
                /*
                // 处理读取的数据（例如，打印或存储）
                for (size_t i = 0; i < chunkSize; ++i)
                {
                    printf("%02X ", Flashout[i]);
                }
                printf("\n");
                */
               
                // 更新地址和剩余长度
                readAddr += chunkSize;
                remainingLength -= chunkSize;

            }
            else
            {
                printf("Failed to read data from Flash. Error code: %ld\n", ret);
                break; // 退出循环，读取失败
            }
        }
        else 
        {
                printf("EFM module is not ready.\n");
        }

    }
    return Flashout;
}
