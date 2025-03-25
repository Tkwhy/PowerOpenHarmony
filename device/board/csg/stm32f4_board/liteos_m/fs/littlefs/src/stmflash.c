#include "stmflash.h"

/**
 * @brief   从指定地址读取一字的数据
 * @param   addr: 指定读取数据的地址
 * @retval  读取到的一字数据
 */
uint32_t stmflash_read_word(uint32_t addr)
{
    return (*(volatile uint32_t *)addr);
}

/**
 * @brief   从指定地址读取指定字的数据
 * @param   addr: 指定读取数据的地址
 * @param   buf: 存储读取数据的起始地址
 * @param   length: 指定读取数据的长度，单位：字
 * @retval  无
 */
void stmflash_read(uint32_t addr, uint32_t *buf, uint32_t length)
{
    uint32_t index;

    for (index=0; index<length; index++)
    {
        buf[index] = stmflash_read_word(addr);
        addr += sizeof(uint32_t);
    }
}

/**
 * @brief   往指定地址不检查地写入指定字的数据
 * @param   addr: 指定写入数据的地址
 * @param   buf: 存储写入数据的起始地址
 * @param   length: 指定写入数据的长度，单位：字
 * @retval  无
 */
void stmflash_write_nocheck(uint32_t addr, uint32_t *buf, uint32_t length)
{
    uint16_t index;

    HAL_FLASH_Unlock();

    __HAL_FLASH_DATA_CACHE_DISABLE();
    for (index=0; index<length; index++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + (index << 2), buf[index]);
        
    }
    __HAL_FLASH_DATA_CACHE_ENABLE();
    HAL_FLASH_Lock();
}

/**
 * @brief   获取Flash地址所在的Flash扇区
 * @param   addr: Flash地址
 * @retval  Flash扇区
 */
static uint32_t stmflash_get_flash_sector(uint32_t addr)
{
    if (addr < 0x08004000)
    {
        return FLASH_SECTOR_0;
    }
    else if (addr < 0x08008000)
    {
        return FLASH_SECTOR_1;
    }
    else if (addr < 0x0800C000)
    {
        return FLASH_SECTOR_2;
    }
    else if (addr < 0x08010000)
    {
        return FLASH_SECTOR_3;
    }
    else if (addr < 0x08020000)
    {
        return FLASH_SECTOR_4;
    }
    else if (addr < 0x08040000)
    {
        return FLASH_SECTOR_5;
    }
    else if (addr < 0x08060000)
    {
        return FLASH_SECTOR_6;
    }
    else if (addr < 0x08080000)
    {
        return FLASH_SECTOR_7;
    }
    else if (addr < 0x080A0000)
    {
        return FLASH_SECTOR_8;
    }
    else if (addr < 0x080C0000)
    {
        return FLASH_SECTOR_9;
    }
    else if (addr < 0x080E0000)
    {
        return FLASH_SECTOR_10;
    }
    else
    {
        return FLASH_SECTOR_11;
    }
}

void stmflash_erase(uint32_t addr, uint32_t length)
{
    uint32_t addrx;
    uint32_t endaddr;
    FLASH_EraseInitTypeDef flash_erase_init_struct = {0};
    uint32_t sectorerr;
    HAL_StatusTypeDef status = HAL_OK;

    if ((!IS_FLASH_ADDRESS(addr)) ||
        (!IS_FLASH_ADDRESS(addr + (length * sizeof(uint32_t)) - 1)) ||
        ((addr & 3) != 0))
    {
        printf("----%s addr check fail\n", __func__);
        return;
    }

    addrx = addr;
    endaddr = addr + (length << 2);
    if (addrx <= FLASH_END)
    {
        while (addrx < endaddr)
        {
            if (stmflash_read_word(addrx) != 0xFFFFFFFF)
            {
                flash_erase_init_struct.TypeErase = FLASH_TYPEERASE_SECTORS;
                flash_erase_init_struct.Banks = FLASH_BANK_1;
                flash_erase_init_struct.Sector = stmflash_get_flash_sector(addrx);
                flash_erase_init_struct.NbSectors = 1;
                flash_erase_init_struct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
                HAL_FLASH_Unlock();
                HAL_Delay(2);
                __HAL_FLASH_DATA_CACHE_DISABLE();
                status = HAL_FLASHEx_Erase(&flash_erase_init_struct, &sectorerr);
                if (status != HAL_OK)
                {
                    printf("----%s, HAL_FLASHEx_Erase status is %d\n", __func__, status);
                    break;
                }
                __HAL_FLASH_DATA_CACHE_ENABLE();
                HAL_FLASH_Lock();
            }
            else
            {
                addrx += sizeof(uint32_t);
            }
        }
    } else {
        printf("----%s, addrx > FLASH_END\n", __func__);
    }
}

/**
 * @brief   往指定地址写入指定字的数据
 * @param   addr: 指定写入数据的地址
 * @param   buf: 存储写入数据的起始地址
 * @param   length: 指定写入数据的长度，单位：字
 * @retval  无
 */
void stmflash_write(uint32_t addr, uint32_t *buf, uint32_t length)
{
    uint32_t addrx;
    uint32_t endaddr;
    FLASH_EraseInitTypeDef flash_erase_init_struct = {0};
    uint32_t sectorerr;
    HAL_StatusTypeDef status = HAL_OK;

    if ((!IS_FLASH_ADDRESS(addr)) ||
        (!IS_FLASH_ADDRESS(addr + (length * sizeof(uint32_t)) - 1)) ||
        ((addr & 3) != 0))
    {
        return;
    }
    
    addrx = addr;
    endaddr = addr + (length << 2);
    if (addrx <= FLASH_END)
    {
        while (addrx < endaddr)
        {
            if (stmflash_read_word(addrx) != 0xFFFFFFFF)
            {
                printf("stmflash_write erase addrx is 0x%x\n", addrx);
                flash_erase_init_struct.TypeErase = FLASH_TYPEERASE_SECTORS;
                flash_erase_init_struct.Banks = FLASH_BANK_1;
                flash_erase_init_struct.Sector = stmflash_get_flash_sector(addrx);
                flash_erase_init_struct.NbSectors = 1;
                flash_erase_init_struct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
                HAL_FLASH_Unlock();
                __HAL_FLASH_DATA_CACHE_DISABLE();
                status = HAL_FLASHEx_Erase(&flash_erase_init_struct, &sectorerr);
                __HAL_FLASH_DATA_CACHE_ENABLE();
                HAL_FLASH_Lock();
                if (status != HAL_OK)
                {
                    break;
                }
            }
            else
            {
                addrx += sizeof(uint32_t);
            }
        }
    }

    if (status == HAL_OK)
    {
        while (addr < endaddr)
        {
            stmflash_write_nocheck(addr, buf, 1);
            addr += sizeof(uint32_t);
            buf++;
        }
    }
}
