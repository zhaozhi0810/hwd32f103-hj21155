

/*
	2021-09-26
	��ʮһ����ֲ���ļ���
	
	��Ҫʹ�ö�ʱ��3CH4��  GPC9

	��Ҫ��
	1.pwm����LCD����
	
*/

#include "includes.h"
#define LCD_PWM


	
uint16_t PWM_DEGREE_MAX = 4000;   //PWMƵ��   ̫����Ӱ���ż���ʵ�飿����
uint8_t g_lcd_pwm = 100;



void lcd_pwm_init(uint8_t degree)
{
#ifndef LCD_PWM	
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_SetBits(GPIOC, GPIO_Pin_9);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;  //
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_SetBits(GPIOC, GPIO_Pin_9);   //�����
#else	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	if(degree > 100)
	{
		degree = 100;
	}

	g_lcd_pwm = degree;   //���浽ȫ�ֱ����С�
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//0.gpio��ʼ��
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;   //���ù���		
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;  //	
	GPIO_Init(GPIOC, &GPIO_InitStruct);  //����������ýṹ����������õ��Ĵ�����
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);  //��ȫ��ӳ�� pc9����ʹ��
	
	//1.TIM3��ʱ���Ĺ������� �Զ����ص�����Ԥ��Ƶ��ֵ
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //����
	TIM_TimeBaseInitStruct.TIM_Period = PWM_DEGREE_MAX-1; //���ص����֣�Ƶ��20kHZ
	TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock/2/1000000)-1; //Ԥ��Ƶ�����õ���1Mhz������    
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	
	//3.pwm����
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;//���ģʽ pwm
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;//������ֵ�� �Ƚ�������ֵ С��ʱ������ĵ�ƽ
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //���ʹ��
	TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStruct.TIM_Pulse = (100-degree) * PWM_DEGREE_MAX/100; //�Ƚ�������ֵ  //ռ�ձ� 50

	TIM_OC4Init(TIM3,&TIM_OCInitStruct);   //TIM3 CH4
	
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
	
	//4.��ʼ������ʹ�� 2021-12-07	
#endif
}




/*
����lcd����ռ�ձ�
*/
void Lcd_pwm_out(uint8_t degree)
{
#ifdef LCD_PWM
	uint32_t value;

//	if(PWM_DEGREE_MIN > degree) degree = PWM_DEGREE_MIN;
//	if(PWM_DEGREE_MAX < degree) degree = PWM_DEGREE_MAX;
	
	if(degree > 100)
	{
		degree = 100;
	}
	g_lcd_pwm = degree; 
	
	value = (100-degree) * PWM_DEGREE_MAX / 100;
	/* CH configuration in PWM mode1,duty cycle  */
	//timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,value);
	TIM_SetCompare4(TIM3, value);
#endif	
}


/*
	����lcdPWM��Ƶ��
*/
void setLcd_pwm_freq(uint16_t freq)
{
#ifdef LCD_PWM
	if(freq > 20)
	{
		PWM_DEGREE_MAX = freq;
		TIM_SetAutoreload(TIM3,freq);   //����Ƶ��
	}
#endif	
}
