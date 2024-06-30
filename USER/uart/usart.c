#include "usart.h"

void usart1_init(u32 boud)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_9;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

 	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_10;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	
 	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	
  USART_InitTypeDef USART_InitStruct;
	
  USART_InitStruct.USART_BaudRate = boud;
  USART_InitStruct.USART_WordLength = USART_WordLength_8b;
  USART_InitStruct.USART_StopBits = USART_StopBits_1;
  USART_InitStruct.USART_Parity = USART_Parity_No ;
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 

	//初始化usart中断
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=1;
	
	//开启接收中断
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	//开启空闲中断
	USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);
	
	NVIC_Init(&NVIC_InitStruct);
	
	USART_Init(USART1,&USART_InitStruct);
	USART_Cmd(USART1,ENABLE);
}



//void USART1_Sendbyte(USART_TypeDef* USARTx, uint16_t Data)
//{
//  while(!USART_GetFlagStatus(USARTx,USART_FLAG_TXE))
//		;
//   USART_SendData(USARTx,Data);
//	
//	  while(!USART_GetFlagStatus(USARTx,USART_FLAG_TXE))
//		;
//}

void USART1_Sendbyte(USART_TypeDef* USARTx, uint16_t Data)
{
	while(!(USART1->SR & 1<<6)); //等待上一次的数据发送完成
	USART1->DR = Data;
	while(!(USART1->SR & 1<<6)); //这一次的数据发送完成
}


void USART_SendStr(USART_TypeDef* USARTx, u8* Data)
{
		u16 i=0;
	while(Data[i]!='\0')
	{
		USART1_Sendbyte(USARTx,Data[i++]);
	}
}


ITU1 u1sat;
void USART1_IRQHandler (void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE)==SET)
	{
		    u1sat.string[u1sat.len1++]=USART_ReceiveData(USART1);
	}
	if(USART_GetITStatus(USART1,USART_IT_IDLE)==SET)	
	{
		USART1->SR;
		USART1->DR;
		u1sat.resflag1=1;
		u1sat.string[u1sat.len1]=0;
	}
}

/********************************************************************
*函数名 ：        fputc          
*功能描述 ：			重写printf函数
*入口参数 ：无
*出口参数 ：无
*函数返回:  无
*调用提示 ：无   
*********************************************************************/
int fputc(int ch ,FILE *f)
{
	 USART1_Sendbyte(USART1,ch);
	 return ch;
}








