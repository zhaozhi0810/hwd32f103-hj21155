
#include "includes.h"
#include "i2c.h"

#define SDA_PIN GPIO_Pin_7
#define SCL_PIN GPIO_Pin_6

#define SDA2_PIN GPIO_Pin_11
#define SCL2_PIN GPIO_Pin_10


#define SDA_IN()  do{GPIOB->CRL &= 0X0FFFFFFF; GPIOB->CRL |= 4<<28;}while(0)    //输入 0100 输入浮空
#define SDA_OUT() do{GPIOB->CRL &= 0X0FFFFFFF; GPIOB->CRL |= 2<<28;} while(0)   //输出  推挽，2m输出
#define SDA2_IN()  do{GPIOB->CRH &= 0XFFFF0FFF; GPIOB->CRL |= 4<<12;}while(0)
#define SDA2_OUT() do{GPIOB->CRH &= 0XFFFF0FFF; GPIOB->CRL |= 2<<12;}while(0)


static uint8_t iic_ready = 0; //初始化了吗？？


/*
	i2c1 软件模拟时序实现
	
	初始化程序
	
	配置引脚的属性
	
	
	PB6  SCL OD输出
	PB7  SDA 有输入有输出，OD输出模式
*/
void iic_init(I2C_TypeDef* I2Cx)
{	
	GPIO_InitTypeDef GPIO_InitStruct;  //定义结构体，把需要配置的内容填写到结构体中
	//1.时钟使能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	if(I2Cx == I2C1)
	{
		if((iic_ready & 1) == 0)  //防止重复初始化
		{
			//1.设置引脚为高		
			GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7 );	
			
			//3.配置为输出方式，输出高电平（熄灭）
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 ;
		//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
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
			//1.设置引脚为高		
			GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11 );	
			
			//3.配置为输出方式，输出高电平（熄灭）
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
		//	GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;   //开漏输出模式
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 ;
		//	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;  //	
			GPIO_Init(GPIOB, &GPIO_InitStruct);  //这个函数，让结构体的内容配置到寄存器中
			iic_ready |= 2;
		}
		else{
			return; //已经初始化了，直接返回
		}
	}	
}


/*
	发送开始信号，前提条件，设备初始化完成
	返回0： 表示成功
		1： 表示失败
*/
int8_t iic_start(I2C_TypeDef* I2Cx)
{
	if(I2Cx == I2C1)
	{
		if((iic_ready & 1) == 0)
		{
			iic_init(I2C1);
		}
		//1.
		SDA_OUT();
		GPIO_SetBits(GPIOB, SDA_PIN | SCL_PIN );    //两个引脚都是高电平
		delay_us(2);
		
		//SCL为高电平的时候，SDA由高变低
		GPIO_ResetBits(GPIOB,  SDA_PIN );   //SDA变低电平
		delay_us(2);
		
		//？？？要不要把SCL拉低
		GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平	
	}
	else if(I2Cx == I2C2)
	{
		if((iic_ready & 2) == 0)
		{
			iic_init(I2C2);
		}
		//1.
		SDA2_OUT();
		GPIO_SetBits(GPIOB, SDA2_PIN | SCL2_PIN );    //两个引脚都是高电平
		delay_us(2);
		
		//SCL为高电平的时候，SDA由高变低
		GPIO_ResetBits(GPIOB,  SDA2_PIN );   //SDA变低电平
		delay_us(2);
		
		//？？？要不要把SCL拉低
		GPIO_ResetBits(GPIOB,  SCL2_PIN );   //SCL变低电平	
	}
	return 0;
}



void iic_stop(I2C_TypeDef* I2Cx)
{
	if(I2Cx == I2C1)
	{
		SDA_OUT();
		//SCL为低电平
		GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
		//SDA为低电平
		GPIO_ResetBits(GPIOB,  SDA_PIN );   //SDA变低电平
		
		delay_us(2);
		
		GPIO_SetBits(GPIOB, SCL_PIN );   //SCL为高
		delay_us(2);
		
		//SDA由低变为高电平 产生stop信号
		GPIO_SetBits(GPIOB, SDA_PIN );   //SDA为高
		delay_us(2);
	}
	else if(I2Cx == I2C2)
	{
		SDA2_OUT();
		//SCL为低电平
		GPIO_ResetBits(GPIOB,  SCL2_PIN );   //SCL变低电平
		//SDA为低电平
		GPIO_ResetBits(GPIOB,  SDA2_PIN );   //SDA变低电平
		
		delay_us(2);
		
		GPIO_SetBits(GPIOB, SCL2_PIN );   //SCL为高
		delay_us(2);
		
		//SDA由低变为高电平 产生stop信号
		GPIO_SetBits(GPIOB, SDA2_PIN );   //SDA为高
		delay_us(2);
	}
	//stop之后，SDA和SCL都为高电平
}



//i2c1 发送一个字节
//这个函数不检验有没有应答
void i2c1_put_byte_data(uint8_t dat)
{
	uint8_t i;
	SDA_OUT();	
	for(i=0;i<8;i++)
	{
		//1.SCL为低
		GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
		delay_us(1);
		
		//2.在SCL为低电平的时候，可以在SDA上准备数据
		if((dat >> (7-i)) & 1)  //高电平
		{
			GPIO_SetBits(GPIOB, SDA_PIN );   //SDA为高
		}
		else  //低电平
		{
			GPIO_ResetBits(GPIOB,  SDA_PIN );   //SDA变低电平
		}
		delay_us(1);
		
		//SCL为高电平，数据就发出去了
		GPIO_SetBits(GPIOB, SCL_PIN);   //SCL为高
		delay_us(4);
	}
	//这个函数退出的时候SCL是低电平
	GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
	delay_us(1);
}


//i2c 发送一个字节
//这个函数不检验有没有应答
void i2c2_put_byte_data(uint8_t dat)
{
	uint8_t i;
	SDA2_OUT();	
	for(i=0;i<8;i++)
	{
		//1.SCL为低
		GPIO_ResetBits(GPIOB,  SCL2_PIN );   //SCL变低电平
		delay_us(1);
		
		//2.在SCL为低电平的时候，可以在SDA上准备数据
		if((dat >> (7-i)) & 1)  //高电平
		{
			GPIO_SetBits(GPIOB, SDA2_PIN );   //SDA为高
		}
		else  //低电平
		{
			GPIO_ResetBits(GPIOB,  SDA2_PIN );   //SDA变低电平
		}
		delay_us(1);
		
		//SCL为高电平，数据就发出去了
		GPIO_SetBits(GPIOB, SCL2_PIN);   //SCL为高
		delay_us(4);
	}
	//这个函数退出的时候SCL是低电平
	GPIO_ResetBits(GPIOB,  SCL2_PIN );   //SCL变低电平
	delay_us(1);
}







//接收从设备的应答信号
//有应答，返回0
//无应答，返回1
uint8_t i2c1_get_ack(void)
{
	uint8_t ret = 0;
	//读取对方的应答	
//	GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
//	delay_us(1);

	//1.拉高SDA	主要的原因是OD输出模式
	GPIO_SetBits(GPIOB, SDA_PIN );   //SDA为高
	delay_us(1);
	
	SDA_IN();
	
	GPIO_SetBits(GPIOB, SCL_PIN);   //SCL为高
	delay_us(2);
	
	//在SCL为高的时候 ，读取SDA的引脚电平
	if(GPIO_ReadInputDataBit(GPIOB, SDA_PIN ))
	{
		ret = 1;
	}
	//这个函数退出的时候SCL是低电平
	GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
	delay_us(1);
	
	return ret;
}



//接收从设备的应答信号
//有应答，返回0
//无应答，返回1
uint8_t i2c2_get_ack(void)
{
	uint8_t ret = 0;
	//读取对方的应答	
//	GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
//	delay_us(1);

	//1.拉高SDA	主要的原因是OD输出模式
	GPIO_SetBits(GPIOB, SDA2_PIN );   //SDA为高
	delay_us(1);
	
	SDA2_IN();
	
	GPIO_SetBits(GPIOB, SCL2_PIN);   //SCL为高
	delay_us(2);
	
	//在SCL为高的时候 ，读取SDA的引脚电平
	if(GPIO_ReadInputDataBit(GPIOB, SDA2_PIN ))
	{
		ret = 1;
	}
	//这个函数退出的时候SCL是低电平
	GPIO_ResetBits(GPIOB,  SCL2_PIN );   //SCL变低电平
	delay_us(1);
	
	return ret;
}



int8_t iic_put_devaddr(I2C_TypeDef* I2Cx,uint8_t dev_addr)
{
	if(I2Cx == I2C1)
	{
		i2c1_put_byte_data(dev_addr);	
		return i2c1_get_ack();   //返回0表示有应答，1表示没应答
	}
	else if(I2Cx == I2C2)
	{
		i2c2_put_byte_data(dev_addr);
		return i2c2_get_ack();   //返回0表示有应答，1表示没应答
	}
	return 1;
}



int8_t iic_put_byte_data(I2C_TypeDef* I2Cx,uint8_t dat)
{
	if(I2Cx == I2C1)
	{
		i2c1_put_byte_data(dat);	
		return i2c1_get_ack();   //返回0表示有应答，1表示没应答
	}
	else if(I2Cx == I2C2)
	{
		i2c2_put_byte_data(dat);
		return i2c2_get_ack();   //返回0表示有应答，1表示没应答
	}
	return 1;
}



/*
	读从机IIC的应答位
	
	返回值：
		0 表示应答成功
		1 表示未应答
*/
uint8_t iic_get_ack(I2C_TypeDef* I2Cx)
{
	if(I2Cx == I2C1)
	{	
		return i2c1_get_ack();   //返回0表示有应答，1表示没应答
	}
	else if(I2Cx == I2C2)
	{
		return i2c2_get_ack();   //返回0表示有应答，1表示没应答
	}
	return 1;
}






/*
	从 从设备 读取一个字节的数据
*/
uint8_t i2c1_get_byte_data(void)
{
	uint8_t i;
	uint8_t dat;

	//1.SCL为低
	GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
	delay_us(1);
	
	//2.拉高SDA	
	GPIO_SetBits(GPIOB, SDA_PIN );   //SDA为高
	delay_us(1);
	
	SDA_IN();
	
	for(i=0;i<8;i++)
	{
		dat <<= 1;
		
		//3.拉高SCL	
		GPIO_SetBits(GPIOB, SCL_PIN );   //SCL为高
		delay_us(1);
		
		//4.SCL为高的时候，才是真正的数据
		if(GPIO_ReadInputDataBit(GPIOB, SDA_PIN ))
			dat |= 1;    //读到的是高电平，那么最低位置一
		
		//5.SCL为低,让24c02准备下一个数据
		GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
		delay_us(4);		
	}
	
	return dat;
}






//iic1发送应答
void i2c1_send_ack(uint8_t ack)
{
	//1.SCL为低
	GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
	
	SDA_OUT();
	delay_us(1);
	
	if(ack == NO_ACK)  //发送非应答信号
	{
		GPIO_SetBits(GPIOB, SDA_PIN );   //SDA为高	
	}
	else
	{
		GPIO_ResetBits(GPIOB, SDA_PIN );   //SDA为低
	}
	delay_us(1);
	
	GPIO_SetBits(GPIOB,  SCL_PIN );   //SCL变高电平，（非）应答周期才发送出去
	delay_us(2);
	
	GPIO_ResetBits(GPIOB,  SCL_PIN );   //SCL变低电平
	delay_us(1);
	
}


/*
	从 从设备 读取一个字节的数据
*/
uint8_t i2c2_get_byte_data(void)
{
	uint8_t i;
	uint8_t dat;

	//1.SCL为低
	GPIO_ResetBits(GPIOB,  SCL2_PIN );   //SCL变低电平
	delay_us(1);
		
	//2.拉高SDA	
	GPIO_SetBits(GPIOB, SDA2_PIN );   //SDA为高
	delay_us(1);
	
	SDA2_IN();
	
	for(i=0;i<8;i++)
	{
		dat <<= 1;
		
		//3.拉高SCL	
		GPIO_SetBits(GPIOB, SCL2_PIN );   //SCL为高
		delay_us(1);
		
		//4.SCL为高的时候，才是真正的数据
		if(GPIO_ReadInputDataBit(GPIOB, SDA2_PIN ))
			dat |= 1;    //读到的是高电平，那么最低位置一
		
		//5.SCL为低,让24c02准备下一个数据
		GPIO_ResetBits(GPIOB,  SCL2_PIN );   //SCL变低电平
		delay_us(4);		
	}
	
	return dat;
}






//iic2发送应答
void i2c2_send_ack(uint8_t ack)
{
	//1.SCL为低
	GPIO_ResetBits(GPIOB,  SCL2_PIN );   //SCL变低电平
	SDA2_OUT();
	delay_us(1);
	
	if(ack == NO_ACK)  //发送非应答信号
	{
		GPIO_SetBits(GPIOB, SDA2_PIN );   //SDA为高	
	}
	else
	{
		GPIO_ResetBits(GPIOB, SDA2_PIN );   //SDA为低
	}
	delay_us(1);
	
	GPIO_SetBits(GPIOB,  SCL2_PIN );   //SCL变高电平，（非）应答周期才发送出去
	delay_us(2);
	
	GPIO_ResetBits(GPIOB,  SCL2_PIN );   //SCL变低电平
	delay_us(1);
	
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
	if(I2Cx == I2C1)
	{	
		return i2c1_get_byte_data();
	}
	else if(I2Cx == I2C2)
	{
		return i2c2_get_byte_data();
	}
	return 0x1000;
}



/*
	设置应答
*/
void iic_send_ack(I2C_TypeDef* I2Cx,uint8_t ack)
{
	if(I2Cx == I2C1)
	{	
		i2c1_send_ack(ack);
	}
	else if(I2Cx == I2C2)
	{
		i2c2_send_ack(ack);
	}
}


