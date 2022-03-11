
#ifndef __INCLUDE_H__
#define __INCLUDE_H__

#include <stm32f10x.h>

#include <stdint.h>
#include <string.h>



//调试出输出，不需要输出的时候，注释掉宏
#define DEBUG
#ifdef  DEBUG
#define MY_DEBUG(fmt, ...)	printf("%s, %d, %s "fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define MY_DEBUG(...)      
#endif





#include "systick_delay.h"
#include "btns_leds.h"
//#include "hard_iic.h"
#include "iic_app.h"
#include "Nca9555.h"
#include "task_cfg.h"
#include "32_gpios.h"
#include "usart3.h"
#include "vol_temp.h"
#include "lcd_pwm.h"
#include "vol_adc.h"
//#include "fan_pwm.h"   //2021-12-09 风扇不使用pwm控制
//#include "18B20.h"   //采集lcd温度
#include "hce75ms.h"  //采集主板和cpu温度
#include <stdio.h>
#include "DS18B20.h"
//#include "siI1161.h"

#define LCD_PWM_HEAT  //lcd使用pwm加热
#ifdef LCD_PWM_HEAT
#include "lcdheat_pwm.h"
#endif

#endif



