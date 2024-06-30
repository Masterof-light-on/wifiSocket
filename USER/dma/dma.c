#include "dma.h"
#include "string.h"


/********************************************************************
*������ ��  void  DMA_InitM2Per (u8 *str)                
*�������� ��DMA�Ѵ洢�������ݰ��͵����裨USART�����ݼĴ����ϣ�
*��ڲ��� ��u8 *str
*���ڲ��� ����
*��������:  ��
*������ʾ ����   
*********************************************************************/
void  DMA_InitM2Per (u8 *str)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_BufferSize =strlen((char*)str);
	DMA_InitStruct.DMA_DIR =DMA_DIR_PeripheralDST;//�洢��������
	DMA_InitStruct.DMA_M2M =DMA_M2M_Disable;
	DMA_InitStruct.DMA_MemoryBaseAddr= (u32)str;
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_Mode=DMA_Mode_Normal;
	DMA_InitStruct.DMA_PeripheralBaseAddr= (u32)&USART1->DR;
	DMA_InitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_Priority=DMA_Priority_VeryHigh;

	DMA_Init(DMA1_Channel4, &DMA_InitStruct);
	
	DMA_Cmd(DMA1_Channel4,ENABLE);
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE);
	while(1)
	{
		if(DMA_GetFlagStatus(DMA1_FLAG_TC4)!=RESET)//�ж��Ƿ�������
	 {
		 DMA_ClearFlag(DMA1_FLAG_TC4);//��ɺ������־λ
		 DMA_Cmd(DMA1_Channel4,DISABLE);//ʧ��DMA
		 break;
	 }
	 	 if(strlen((char*)str)==0)
	 {
		 DMA_ClearFlag(DMA1_FLAG_TC4);
		 DMA_Cmd(DMA1_Channel4,DISABLE);
		 break;
	 }
	}
}

/********************************************************************
*������ ��  void  DMA_InitM2Per (u8 *str)                
*�������� ��DMA�Ѵ洢�������ݰ��͵����裨USART�����ݼĴ����ϣ�
*��ڲ��� ��u8 *str
*���ڲ��� ����
*��������:  ��
*������ʾ ����   
*********************************************************************/
void  DMA_InitM2M (const u8 *src ,const u8 * tar)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_BufferSize =strlen((char*)src);
	DMA_InitStruct.DMA_DIR =DMA_DIR_PeripheralDST;//�洢�����洢��
	DMA_InitStruct.DMA_M2M =DMA_M2M_Enable;
	DMA_InitStruct.DMA_MemoryBaseAddr= (u32)src;
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_Mode=DMA_Mode_Normal;
	DMA_InitStruct.DMA_PeripheralBaseAddr= (u32)tar;
	DMA_InitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Enable;
	DMA_InitStruct.DMA_Priority=DMA_Priority_VeryHigh;

	DMA_Init(DMA1_Channel6, &DMA_InitStruct);
	DMA_Cmd(DMA1_Channel6,ENABLE);
	while(1)
	{
		 if(DMA_GetFlagStatus(DMA1_FLAG_TC6)!=RESET)
	 {
		 DMA_ClearFlag(DMA1_FLAG_TC6);
		 DMA_Cmd(DMA1_Channel6,DISABLE);
		 break;
	 }
 }
}

