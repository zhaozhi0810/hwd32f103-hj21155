/*
	2021-09-24
	��ʮһ����ֲ���ļ���(��Ϊ�����٣�ֻ�����͹رգ�����ļ�ûд����������������ʹ��)
	
	��Ҫʹ�ö�ʱ��2��3��  GPB3��GPB4 ͬʱ�����ͬ�Ĳ��Ρ�ͬʱ������ͬʱ�ر�

	��Ҫ��
	1.pwm���Ʒ���
	
*/

#include "includes.h"



void pwm_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//1.TIM2��ʱ���Ĺ������� �Զ����ص�����Ԥ��Ƶ��ֵ
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //����
	TIM_TimeBaseInitStruct.TIM_Period = 20000-1; //���ص����֣�Ƶ��20kHZ
	TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock/1000000)-1; //Ԥ��Ƶ�����õ���1Mhz������
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;    
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	//2.TIM3��ʱ���Ĺ������� �Զ����ص�����Ԥ��Ƶ��ֵ����TIM2��ͬ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	
	//3.pwm����
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;//���ģʽ pwm
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;//������ֵ�� �Ƚ�������ֵ С��ʱ������ĵ�ƽ
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //���ʹ��
	TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStruct.TIM_Pulse = 100; //�Ƚ�������ֵ  //ռ�ձ�100/20000

	TIM_OC2Init(TIM2,&TIM_OCInitStruct);   //TIM2 CH2
	TIM_OC1Init(TIM3,&TIM_OCInitStruct);   //TIM3 CH1
	
	//4.������ʱ��
	TIM_Cmd(TIM2, ENABLE);
	TIM_CtrlPWMOutputs(TIM2,ENABLE);
	
	TIM_Cmd(TIM3, ENABLE);
	TIM_CtrlPWMOutputs(TIM3,ENABLE);
	
}




