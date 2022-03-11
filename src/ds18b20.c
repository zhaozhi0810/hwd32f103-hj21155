#include "includes.h"

#define DS18B20_GPIO_NUM				 GPIO_Pin_0
#define DS18B20_GPIO_X					GPIOA
#define RCC_APB2Periph_DS18B20_GPIO_X	RCC_APB2Periph_GPIOA

#define DS18B20_DQ_OUT_Low			GPIO_ResetBits(DS18B20_GPIO_X,DS18B20_GPIO_NUM) 
#define DS18B20_DQ_OUT_High			GPIO_SetBits(DS18B20_GPIO_X,DS18B20_GPIO_NUM) 
#define DS18B20_DQ_IN				GPIO_ReadInputDataBit(DS18B20_GPIO_X,DS18B20_GPIO_NUM) 

#define MaxSensorNum 2
#define Delay_us delay_us



unsigned char DS18B20_ID[MaxSensorNum][8];	// 存检测到的传感器DS18B20_ID的数组,前面的维数代表单根线传感器数量上限
unsigned char DS18B20_SensorNum;			// 检测到的传感器数量(从1开始，例如显示1代表1个，8代表8个)


#define EnableINT()  __enable_irq()
#define DisableINT() __disable_irq()


// 配置DS18B20用到的I/O口
void DS18B20_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_DS18B20_GPIO_X, ENABLE);
	GPIO_InitStructure.GPIO_Pin = DS18B20_GPIO_NUM;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(DS18B20_GPIO_X, &GPIO_InitStructure);
	GPIO_SetBits(DS18B20_GPIO_X, DS18B20_GPIO_NUM);
}

// 引脚输入
void DS18B20_Mode_IPU(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DS18B20_GPIO_NUM;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(DS18B20_GPIO_X, &GPIO_InitStructure);
}

// 引脚输出
void DS18B20_Mode_Out(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DS18B20_GPIO_NUM;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(DS18B20_GPIO_X, &GPIO_InitStructure);

}

// 复位，主机给从机发送复位脉冲
void DS18B20_Rst(void)
{
	DS18B20_Mode_Out();
	DS18B20_DQ_OUT_Low;		// 产生至少480us的低电平复位信号
	Delay_us(480);
	DS18B20_DQ_OUT_High;	// 在产生复位信号后，需将总线拉高
	Delay_us(15);
}

// 检测从机给主机返回的应答脉冲。从机接收到主机的复位信号后，会在15~60us后给主机发一个应答脉冲
u8 DS18B20_Answer_Check(void)
{
	u8 delay = 0;
	DS18B20_Mode_IPU(); // 主机设置为上拉输入
	// 等待应答脉冲（一个60~240us的低电平信号 ）的到来
	// 如果100us内，没有应答脉冲，退出函数，注意：从机接收到主机的复位信号后，会在15~60us后给主机发一个存在脉冲
	while (DS18B20_DQ_IN&&delay < 100)
	{
		delay++;
		Delay_us(1);
	}
	// 经过100us后，如果没有应答脉冲，退出函数
	if (delay >= 100)//Hu200
		return 1;
	else
		delay = 0;
	// 有应答脉冲，且存在时间不超过240us
	while (!DS18B20_DQ_IN&&delay < 240)
	{
		delay++;
		Delay_us(1);
	}
	if (delay >= 240)
		return 1;
	return 0;
}

// 从DS18B20读取1个位
u8 DS18B20_Read_Bit(void)
{
	u8 data;
	DS18B20_Mode_Out();
	DS18B20_DQ_OUT_Low; // 读时间的起始：必须由主机产生 >1us <15us 的低电平信号
	Delay_us(2);
	DS18B20_DQ_OUT_High;
	Delay_us(12);
	DS18B20_Mode_IPU();// 设置成输入，释放总线，由外部上拉电阻将总线拉高
	if (DS18B20_DQ_IN)
		data = 1;
	else
		data = 0;
	Delay_us(50);
	return data;
}

// 从DS18B20读取2个位
u8 DS18B20_Read_2Bit(void)//读二位 子程序
{
	u8 i;
	u8 dat = 0;
	for (i = 2; i > 0; i--)
	{
		dat = dat << 1;
		DS18B20_Mode_Out();
		DS18B20_DQ_OUT_Low;
		Delay_us(2);
		DS18B20_DQ_OUT_High;
		DS18B20_Mode_IPU();
		Delay_us(12);
		if (DS18B20_DQ_IN)	dat |= 0x01;
		Delay_us(50);
	}
	return dat;
}

// 从DS18B20读取1个字节
u8 DS18B20_Read_Byte(void)	// read one byte
{
	u8 i, j, dat;
	dat = 0;
	for (i = 0; i < 8; i++)
	{
		j = DS18B20_Read_Bit();
		dat = (dat) | (j << i);
	}
	return dat;
}

// 写1位到DS18B20
void DS18B20_Write_Bit(u8 dat)
{
	DS18B20_Mode_Out();
	if (dat)
	{
		DS18B20_DQ_OUT_Low;// Write 1
		Delay_us(2);
		DS18B20_DQ_OUT_High;
		Delay_us(60);
	}
	else
	{
		DS18B20_DQ_OUT_Low;// Write 0
		Delay_us(60);
		DS18B20_DQ_OUT_High;
		Delay_us(2);
	}
}

// 写1字节到DS18B20
void DS18B20_Write_Byte(u8 dat)
{
	u8 j;
	u8 testb;
	DS18B20_Mode_Out();
	for (j = 1; j <= 8; j++)
	{
		testb = dat & 0x01;
		dat = dat >> 1;
		if (testb)
		{
			DS18B20_DQ_OUT_Low;// 写1
			Delay_us(10);
			DS18B20_DQ_OUT_High;
			Delay_us(50);
		}
		else
		{
			DS18B20_DQ_OUT_Low;// 写0
			Delay_us(60);
			DS18B20_DQ_OUT_High;// 释放总线
			Delay_us(2);
		}
	}
}

//初始化DS18B20的IO口，同时检测DS的存在
u8 DS18B20_Init(void)
{
	uint8_t ret;
	DS18B20_GPIO_Config();
	DS18B20_Rst();
	ret = DS18B20_Answer_Check();
	DS18B20_Search_Rom();
	return ret;
}



//对外接口
int NS18B20StartConvert(void)
{
	int err = 0;
	
	//没有检测到18B20
	if(DS18B20_SensorNum<1)
		return -1;
		
	DisableINT();
	DS18B20_Rst();
	err = DS18B20_Answer_Check();
	if(err) 
	{
		EnableINT();  //开启中断
		return err;
	}
	//开始转换
	DS18B20_Write_Byte(SKIP_ROM); 
	DS18B20_Write_Byte(CONVERT_T); 
	EnableINT();
	
	return 0;
}





// 从ds18b20得到温度值，精度：0.1C，返回温度值（-550~1250），Temperature1返回浮点实际温度
//-55~+125度，-880 ~ 2000
short DS18B20_Get_Temp(u8 i)
{
	int err = 0;
	u8 j;//匹配的字节
	u8 TL, TH;
	short Temperature;

	if(i>=DS18B20_SensorNum)
		return -2500;

	DisableINT();

	DS18B20_Rst();
	err = DS18B20_Answer_Check();
	if(err) 
	{
		EnableINT();  //开启中断
		return -2600;
	}
	// DS18B20_Write_Byte(0xcc);// skip rom
	//匹配ID，i为形参
	DS18B20_Write_Byte(0x55);
	for (j = 0; j < 8; j++)
	{
		DS18B20_Write_Byte(DS18B20_ID[i][j]);
	}

	DS18B20_Write_Byte(0xbe);// convert
	TL = DS18B20_Read_Byte(); // LSB   
	TH = DS18B20_Read_Byte(); // MSB 

	Temperature = (TH << 8) | TL;

	EnableINT();

	return Temperature;
}


//对外接口
int ns18b20_read(u8 i,short *temperature)
{
	short temp;
	
/*///////////////// 测试使用	
	static uint16_t n = 0;
	n++;
	if(n<150 && n>50)
	{
		
		*temperature = -320;   //-20度
		return 0;
	}
	else
	{
		*temperature = 0;   //-20度
		return 0;
	}
////////////////    
	*/

	temp = DS18B20_Get_Temp(i);
	if((temp > -900) && (temp <= 2000))	//有效范围	
	{
		*temperature = temp;
		return 0;
	}
	return -1;
}



// 自动搜索ROM
void DS18B20_Search_Rom(void)
{
	u8 k, l, chongtuwei, m, n, num;
	u8 zhan[5] = {0};   //初始化
	u8 ss[64];
	u8 tempp;
	u8 i = 0;   //统计计数 》8 就退出
	
	l = 0;
	num = 0;
	do
	{
		DS18B20_Rst(); //注意：复位的延时不够
		Delay_us(480); //480、720
		DS18B20_Write_Byte(0xf0);
		for (m = 0; m < 8; m++)
		{
			u8 s = 0;
			for (n = 0; n < 8; n++)
			{
				k = DS18B20_Read_2Bit();//读两位数据

				k = k & 0x03;
				s >>= 1;
				if (k == 0x01)//01读到的数据为0 写0 此位为0的器件响应
				{
					DS18B20_Write_Bit(0);
					ss[(m * 8 + n)] = 0;   //ss中也是位的数据
				}
				else if (k == 0x02)//读到的数据为1 写1 此位为1的器件响应
				{
					s = s | 0x80;    //s保存的是数据
					DS18B20_Write_Bit(1);
					ss[(m * 8 + n)] = 1;
				}
				else if (k == 0x00)//读到的数据为00 有冲突位 判断冲突位
				{
					//如果冲突位大于栈顶写0 小于栈顶写以前数据 等于栈顶写1
					chongtuwei = m * 8 + n + 1;    //记录冲突位，1-64
					if (chongtuwei > zhan[l])   //第一次碰到冲突
					{
						DS18B20_Write_Bit(0);
						ss[(m * 8 + n)] = 0;
						zhan[++l] = chongtuwei;   //记录冲突位
					}
					else if (chongtuwei < zhan[l])  //冲突比记录的小，应该不可能发生吧？？？
					{
						s = s | ((ss[(m * 8 + n)] & 0x01) << 7);
						DS18B20_Write_Bit(ss[(m * 8 + n)]);
					}
					else if (chongtuwei == zhan[l])   //记录与冲突位相同，表明是第二次冲突
					{
						s = s | 0x80;
						DS18B20_Write_Bit(1);
						ss[(m * 8 + n)] = 1;
						l = l - 1;
					}
				}
				else
				{
					//没有搜索到
					break;   //2021-12-23   ???
				}
			}
			tempp = s;
			DS18B20_ID[num][m] = tempp; // 保存搜索到的ID
		}
		num = num + 1;// 保存搜索到的个数			
		if(++i> 20)
			break;
	} while (zhan[l] != 0 && (num < MaxSensorNum));
	DS18B20_SensorNum = num;
	//printf("DS18B20_SensorNum=%d\r\n",DS18B20_SensorNum);
}



