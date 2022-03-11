

#ifndef __SYSTICK_DELAY_H__
#define __SYSTICK_DELAY_H__

#include <stdint.h>

//1ms定时
int systick_config(void);

//计算tick的总数，这个计数不准确，
//风险：：如果执行的任务超过1ms，但是这个tick并没有及时增加。
void tasktick_increase(void);
//定时任务。count表示多少ms执行一次
int8_t tasktick_1ms(uint32_t count);

//延时1ms
void delay_1ms(uint32_t ncount);

//systick中断处理函数
void systick_int_update(void);


//us级延时
void delay_us(uint32_t nus);
#endif
