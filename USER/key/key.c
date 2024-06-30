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
* �� �� �� : Scanf_Key
* ����˵�� : ����ɨ��
* ��    �� : ��
* �� �� ֵ : true������false�̰�������ֵ�ް�������
* ��    ע : PA0��SWITCH_KEY��
*********************************************************************************************************/ 
char Scanf_Key(void)
{
	static unsigned char state = 0;
	static unsigned char cnt = 0;
	switch(state){
		case 0:
			if(KEY1 == 0){	//����Ƿ��£�������£���������״̬
				delay_ms(50);//����״̬
			if(KEY1 == 0)		
				state = 1;
			else
				state = 0;
			}
			break;
		case 1:
			if(KEY1 == 0)	//������µ�ʱ�䳬��1s����ô��Ϊ�ǳ��������ؽ������������һ��״̬Ϊ�ȴ�����̬
			{
				if(++cnt > 100)
				{
					state = 3;
					cnt=0;
					return 0;
				}
			}
			else					//�����1s��ʱ�������֣���ô��Ϊ�̰������ؽ������������һ��״̬Ϊ�ȴ�����̬
			{
				state = 3;
				cnt=0;
				return 1;
			}
			break;
		case 3:
			if(KEY1 == 1)	//������֣�����Ϊ���ΰ��½������ص������״̬
				state = 0;
			break;
	}
	return 100;
}

