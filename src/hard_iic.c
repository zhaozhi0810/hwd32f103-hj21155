

#include "stm32f10x.h"
#include "hard_iic.h"
//#include "systick_delay.h"  //使用了delay函数

/*
	！！！！！！！！！！！！！！！！！！！
	
	第四个移植的文件。
	
	这个是最底层的控制部分，只由iic_app.c的函数调用。其他不需要调用。

I2C1:
	PB6  SCL 
	PB7  SDA
I2c2:
	PB10  SCL 
	PB11  SDA
*/
static uint8_t iic_ready = 0; //初始化了吗？？


//iic的硬件初始化
void iic_init(I2C_TypeDef* I2Cx)
{		
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	if(I2Cx == I2C1)   //
	{	
		if((iic_ready & 1) == 0)  //防止重复初始化
		{
			//1.时钟使能		
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
			
			//2.gpio的功能配置，复用功能
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;		
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 ;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;  //	
			GPIO_Init(GPIOB, &GPIO_InitStruct);  //这个函数，让结构体的内容配置到寄存器中
			
			
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;   //clk输出		
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 ;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;  //	
			GPIO_Init(GPIOB, &GPIO_InitStruct);  //这个函数，让结构体的内容配置到寄存器中
			iic_ready |= 1;
		}
		else{
			return;  //已经初始化了，直接返回
		}
	}
	else if(I2Cx == I2C2)
	{
		if((iic_ready & 2) == 0)
		{
			//1.时钟使能
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
			
			//2.gpio的功能配置，复用功能
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;		
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 ;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;  //	
			GPIO_Init(GPIOB, &GPIO_InitStruct);  //这个函数，让结构体的内容配置到寄存器中
			
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;	//scl推挽输出	
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 ;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;  //	
			GPIO_Init(GPIOB, &GPIO_InitStruct);  //这个函数，让结构体的内容配置到寄存器中
			
			
			iic_ready |= 2;
		}
		else{
			return; //已经初始化了，直接返回
		}
	}
	else  //参数有误
	{
		return;
	}
	
	//3.iic控制器的初始化
	I2C_StructInit(&I2C_InitStruct);     //应答是非应答
	I2C_InitStruct.I2C_ClockSpeed = 100000;   //最大不能超过400k
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;        /*应答使能 */
	I2C_Init(I2Cx, &I2C_InitStruct);
	
	I2C_Cmd(I2Cx, ENABLE);
	
//	delay_1ms(100);
	
}



//去初始化。2021-12-10 增加。
static void deinit_iic(I2C_TypeDef* I2Cx)
{
//	GPIO_InitTypeDef GPIO_InitStruct;
//	I2C_InitTypeDef I2C_InitStruct;
	
	if(I2Cx == I2C1)   //
	{	
		//1.时钟使能		
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);			
		iic_ready &= ~1;
	}
	else if(I2Cx == I2C2)
	{
		//1.时钟使能
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);		
		iic_ready &= ~2;
	}
	else  //参数有误
	{
		return;
	}
	
	//3.iic控制器的去初始化
	I2C_DeInit(I2Cx);	
	I2C_Cmd(I2Cx, DISABLE);
}


static void iic_restart(I2C_TypeDef* I2Cx)
{
	deinit_iic(I2Cx);
	iic_init(I2Cx);
}


/*
	发送开始信号，前提条件，设备初始化完成
	返回0： 表示成功
		1： 表示失败
*/
int8_t iic_start(I2C_TypeDef* I2Cx)
{
	uint16_t times = 0;
	
	if(I2Cx == I2C1)   //参数检查
	{
		if((iic_ready & 1) == 0)  //未初始化？？？
		{
			iic_init(I2C1);
		}
	}
	else if(I2Cx == I2C2)
	{
		if((iic_ready & 2) == 0)  //未初始化？？？
		{
			iic_init(I2C2);
		}
	}
	else{
		return 1;    //失败
	}
	
	//产生起始信号
	I2C_GenerateSTART(I2Cx, ENABLE);
	
	//检查EV5
	times = 0;
	while(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) != SUCCESS)
	{
		times++;
		if(times > 500)
		{		
			I2C_GenerateSTOP(I2Cx, ENABLE);		
		}
	}
	return 0;
}

void iic_stop(I2C_TypeDef* I2Cx)
{
	I2C_GenerateSTOP(I2Cx, ENABLE);	
}



/*
	前提条件：起始信号已发送
	发送IIC 设备地址（最低位包含读写操作）
	返回值：
		0表示成功
		非0 表示失败
*/
int8_t iic_put_devaddr(I2C_TypeDef* I2Cx,uint8_t dev_addr)
{
	uint32_t envent;
	uint16_t times = 0;
	
	if(dev_addr&1)  //最低位是1，表示读操作
	{
		I2C_Send7bitAddress(I2Cx,dev_addr, I2C_Direction_Receiver);
		envent = I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED;
	}
	else
	{
		I2C_Send7bitAddress(I2Cx,dev_addr, I2C_Direction_Transmitter);
		envent = I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED;
	}

	//检查EV6
	times = 0;
	while(I2C_CheckEvent(I2Cx, envent) != SUCCESS)
	{
		times++;
		if(times > 2000)
		{
			//iic_stop(I2Cx);
			iic_restart(I2Cx);    //2021-12-10
			return -1;
		}
	}	
	return	0;
}


/*
	前提条件：设备地址已发送
	发送数据
*/
int8_t iic_put_byte_data(I2C_TypeDef* I2Cx,uint8_t dat)
{			
	I2C_SendData(I2Cx, dat);	
	return	0;	
}


/*
	读从机IIC的应答位
	
	返回值：
		0 表示应答成功
		1 表示未应答
*/
uint8_t iic_get_ack(I2C_TypeDef* I2Cx)
{
	uint16_t times = 0;
	//检查EV8_2,发送完成吗？
	times = 0;
	while(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED) != SUCCESS)
	{
		times++;
		if(times > 2000)
		{
			iic_stop(I2Cx);
			return 1;
		}
	}
	return 0;
}

/*
	读一个字节操作：
		先决条件： 开始信号正常，读操作位已发送

	返回值：
		高8位： 0 表示读取成功
				非0 表示失败
		低8位： 成功时，表示收到的数据
*/
uint16_t iic_get_byte_data(I2C_TypeDef* I2Cx)
{
	uint8_t dat;
	uint16_t times = 0;

	//ev7
	while(I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) != SUCCESS)
	{
		times++;
		if(times > 1000)
		{
			//iic_stop(I2Cx);
			iic_restart(I2Cx);    //2021-12-10
			return 1<<8;    //错误
		}
	}
			
	dat = I2C_ReceiveData(I2Cx);
	
	return dat;
}


/*
	设置应答
*/
void iic_send_ack(I2C_TypeDef* I2Cx,uint8_t ack)
{
	if(ack == NO_ACK)
	{
		I2C_NACKPositionConfig(I2Cx, I2C_NACKPosition_Current);
	}
	else
	{
		I2C_AcknowledgeConfig(I2Cx, ENABLE);
	}
}




