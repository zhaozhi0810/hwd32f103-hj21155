
#include "includes.h"


/*
	2021-09-16
	
	1.第三个移植的文件 （按键和led需要用到这个器件）
	
	四片 （左右各两块，一块用于9个按键和另一块用于9个led）
	一片连接9个led，输出模式
	一片连接9个按键，可以使用外部中断。
	
	左右相同。
	
	（NCA9555 国产替代NXP PCA9555PW）完成的 （I2C2完成，PB10，11，12（中断））
		按键： I2C 地址 0x4（低三位 100）
		LED ： I2C 地址 0x0（低三位 000）

	两个iic控制器，一个控制左边，一个控制右边


Command Register
0         Input port 0   //端口0的输入数据
1         Input port 1
2         Output port 0   //端口0的输出数据
3         Output port 1
4         Polarity inversion port 0  //将输入的数据极性取反
5         Polarity inversion port 1
6         Configuration port 0     //配置引脚的功能，1 输入（默认） 0 输出
7         Configuration port 1

*/


const I2C_TypeDef* NCA9555_IIC_CONTROLER = I2C2;
#define DATA_LENGTH 2





void nca9555_init(void)
{
#ifdef 	BTNS_USE_INT
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
#endif	
	
	IICapp_init(I2C2);  //控制器初始化
	IICapp_init(I2C1);  //控制器初始化,两个控制器，分别控制左右的灯和按键

#ifdef 	BTNS_USE_INT   //宏在btns_leds.h中定义
	//中断引脚初始化
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_IN_FLOATING;//GPIO_Mode_IPU;    //中断输入引脚，低触发中断
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;   //12和13两个引脚都是中断了
	GPIO_Init(GPIOB, &GPIO_InitStruct);
		
	//外部中断配置
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line12;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  //中断是低电平，所以选择下降沿触发
	EXTI_Init(&EXTI_InitStruct);     //由于是引脚的变化会产生中断，所以松开和按下都会产生中断
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line13;    //外部中断13也使能
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  //中断是低电平，所以选择下降沿触发
	EXTI_Init(&EXTI_InitStruct); 
	
	
	//中断控制器使能，使用的是外部中断12，13
	NVIC_InitStruct.NVIC_IRQChannel= EXTI15_10_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;    //使能
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
#endif	
}




//读到的数据由参数返回
/*
	返回值非0表示错误
		0表示成功

	参数dev_addr 只考虑低4位，最低位保持为0即可。只需要高7位！！！！
*/
uint8_t nca9555_read_2inport(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_read_bytes(IICn,dev_addr,INPORT0,dat,DATA_LENGTH);
}



/*
	返回值非0表示错误
		0表示成功

	参数dev_addr 只考虑低4位，最低位保持为0即可。只需要高7位！！！！
*/
uint8_t nca9555_read_2outport(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_read_bytes(IICn,dev_addr,OUTPORT0,dat,DATA_LENGTH);
}

/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_write_2config(I2C_TypeDef*IICn,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_write_bytes(IICn,dev_addr,CFGPORT0,dat,DATA_LENGTH);
}

//2022-01-03 读取配置寄存器的值，用于判断单片机需要重启还是复位
//uint8_t nca9555_read_2config(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[])
//{
//	dev_addr &= 0xe;
//	dev_addr |= PCA9555_DEVADDR;
//	return IICapp_read_bytes(IICn,dev_addr,CFGPORT0,dat,DATA_LENGTH);
//}

//uint8_t nca9555_read_2outport(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[])
//{
//	dev_addr &= 0xe;
//	dev_addr |= PCA9555_DEVADDR;
//	return IICapp_read_bytes(IICn,dev_addr,OUTPORT0,dat,DATA_LENGTH);
//}

//2022-01-03增加结束

/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_write_2outport(I2C_TypeDef*IICn,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_write_bytes(IICn,dev_addr,OUTPORT0,dat,DATA_LENGTH);
}

/*
	返回值非0表示错误
		0表示成功
*/
uint8_t nca9555_write_2inv_cfg(I2C_TypeDef*IICn,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_write_bytes(IICn,dev_addr,INVPORT0,dat,DATA_LENGTH);
}

