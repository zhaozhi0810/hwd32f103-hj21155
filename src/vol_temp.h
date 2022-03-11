
#ifndef __VOL_TEMP_H__
#define __VOL_TEMP_H__


#include <stm32f10x.h>


//extern uint8_t ns18b20_answer;   //18b20有应答吗，1表示有应答，0表示没有



void vol_temp_init(void);

//获得电压值
/*
	通过参数返回

	返回值表示成功与否
		0表示成功
		非零表示失败
*/
uint8_t get_vol_monitor(uint16_t *cpu_vol,uint16_t *ls7a_vol,uint16_t *p12_vol);


//获得主板和cpu的温度
//uint8_t get_cpu_board_temp(int16_t *cpu_temp,int16_t *board_temp);



//获得lcd的温度
uint8_t get_lcd_temp(int16_t* lcd1_temp,int16_t* lcd2_temp);


void get_temp_vol_task(void);



/*
	增加一个任务，定时上报温度，电压等状态信息
	每500ms进入一次，每一次汇报一种信息
*/
void report_vol_temp_status(void);


//返回液晶的温度是否小于-15，小于返回1，不小于返回0
int lcd_temp_too_low(void);

#endif

