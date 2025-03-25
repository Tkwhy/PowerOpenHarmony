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

#include "stmflash.h"
#if (LOSCFG_LITTLEFS_FLASH == 1) //?¨²2?FLASH
int LittlefsRead(int partition, UINT32 *offset, void *buf, UINT32 size)
{
    uint32_t startAddr = *offset;
    uint32_t partitionEnd;
    HalLogicPartition *halPartitionsInfo = getPartitionInfo();
    partitionEnd = halPartitionsInfo[partition].partitionStartAddr + halPartitionsInfo[partition].partitionLength;
    if (startAddr >= partitionEnd) {
        printf("flash over read, 0x%x, 0x%x\r\n", startAddr, partitionEnd);
        return -1;
    }
    if ((startAddr + size) > partitionEnd) {
        size = partitionEnd - startAddr;
        printf("flash over read, new len is %d\r\n", size);
    }

    uint32_t intSave = LOS_IntLock();

    STMFLASH_Read(startAddr, buf, size/ 4);

    LOS_IntRestore(intSave);
    return 0;
}

int LittlefsProg(int partition, UINT32 *offset, const void *buf, UINT32 size)
{
    uint32_t startAddr = *offset;
    uint32_t partitionEnd;
    HalLogicPartition *halPartitionsInfo = getPartitionInfo();
    partitionEnd = halPartitionsInfo[partition].partitionStartAddr + halPartitionsInfo[partition].partitionLength;
    if (startAddr >= partitionEnd) {
        printf("flash over write, 0x%x, 0x%x\r\n", startAddr, partitionEnd);
        return -1;
    }
    if ((startAddr + size) > partitionEnd) {
        size = partitionEnd - startAddr;
        printf("flash over write, new len is %d\r\n", size);
    }

    uint32_t intSave = LOS_IntLock();

    STMFLASH_Write(startAddr, buf, size/ 8);

    LOS_IntRestore(intSave);
    return 0;
}

int LittlefsErase(int partition, UINT32 offset, UINT32 size)
{
    uint32_t startAddr = offset;
    uint32_t partitionEnd;
    HalLogicPartition *halPartitionsInfo = getPartitionInfo();
    partitionEnd = halPartitionsInfo[partition].partitionStartAddr + halPartitionsInfo[partition].partitionLength;
    if (startAddr >= partitionEnd) {
        printf("flash over erase, 0x%x, 0x%x\r\n", startAddr, partitionEnd);
        printf("flash over write\r\n");
        return -1;
    }
    if ((startAddr + size) > partitionEnd) {
        size = partitionEnd - startAddr;
        printf("flash over write, new len is %d\r\n", size);
    }

    uint32_t intSave = LOS_IntLock();
    HAL_Delay(3);
    STMFLASH_Erase(startAddr, size/ 8);

    LOS_IntRestore(intSave);

    return 0;
}
#endif

#if (LOSCFG_LITTLEFS_W25QXX == 1) //use w25qxx
int LittlefsRead(int partition, UINT32 *offset, void *buf, UINT32 size)
{
    W25QXX_Read((uint8_t*)buf, *offset, size);
    return LFS_ERR_OK;
}

int LittlefsProg(int partition, UINT32 *offset, const void *buf, UINT32 size)
{
    W25QXX_Write_NoCheck((uint8_t*)buf, *offset, size);
    return LFS_ERR_OK;
}

int LittlefsErase(int partition, UINT32 offset, UINT32 size)
{
    W25QXX_Erase_Sector(offset);
    return LFS_ERR_OK;
}
#endif
