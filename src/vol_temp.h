
#ifndef __VOL_TEMP_H__
#define __VOL_TEMP_H__


#include <stm32f10x.h>


//extern uint8_t ns18b20_answer;   //18b20��Ӧ����1��ʾ��Ӧ��0��ʾû��



void vol_temp_init(void);

//��õ�ѹֵ
/*
	ͨ����������

	����ֵ��ʾ�ɹ����
		0��ʾ�ɹ�
		�����ʾʧ��
*/
uint8_t get_vol_monitor(uint16_t *cpu_vol,uint16_t *ls7a_vol,uint16_t *p12_vol);


//��������cpu���¶�
//uint8_t get_cpu_board_temp(int16_t *cpu_temp,int16_t *board_temp);



//���lcd���¶�
uint8_t get_lcd_temp(int16_t* lcd1_temp,int16_t* lcd2_temp);


void get_temp_vol_task(void);



/*
	����һ�����񣬶�ʱ�ϱ��¶ȣ���ѹ��״̬��Ϣ
	ÿ500ms����һ�Σ�ÿһ�λ㱨һ����Ϣ
*/
void report_vol_temp_status(void);


//����Һ�����¶��Ƿ�С��-15��С�ڷ���1����С�ڷ���0
int lcd_temp_too_low(void);

#endif

