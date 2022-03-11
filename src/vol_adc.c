
/*
	三个电压都是自带的adc采集的。
	
	核电压，7A电压，12V电压
	分别是in11，in12，in13
	
	PC1,2,3
	
*/

#include "includes.h"




//初始化ADC															   
void  Vol_Adc_Init(void)
{    
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  //GPIO时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);  //adc时钟使能
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //使能ADC1时钟
	  /* Initialize the ADC_Mode member */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	/* initialize the ADC_ScanConvMode member */
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//非扫描模式
	/* Initialize the ADC_ContinuousConvMode member */
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//关闭连续转换
	/* Initialize the ADC_ExternalTrigConv member */
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //禁止触发检测，使用软件触发
	/* Initialize the ADC_DataAlign member */
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//右对齐
	/* Initialize the ADC_NbrOfChannel member */
	ADC_InitStructure.ADC_NbrOfChannel = 1;//1个转换在规则序列中 也就是只转换规则序列1 
	
	ADC_Init(ADC2, &ADC_InitStructure);
	
	//先初始化ADC1通道0 IO口 PC1,2,3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_3 ;//PC0 通道1，2，3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//模拟输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化 PC1,2,3 
	
	ADC_Cmd(ADC2, ENABLE);//开启AD转换器	

}				  
//获得ADC值
//ch: @ref ADC_channels 
//通道值 0~16取值范围为：ADC_Channel_0~ADC_Channel_16
//返回值:转换结果
static u16 getAdc(u8 ch)   
{
	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,480个周期,提高采样时间可以提高精确度			    
  
	//ADC_SoftwareStartConv(ADC1);		//使能指定的ADC1的软件转换启动功能	
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);	
	
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC2);	//返回最近一次ADC1规则组的转换结果
}
//获取通道ch的转换值，取times次,然后平均 
//ch:通道编号
//times:获取次数
//返回值:通道ch的times次转换结果平均值
static u16 getAdcAverage(u8 ch,u8 times)
{
	u32 temp_val=0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val+=getAdc(ch);
		//delay_ms(5);
		delay_1ms(5);
	}
	return temp_val/times;
} 
	 


/*
	core电压1.15V  1150mv
*/
u16 ADCgetCoreVol(void)
{	
	return getAdcAverage(ADC_Channel_11,3);
}


/*
	7A电压1.1V   1100mv
*/
u16 ADCget7AVol(void)
{	
	return getAdcAverage(ADC_Channel_12,3);
}


/*
	12V的电压被分压了，需要*11才是实际电压
	得到的值大概也是1090mv左右
*/
u16 ADCget12VVol(void)
{	
	return getAdcAverage(ADC_Channel_13,3);
}


