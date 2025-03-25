#include "stm32f4xx.h"
#include "core_cm4.h"
#include "stm32f4xx_hal.h"
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

#ifndef __STMFLASH_H
#define __STMFLASH_H

/* 函数声明 */
uint32_t stmflash_read_word(uint32_t addr);                                 /* 从指定地址读取一字的数据 */
void stmflash_read(uint32_t addr, uint32_t *buf, uint32_t length);          /* 从指定地址读取指定字的数据 */
void stmflash_write_nocheck(uint32_t addr, uint32_t *buf, uint32_t length); /* 往指定地址不检查地写入指定字的数据 */
void stmflash_write(uint32_t addr, uint32_t *buf, uint32_t length);         /* 往指定地址写入指定字的数据 */
void stmflash_erase(uint32_t addr, uint32_t length);

#endif
