/*
	2021-09-24
	第十一个移植的文件。(改为不调速，只开启和关闭，这个文件没写完整！！！！不再使用)
	
	需要使用定时器2和3。  GPB3和GPB4 同时输出相同的波形。同时开启，同时关闭

	主要是
	1.pwm控制风扇
	
*/

#include "includes.h"



void pwm_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_OCInitTypeDef TIM_OCInitStruct;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	//1.TIM2定时器的功能配置 自动重载的数，预分频数值
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;  //递增
	TIM_TimeBaseInitStruct.TIM_Period = 20000-1; //重载的数字，频率20kHZ
	TIM_TimeBaseInitStruct.TIM_Prescaler = (SystemCoreClock/1000000)-1; //预分频数，得到是1Mhz的脉冲
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;    
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	//2.TIM3定时器的功能配置 自动重载的数，预分频数值，与TIM2相同
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
	
	
	//3.pwm设置
	TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;//输出模式 pwm
	TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;//当计数值比 比较器的数值 小的时候输出的电平
	TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //输出使能
	TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStruct.TIM_Pulse = 100; //比较器的数值  //占空比100/20000

	TIM_OC2Init(TIM2,&TIM_OCInitStruct);   //TIM2 CH2
	TIM_OC1Init(TIM3,&TIM_OCInitStruct);   //TIM3 CH1
	
	//4.开启定时器
	TIM_Cmd(TIM2, ENABLE);
	TIM_CtrlPWMOutputs(TIM2,ENABLE);
	
	TIM_Cmd(TIM3, ENABLE);
	TIM_CtrlPWMOutputs(TIM3,ENABLE);
	
}




