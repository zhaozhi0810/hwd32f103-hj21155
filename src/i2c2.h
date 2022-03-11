//使用IIC1 挂载M24C02,OLED,LM75AD,HT1382    PB6,PB7
#ifndef __I2C2_H__
#define __I2C2_H__


#include "stm32f10x.h"
#include "sys.h"

#define SDA2_IN()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=(u32)8<<12;}
#define SDA2_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=(u32)2<<12;}
 
//IO操作函数	 
#define IIC2_SCL    PBout(10) //SCL
#define IIC2_SDA    PBout(11) //SDA	 
#define READ_SDA2   PBin(11)  //输入SDA 
 
//IIC所有操作函数
void IIC2_Init(void);                //初始化IIC的IO口				 
void IIC2_Start(void);				//发送IIC开始信号
void IIC2_Stop(void);	  			//发送IIC停止信号
void IIC2_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC2_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC2_Wait_Ack(void); 				//IIC等待ACK信号
void IIC2_Ack(void);					//IIC发送ACK信号
void IIC2_NAck(void);				//IIC不发送ACK信号
 
//void I2C2_WriteByte(uint16_t addr,uint8_t data,uint8_t device_addr);
//uint16_t I2C2_ReadByte(uint16_t addr,uint8_t device_addr,uint8_t ByteNumToRead);//寄存器地址，器件地址，要读的字节数 

#endif

