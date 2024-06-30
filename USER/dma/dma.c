#include "dma.h"
#include "string.h"


/********************************************************************
*函数名 ：  void  DMA_InitM2Per (u8 *str)                
*功能描述 ：DMA把存储器的数据搬送到外设（USART的数据寄存器上）
*入口参数 ：u8 *str
*出口参数 ：无
*函数返回:  无
*调用提示 ：无   
*********************************************************************/
void  DMA_InitM2Per (u8 *str)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_BufferSize =strlen((char*)str);
	DMA_InitStruct.DMA_DIR =DMA_DIR_PeripheralDST;//存储器到外设
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
		if(DMA_GetFlagStatus(DMA1_FLAG_TC4)!=RESET)//判断是否搬运完成
	 {
		 DMA_ClearFlag(DMA1_FLAG_TC4);//完成后清除标志位
		 DMA_Cmd(DMA1_Channel4,DISABLE);//失能DMA
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
*函数名 ：  void  DMA_InitM2Per (u8 *str)                
*功能描述 ：DMA把存储器的数据搬送到外设（USART的数据寄存器上）
*入口参数 ：u8 *str
*出口参数 ：无
*函数返回:  无
*调用提示 ：无   
*********************************************************************/
void  DMA_InitM2M (const u8 *src ,const u8 * tar)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_BufferSize =strlen((char*)src);
	DMA_InitStruct.DMA_DIR =DMA_DIR_PeripheralDST;//存储器到存储器
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

