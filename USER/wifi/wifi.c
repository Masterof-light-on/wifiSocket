#include "wifi.h"
#include "usart.h"
#include "delay.h"
#include "stdio.h"
#include "key.h"
#include "led.h"
#include <stdio.h>
#include <string.h>
#include "stmflash.h"
#include "hmacsha1.h"

//#include <stdbool.h>


static unsigned char ConnectFlag = false;		//true：已连接，false：未连接

static unsigned char ssid[MESSAGE_MAX/2] = {0}, pwsd[MESSAGE_MAX/2] = {0};  //ssid  pwsd
static unsigned char houst_pwd[256];				//打包数据缓冲区
static unsigned char state 		= AT_AT;	//当前状态
static unsigned char before 		= AT_AT;	//上一个状态
static unsigned char next 		= AT_AT;	//下一个状态
static unsigned char wishack 		= AT_OK;	//希望应答
static unsigned char response 	= AT_OK;	//得到的应答
static unsigned short timeout 	= WAIT_100;	//超时时间
static unsigned short interval	= WAIT_100;	//间隔，发太快会出现busy...，尽量在100ms以上
static unsigned short count 		= TRY_COUNT;//计数，超过一定次数则触发某个动作



ITU2 u2sat;
void usart2_wifi_init(u32 boud)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_2;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;

 	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin  = GPIO_Pin_3;
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
	NVIC_InitStruct.NVIC_IRQChannel=USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
	
	//开启接收中断
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	//开启空闲中断
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);
	
	NVIC_Init(&NVIC_InitStruct);
	
	USART_Init(USART2,&USART_InitStruct);
	USART_Cmd(USART2,ENABLE);
}

void USART2_wifi_Sendbyte(uint16_t Data)
{
	while(!(USART2->SR & 1<<6)); //等待上一次的数据发送完成
	USART2->DR = Data;
	while(!(USART2->SR & 1<<6)); //这一次的数据发送完成
}


void USART2_wifi_SendStr( u8* Data)
{
	u16 i=0;
	while(Data[i]!='\0')
	{
		USART2_wifi_Sendbyte(Data[i++]);
	}
}


void USART2_IRQHandler (void)
{
	if(USART_GetITStatus(USART2,USART_IT_RXNE)==SET)
	{
		u2sat.wifi_rec[u2sat.len2++]=USART_ReceiveData(USART2);
		
	}
	if(USART_GetITStatus(USART2,USART_IT_IDLE)==SET)	
	{
		USART2->SR;
		USART2->DR;
		u2sat.wifi_rec[u2sat.len2]=0;
		u2sat.resflag2=true;
		//USART2_wifi_SendStr(u2sat.wifi_rec);
	}
}

void wifi_init(void)
{
	 printf("uart2 init wait\r\n");
   usart2_wifi_init(115200);
	 printf("uart2 init ok\r\n");

}




void RevampState(char *cmd, unsigned char sta, unsigned char bef, unsigned char nex,
									unsigned char ack, unsigned short to, unsigned short inte)
{
//	memset(u2sat.wifi_rec, 0, sizeof(u2sat.wifi_rec));
//	WifiUsart.RecLen = 0;
	memset(&u2sat,0,sizeof(u2sat));
	if(cmd != NULL)
		USART2_wifi_SendStr((unsigned char *)cmd);
	state = sta;		//发完命令进入等待应答状态
	before = bef;		//记录上一个状态
	next = nex;			//下一个要执行的状态
	wishack	  = ack;	//希望的应答
	timeout = to;		//超时时间为100ms
	interval = inte;	//和下一个状态的间隔时间为100ms
	//response = UNKNOWN;	//清除上一次收到的应答
}


/*********************************************************************************************************
* 函 数 名 : JumpState
* 功能说明 : 状态跳转
* 形    参 : sta：当前状态，mode：true下一个状态，false上一个状态
* 返 回 值 : 无
* 备    注 : 无
*********************************************************************************************************/ 
static void JumpState(unsigned char sta, unsigned char offset)
{
	if(sta == AT_MQTTSUB)	state = FINISH;
	else					state = offset;
}


/*********************************************************************************************************
* 函 数 名 : MeasuringResponse
* 功能说明 : 检测应答
* 形    参 : ack：希望的应答类型
* 返 回 值 : 非NULL_为有应答
* 备    注 : 无
*********************************************************************************************************/ 
static unsigned char MeasuringResponse(unsigned char ack)
{
	switch(ack)
	{
		case AT_OK:
			//printf("%s\r\n",u2sat.wifi_rec);
			if(strstr((char *)u2sat.wifi_rec, "OK"))			return AT_OK;
			else if(strstr((char *)u2sat.wifi_rec, "ERROR"))	return AT_ERR;	
			break;
		case AT_OK|AT_ERR:
			//printf("%s\r\n",u2sat.wifi_rec);
			return AT_OK;
			break;
		case AT_IP:
			//printf("%s\r\n",u2sat.wifi_rec);
			if(strstr((char *)u2sat.wifi_rec, "192.168"))		return AT_IPOK;
			if(strstr((char *)u2sat.wifi_rec, "0.0.0.0"))		return AT_IPERR;
			break;
		case DISCONNECT:
			//printf("%s\r\n",u2sat.wifi_rec);
			if(strstr((char *)u2sat.wifi_rec, "WIFI DISCONNECT"))			return DISCONNECT;
			if(strstr((char *)u2sat.wifi_rec, "+MQTTDISCONNECTED"))		return DISCONNECT;
			break;
	}
	return NULL_;
}
/*********************************************************************************************************
* 函 数 名 : TaskKeyMsg_Handle
* 功能说明 : 按键处理
* 形    参 : mode：true连按，false单击
* 返 回 值 : true长按，false短按，其他值无按键按下
* 备    注 : PA0（SWITCH_KEY）
*********************************************************************************************************/ 
void TaskKeyMsg_Handle(void)
{
	switch(Scanf_Key())
	{
		case 1:	
		LED = !LED;
			break;
		case 0:		
			Set_Esp12ConnectionStatus(THREE);	//进入配网模#式
			STMFLASH_Write(STM32_FLASH_BASE, 0, MESSAGE_MAX);//清空STM32_FLASH_BASE闪存的内容
			RevampState(NULL, AT_CWMODE2, AT_CWMODE2, AT_CWMODE2, AT_OK, WAIT_1000, WAIT_100);
			break;   //确定初始化状态为   -------AT_CWMODE2
	}
}

/*********************************************************************************************************
* 函 数 名 : DownData_Handle
* 功能说明 : 处理下发数据
* 形    参 : 无
* 返 回 值 : 无
* 备    注 : 检测到断开连接会触发重连机制，检测到用户发来账号密码会重新配置sta
*********************************************************************************************************/ 
static unsigned char DownData_Handle(void)
{
	unsigned char temp = 0;
	char *sp = NULL;
	if(u2sat.resflag2==true && (Get_Esp12ConnectionStatus()==true || Get_Esp12ConnectionStatus()==THREE))
	{
		u2sat.resflag2 = false;
		u2sat.len2 = 0;
		//printf("%s\r\n", u2sat.wifi_rec);
		if((sp=strstr((char *)u2sat.wifi_rec, "powerstate")) != NULL)
		{
			sp = strstr(sp, ":");
			sp++;
			if(*sp == '1')	
			{
				LED = 0;
				//LED = 0;
			}
			else 			
			{
				LED = 1;
				//LED = 1;
			}
		}
		else if(strstr((char *)u2sat.wifi_rec, "WIFI DISCONNECT") || strstr((char *)u2sat.wifi_rec, "+MQTTDISCONNECTED"))	
		{
			Set_Esp12ConnectionStatus(false);
			temp = DISCONNECT;
			printf("-------------DISCONNECT\r\n");
			printf("%s\r\n",u2sat.wifi_rec);
		}
		else if(strstr((char *)u2sat.wifi_rec, "ssid"))
		{
			if(CheckFlashOrBuffer(ssid, pwsd, false) == true)
			{
				printf("收到的信息包含ssid\r\n");
				temp = WAIT_CONF;
			}
		}
		memset(u2sat.wifi_rec, 0, sizeof(u2sat.wifi_rec));
	}
	return temp;
}

/*********************************************************************************************************
* 函 数 名 : PublishMessage
* 功能说明 : 上报数据
* 形    参 : StructMessage：自行强转指针类型即可上报任意类型的数据
* 返 回 值 : 无
* 备    注 : MQTT固件内部自带15S心跳，所以不需要间隔上报数据也可以保持长连接
*********************************************************************************************************/ 
void PublishMessage(void )	
{	/*一次上报的数据包不能大于缓冲区的大小，字段多时应分段上报，否则......*/
	memset(houst_pwd, 0, sizeof(houst_pwd));
	sprintf((char *)houst_pwd,	
		"AT+MQTTPUB=0,\"%s\",\"{\\\"method\\\":\\\"thing.service.property.set\\\"\\,\\\"id\\\":\\\"2012934115\\\"\\,\
		\\\"params\\\":{\\\"temperature\\\":%0.2f}\\,\\\"version\\\":\\\"1.0.0\\\"}\",1,0\r\n",
		PublishMessageTopPost,
		25.5
	);
	USART2_wifi_SendStr(houst_pwd);
}
void TaskEsp12Msg_Handle(void)
{
	switch(state)
	{
		case AT_AT:	
			RevampState("AT\r\n", WAIT, AT_AT, ECHO, AT_OK, WAIT_1000, WAIT_100);
			break;
		case ECHO:	
			RevampState("ATE1\r\n", WAIT, ECHO, AT_CWMODE1, AT_OK, WAIT_1000, WAIT_100);
			break;
		case AT_CWMODE1:
			RevampState("AT+CWMODE=1\r\n", WAIT, AT_CWMODE1, AT_CIFSR, AT_OK, WAIT_1000, WAIT_200);
			break;
		case AT_CIFSR:
			RevampState("AT+CIFSR\r\n", WAIT, AT_CIFSR, AT_CWJAP, AT_IP, WAIT_1000, WAIT_100);
			break;
		case AT_CWJAP:
			memset(houst_pwd, 0, sizeof(houst_pwd));
			sprintf((char *)houst_pwd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwsd);
			RevampState((char *)houst_pwd, WAIT, AT_CWJAP, AT_MQTTCLEAN, AT_OK, WAIT_10000, WAIT_200);
					memset(houst_pwd, 0, sizeof(houst_pwd));
			sprintf((char *)houst_pwd, "ssid:%s pwsd:%s", ssid, pwsd);
			for(timeout=0; timeout<MESSAGE_MAX; timeout++)
				if(houst_pwd[timeout] == 0)
					break;
			houst_pwd[timeout] = 0xaa;
			STMFLASH_Write(STM32_FLASH_BASE, houst_pwd, MESSAGE_MAX);	//连接成功后更新flash中的信息
			printf("Platform connection successful   %s\r\n",houst_pwd);
			break;
		case AT_CWMODE2:
			RevampState("AT+CWMODE=2\r\n", WAIT, AT_CWMODE2, AT_MQTTCLEAN, AT_OK, WAIT_1000, WAIT_100);
			break;
		case AT_MQTTCLEAN:	
			if(before == AT_CWMODE2)	//如果是从AT_CWMODE2状态过来的，那么下一个状态到AT_CIPMUX
			{
				memset(ssid, 0, MESSAGE_MAX/2);			//清除本地缓冲信息
				memset(pwsd, 0, MESSAGE_MAX/2);
				memset(houst_pwd, 0, sizeof(houst_pwd));//清空STM32_FLASH_BASE闪存的内容
				STMFLASH_Write(STM32_FLASH_BASE, houst_pwd, MESSAGE_MAX);//清空STM32_FLASH_BASE闪存的内容
				RevampState("AT+MQTTCLEAN=0\r\n", WAIT, AT_MQTTCLEAN, AT_CIPMUX, AT_OK|AT_ERR, WAIT_1000, WAIT_200);
			}
			else RevampState("AT+MQTTCLEAN=0\r\n", WAIT, AT_MQTTCLEAN, AT_CIPSNTPCFG, AT_OK|AT_ERR, WAIT_1000, WAIT_200);
			break;
		case AT_CIPSNTPCFG:
			RevampState("AT+CIPSNTPCFG=1,8,\"ntp1.alliyun.com\"\r\n", WAIT, AT_CIPSNTPCFG, AT_MQTTUSERCFG, AT_OK, WAIT_1000, WAIT_200);
			break;
		case AT_MQTTUSERCFG:{
			unsigned char PassWord[50] = {0};
			CalculateSha1(PassWord);
			memset(houst_pwd, 0, sizeof(houst_pwd));
			sprintf((char *)houst_pwd, "AT+MQTTUSERCFG=0,1,\"NULL\",\"%s\",\"%s\",0,0,\"\"\r\n", UserName, "2135E24080A0B43C1C9E215A5EBE8622B92BE9C6");
			RevampState((char *)houst_pwd, WAIT, AT_MQTTUSERCFG, AT_MQTTCLIENTID, AT_OK, WAIT_1000, WAIT_100);
		}	break;
		case AT_MQTTCLIENTID:
			memset(houst_pwd, 0, sizeof(houst_pwd));
			sprintf((char *)houst_pwd, "AT+MQTTCLIENTID=0,\"%s\"\r\n", ClientId);
			RevampState((char *)houst_pwd, WAIT, AT_MQTTCLIENTID, AT_MQTTCONN, AT_OK, WAIT_1000, WAIT_100);
			break;
		case AT_MQTTCONN:
			memset(houst_pwd, 0, sizeof(houst_pwd));
			sprintf((char *)houst_pwd, "AT+MQTTCONN=0,\"%s\",1883,1\r\n", IP);
			RevampState((char *)houst_pwd, WAIT, AT_MQTTCONN, AT_MQTTSUB, AT_OK, WAIT_1000, WAIT_100);
			break;
		case AT_MQTTSUB:
			memset(houst_pwd, 0, sizeof(houst_pwd));
			sprintf((char *)houst_pwd, "AT+MQTTSUB=0,\"%s\",1\r\n", PublishMessageTopSet);
			RevampState((char *)houst_pwd, WAIT, AT_MQTTSUB, FINISH, AT_OK, WAIT_1000, WAIT_100);
			break;
		
		case AT_CIPMUX:
			RevampState("AT+CIPMUX=1\r\n", WAIT, AT_CIPMUX, AT_CWSAP, AT_OK, WAIT_1000, WAIT_100);
			break;
		case AT_CWSAP:
			memset(houst_pwd, 0, sizeof(houst_pwd));
			RevampState("AT+CWSAP=\"WIFI_TCX111\",\"12345678\",3,0,3,0\r\n", 
				WAIT, AT_CWSAP, AT_CIPSERVER, AT_OK, WAIT_1000, WAIT_100);	
			break;
		case AT_CIPSERVER:
			RevampState("AT+CIPSERVER=1,8086\r\n", WAIT,AT_CIPSERVER, WAIT_CONF, AT_OK, WAIT_1000, WAIT_100);
			break;
		case WAIT_CONF:	
			if(DownData_Handle() == WAIT_CONF)
			{
				printf("DownData_Handle\r\n");
				//printf("--%s %s--\r\n", ssid, pwsd);
				Set_Esp12ConnectionStatus(false);
				RevampState(NULL, AT_AT, AT_AT, AT_AT, AT_OK, WAIT_1000, WAIT_100);
				printf("ssid=%s,pwsd=%s\r\n",ssid ,pwsd);
			}
			//printf("downf=%d\r\n",downf);
			break;
			
			
		case AT_RST:	//4S后结束复位状态，回到最开始的状态
			if(--timeout == 0)	
				RevampState("AT\r\n", WAIT, AT_AT, AT_AT, AT_OK, WAIT_1000, WAIT_100);
			break;
		case FINISH:	//完成连接后做一些准备工作，随后跳转空闲状态
			Set_Esp12ConnectionStatus(true);
			LED = 0;
//			memset(houst_pwd, 0, sizeof(houst_pwd));
//			sprintf((char *)houst_pwd, "ssid:%s pwsd:%s", ssid, pwsd);
//			for(timeout=0; timeout<MESSAGE_MAX; timeout++)
//				if(houst_pwd[timeout] == 0)
//					break;
//			houst_pwd[timeout] = 0xaa;
//			STMFLASH_Write(STM32_FLASH_BASE, houst_pwd, MESSAGE_MAX);	//连接成功后更新flash中的信息
//			printf("Platform connection successful   %s\r\n",houst_pwd);
			RevampState(NULL, FREE, FINISH, FREE, AT_OK, WAIT_1000, WAIT_100);
			break;
		case FREE:	
			if(DownData_Handle() == DISCONNECT){			//断线时回到最初的状态
				RevampState("AT\r\n", WAIT, AT_AT, AT_AT, AT_OK, WAIT_1000, WAIT_100);
				printf("RST,DISCONNECT=%d\r\n",DISCONNECT);
			}
			//printf("正常\r\n");
			break;
			
			
	case WAIT:
			response = MeasuringResponse(wishack);   //返回  ok   error
			switch(response)
			{
				case AT_OK:
					if(--interval == 0)	//间隔一定的时间后跳转到下一个状态
					{
						#if WIFI_DEGBUG
						printf("***%d->%d***\r\n", before, next);
						printf("%s\r\n",u2sat.wifi_rec);
						#endif
						count = TRY_COUNT;			//重发次数
						JumpState(before, next);	//跳转到下一个状态
					}	
					break;
				case AT_ERR:
					if(--interval == 0)	//间隔一定的时间后跳转到上一个状态
					{
						#if WIFI_DEGBUG
							printf("###%d->%d###\r\n", before, before);
					  	printf("%s\r\n",u2sat.wifi_rec);
						#endif
						JumpState(before, before);	//跳转到上一个状态			
						if(--count == 0)			//error计数，超过一定次数会触发复位状态
						{
							RevampState("AT+RST\r\n", AT_RST, AT_ERR, AT_RST, AT_OK, WAIT_4000, WAIT_100);
							printf("11111111111111111111111111111111111111111\r\n");
							count = TRY_COUNT;
						}
					}
					break;	
				case AT_IPOK:
					#if WIFI_DEGBUG
						printf("已经获取到ip\r\n");
					#endif
						printf("###%d->%d###\r\n", before, before);
						printf("%s\r\n",u2sat.wifi_rec);
					  RevampState(NULL, AT_MQTTCLEAN, AT_IPOK, AT_MQTTCLEAN, AT_OK, WAIT_1000, WAIT_100);
						printf("获取到ip连接AT_MQTTCLEAN\r\n");
					break;
				case AT_IPERR:
					#if WIFI_DEGBUG
						printf("未获取到ip使用默认HOUST_password\r\n");
				    printf("%s\r\n",u2sat.wifi_rec);
					#endif				
					if((CheckFlashOrBuffer(ssid, pwsd, true)==true) || (ssid[0]!=0 || pwsd[0]!=0))	//如果已经存有账号密码，先尝试连接
					{
						printf("获取存储热点成功ssid：%s pwsd：%s\r\n", ssid, pwsd);
						printf("跳转AT_CWMODE2连接热点\r\n");
						RevampState(NULL, AT_CWJAP, AT_CWJAP, AT_CWJAP, AT_OK, WAIT_1000, WAIT_100);
			  	}
					else								//否则进入sta模式，等待用户输入
					{
						printf("等待用户配置wifi信息\r\n");
						Set_Esp12ConnectionStatus(THREE);
						RevampState(NULL, AT_CWMODE2, AT_IPERR, AT_CWMODE2, AT_OK, WAIT_1000, WAIT_100);
					}
					break;
				
				
		    	}
			break;
		}
	
}
	




/*********************************************************************************************************
* 函 数 名 : CheckFlashOrBuffer
* 功能说明 : 检查内部flash或者串口接收缓冲区中是否存在账号密码
* 形    参 : ssid账号存储区，pwsd密码存储区，mode：true检测内部flash，false检查串口接收缓冲区
* 返 回 值 : true存在，falsh不存在
* 备    注 : 无
*********************************************************************************************************/ 
unsigned char CheckFlashOrBuffer(unsigned char *ssid, unsigned char *pwsd, char mode)
{
	unsigned char cnt = 0;
	unsigned char *sp = NULL;
	memset(houst_pwd, 0, sizeof(houst_pwd));
	if(mode == true)
	{
		
		STMFLASH_Read(STM32_FLASH_BASE, houst_pwd, MESSAGE_MAX);
	}
	else 
	{
		strcpy((char *)houst_pwd, (char *) u2sat.wifi_rec);
	}
	if(strstr((char *)houst_pwd, "ssid") != NULL)
	{
		sp = (unsigned char *)strstr((char *)houst_pwd, ":");
		sp++;
		if(mode == false)
		{
			sp = (unsigned char *)strstr((char *)sp, ":");
			sp++;
		}
		for(cnt=0; cnt<MESSAGE_MAX; cnt++)
		{
			if(*sp == ' ')
				break;
			*ssid++ = *sp++;
		}
		sp = (unsigned char *)strstr((char *)sp, ":");
		sp++;
		for(cnt=0; cnt<MESSAGE_MAX; cnt++)
		{
			if(*sp == 0xaa)	
				break;
			*pwsd++ = *sp++;
		}
	}else return false;
	return true;
}



unsigned char Get_Esp12ConnectionStatus(void)
{
	return ConnectFlag;
}


void Set_Esp12ConnectionStatus(unsigned char value)
{
	ConnectFlag = value;
}

/*********************************************************************************************************
* 函 数 名 : mstrcat
* 功能说明 : 字符串连接
* 形    参 : s1：目标， s2：源
* 返 回 值 : 无
* 备    注 : 无
*********************************************************************************************************/ 
static void mstrcat(char *s1, const char *s2)
{
	if(*s1 != NULL)
		while(*++s1);
	while((*s1++ = *s2++));
}
/*********************************************************************************************************
* 函 数 名 : CalculateSha1
* 功能说明 : 计算sha1密匙
* 形    参 : password：密匙存放缓冲区
* 返 回 值 : 无
* 备    注 : 无
*********************************************************************************************************/ 
static void CalculateSha1(unsigned char *password)
{
	unsigned char temp[3] = {0};
	unsigned char cnt = 0;
	hmac_sha1((unsigned char *)DeviceSecret,32,(unsigned char *)Encryption,46,houst_pwd);
	memset(temp, 0, sizeof(temp));
	for(cnt=0;cnt<20;cnt++)
	{
		sprintf((char *)temp,"%02X",houst_pwd[cnt]);
		mstrcat((char *)password,(char *)temp);
	}
}

