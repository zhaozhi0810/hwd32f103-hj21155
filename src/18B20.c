#include "includes.h" 

#define  SkipROM    0xCC     //����ROM
#define  SearchROM  0xF0  //����ROM
#define  ReadROM    0x33  //��ROM
#define  MatchROM   0x55  //ƥ��ROM
#define  AlarmROM   0xEC  //�澯ROM

#define  StartConvert    0x44  //��ʼ�¶�ת�������¶�ת���ڼ����������0��ת�����������1
#define  ReadScratchpad  0xBE  //���ݴ�����9���ֽ�
#define  WriteScratchpad 0x4E  //д�ݴ������¶ȸ澯TH��TL
#define  CopyScratchpad  0x48  //���ݴ������¶ȸ澯���Ƶ�EEPROM���ڸ����ڼ����������0������������1
#define  RecallEEPROM    0xB8    //��EEPROM���¶ȸ澯���Ƶ��ݴ����У������ڼ����0��������ɺ����1
#define  ReadPower       0xB4    //����Դ�Ĺ��緽ʽ��0Ϊ������Դ���磻1Ϊ�ⲿ��Դ����

#define EnableINT()  __enable_irq()
#define DisableINT() __disable_irq()

//PB15 ����Ļ�¶�
#define NS_PORT		GPIOA          //NS18B20���ӿ�
#define NS_DQIO		GPIO_Pin_0
#define NS_RCC_PORT	RCC_APB2Periph_GPIOA

#define NS_PRECISION    0x7f   //�������üĴ��� 1f=9λ; 3f=10λ; 5f=11λ; 7f=12λ;
#define NS_AlarmTH      0x7d //125 /*0x64*/
#define NS_AlarmTL      0xc9 //-55 /*0x8a*/
#define NS_CONVERT_TICK 1000

#define TIMEOUT_WAITING_DQ	500

#define ResetDQ() GPIO_ResetBits(NS_PORT,NS_DQIO)//gpio_bit_reset(NS_PORT,NS_DQIO)
#define SetDQ()   GPIO_SetBits(NS_PORT,NS_DQIO)//gpio_bit_set(NS_PORT,NS_DQIO)
#define GetDQ()   GPIO_ReadInputDataBit(NS_PORT,NS_DQIO)//gpio_input_bit_get(NS_PORT, NS_DQIO)

#define MaxSensorNum 2   //�ܹ�����
static unsigned char DS18B20_ID[MaxSensorNum][8];	// ���⵽�Ĵ�����DS18B20_ID������,ǰ���ά���������ߴ�������������
static unsigned char DS18B20_SensorNum = 0;			// ��⵽�Ĵ���������(��1��ʼ��������ʾ1����1����8����8��)
 


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
	delay_us(500);  //500us ����ʱ���ʱ�䷶Χ���Դ�480��960΢�룩
	SetDQ();
	delay_us(40);  //40us
	//resport = GetDQ();
	while(GetDQ())
	{
		delay_us(1);
		if((timeout--) < 0) // ��ʱδ����DQ״̬,˵��Ӳ����DQδ����
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
		ResetDQ();     //��15u���������������ϣ�DS18B20��15-60u����
		delay_us(5);    //5us
		if(Dat & 0x01)
			SetDQ();
		else
			ResetDQ();
		delay_us(65);    //65us
		SetDQ();
		delay_us(2);    //������λ��Ӧ����1us
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
		ResetDQ();     //�Ӷ�ʱ��ʼ�������ź��߱�����15u�ڣ��Ҳ�������������15u�����
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


//18b20��ʼ��
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

	while(!GetDQ())  //�ȴ�������� ///////////
	{
		delay_us(1);
		if((timeout--) < 0) // ��ʱδ����DQ״̬,˵��Ӳ����DQδ����
		{
			err = -1;
			break;
		}		
	}
	
	return err;
}


//����ӿ�
int NS18B20StartConvert(void)
{
	int err = 0;
	
	//û�м�⵽18B20
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
	
	//1.io�����漰��ʱ�Ӳ��ٳ�ʼ����ǰ�涼ͳһ��ʼ���ˡ�
	
	//2. ����������ŵĳ�ʼ��(PA10,PB2,PB8,PB9,PB15,PC4,PC7,PC8,PC10-PC15,PD1��PB3��PB4)
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
		
	//�ϵ����õ�ʱ������һ��18b20��֮��������������
	err = DS18B20_Search_Rom();
	if(err) return err;
	
	//����ת��
//	err = NS18B20StartConvert();
//	if(err) return err;
	
	return err;
}


//����ӿ�
int ns18b20_read(u8 i,short *temperature)
{
	unsigned char DL, DH;
	short TemperatureData; //��Ҫ��ȡ��ֵ,�������޷���
	int err;
	uint8_t j;
	
	//û�м�⵽18B20
	if(DS18B20_SensorNum<i)   //û�м�⵽�ͷ��ذɡ�
		return -1;
	
	DisableINT();	
	err = ResetNS18B20();
	if(err) return err;

	//NS18B20WriteByte(SkipROM);  //������rom�� 
		//ƥ��ID��iΪ�β�
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

	*temperature = TemperatureData; //ʵ���¶�ֵӦ�ó�0.0625

	//����֮��������һ��ת��
	err = NS18B20StartConvert();
	if(err) return err;
		
	return  0;
}




// ��DS18B20��ȡ2��λ
static u8 DS18B20_Read_2Bit(void)//����λ �ӳ���
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


// д1λ��DS18B20
static void DS18B20_Write_Bit(u8 dat)
{
	ResetDQ();     //��15u���������������ϣ�DS18B20��15-60u����
	delay_us(5);    //5us
	if(dat & 0x01)
		SetDQ();
	else
		ResetDQ();
	delay_us(65);    //65us
	SetDQ();
	delay_us(2);    //������λ��Ӧ����1us
}




//id�Զ�����
// �Զ�����ROM
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
		//DS18B20_Rst(); //ע�⣺��λ����ʱ����
		//Delay_us(480); //480��720
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
				k = DS18B20_Read_2Bit();//����λ����
 
				k = k & 0x03;
				s >>= 1;
				if (k == 0x01)//01����������Ϊ0 д0 ��λΪ0��������Ӧ
				{
					//DS18B20_Write_Bit(0);
					ss[(m * 8 + n)] = 0;
				}
				else if (k == 0x02)//����������Ϊ1 д1 ��λΪ1��������Ӧ
				{
					s = s | 0x80;
					DS18B20_Write_Bit(1);
					ss[(m * 8 + n)] = 1;
				}
				else if (k == 0x00)//����������Ϊ00 �г�ͻλ �жϳ�ͻλ
				{
					//�����ͻλ����ջ��д0 С��ջ��д��ǰ���� ����ջ��д1
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
					//û��������
				}
			}
			tempp = s;
			DS18B20_ID[num][m] = tempp; // ������������ID
		}
		num = num + 1;// �����������ĸ���
	} while (zhan[l] != 0 && (num < MaxSensorNum));
	DS18B20_SensorNum = num;
	//printf("DS18B20_SensorNum=%d\r\n",DS18B20_SensorNum);
	
	return 0;
}





