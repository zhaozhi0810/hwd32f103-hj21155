
#include "stm32f10x.h"
#include "siI1161.h"
#include "iic_app.h"
//#include "btns_leds.h"



#define SII1161_IIC I2C1
#define SII1161_DEVADDR (0x76)

void SiL1161_init(void)
{
//	IICapp_init(I2C2);  //控制器初始化
	IICapp_init(I2C1);  //控制器初始化,两个控制器，分别控制左右的灯和按键	
}



//读到的数据由参数返回
/*
	返回值非0表示错误
		0表示成功

	参数reg_addr 寄存器地址
*/
uint8_t SiL1161_read_reg(uint8_t reg_addr,uint8_t dat[])
{	
	uint8_t dev_addr;	
	dev_addr = SII1161_DEVADDR;
	return IICapp_read_bytes(SII1161_IIC,dev_addr,reg_addr,dat,1);
}





/*
	返回值非0表示错误
		0表示成功
	参数reg_addr 寄存器地址
*/
uint8_t SiL1161_write_reg(uint8_t reg_addr,uint8_t dat)
{
	uint8_t dev_addr;
	dev_addr = SII1161_DEVADDR;
	return IICapp_write_bytes(SII1161_IIC,dev_addr,reg_addr,&dat,1);
}




