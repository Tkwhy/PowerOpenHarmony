#include "stm32l4xx.h"
#include "core_cm4.h"
#include "stm32l4xx_hal.h"
#include "fs_init_sdk.h"
#include <sys/mount.h>
#include "littlefs.h"
#include <sys/stat.h>
#include <dirent.h>
#include "los_config.h"
#include "los_fs.h"
#include "los_interrupt.h"

#include "hdf_log.h"
#include "hdf_device_desc.h"
#ifdef LOSCFG_DRIVERS_HDF_CONFIG_MACRO
#include "hcs_macro.h"
#include "hdf_config_macro.h"
#else
#include "device_resource_if.h"
#endif

#include <stdio.h>
#include <string.h>
#include "los_memory.h"
#include "w25qxx.h"

//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 //STM32 FLASH的起始地址

typedef enum {
    FLASH_PARTITION_DATA0 = 0,
    FLASH_PARTITION_DATA1 = 1,
    FLASH_PARTITION_MAX,
} HalPartition;

typedef struct {
    const CHAR *partitionDescription;
    uint32_t partitionStartAddr;
    uint32_t partitionLength;
    uint32_t partitionOptions;
} HalLogicPartition;

HalLogicPartition *getPartitionInfo(VOID);

unsigned int STMFLASH_ReadWord(unsigned int faddr); //读出字  
void STMFLASH_Write(unsigned int WriteAddr,unsigned char *pBuffer,unsigned int NumToWrite); //从指定地址开始写入指定长度的数据
void STMFLASH_Erase(unsigned int WriteAddr,unsigned int NumToWrite);
void STMFLASH_Read(unsigned int ReadAddr,unsigned int *pBuffer,unsigned int NumToRead); //从指定地址开始读出指定长度的数据
