#include "delay.h"

u32 sysdelay_ms;

void delay_init(void)
{
	//		//uint32_t SysTick_Config(uint32_t ticks);
  SysTick->CTRL &=~(0x1<<2);
	SysTick->CTRL |=0x1<<1;
	SysTick->LOAD  = 9000-1;
	NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1); 
	SysTick->CTRL &= ~(0x1<<0);
}
void SysTick_Handler (void)
{
	if(sysdelay_ms > 0 )
		sysdelay_ms--;

}

void delay_ms (u32 ms)
{
	sysdelay_ms=ms;
	SysTick->CTRL |= 0x1<<0;
	while(sysdelay_ms)
		;
  SysTick->CTRL &= ~(0x1<<0);
}
