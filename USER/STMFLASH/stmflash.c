#include "stmflash.h"
/*********************************************************************************************************
* 函 数 名 : STMFLASH_Write
* 功能说明 : 向内部flash写入数据
* 形    参 : WriteAddr要写入的地址，pBuffer写入的内容，NumToWrite写入个数
* 返 回 值 : 无
* 备    注 : 无
*********************************************************************************************************/ 
void STMFLASH_Write(unsigned int WriteAddr,unsigned char *pBuffer,unsigned short NumToWrite)   
{ 			 		 
	unsigned short i;
	unsigned short temp = 0;
	
	FLASH_Unlock();	//解锁
    
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR );	//清除所有的标志位
	
	if(FLASH_COMPLETE != FLASH_ErasePage(STM32_FLASH_BASE))		//页擦除
        return;
	
	for(i=0;i<NumToWrite/2;i++)
	{
		temp = pBuffer[2*i+1]<<8 | pBuffer[2*i];
		FLASH_ProgramHalfWord(WriteAddr,temp);
	   	WriteAddr+=2;
	}  
	
	FLASH_Lock();	//上锁
} 
/*********************************************************************************************************
* 函 数 名 : STMFLASH_Read
* 功能说明 : 从内部flash中读取数据
* 形    参 : ReadAddr读取的地址，pBuffer数据缓冲区，NumToRead读取的个数
* 返 回 值 : 无
* 备    注 : 无
*********************************************************************************************************/ 
void STMFLASH_Read(unsigned int ReadAddr,unsigned char *pBuffer,unsigned short NumToRead)   	
{
	unsigned short i;
	
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]= *(unsigned char *)(ReadAddr+i);
	}
	
}
















