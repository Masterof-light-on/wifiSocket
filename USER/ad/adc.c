#include "adc.h"


 u16 bufftemp[200];

void ADC1init (void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode= GPIO_Mode_AIN;
	GPIO_InitStruct.GPIO_Pin =GPIO_Pin_0|GPIO_Pin_1;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	
	ADC_InitTypeDef ADC_InitStruct;
	ADC_InitStruct.ADC_Mode              =ADC_Mode_Independent;
	ADC_InitStruct.ADC_DataAlign				 =ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfChannel			 =2;
	ADC_InitStruct.ADC_ScanConvMode      =ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConv  =ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_ContinuousConvMode=ENABLE;
	ADC_Init(ADC1,&ADC_InitStruct);
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_8,1,ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_9,2,ADC_SampleTime_239Cycles5);
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	DMA_InitTypeDef DMA_InitStruct;
	DMA_InitStruct.DMA_BufferSize =200;
	DMA_InitStruct.DMA_DIR =DMA_DIR_PeripheralSRC;
	DMA_InitStruct.DMA_M2M =DMA_M2M_Disable;
	DMA_InitStruct.DMA_MemoryBaseAddr= (u32) bufftemp;
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_Mode=DMA_Mode_Circular;
	DMA_InitStruct.DMA_PeripheralBaseAddr= (u32)&ADC1->DR;
	DMA_InitStruct.DMA_PeripheralDataSize=DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_Priority=DMA_Priority_VeryHigh;

	DMA_Init(DMA1_Channel1, &DMA_InitStruct);
	
	DMA_Cmd(DMA1_Channel1,ENABLE);
	ADC_DMACmd(ADC1,ENABLE);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//分频  ADC不能大于14M  72/6=12m
	ADC_Cmd(ADC1,ENABLE);
	
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
}

/********************************************************************
*函数名 ：      u16 ADC_GetValue (ADC_TypeDef* ADCx, uint8_t ADC_Channel)            
*功能描述 ：ADCx  ADC0-3  。ADC_Channel转换通道0-15
*入口参数 ：ADCx   ADC_Channel
*出口参数 ：u16
*函数返回:  无
*调用提示 ：无   
*********************************************************************/
u16 ADC_GetValue (ADC_TypeDef* ADCx, uint8_t ADC_Channel)
{
			ADC_RegularChannelConfig(ADCx,ADC_Channel,1,ADC_SampleTime_239Cycles5);
			ADC_SoftwareStartConvCmd(ADCx,ENABLE);//开启转换
			while(!ADC_GetFlagStatus(ADCx,ADC_FLAG_EOC));//等待AD转换完成
			return ADC_GetConversionValue(ADCx);
}

ADC1buff adcbuff;

void ADC_FilterMAX(u16*  bufftemp)
{
	u16 i;
	u32 sum[2]={0};
	for(i=0;i<200;i=i+2)
	{
		sum[0] +=bufftemp[i];
		sum[1] +=bufftemp[i+1];
	}
   adcbuff.Vstr=sum[0]/100;
   adcbuff.Vend=sum[1]/100;
}
