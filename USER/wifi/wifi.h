#ifndef  WIFI_H_
#define  WIFI_H_
#include "stm32f10x.h"
//#include "io_bit.h"

#define false 0
#define true 1
/***************************************************************************/
/**********************用户需要修改的，其他不要动***************************/
//#define SSID "Nmg"									//热点名称
//#define PWSD "12345679"								//热点密码
#define MODE "TCP"									//连接方式
#define IP   "203.107.45.14"						//服务器IP或域名，203.107.45.14	深圳
#define PORT 1883									//连接端口号，MQTT默认1883

//#define DeviceName 		"FEN11"	
//#define ProductKey 		"a1w5hhwMrXh"
//#define DeviceSecret	"374fe27f993c261eaabaa9c72e4e10c4"

#define DeviceName 		"wificz"								//设备名称
#define ProductKey 		"a15XtSM8gRX"						//产品密匙
#define DeviceSecret	"118cca9815e4de4b8f7b78cbdac4478e"	//设备密匙
/***************************************************************************/
/***************************************************************************/
#define DEGBUG 0
#define SYS "/sys/"
#define LINK "/"
#define TOP "/thing/event/property/"
#define POST "post"
#define SET_  "set"
#define PublishMessageTopPost 	(SYS ProductKey LINK DeviceName TOP POST)
#define PublishMessageTopSet 	(SYS ProductKey LINK DeviceName TOP SET_)

#define Client 		"clientid12345deviceName"					
#define productKey  "productKey"
#define Encryption  (Client DeviceName productKey ProductKey)


#define AND "&"
#define ClientId "wf|securemode=3\\,signmethod=hmacsha1|"		//客户端ID
//wf|securemode=3,signmethod=hmacsha1|
#define UserName (DeviceName AND ProductKey)					//用户名
//#define PassWord "20C185291C1767917341AF3960B15A17D86CB653" //密码		//由代码自动生成



#define MESSAGE_MAX 50				//账号密码总长
#define THREE 3
#define WIFI_DEGBUG 1
typedef struct
{
u8 wifi_rec[256];
u8 len2;
u8 resflag2;
}ITU2;

extern ITU2 u2sat;


typedef enum
{
	NULL_ = 0,
	FREE,				//Detect the data issued by the platform when idle            
	AT_AT,				//Tests AT startup
	ECHO,				//Configures echoing of AT commands
	AT_CWMODE1,			//Sets the Wi-Fi Mode(Station/SoftAP/Station+SoftAP)
	AT_CIFSR,			//Gets the Local IP Address
	AT_CWJAP,			//Connects to an AP
	AT_MQTTCLEAN,		//Close the MQTT Connection
	AT_CIPSNTPCFG,		//Configures the time domain and SNTP server
	AT_MQTTUSERCFG,		//Set MQTT User Config
	AT_MQTTCLIENTID,	//MQTT client configuration
	AT_MQTTCONN,		//Connect to MQTT Broker
	AT_MQTTSUB,			//Subscribe to MQTT Topic
	AT_RST,				//Restarts a module
	FINISH,				//Initialization completed
	DOWN_DATA,			//Platform Downlink Data
	
	AT_CWMODE2,			//Setting the site ap mode 
	AT_CIPMUX,			//Open multiple links
	AT_CWSAP,			//Configuration of the ESP12 SoftAP
	AT_CIPSERVER,		//Deletes/Creates TCP or SSL Server
	WAIT_CONF,			//Wait for the user to configure WiFi
	
	WAIT = 50,			//Waiting for Device Response
	AT_OK,				//Response OK   
	AT_ERR,				//Response error
	AT_IP,				//Check if IP is obtained
	AT_IPOK,			//The IP has been obtained
	AT_IPERR,			//IP not obtained
	DISCONNECT,			//Hot spot disconnection
	MQTTDISCONNECTED,	//MQTT disconnected
	UNKNOWN,			//Unknown
}ESP12_STATE;

typedef enum
{
	WAIT_100 = 10,		//Waiting time unit：10ms，10*10=100ms
	WAIT_200 = 20,		//The following up
	WAIT_300 = 30,
	WAIT_400 = 40, 
	WAIT_500 = 50,
	WAIT_1000 = 100,
	WAIT_4000 = 400,
	WAIT_10000 = 1000,
	WAIT_3000	=300,
	
	TRY_COUNT = 3,		//Retransmission times
}AT_TIME_OUT;


void PublishMessage(void )	;
void wifi_init(void);
void USART2_wifi_SendStr( u8* Data);
void TaskEsp12Msg_Handle(void);
void TaskKeyMsg_Handle(void);
unsigned char CheckFlashOrBuffer(unsigned char *ssid, unsigned char *pwsd, char mode);

unsigned char Get_Esp12ConnectionStatus(void);
void Set_Esp12ConnectionStatus(unsigned char value);
static void CalculateSha1(unsigned char *password);
#endif


