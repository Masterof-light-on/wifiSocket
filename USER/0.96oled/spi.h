#ifndef _SPI_H
#define _SPI_H
#include "stm32f10x.h"

void SPI1_Init(void);
u16 SPI_SendRecByte(SPI_TypeDef* SPIx, uint16_t Data);

#endif
