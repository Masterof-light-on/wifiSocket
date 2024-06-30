#include "spi.h"

void SPI1_Init(void)
{
 //开启
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode= GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_7 ;
	GPIO_InitStruct.GPIO_Speed= GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	SPI_InitTypeDef SPI_InitStruct;
	
  SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//全双工主从模式
  SPI_InitStruct.SPI_Mode = SPI_Mode_Master;//主从模式
  SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStruct.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;//软件
  SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;//2分频
  SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;//数据高位在前
  SPI_InitStruct.SPI_CRCPolynomial = 7;
	
	SPI_Init(SPI1,&SPI_InitStruct);
	SPI_Cmd(SPI1,ENABLE);

}
u16 SPI_SendRecByte(SPI_TypeDef* SPIx, uint16_t Data)
{
while( !SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_TXE) )
	;
    SPI_I2S_SendData(SPIx,Data);
while( !SPI_I2S_GetFlagStatus(SPIx,SPI_I2S_FLAG_RXNE) )
	;
return SPI_I2S_ReceiveData(SPIx);
}
