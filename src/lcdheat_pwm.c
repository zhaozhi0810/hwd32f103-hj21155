/*
	2021-12-16  增加	
	需要使用定时器1。  GPB15 输出波形。定时器1，反向通道3

	主要是
	1.pwm控制lcd加热功率，控制整体电源功耗
	
*/

#include "includes.h"


#ifdef LCD_PWM_HEAT

uint8_t g_lcdHeat_pwm = 0;  //用于保存设置值

uint16_t LCDHEAT_PWM_DEGREE_MAX = 5000 ;  //200hz



void LcdHeat_pwm_init(int degree)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;
		
	if(degree == 0)   //2022-0104 修改，禁止加热
	{
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;   //复用功能		
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 ;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;  //	
		GPIO_Init(GPIOB, &GPIO_InitStruct);  //这个函数，让结构体的内容配置到寄存器中		
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);   //输出低电平		
		
		g_lcdHeat_pwm = 0;  //2022-0104 修改，禁止加热
		return ;
	}
		
	if(degree > 100)
	{
		degree = 100;
	}

	g_lcdHeat_pwm = degree;   //保存到全局变量中。
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	//0.gpio初始化
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;   //复用功能		
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;  //	
	GPIO_Init(GPIOB, &GPIO_InitStruct);  //这个函数，让结构体的内容配置到寄存器中
	
	//GPIO_PinRemapConfig(GPIO_FullRemap_TIM3, ENABLE);  //完全重映射 pc9才能使用
	
	//1.TIM3定时器的功能配置 自动重载的数，预分频数值
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //递增
	TIM_TimeBaseInitStruct.TIM_Period = LCDHEAT_PWM_DEGREE_MAX-1; //重载的数字，频率20kHZ
	TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock/1000000)-1; //预分频数，得到是1Mhz的脉冲    
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStruct);
	
	
	//3.pwm设置
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;//输出模式 pwm
//	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;//当计数值比 比较器的数值 小的时候输出的电平
//	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //输出使能
	TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStruct.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
//	TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;

	TIM_OCInitStruct.TIM_Pulse = (100-degree) * LCDHEAT_PWM_DEGREE_MAX/100; //比较器的数值  //占空比 50

	TIM_OC3Init(TIM1,&TIM_OCInitStruct);   //TIM1 CH3
	
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
		
	//4.初始化，不使能 2021-12-07		
}


void LcdHeat_pwm_Enable(void)
{
	LcdHeat_pwm_init(25);   //占空比25
	TIM_CtrlPWMOutputs(TIM1,ENABLE);
	TIM_Cmd(TIM1,ENABLE);
}


void LcdHeat_pwm_Disable(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	//1.关闭定时器
	TIM_CtrlPWMOutputs(TIM1,DISABLE);
	TIM_Cmd(TIM1,DISABLE);
	
	g_lcdHeat_pwm = 0;	//2022-01-13 v1.4 //g_lcd_pwm = 0;  //停止加热了。  2022-0104 
	
	//2.io引脚输出低
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;   //复用功能		
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;  //	
	GPIO_Init(GPIOB, &GPIO_InitStruct);  //这个函数，让结构体的内容配置到寄存器中	
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);   //输出低电平
}

/*
设置lcd加热占空比
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
	设置lcdPWM加热的频率
*/
void LcdHeat_pwm_freq(uint16_t freq)
{
	if(freq > 20)
	{
		LCDHEAT_PWM_DEGREE_MAX = freq;
		TIM_SetAutoreload(TIM1,freq);   //设置频率
	}
}




#endif

