
#ifndef __smc2990_H__
#define __smc2990_H__

#include <stm32f10x.h>
/*

	ֻ��vol_temp.c�ṩ�ӿڣ����������ļ��ṩ�ӿ�

	sm2990 ��3��
	1.4·��ѹ��� ��ַ0x3
	2.cpu�������¶ȼ�� ��ַ0x1
	3.lcdҺ�����¶ȼ�⣨��·�� ��ַ0x0
*/


#define SM2990_DEV_ADDR_VOL  0x3<<1     //��λ��Ч,���һλ�Ƕ�д����
#define SM2990_DEV_ADDR_CPU_TEMP  0x1<<1  //��λ��Ч,���һλ�Ƕ�д
#define SM2990_DEV_ADDR_LCD_TEMP  0x0<<1  //��λ��Ч,���һλ�Ƕ�д


void sm2990_init(void);



// Reads a 14-bit adc_code from SM2990.
//int8_t SM2990_adc_read(uint8_t i2c_address, uint8_t msb_register_address, int16_t *adc_code, int8_t *data_valid);


// Reads len * 14-bit adc_code from SM2990.
int8_t SM2990_adc_read_v1v4(uint8_t i2c_address,uint8_t *dat, uint8_t len);


// Calculates the SM2990 temperature,ֻ���������֣�С�����ֲ����ǣ������¶ȼ���
int16_t SM2990_temperature_int(int16_t adc_code);

#endif
