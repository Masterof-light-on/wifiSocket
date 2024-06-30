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


static unsigned char ConnectFlag = false;		//true�������ӣ�false��δ����

static unsigned char ssid[MESSAGE_MAX/2] = {0}, pwsd[MESSAGE_MAX/2] = {0};  //ssid  pwsd
static unsigned char houst_pwd[256];				//������ݻ�����
static unsigned char state 		= AT_AT;	//��ǰ״̬
static unsigned char before 		= AT_AT;	//��һ��״̬
static unsigned char next 		= AT_AT;	//��һ��״̬
static unsigned char wishack 		= AT_OK;	//ϣ��Ӧ��
static unsigned char response 	= AT_OK;	//�õ���Ӧ��
static unsigned short timeout 	= WAIT_100;	//��ʱʱ��
static unsigned short interval	= WAIT_100;	//�������̫������busy...��������100ms����
static unsigned short count 		= TRY_COUNT;//����������һ�������򴥷�ĳ������



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

	//��ʼ��usart�ж�
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=0;
	
	//���������ж�
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	//���������ж�
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);
	
	NVIC_Init(&NVIC_InitStruct);
	
	USART_Init(USART2,&USART_InitStruct);
	USART_Cmd(USART2,ENABLE);
}

void USART2_wifi_Sendbyte(uint16_t Data)
{
	while(!(USART2->SR & 1<<6)); //�ȴ���һ�ε����ݷ������
	USART2->DR = Data;
	while(!(USART2->SR & 1<<6)); //��һ�ε����ݷ������
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
	state = sta;		//�����������ȴ�Ӧ��״̬
	before = bef;		//��¼��һ��״̬
	next = nex;			//��һ��Ҫִ�е�״̬
	wishack	  = ack;	//ϣ����Ӧ��
	timeout = to;		//��ʱʱ��Ϊ100ms
	interval = inte;	//����һ��״̬�ļ��ʱ��Ϊ100ms
	//response = UNKNOWN;	//�����һ���յ���Ӧ��
}


/*********************************************************************************************************
* �� �� �� : JumpState
* ����˵�� : ״̬��ת
* ��    �� : sta����ǰ״̬��mode��true��һ��״̬��false��һ��״̬
* �� �� ֵ : ��
* ��    ע : ��
*********************************************************************************************************/ 
static void JumpState(unsigned char sta, unsigned char offset)
{
	if(sta == AT_MQTTSUB)	state = FINISH;
	else					state = offset;
}


/*********************************************************************************************************
* �� �� �� : MeasuringResponse
* ����˵�� : ���Ӧ��
* ��    �� : ack��ϣ����Ӧ������
* �� �� ֵ : ��NULL_Ϊ��Ӧ��
* ��    ע : ��
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
* �� �� �� : TaskKeyMsg_Handle
* ����˵�� : ��������
* ��    �� : mode��true������false����
* �� �� ֵ : true������false�̰�������ֵ�ް�������
* ��    ע : PA0��SWITCH_KEY��
*********************************************************************************************************/ 
void TaskKeyMsg_Handle(void)
{
	switch(Scanf_Key())
	{
		case 1:	
		LED = !LED;
			break;
		case 0:		
			Set_Esp12ConnectionStatus(THREE);	//��������ģ#ʽ
			STMFLASH_Write(STM32_FLASH_BASE, 0, MESSAGE_MAX);//���STM32_FLASH_BASE���������
			RevampState(NULL, AT_CWMODE2, AT_CWMODE2, AT_CWMODE2, AT_OK, WAIT_1000, WAIT_100);
			break;   //ȷ����ʼ��״̬Ϊ   -------AT_CWMODE2
	}
}

/*********************************************************************************************************
* �� �� �� : DownData_Handle
* ����˵�� : �����·�����
* ��    �� : ��
* �� �� ֵ : ��
* ��    ע : ��⵽�Ͽ����ӻᴥ���������ƣ���⵽�û������˺��������������sta
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
				printf("�յ�����Ϣ����ssid\r\n");
				temp = WAIT_CONF;
			}
		}
		memset(u2sat.wifi_rec, 0, sizeof(u2sat.wifi_rec));
	}
	return temp;
}

/*********************************************************************************************************
* �� �� �� : PublishMessage
* ����˵�� : �ϱ�����
* ��    �� : StructMessage������ǿתָ�����ͼ����ϱ��������͵�����
* �� �� ֵ : ��
* ��    ע : MQTT�̼��ڲ��Դ�15S���������Բ���Ҫ����ϱ�����Ҳ���Ա��ֳ�����
*********************************************************************************************************/ 
void PublishMessage(void )	
{	/*һ���ϱ������ݰ����ܴ��ڻ������Ĵ�С���ֶζ�ʱӦ�ֶ��ϱ�������......*/
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
			STMFLASH_Write(STM32_FLASH_BASE, houst_pwd, MESSAGE_MAX);	//���ӳɹ������flash�е���Ϣ
			printf("Platform connection successful   %s\r\n",houst_pwd);
			break;
		case AT_CWMODE2:
			RevampState("AT+CWMODE=2\r\n", WAIT, AT_CWMODE2, AT_MQTTCLEAN, AT_OK, WAIT_1000, WAIT_100);
			break;
		case AT_MQTTCLEAN:	
			if(before == AT_CWMODE2)	//����Ǵ�AT_CWMODE2״̬�����ģ���ô��һ��״̬��AT_CIPMUX
			{
				memset(ssid, 0, MESSAGE_MAX/2);			//������ػ�����Ϣ
				memset(pwsd, 0, MESSAGE_MAX/2);
				memset(houst_pwd, 0, sizeof(houst_pwd));//���STM32_FLASH_BASE���������
				STMFLASH_Write(STM32_FLASH_BASE, houst_pwd, MESSAGE_MAX);//���STM32_FLASH_BASE���������
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
			
			
		case AT_RST:	//4S�������λ״̬���ص��ʼ��״̬
			if(--timeout == 0)	
				RevampState("AT\r\n", WAIT, AT_AT, AT_AT, AT_OK, WAIT_1000, WAIT_100);
			break;
		case FINISH:	//������Ӻ���һЩ׼�������������ת����״̬
			Set_Esp12ConnectionStatus(true);
			LED = 0;
//			memset(houst_pwd, 0, sizeof(houst_pwd));
//			sprintf((char *)houst_pwd, "ssid:%s pwsd:%s", ssid, pwsd);
//			for(timeout=0; timeout<MESSAGE_MAX; timeout++)
//				if(houst_pwd[timeout] == 0)
//					break;
//			houst_pwd[timeout] = 0xaa;
//			STMFLASH_Write(STM32_FLASH_BASE, houst_pwd, MESSAGE_MAX);	//���ӳɹ������flash�е���Ϣ
//			printf("Platform connection successful   %s\r\n",houst_pwd);
			RevampState(NULL, FREE, FINISH, FREE, AT_OK, WAIT_1000, WAIT_100);
			break;
		case FREE:	
			if(DownData_Handle() == DISCONNECT){			//����ʱ�ص������״̬
				RevampState("AT\r\n", WAIT, AT_AT, AT_AT, AT_OK, WAIT_1000, WAIT_100);
				printf("RST,DISCONNECT=%d\r\n",DISCONNECT);
			}
			//printf("����\r\n");
			break;
			
			
	case WAIT:
			response = MeasuringResponse(wishack);   //����  ok   error
			switch(response)
			{
				case AT_OK:
					if(--interval == 0)	//���һ����ʱ�����ת����һ��״̬
					{
						#if WIFI_DEGBUG
						printf("***%d->%d***\r\n", before, next);
						printf("%s\r\n",u2sat.wifi_rec);
						#endif
						count = TRY_COUNT;			//�ط�����
						JumpState(before, next);	//��ת����һ��״̬
					}	
					break;
				case AT_ERR:
					if(--interval == 0)	//���һ����ʱ�����ת����һ��״̬
					{
						#if WIFI_DEGBUG
							printf("###%d->%d###\r\n", before, before);
					  	printf("%s\r\n",u2sat.wifi_rec);
						#endif
						JumpState(before, before);	//��ת����һ��״̬			
						if(--count == 0)			//error����������һ�������ᴥ����λ״̬
						{
							RevampState("AT+RST\r\n", AT_RST, AT_ERR, AT_RST, AT_OK, WAIT_4000, WAIT_100);
							printf("11111111111111111111111111111111111111111\r\n");
							count = TRY_COUNT;
						}
					}
					break;	
				case AT_IPOK:
					#if WIFI_DEGBUG
						printf("�Ѿ���ȡ��ip\r\n");
					#endif
						printf("###%d->%d###\r\n", before, before);
						printf("%s\r\n",u2sat.wifi_rec);
					  RevampState(NULL, AT_MQTTCLEAN, AT_IPOK, AT_MQTTCLEAN, AT_OK, WAIT_1000, WAIT_100);
						printf("��ȡ��ip����AT_MQTTCLEAN\r\n");
					break;
				case AT_IPERR:
					#if WIFI_DEGBUG
						printf("δ��ȡ��ipʹ��Ĭ��HOUST_password\r\n");
				    printf("%s\r\n",u2sat.wifi_rec);
					#endif				
					if((CheckFlashOrBuffer(ssid, pwsd, true)==true) || (ssid[0]!=0 || pwsd[0]!=0))	//����Ѿ������˺����룬�ȳ�������
					{
						printf("��ȡ�洢�ȵ�ɹ�ssid��%s pwsd��%s\r\n", ssid, pwsd);
						printf("��תAT_CWMODE2�����ȵ�\r\n");
						RevampState(NULL, AT_CWJAP, AT_CWJAP, AT_CWJAP, AT_OK, WAIT_1000, WAIT_100);
			  	}
					else								//�������staģʽ���ȴ��û�����
					{
						printf("�ȴ��û�����wifi��Ϣ\r\n");
						Set_Esp12ConnectionStatus(THREE);
						RevampState(NULL, AT_CWMODE2, AT_IPERR, AT_CWMODE2, AT_OK, WAIT_1000, WAIT_100);
					}
					break;
				
				
		    	}
			break;
		}
	
}
	




/*********************************************************************************************************
* �� �� �� : CheckFlashOrBuffer
* ����˵�� : ����ڲ�flash���ߴ��ڽ��ջ��������Ƿ�����˺�����
* ��    �� : ssid�˺Ŵ洢����pwsd����洢����mode��true����ڲ�flash��false��鴮�ڽ��ջ�����
* �� �� ֵ : true���ڣ�falsh������
* ��    ע : ��
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
* �� �� �� : mstrcat
* ����˵�� : �ַ�������
* ��    �� : s1��Ŀ�꣬ s2��Դ
* �� �� ֵ : ��
* ��    ע : ��
*********************************************************************************************************/ 
static void mstrcat(char *s1, const char *s2)
{
	if(*s1 != NULL)
		while(*++s1);
	while((*s1++ = *s2++));
}
/*********************************************************************************************************
* �� �� �� : CalculateSha1
* ����˵�� : ����sha1�ܳ�
* ��    �� : password���ܳ״�Ż�����
* �� �� ֵ : ��
* ��    ע : ��
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

