

/*
	2021-09-26
	第十一个移植的文件。
	
	需要使用定时器3CH4。  GPC9

	主要是
	1.pwm控制LCD亮度
	
*/

#include "includes.h"
#define LCD_PWM


	
uint16_t PWM_DEGREE_MAX = 4000;   //PWM频率   太高了影响电磁兼容实验？？？
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
	
	GPIO_SetBits(GPIOC, GPIO_Pin_9);   //输出高
#else	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
	
	if(degree > 100)
	{
		degree = 100;
	}

	g_lcd_pwm = degree;   //保存到全局变量中。
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//0.gpio初始化
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;   //复用功能		
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;  //	
	GPIO_Init(GPIOC, &GPIO_InitStruct);  //这个函数，让结构体的内容配置到寄存器中
	
	GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);  //完全重映射 pc9才能使用
	
	//1.TIM3定时器的功能配置 自动重载的数，预分频数值
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //递增
	TIM_TimeBaseInitStruct.TIM_Period = PWM_DEGREE_MAX-1; //重载的数字，频率20kHZ
	TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock/2/1000000)-1; //预分频数，得到是1Mhz的脉冲    
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	
	//3.pwm设置
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;//输出模式 pwm
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;//当计数值比 比较器的数值 小的时候输出的电平
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //输出使能
	TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStruct.TIM_Pulse = (100-degree) * PWM_DEGREE_MAX/100; //比较器的数值  //占空比 50

	TIM_OC4Init(TIM3,&TIM_OCInitStruct);   //TIM3 CH4
	
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
	
	//4.初始化，不使能 2021-12-07	
#endif
}




/*
设置lcd亮度占空比
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
	设置lcdPWM的频率
*/
void setLcd_pwm_freq(uint16_t freq)
{
#ifdef LCD_PWM
	if(freq > 20)
	{
		PWM_DEGREE_MAX = freq;
		TIM_SetAutoreload(TIM3,freq);   //设置频率
	}
#endif	
}
