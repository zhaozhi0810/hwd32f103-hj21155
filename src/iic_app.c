
#include "stm32f10x.h"
#include "i2c2.h"
#include "i2c1.h"
/*
	第五个移植的文件。
	
	这里是IIC的应用层，实现两个函数
		需要调用底层的hard_iiC.c中的实现，或者使用模拟的方式实现
		
	1. 任意位置读操作  IICapp_read_bytes
	2. 任意位置写操作  IICapp_write_bytes
	
*/

void IICapp_init(I2C_TypeDef* I2Cx)
{
//	iic_init(I2Cx);
	if(I2Cx == I2C1)	
		IIC_Init();
	else if (I2Cx == I2C2)
		IIC2_Init();
}

/*
	当前位置读多个字节。最多一次读取256个字节！！！！
	返回    0 表示成功，非0表示失败
	!!!!!不对外提供接口
*/
static uint8_t IICapp_read_byte_cur(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t *dat,uint8_t len)
{
	uint8_t i;
	
	if(I2Cx == I2C1)
	{
		IIC_Start();
				
		//2.发送设备地址
		IIC_Send_Byte(dev_addr | 1);	    //发器件地址
		if(IIC_Wait_Ack()!= 0) 
		//if(iic_put_devaddr(I2Cx,dev_addr | 1) != 0)   //最低位是1，表示读操作
		{
			//printf("i2c  read_byte_cur send dev addr error!\n");
			return 2;
		}
		
		for(i=0;i<len;i++)
		{					
			//3.获得一个字节的数据
			//dat[i] = iic_get_byte_data(I2Cx);
		
			//4.只有最后一个数据发送非应答
			if(i == len -1)
			{
				dat[i]=IIC_Read_Byte(0);
			}
			else
			{
				dat[i]=IIC_Read_Byte(1);
			}
				
		}		
		//.发送stop时序
		IIC_Stop();//产生一个停止条件
		
	}
	else if(I2Cx == I2C2)
	{
		IIC2_Start();
				
		//2.发送设备地址
		IIC2_Send_Byte(dev_addr | 1);	    //发器件地址
		if(IIC2_Wait_Ack()!= 0) 
		//if(iic_put_devaddr(I2Cx,dev_addr | 1) != 0)   //最低位是1，表示读操作
		{
			//printf("i2c  read_byte_cur send dev addr error!\n");
			return 2;
		}
		
		for(i=0;i<len;i++)
		{					
			//3.获得一个字节的数据
			//dat[i] = iic_get_byte_data(I2Cx);
		
			//4.只有最后一个数据发送非应答
			if(i == len -1)
			{
				dat[i]=IIC2_Read_Byte(0);
			}
			else
			{
				dat[i]=IIC2_Read_Byte(1);
			}
				
		}		
		//.发送stop时序
		//iic_stop(I2Cx);     //发送停止信号，总线就空闲了
		IIC2_Stop();//产生一个停止条件	
	}
	
	return 0;
}

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
uint8_t IICapp_write_bytes(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t word_addr,const uint8_t *dat,uint8_t len)
{
	uint8_t i;
	
	if(I2Cx == I2C1)
	{
		IIC_Start();
		
		//2.发送设备地址
		IIC_Send_Byte(dev_addr & 0xfe);	    //发器件地址
		if(IIC_Wait_Ack()!= 0) 
		{
			//printf("i2c  read_byte_cur send dev addr error!\n");
			IIC_Stop();
			return 2;
		}

		//3.发送空间地址
		IIC_Send_Byte(word_addr);     //发送字节							    
		if(IIC_Wait_Ack()!= 0)  //如果没有应答，直接退出
		{
			//printf("send word addr error!\n");
			IIC_Stop(); //iic_stop(I2Cx);     //发送停止信号，总线就空闲了
			return 3;
		}
		
		//len等于0的时候，我认为是随机读的一个有效操作
		if(len == 0)
		{
			return 255;   //这是个特殊情况
		}
		
		for(i=0;i<len;i++)
		{		
			//4.发送内容
			IIC_Send_Byte(dat[i]);     //发送字节							    
			if(IIC_Wait_Ack()!= 0)  //如果没有应答，直接退出
			//if(iic_put_byte_data(I2Cx,dat[i]))  //如果没有应答，直接退出
			{
				//printf("send data error!\n");
				IIC_Stop(); //iic_stop(I2Cx);     //发送停止信号，总线就空闲了
				return 4;
			}
		}
		
		//5.结束，结束总线的占用
		IIC_Stop(); //iic_stop(I2Cx);
	}
	else if(I2Cx == I2C2)
	{
		IIC2_Start();
		
		//2.发送设备地址
		IIC2_Send_Byte(dev_addr & 0xfe);	    //发器件地址
		if(IIC2_Wait_Ack()!= 0) 
		{
			//printf("i2c  read_byte_cur send dev addr error!\n");
			return 2;
		}

		//3.发送空间地址
		IIC2_Send_Byte(word_addr);     //发送字节							    
		if(IIC2_Wait_Ack()!= 0)  //如果没有应答，直接退出
		{
			//printf("send word addr error!\n");
			IIC2_Stop(); //iic_stop(I2Cx);     //发送停止信号，总线就空闲了
			return 3;
		}
		
		//len等于0的时候，我认为是随机读的一个有效操作
		if(len == 0)
		{
			return 255;   //这是个特殊情况
		}
		
		for(i=0;i<len;i++)
		{		
			//4.发送内容
			IIC2_Send_Byte(dat[i]);     //发送字节							    
			if(IIC2_Wait_Ack()!= 0)  //如果没有应答，直接退出
			{
				//printf("send data error!\n");
				IIC2_Stop(); //iic_stop(I2Cx);     //发送停止信号，总线就空闲了
				return 4;
			}
		}		
		//5.结束，结束总线的占用
		IIC2_Stop(); //iic_stop(I2Cx);
	}	
	
	return 0;
}





/*
	随机读数据
	参数 ： word_addr 指定我要读取的位置
			dat 表示存储数据缓存的首地址
			len 表示需要读取数据的个数
	返回值： 0 表示成功
			非0 表示失败
*/
uint8_t IICapp_read_bytes(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t word_addr,uint8_t *dat,uint8_t len)
{
	uint8_t ret;
	
	ret = IICapp_write_bytes(I2Cx,dev_addr,word_addr,(void*)0,0);
	
	if(ret == 255)	//特殊情况的处理
		return IICapp_read_byte_cur(I2Cx,dev_addr,dat,len);
	
	return ret;   //在这返回
}

