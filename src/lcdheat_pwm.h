


#ifndef __LCDHEAT_PWM_H__
#define __LCDHEAT_PWM_H__

#include <stm32f10x.h>

void LcdHeat_pwm_init(int degree);

/*
����lcd����ռ�ձ�
*/
void LcdHeat_pwm_out(uint8_t degree);

/*
	����fan PWM��Ƶ��
*/
void LcdHeat_pwm_freq(uint16_t freq);


extern uint8_t g_lcdHeat_pwm;  //���ڱ�������ֵ


void LcdHeat_pwm_Enable(void);
void LcdHeat_pwm_Disable(void);

#endif


