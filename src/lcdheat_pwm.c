/*
	2021-12-16  ����	
	��Ҫʹ�ö�ʱ��1��  GPB15 ������Ρ���ʱ��1������ͨ��3

	��Ҫ��
	1.pwm����lcd���ȹ��ʣ����������Դ����
	
*/

#include "includes.h"


#ifdef LCD_PWM_HEAT

uint8_t g_lcdHeat_pwm = 0;  //���ڱ�������ֵ

uint16_t LCDHEAT_PWM_DEGREE_MAX = 5000 ;  //200hz



void LcdHeat_pwm_init(int degree)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
		
	if(degree == 0)   //2022-0104 �޸ģ���ֹ����
	{
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;   //���ù���		
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 ;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;  //	
		GPIO_Init(GPIOB, &GPIO_InitStruct);  //����������ýṹ����������õ��Ĵ�����		
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);   //����͵�ƽ		
		
		g_lcdHeat_pwm = 0;  //2022-0104 �޸ģ���ֹ����
		return ;
	}
		
	if(degree > 100)
	{
		degree = 100;
	}

	g_lcdHeat_pwm = degree;   //���浽ȫ�ֱ����С�
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	//0.gpio��ʼ��
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;   //���ù���		
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;  //	
	GPIO_Init(GPIOB, &GPIO_InitStruct);  //����������ýṹ����������õ��Ĵ�����
	
	//GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);  //��ȫ��ӳ�� pc9����ʹ��
	
	//1.TIM3��ʱ���Ĺ������� �Զ����ص�����Ԥ��Ƶ��ֵ
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //����
	TIM_TimeBaseInitStruct.TIM_Period = LCDHEAT_PWM_DEGREE_MAX-1; //���ص����֣�Ƶ��20kHZ
	TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock/1000000)-1; //Ԥ��Ƶ�����õ���1Mhz������    
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);
	
	
	//3.pwm����
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;//���ģʽ pwm
//	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;//������ֵ�� �Ƚ�������ֵ С��ʱ������ĵ�ƽ
//	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //���ʹ��
	TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
//	TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;

	TIM_OCInitStruct.TIM_Pulse = (100-degree) * LCDHEAT_PWM_DEGREE_MAX/100; //�Ƚ�������ֵ  //ռ�ձ� 50

	TIM_OC3Init(TIM1,&TIM_OCInitStruct);   //TIM1 CH3
	
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
		
	//4.��ʼ������ʹ�� 2021-12-07		
}


void LcdHeat_pwm_Enable(void)
{
	LcdHeat_pwm_init(25);   //ռ�ձ�25
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
	TIM_Cmd(TIM1,ENABLE);
}


void LcdHeat_pwm_Disable(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//1.�رն�ʱ��
	TIM_CtrlPWMOutputs(TIM1,DISABLE);
	TIM_Cmd(TIM1,DISABLE);
	
	g_lcdHeat_pwm = 0;	//2022-01-13 v1.4 //g_lcd_pwm = 0;  //ֹͣ�����ˡ�  2022-0104 
	
	//2.io���������
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;   //���ù���		
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;  //	
	GPIO_Init(GPIOB, &GPIO_InitStruct);  //����������ýṹ����������õ��Ĵ�����	
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);   //����͵�ƽ
}

/*
����lcd����ռ�ձ�
*/
void LcdHeat_pwm_out(uint8_t degree)
{
	uint32_t value;
	
	if(degree > 100)
	{
		degree = 100;
	}
	g_lcd_pwm = degree; 
	
	value = (100-degree) * LCDHEAT_PWM_DEGREE_MAX / 100;
	/* CH configuration in PWM mode1,duty cycle  */
	//timer_channel_output_pulse_value_config(TIMER1,TIMER_CH_0,value);
	TIM_SetCompare3(TIM1, value);
}


/*
	����lcdPWM���ȵ�Ƶ��
*/
void LcdHeat_pwm_freq(uint16_t freq)
{
	if(freq > 20)
	{
		LCDHEAT_PWM_DEGREE_MAX = freq;
		TIM_SetAutoreload(TIM1,freq);   //����Ƶ��
	}
}




#endif

