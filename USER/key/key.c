#include "key.h"
#include "delay.h"

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_StructInit(&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 |GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC,&GPIO_InitStruct);

}
/*********************************************************************************************************
* 函 数 名 : Scanf_Key
* 功能说明 : 按键扫描
* 形    参 : 无
* 返 回 值 : true长按，false短按，其他值无按键按下
* 备    注 : PA0（SWITCH_KEY）
*********************************************************************************************************/ 
char Scanf_Key(void)
{
	static unsigned char state = 0;
	static unsigned char cnt = 0;
	switch(state){
		case 0:
			if(KEY1 == 0){	//检测是否按下，如果按下，进入消抖状态
				delay_ms(50);//消抖状态
			if(KEY1 == 0)		
				state = 1;
			else
				state = 0;
			}
			break;
		case 1:
			if(KEY1 == 0)	//如果按下的时间超过1s，那么认为是长按，返回结果，并设置下一个状态为等待松手态
			{
				if(++cnt > 100)
				{
					state = 3;
					cnt=0;
					return 0;
				}
			}
			else					//如果在1s的时间内松手，那么认为短按，返回结果，并设置下一个状态为等待松手态
			{
				state = 3;
				cnt=0;
				return 1;
			}
			break;
		case 3:
			if(KEY1 == 1)	//如果松手，则认为本次按下结束，回到最初的状态
				state = 0;
			break;
	}
	return 100;
}

