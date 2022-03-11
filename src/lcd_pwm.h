
#ifndef __LCD_PWM_H__
#define __LCD_PWM_H__

#include <stm32f10x.h>

void lcd_pwm_init(uint8_t degree);

void Lcd_pwm_out(uint8_t degree);

//用于上报设置值
extern uint8_t g_lcd_pwm;
#endif

