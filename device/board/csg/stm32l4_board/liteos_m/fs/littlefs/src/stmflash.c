#include "stmflash.h"

/**
 * @brief	读取指定地址的字(32位数据)
 *
 * @param   faddr	读地址
 *
 * @return  unsigned int		读取的数据
 */
unsigned int STMFLASH_ReadWord(unsigned int faddr)
{
    return *(volatile uint32_t*)faddr;
}


/**
 * @brief	识别FLASH地址所在BANK(该芯片有两个BANK)
 *
 * @param   Addr	FLASH地址
 *
 * @return  unsigned int		FLASH地址所属BANK区
 */
static uint32_t Get_FLASH_Bank(uint32_t Addr)
{
    uint32_t bank = 0;

    /* No Bank swap */
    if(Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
        bank = FLASH_BANK_1;
    }

    else
    {
        bank = FLASH_BANK_2;
    }

    return bank;
}


/**
 * @brief	不检查的写入数据(64位)
 *
 * @param  	WriteAddr	起始地址(此地址必须为8的倍数!!)
 * @param  	pBuffer		数据指针
 * @param  	NumToWrite	数据(64位)量(就是要写入的64位数据的个数)
 *
 * @return  void
 */
void STMFLASH_Write_NoCheck(unsigned int WriteAddr, uint64_t *pBuffer, unsigned int NumToWrite)
{
    unsigned int i;

    for(i = 0; i < NumToWrite; i++)
    {
        if( HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, WriteAddr, pBuffer[i]) != HAL_OK )
		{
			return;		//写入错误！！！
		}
        WriteAddr += 8; //地址增加
    }
}

#define STM_SECTOR_SIZE	2048
static  uint64_t STMFLASH_BUF[STM_SECTOR_SIZE / 8]; //最多是2K字节

void STMFLASH_Erase(unsigned int WriteAddr, unsigned int NumToWrite)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t	SectorError;

    if(WriteAddr < STM32_FLASH_BASE || WriteAddr % 8)return;	//非法地址

    unsigned short i;
    unsigned int secpos;	   //扇区地址
    unsigned short secoff;	   //扇区内偏移地址(16位字计算)
    unsigned short secremain; //扇区内剩余地址(16位字计算)
    unsigned int offaddr;   //去掉0X08000000后的地址

    offaddr = WriteAddr - STM32_FLASH_BASE;		//实际偏移地址.
    secpos = offaddr / STM_SECTOR_SIZE;			//扇区地址
    secoff = (offaddr % STM_SECTOR_SIZE) / 8;		//在页内的偏移(8个字节为基本单位)
    secremain = (STM_SECTOR_SIZE / 8) - secoff;		//扇区剩余空间大小

    if(NumToWrite <= secremain)secremain = NumToWrite; //不大于该扇区范围

    HAL_FLASH_Unlock();             //解锁

    while(1)
    {
        STMFLASH_Read(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, (unsigned int*)STMFLASH_BUF, STM_SECTOR_SIZE / 4);	//读出该页全部数据

        for(i = 0; i < secremain; i++)
        {
            if(STMFLASH_BUF[secoff + i] != 0xFFFFFFFFFFFFFFFF) break;//有非0xFFFFFFFFFFFFFFFF的地方,要擦除这个扇区
        }

        if(i < secremain)//需要擦除
        {
            FlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES; //擦除类型，扇区擦除
            FlashEraseInit.Banks = Get_FLASH_Bank(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE); //要擦除的Bank
            FlashEraseInit.Page = secpos;    			//要擦出的页
            FlashEraseInit.NbPages = 1;      			//需要删除的页数
            if (FlashEraseInit.Banks == FLASH_BANK_2) {
                FlashEraseInit.Page += 128;
            }

            if(HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
            {
                break;//发生错误了
            }

        }

        if(NumToWrite == secremain)break; //写入结束了

        else
        {
            secpos++;				//扇区地址增1
            secoff = 0;				//偏移位置为0
            WriteAddr += (secremain * 8); //写地址偏移
            NumToWrite -= secremain;	//字节(64位)数递减

            if(NumToWrite > (STM_SECTOR_SIZE / 8))secremain = STM_SECTOR_SIZE / 8; //下一个扇区还是写不完

            else secremain = NumToWrite; //下一个扇区可以写完了
        }
    }

    HAL_FLASH_Lock();           //上锁
}

void STMFLASH_Write(unsigned int WriteAddr, unsigned char *pBuffer, unsigned int NumToWrite)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    uint32_t	SectorError;

    if(WriteAddr < STM32_FLASH_BASE || WriteAddr % 8)return;	//非法地址

    unsigned short i;
    unsigned int secpos;	   //扇区地址
    unsigned short secoff;	   //扇区内偏移地址(16位字计算)
    unsigned short secremain; //扇区内剩余地址(16位字计算)
    unsigned int offaddr;   //去掉0X08000000后的地址

    offaddr = WriteAddr - STM32_FLASH_BASE;		//实际偏移地址.
    secpos = offaddr / STM_SECTOR_SIZE;			//扇区地址
    secoff = (offaddr % STM_SECTOR_SIZE) / 8;		//在页内的偏移(8个字节为基本单位)
    secremain = (STM_SECTOR_SIZE / 8) - secoff;		//扇区剩余空间大小

    if(NumToWrite <= secremain)secremain = NumToWrite; //不大于该扇区范围

    HAL_FLASH_Unlock();             //解锁

    while(1)
    {
        STMFLASH_Read(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, (unsigned int*)STMFLASH_BUF, STM_SECTOR_SIZE / 4);	//读出该页全部数据

        for(i = 0; i < secremain; i++)
        {
            if(STMFLASH_BUF[secoff + i] != 0xFFFFFFFFFFFFFFFF) break;//有非0xFFFFFFFFFFFFFFFF的地方,要擦除这个扇区
        }

        if(i < secremain)//需要擦除
        {
            FlashEraseInit.TypeErase = FLASH_TYPEERASE_PAGES; //擦除类型，扇区擦除
            FlashEraseInit.Banks = Get_FLASH_Bank(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE); //要擦除的Bank
            FlashEraseInit.Page = secpos;    			//要擦出的页
            FlashEraseInit.NbPages = 1;      			//需要删除的页数
            if (FlashEraseInit.Banks == FLASH_BANK_2) {
                FlashEraseInit.Page += 128;
            }

            if(HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
            {
                break;//发生错误了
            }

            memcpy(STMFLASH_BUF,pBuffer,secremain*8);
                
            STMFLASH_Write_NoCheck(secpos * STM_SECTOR_SIZE + STM32_FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 8);
        }

        else//写已经擦除了的,直接写入扇区剩余区间.
        {
            memcpy(STMFLASH_BUF,pBuffer,secremain*8);
            STMFLASH_Write_NoCheck(WriteAddr, STMFLASH_BUF, secremain);
        }

        if(NumToWrite == secremain)break; //写入结束了

        else
        {
            secpos++;				//扇区地址增1
            secoff = 0;				//偏移位置为0
            pBuffer += secremain;  	//指针偏移
            WriteAddr += (secremain * 8); //写地址偏移
            NumToWrite -= secremain;	//字节(64位)数递减

            if(NumToWrite > (STM_SECTOR_SIZE / 8))secremain = STM_SECTOR_SIZE / 8; //下一个扇区还是写不完

            else secremain = NumToWrite; //下一个扇区可以写完了
        }
    }

    HAL_FLASH_Lock();           //上锁
}

/**
 * @brief	从指定地址开始读出指定长度的数据，每次读出一个字(32位)
 *
 * @param  	ReadAddr	起始地址
 * @param  	pBuffer		数据指针
 * @param  	NumToRead	读出字(32位)数
 *
 * @return  void
 */
void STMFLASH_Read(unsigned int ReadAddr, unsigned int *pBuffer, unsigned int NumToRead)
{
    unsigned int i;

    for(i = 0; i < NumToRead; i++)
    {
        pBuffer[i] = STMFLASH_ReadWord(ReadAddr); //读取4个字节.
        ReadAddr += 4; //偏移4个字节.
    }
}




