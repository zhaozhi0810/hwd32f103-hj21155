

#ifndef __sil1161_h__
#define __sil1161_h__

#include "stm32f10x.h"


void SiL1161_init(void);

//�����������ɲ�������
/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�

	����reg_addr �Ĵ�����ַ
*/
uint8_t SiL1161_read_reg(uint8_t reg_addr,uint8_t dat[]);
/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
	����reg_addr �Ĵ�����ַ
*/
uint8_t SiL1161_write_reg(uint8_t reg_addr,uint8_t dat);

#endif

