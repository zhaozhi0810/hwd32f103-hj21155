


#ifndef __FAN_PWM_H__
#define __FAN_PWM_H__

#include <stm32f10x.h>

void fan_pwm_init(int degree);

/*
设置lcd亮度占空比
*/
void Fan_pwm_out(uint8_t degree);

/*
	设置fan PWM的频率
*/
void setFan_pwm_freq(uint16_t freq);


extern uint8_t g_fan_pwm;  //用于保存设置值

#endif


