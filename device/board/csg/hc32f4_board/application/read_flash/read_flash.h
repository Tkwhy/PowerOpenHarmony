#ifndef __READ_FLASH_H
#define __READ_FLASH_H

#define FLASH_READ_ADDRESS  (0x0000000000100000UL) // 指定的Flash读取地址
#define DATA_LENGTH (FLASH_END_ADDRESS - FLASH_READ_ADDRESS + 1U) //计算数据长度，加1是因为包括结束地址本身
#define READ_BUFFER_SIZE    (DATA_LENGTH > 1024U ? 1024U : DATA_LENGTH)// 读取数据的大小，这里为16个字节
#define FLASH_END_ADDRESS   (0x00103FFFUL) //结束符





uint8_t* ReadFlash(void);


#endif