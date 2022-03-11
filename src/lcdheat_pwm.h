


#ifndef __LCDHEAT_PWM_H__
#define __LCDHEAT_PWM_H__

#include <stm32f10x.h>

void LcdHeat_pwm_init(int degree);

/*
设置lcd亮度占空比
*/
void LcdHeat_pwm_out(uint8_t degree);

/*
	设置fan PWM的频率
*/
void LcdHeat_pwm_freq(uint16_t freq);


extern uint8_t g_lcdHeat_pwm;  //用于保存设置值


void LcdHeat_pwm_Enable(void);
void LcdHeat_pwm_Disable(void);

#endif


