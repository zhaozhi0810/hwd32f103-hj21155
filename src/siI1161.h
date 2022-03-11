

#ifndef __sil1161_h__
#define __sil1161_h__

#include "stm32f10x.h"


void SiL1161_init(void);

//读到的数据由参数返回
/*
	返回值非0表示错误
		0表示成功

	参数reg_addr 寄存器地址
*/
uint8_t SiL1161_read_reg(uint8_t reg_addr,uint8_t dat[]);
/*
	返回值非0表示错误
		0表示成功
	参数reg_addr 寄存器地址
*/
uint8_t SiL1161_write_reg(uint8_t reg_addr,uint8_t dat);

#endif

