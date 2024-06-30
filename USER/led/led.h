#ifndef _LED_H
#define _LED_H
#include "stm32f10x.h"
#include "io_bit.h"


#define LED1_OFF   GPIO_SetBits(GPIOA,GPIO_Pin_8)
#define LED1_ON    GPIO_ResetBits(GPIOA,GPIO_Pin_8)
#define   LED  PAout(8)

void led_init(void);

#endif
