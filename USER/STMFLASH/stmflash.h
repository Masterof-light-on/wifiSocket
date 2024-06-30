#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#include "io_bit.h"  
#include "stm32f10x_flash.h"

#define STM32_FLASH_BASE 0x08003C00 	//µÚ15Ò³µÄµØÖ·
 
void STMFLASH_Write(unsigned int WriteAddr,unsigned char *pBuffer,unsigned short NumToWrite);		
void STMFLASH_Read(unsigned int ReadAddr,unsigned char *pBuffer,unsigned short NumToRead);   
					   
#endif

















