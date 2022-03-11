
/*
	������ѹ�����Դ���adc�ɼ��ġ�
	
	�˵�ѹ��7A��ѹ��12V��ѹ
	�ֱ���in11��in12��in13
	
	PC1,2,3
	
*/

#include "includes.h"




//��ʼ��ADC															   
void  Vol_Adc_Init(void)
{    
	GPIO_InitTypeDef  GPIO_InitStructure;
	ADC_InitTypeDef       ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  //GPIOʱ��ʹ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);  //adcʱ��ʹ��
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); //ʹ��ADC1ʱ��
	  /* Initialize the ADC_Mode member */
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	/* initialize the ADC_ScanConvMode member */
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;//��ɨ��ģʽ
	/* Initialize the ADC_ContinuousConvMode member */
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;//�ر�����ת��
	/* Initialize the ADC_ExternalTrigConv member */
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //��ֹ������⣬ʹ���������
	/* Initialize the ADC_DataAlign member */
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;//�Ҷ���
	/* Initialize the ADC_NbrOfChannel member */
	ADC_InitStructure.ADC_NbrOfChannel = 1;//1��ת���ڹ��������� Ҳ����ֻת����������1 
	
	ADC_Init(ADC2, &ADC_InitStructure);
	
	//�ȳ�ʼ��ADC1ͨ��0 IO�� PC1,2,3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 |GPIO_Pin_3 ;//PC0 ͨ��1��2��3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;//ģ������
	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ�� PC1,2,3 
	
	ADC_Cmd(ADC2, ENABLE);//����ADת����	

}				  
//���ADCֵ
//ch: @ref ADC_channels 
//ͨ��ֵ 0~16ȡֵ��ΧΪ��ADC_Channel_0~ADC_Channel_16
//����ֵ:ת�����
static u16 getAdc(u8 ch)   
{
	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC2, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,480������,��߲���ʱ�������߾�ȷ��			    
  
	//ADC_SoftwareStartConv(ADC1);		//ʹ��ָ����ADC1�����ת����������	
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);	
	
	while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC2);	//�������һ��ADC1�������ת�����
}
//��ȡͨ��ch��ת��ֵ��ȡtimes��,Ȼ��ƽ�� 
//ch:ͨ�����
//times:��ȡ����
//����ֵ:ͨ��ch��times��ת�����ƽ��ֵ
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
	core��ѹ1.15V  1150mv
*/
u16 ADCgetCoreVol(void)
{	
	return getAdcAverage(ADC_Channel_11,3);
}


/*
	7A��ѹ1.1V   1100mv
*/
u16 ADCget7AVol(void)
{	
	return getAdcAverage(ADC_Channel_12,3);
}


/*
	12V�ĵ�ѹ����ѹ�ˣ���Ҫ*11����ʵ�ʵ�ѹ
	�õ���ֵ���Ҳ��1090mv����
*/
u16 ADCget12VVol(void)
{	
	return getAdcAverage(ADC_Channel_13,3);
}


