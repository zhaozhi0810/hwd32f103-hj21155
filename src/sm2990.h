
#ifndef __smc2990_H__
#define __smc2990_H__

#include <stm32f10x.h>
/*

	只对vol_temp.c提供接口，不对其他文件提供接口

	sm2990 有3个
	1.4路电压监控 地址0x3
	2.cpu和主板温度检测 地址0x1
	3.lcd液晶屏温度检测（两路） 地址0x0
*/


#define SM2990_DEV_ADDR_VOL  0x3<<1     //两位有效,最低一位是读写控制
#define SM2990_DEV_ADDR_CPU_TEMP  0x1<<1  //两位有效,最低一位是读写
#define SM2990_DEV_ADDR_LCD_TEMP  0x0<<1  //两位有效,最低一位是读写


void sm2990_init(void);



// Reads a 14-bit adc_code from SM2990.
//int8_t SM2990_adc_read(uint8_t i2c_address, uint8_t msb_register_address, int16_t *adc_code, int8_t *data_valid);


// Reads len * 14-bit adc_code from SM2990.
int8_t SM2990_adc_read_v1v4(uint8_t i2c_address,uint8_t *dat, uint8_t len);


// Calculates the SM2990 temperature,只有整数部分，小数部分不考虑，摄氏温度计算
int16_t SM2990_temperature_int(int16_t adc_code);

#endif
