#ifndef USART_H_
#define USART_H_
#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>

typedef struct
{
u8 string[256];
u8 len1;
u8 resflag1;
}ITU1;


extern ITU1 u1sat;


void usart1_init(u32 boud);
void usart2_wifi_init(u32 boud);
void USART1_Sendbyte(USART_TypeDef* USARTx, uint16_t Data);
void USART_SendStr(USART_TypeDef* USARTx, u8* Data);
void USART_SendStrToReceiveStr(USART_TypeDef* USARTx, u8* Data);
int fputc(int ch ,FILE *f);

#endif
