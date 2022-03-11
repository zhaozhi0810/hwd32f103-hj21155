
#ifndef __IIC_APP_H__
#define __IIC_APP_H__

#include <stm32f10x.h>


void IICapp_init(I2C_TypeDef* I2Cx);

/*
	IICapp写多个数据，最多写入256个字节！！！
	参数：
		word_addr 空间地址，就是要把字节dat写到24c02的哪一个存储单元中去
		dat       实际数据的首地址，
		len       实际需要写入数据的个数
	返回值：
		0  ：     成功
		非0：     失败
*/
uint8_t IICapp_write_bytes(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t word_addr,const uint8_t *dat,uint8_t len);



/*
	随机读数据
	参数 ： word_addr 指定我要读取的位置
			dat 表示存储数据缓存的首地址
			len 表示需要读取数据的个数
	返回值： 0 表示成功
			非0 表示失败
*/
uint8_t IICapp_read_bytes(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t word_addr,uint8_t *dat,uint8_t len);


#endif
