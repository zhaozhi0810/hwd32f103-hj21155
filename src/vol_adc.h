
#ifndef __VAL_ADC_H__
#define __VAL_ADC_H__

#include <stm32f10x.h>

void  Vol_Adc_Init(void);

/*
	core��ѹ1.15V  1150mv
*/
u16 ADCgetCoreVol(void);


/*
	7A��ѹ1.1V   1100mv
*/
u16 ADCget7AVol(void);


/*
	12V�ĵ�ѹ����ѹ�ˣ���Ҫ*11����ʵ�ʵ�ѹ
	�õ���ֵ���Ҳ��1090mv����
*/
u16 ADCget12VVol(void);


#endif
