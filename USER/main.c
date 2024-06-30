#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "wifi.h"
//#include <stdbool.h>
#include "stmflash.h"
#include "hmacsha1.h"
int main()
{
	unsigned char ARR[50]={0};
	unsigned short count = 500;
		//分配抢占优先级组数
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	delay_init();
	led_init();
	Key_Init();
	usart1_init(115200);
	wifi_init();
	printf("init ok\r\n");
	STMFLASH_Read(STM32_FLASH_BASE,ARR,50);
	printf("init ok--------------ARR=%s\r\n",ARR);
  while(1)
  {
		 TaskKeyMsg_Handle();
    TaskEsp12Msg_Handle();

			if(++count >= 500)
			count = 0;
		if((Get_Esp12ConnectionStatus()==false) && (count%50==0))	//无网络连接时慢闪
		{
			LED = !LED;
		}
		if((Get_Esp12ConnectionStatus()==THREE) && (count%10==0))	//用户配网时快闪
		{
			LED = !LED;
		}
		 delay_ms(10);

  }
}
