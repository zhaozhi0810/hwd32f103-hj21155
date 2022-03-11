#include "includes.h" 

#define  SkipROM    0xCC     //跳过ROM
#define  SearchROM  0xF0  //搜索ROM
#define  ReadROM    0x33  //读ROM
#define  MatchROM   0x55  //匹配ROM
#define  AlarmROM   0xEC  //告警ROM

#define  StartConvert    0x44  //开始温度转换，在温度转换期间总线上输出0，转换结束后输出1
#define  ReadScratchpad  0xBE  //读暂存器的9个字节
#define  WriteScratchpad 0x4E  //写暂存器的温度告警TH和TL
#define  CopyScratchpad  0x48  //将暂存器的温度告警复制到EEPROM，在复制期间总线上输出0，复制完后输出1
#define  RecallEEPROM    0xB8    //将EEPROM的温度告警复制到暂存器中，复制期间输出0，复制完成后输出1
#define  ReadPower       0xB4    //读电源的供电方式：0为寄生电源供电；1为外部电源供电

#define EnableINT()  __enable_irq()
#define DisableINT() __disable_irq()

//PB15 测屏幕温度
#define NS_PORT		GPIOA          //NS18B20连接口
#define NS_DQIO		GPIO_Pin_0
#define NS_RCC_PORT	RCC_APB2Periph_GPIOA

#define NS_PRECISION    0x7f   //精度配置寄存器 1f=9位; 3f=10位; 5f=11位; 7f=12位;
#define NS_AlarmTH      0x7d //125 /*0x64*/
#define NS_AlarmTL      0xc9 //-55 /*0x8a*/
#define NS_CONVERT_TICK 1000

#define TIMEOUT_WAITING_DQ	500

#define ResetDQ() GPIO_ResetBits(NS_PORT,NS_DQIO)//gpio_bit_reset(NS_PORT,NS_DQIO)
#define SetDQ()   GPIO_SetBits(NS_PORT,NS_DQIO)//gpio_bit_set(NS_PORT,NS_DQIO)
#define GetDQ()   GPIO_ReadInputDataBit(NS_PORT,NS_DQIO)//gpio_input_bit_get(NS_PORT, NS_DQIO)

#define MaxSensorNum 2   //总共两个
static unsigned char DS18B20_ID[MaxSensorNum][8];	// 存检测到的传感器DS18B20_ID的数组,前面的维数代表单根线传感器数量上限
static unsigned char DS18B20_SensorNum = 0;			// 检测到的传感器数量(从1开始，例如显示1代表1个，8代表8个)
 


/*

*/
//void Delay_us(uint32_t nTime)
//{
//  uint32_t uCount;
//  
////  uCount = nTime*27;
//  uCount = (uint32_t)(nTime*49);
//  while(uCount--);
//}

static int ResetNS18B20(void)
{
	int timeout = TIMEOUT_WAITING_DQ;
	int err = 0;
	SetDQ();
	delay_us(50);

	ResetDQ();
	delay_us(500);  //500us （该时间的时间范围可以从480到960微秒）
	SetDQ();
	delay_us(40);  //40us
	//resport = GetDQ();
	while(GetDQ())
	{
		delay_us(1);
		if((timeout--) < 0) // 超时未读到DQ状态,说明硬件上DQ未连接
		{
			err = -1;
			break;
		}
	}
	delay_us(500);  //500us
	SetDQ();
	
	return err;
}

static void NS18B20WriteByte(unsigned char Dat)
{
	int i;
	
	for(i=8;i>0;i--)
	{
		ResetDQ();     //在15u内送数到数据线上，DS18B20在15-60u读数
		delay_us(5);    //5us
		if(Dat & 0x01)
			SetDQ();
		else
			ResetDQ();
		delay_us(65);    //65us
		SetDQ();
		delay_us(2);    //连续两位间应大于1us
		Dat >>= 1; 
	} 
}

static unsigned char NS18B20ReadByte(void)
{
	int i;
	unsigned char Dat=0;
	
	SetDQ();
	delay_us(5);
	for(i=8;i>0;i--)
	{
		Dat >>= 1;
		ResetDQ();     //从读时序开始到采样信号线必须在15u内，且采样尽量安排在15u的最后
		delay_us(5);   //5us
		SetDQ();
		delay_us(5);   //5us
		if(GetDQ())
			Dat|=0x80;
		else
			Dat&=0x7f;  
		delay_us(65);   //65us
		SetDQ();
	}
	
	return Dat;
}

#if 0
static void ReadRom(unsigned char *Read_Addr)
{
	int i;

	NS18B20WriteByte(ReadROM);

	for(i=8;i>0;i--)
	{
		*Read_Addr=NS18B20ReadByte();
		Read_Addr++;
	}
}
#endif


//18b20初始化
static int NS18B20Init(unsigned char Precision,unsigned char AlarmTH,unsigned char AlarmTL)
{
	int err = 0;
	int timeout = TIMEOUT_WAITING_DQ;
	
	DisableINT();
	err = ResetNS18B20();
	if(err) return err;
	
	NS18B20WriteByte(SkipROM); 
	NS18B20WriteByte(WriteScratchpad);
	NS18B20WriteByte(AlarmTL);
	NS18B20WriteByte(AlarmTH);
	NS18B20WriteByte(Precision);

	err = ResetNS18B20();
	if(err) return err;
	
	NS18B20WriteByte(SkipROM); 
	NS18B20WriteByte(CopyScratchpad);
	EnableINT();

	while(!GetDQ())  //等待复制完成 ///////////
	{
		delay_us(1);
		if((timeout--) < 0) // 超时未读到DQ状态,说明硬件上DQ未连接
		{
			err = -1;
			break;
		}		
	}
	
	return err;
}


//对外接口
int NS18B20StartConvert(void)
{
	int err = 0;
	
	//没有检测到18B20
	if(DS18B20_SensorNum<1)
		return -1;
	
	
	DisableINT();
	err = ResetNS18B20();
	if(err) return err;
	
	NS18B20WriteByte(SkipROM); 
	NS18B20WriteByte(StartConvert); 
	EnableINT();
	
	return err;
}





static void NS18B20_Configuration()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* enable the led clock */
	//rcu_periph_clock_enable(NS_RCC_PORT);
	RCC_APB2PeriphClockCmd(NS_RCC_PORT, ENABLE);
    /* configure GPIO output  */ 
    //gpio_mode_set(NS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,NS_DQIO);
    //gpio_output_options_set(NS_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,NS_DQIO);
	
	//1.io引脚涉及的时钟不再初始化，前面都统一初始化了。
	
	//2. 输出功能引脚的初始化(PA10,PB2,PB8,PB9,PB15,PC4,PC7,PC8,PC10-PC15,PD1，PB3，PB4)
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;    
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	
	//gpio_bit_set(NS_PORT,NS_DQIO);//high	
	SetDQ();
	
}


static int DS18B20_Search_Rom(void);

int ns18b20_init(void)
{
	int err = 0;
	
	NS18B20_Configuration();
	err = NS18B20Init(NS_PRECISION, NS_AlarmTH, NS_AlarmTL);
	if(err) return err;
		
	//上电配置的时候搜索一下18b20，之后不再搜索！！！
	err = DS18B20_Search_Rom();
	if(err) return err;
	
	//启动转换
//	err = NS18B20StartConvert();
//	if(err) return err;
	
	return err;
}


//对外接口
int ns18b20_read(u8 i,short *temperature)
{
	unsigned char DL, DH;
	short TemperatureData; //需要读取负值,不能用无符号
	int err;
	uint8_t j;
	
	//没有检测到18B20
	if(DS18B20_SensorNum<i)   //没有检测到就返回吧。
		return -1;
	
	DisableINT();	
	err = ResetNS18B20();
	if(err) return err;

	//NS18B20WriteByte(SkipROM);  //不跳过rom了 
		//匹配ID，i为形参
	//DS18B20_Write_Byte(0x55);
	NS18B20WriteByte(0x55);
	for (j = 0; j < 8; j++)
	{
		//DS18B20_Write_Byte(DS18B20_ID[i][j]);
		NS18B20WriteByte(DS18B20_ID[i][j]);
	}
		
	NS18B20WriteByte(ReadScratchpad);
	DL = NS18B20ReadByte();
	DH = NS18B20ReadByte();
	EnableINT();

	if((DL==0xff) && (DH==0xff))
		return -1;

	TemperatureData = DH;
	TemperatureData <<= 8;
	TemperatureData |= DL;

	*temperature = TemperatureData; //实际温度值应该乘0.0625

	//读完之后，启动下一次转换
	err = NS18B20StartConvert();
	if(err) return err;
		
	return  0;
}




// 从DS18B20读取2个位
static u8 DS18B20_Read_2Bit(void)//读二位 子程序
{
	u8 i;
	u8 dat = 0;
	for (i = 2; i > 0; i--)
	{
		dat = dat << 1;
		ResetDQ();  //DS18B20_Mode_Out();
		//DS18B20_DQ_OUT_Low;
		delay_us(2);//Delay_us(2);
		SetDQ();   //DS18B20_DQ_OUT_High;
		//DS18B20_Mode_IPU();
		delay_us(12);//Delay_us(12);
		if (GetDQ())	dat |= 0x01;
		delay_us(50);
	}
	return dat;
}


// 写1位到DS18B20
static void DS18B20_Write_Bit(u8 dat)
{
	ResetDQ();     //在15u内送数到数据线上，DS18B20在15-60u读数
	delay_us(5);    //5us
	if(dat & 0x01)
		SetDQ();
	else
		ResetDQ();
	delay_us(65);    //65us
	SetDQ();
	delay_us(2);    //连续两位间应大于1us
}




//id自动搜索
// 自动搜索ROM
static int DS18B20_Search_Rom(void)
{
	u8 k, l, chongtuwei, m, n, num;
	u8 zhan[5];
	u8 ss[64];
	u8 tempp;
	int err;
	
	l = 0;
	num = 0;
	do
	{
		//DS18B20_Rst(); //注意：复位的延时不够
		//Delay_us(480); //480、720
		//DS18B20_Write_Byte(0xf0);
		err = ResetNS18B20();
		if(err) return err;
		delay_us(480);
		NS18B20WriteByte(0xf0);
				
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
					//DS18B20_Write_Bit(0);
					ss[(m * 8 + n)] = 0;
				}
				else if (k == 0x02)//读到的数据为1 写1 此位为1的器件响应
				{
					s = s | 0x80;
					DS18B20_Write_Bit(1);
					ss[(m * 8 + n)] = 1;
				}
				else if (k == 0x00)//读到的数据为00 有冲突位 判断冲突位
				{
					//如果冲突位大于栈顶写0 小于栈顶写以前数据 等于栈顶写1
					chongtuwei = m * 8 + n + 1;
					if (chongtuwei > zhan[l])
					{
						DS18B20_Write_Bit(0);
						ss[(m * 8 + n)] = 0;
						zhan[++l] = chongtuwei;
					}
					else if (chongtuwei < zhan[l])
					{
						s = s | ((ss[(m * 8 + n)] & 0x01) << 7);
						DS18B20_Write_Bit(ss[(m * 8 + n)]);
					}
					else if (chongtuwei == zhan[l])
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
				}
			}
			tempp = s;
			DS18B20_ID[num][m] = tempp; // 保存搜索到的ID
		}
		num = num + 1;// 保存搜索到的个数
	} while (zhan[l] != 0 && (num < MaxSensorNum));
	DS18B20_SensorNum = num;
	//printf("DS18B20_SensorNum=%d\r\n",DS18B20_SensorNum);
	
	return 0;
}





