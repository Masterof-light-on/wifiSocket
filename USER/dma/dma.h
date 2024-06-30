#ifndef _DMA_H
#define _DMA_H
#include "stm32f10x.h"
#include "usart.h"





void DMA_InitM2Per (u8 *str);
void  DMA_InitM2M (const u8 *src ,const u8 * tar);
void  DMA_InitPer2M (void);
#endif
