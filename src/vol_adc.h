
#ifndef __VAL_ADC_H__
#define __VAL_ADC_H__

#include <stm32f10x.h>

void  Vol_Adc_Init(void);

/*
	core电压1.15V  1150mv
*/
u16 ADCgetCoreVol(void);


/*
	7A电压1.1V   1100mv
*/
u16 ADCget7AVol(void);


/*
	12V的电压被分压了，需要*11才是实际电压
	得到的值大概也是1090mv左右
*/
u16 ADCget12VVol(void);


#endif
