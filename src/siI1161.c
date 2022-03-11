
#include "stm32f10x.h"
#include "siI1161.h"
#include "iic_app.h"
//#include "btns_leds.h"



#define SII1161_IIC I2C1
#define SII1161_DEVADDR (0x76)

void SiL1161_init(void)
{
//	IICapp_init(I2C2);  //��������ʼ��
	IICapp_init(I2C1);  //��������ʼ��,�������������ֱ�������ҵĵƺͰ���	
}



//�����������ɲ�������
/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�

	����reg_addr �Ĵ�����ַ
*/
uint8_t SiL1161_read_reg(uint8_t reg_addr,uint8_t dat[])
{	
	uint8_t dev_addr;	
	dev_addr = SII1161_DEVADDR;
	return IICapp_read_bytes(SII1161_IIC,dev_addr,reg_addr,dat,1);
}





/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
	����reg_addr �Ĵ�����ַ
*/
uint8_t SiL1161_write_reg(uint8_t reg_addr,uint8_t dat)
{
	uint8_t dev_addr;
	dev_addr = SII1161_DEVADDR;
	return IICapp_write_bytes(SII1161_IIC,dev_addr,reg_addr,&dat,1);
}




