#include "stmflash.h"
/*********************************************************************************************************
* �� �� �� : STMFLASH_Write
* ����˵�� : ���ڲ�flashд������
* ��    �� : WriteAddrҪд��ĵ�ַ��pBufferд������ݣ�NumToWriteд�����
* �� �� ֵ : ��
* ��    ע : ��
*********************************************************************************************************/ 
void STMFLASH_Write(unsigned int WriteAddr,unsigned char *pBuffer,unsigned short NumToWrite)   
{ 			 		 
	unsigned short i;
	unsigned short temp = 0;
	
	FLASH_Unlock();	//����
    
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR );	//������еı�־λ
	
	if(FLASH_COMPLETE != FLASH_ErasePage(STM32_FLASH_BASE))		//ҳ����
        return;
	
	for(i=0;i<NumToWrite/2;i++)
	{
		temp = pBuffer[2*i+1]<<8 | pBuffer[2*i];
		FLASH_ProgramHalfWord(WriteAddr,temp);
	   	WriteAddr+=2;
	}  
	
	FLASH_Lock();	//����
} 
/*********************************************************************************************************
* �� �� �� : STMFLASH_Read
* ����˵�� : ���ڲ�flash�ж�ȡ����
* ��    �� : ReadAddr��ȡ�ĵ�ַ��pBuffer���ݻ�������NumToRead��ȡ�ĸ���
* �� �� ֵ : ��
* ��    ע : ��
*********************************************************************************************************/ 
void STMFLASH_Read(unsigned int ReadAddr,unsigned char *pBuffer,unsigned short NumToRead)   	
{
	unsigned short i;
	
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]= *(unsigned char *)(ReadAddr+i);
	}
	
}
















