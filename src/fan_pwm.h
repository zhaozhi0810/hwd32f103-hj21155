


#ifndef __FAN_PWM_H__
#define __FAN_PWM_H__

#include <stm32f10x.h>

void fan_pwm_init(int degree);

/*
����lcd����ռ�ձ�
*/
void Fan_pwm_out(uint8_t degree);

/*
	����fan PWM��Ƶ��
*/
void setFan_pwm_freq(uint16_t freq);


extern uint8_t g_fan_pwm;  //���ڱ�������ֵ

#endif


