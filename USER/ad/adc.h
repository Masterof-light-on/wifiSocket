#ifndef _ADC_H_
#define _ADC_H_
#include "stm32f10x.h"

typedef struct
{
u32 Vstr;
u32 Vend;	
	
}ADC1buff;

extern ADC1buff adcbuff;
extern u16 bufftemp[200];

void ADC1init (void);
void ADC_FilterMAX(u16*  bufftemp);
u16 ADC_GetValue (ADC_TypeDef* ADCx, uint8_t ADC_Channel);

#endif
