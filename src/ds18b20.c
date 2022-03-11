#include "includes.h"

#define DS18B20_GPIO_NUM				 GPIO_Pin_0
#define DS18B20_GPIO_X					GPIOA
#define RCC_APB2Periph_DS18B20_GPIO_X	RCC_APB2Periph_GPIOA

#define DS18B20_DQ_OUT_Low			GPIO_ResetBits(DS18B20_GPIO_X,DS18B20_GPIO_NUM) 
#define DS18B20_DQ_OUT_High			GPIO_SetBits(DS18B20_GPIO_X,DS18B20_GPIO_NUM) 
#define DS18B20_DQ_IN				GPIO_ReadInputDataBit(DS18B20_GPIO_X,DS18B20_GPIO_NUM) 

#define MaxSensorNum 2
#define Delay_us delay_us



unsigned char DS18B20_ID[MaxSensorNum][8];	// ���⵽�Ĵ�����DS18B20_ID������,ǰ���ά���������ߴ�������������
unsigned char DS18B20_SensorNum;			// ��⵽�Ĵ���������(��1��ʼ��������ʾ1����1����8����8��)


#define EnableINT()  __enable_irq()
#define DisableINT() __disable_irq()


// ����DS18B20�õ���I/O��
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

// ��������
void DS18B20_Mode_IPU(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DS18B20_GPIO_NUM;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(DS18B20_GPIO_X, &GPIO_InitStructure);
}

// �������
void DS18B20_Mode_Out(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DS18B20_GPIO_NUM;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(DS18B20_GPIO_X, &GPIO_InitStructure);

}

// ��λ���������ӻ����͸�λ����
void DS18B20_Rst(void)
{
	DS18B20_Mode_Out();
	DS18B20_DQ_OUT_Low;		// ��������480us�ĵ͵�ƽ��λ�ź�
	Delay_us(480);
	DS18B20_DQ_OUT_High;	// �ڲ�����λ�źź��轫��������
	Delay_us(15);
}

// ���ӻ����������ص�Ӧ�����塣�ӻ����յ������ĸ�λ�źź󣬻���15~60us���������һ��Ӧ������
u8 DS18B20_Answer_Check(void)
{
	u8 delay = 0;
	DS18B20_Mode_IPU(); // ��������Ϊ��������
	// �ȴ�Ӧ�����壨һ��60~240us�ĵ͵�ƽ�ź� ���ĵ���
	// ���100us�ڣ�û��Ӧ�����壬�˳�������ע�⣺�ӻ����յ������ĸ�λ�źź󣬻���15~60us���������һ����������
	while (DS18B20_DQ_IN&&delay < 100)
	{
		delay++;
		Delay_us(1);
	}
	// ����100us�����û��Ӧ�����壬�˳�����
	if (delay >= 100)//Hu200
		return 1;
	else
		delay = 0;
	// ��Ӧ�����壬�Ҵ���ʱ�䲻����240us
	while (!DS18B20_DQ_IN&&delay < 240)
	{
		delay++;
		Delay_us(1);
	}
	if (delay >= 240)
		return 1;
	return 0;
}

// ��DS18B20��ȡ1��λ
u8 DS18B20_Read_Bit(void)
{
	u8 data;
	DS18B20_Mode_Out();
	DS18B20_DQ_OUT_Low; // ��ʱ�����ʼ���������������� >1us <15us �ĵ͵�ƽ�ź�
	Delay_us(2);
	DS18B20_DQ_OUT_High;
	Delay_us(12);
	DS18B20_Mode_IPU();// ���ó����룬�ͷ����ߣ����ⲿ�������轫��������
	if (DS18B20_DQ_IN)
		data = 1;
	else
		data = 0;
	Delay_us(50);
	return data;
}

// ��DS18B20��ȡ2��λ
u8 DS18B20_Read_2Bit(void)//����λ �ӳ���
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

// ��DS18B20��ȡ1���ֽ�
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

// д1λ��DS18B20
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

// д1�ֽڵ�DS18B20
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
			DS18B20_DQ_OUT_Low;// д1
			Delay_us(10);
			DS18B20_DQ_OUT_High;
			Delay_us(50);
		}
		else
		{
			DS18B20_DQ_OUT_Low;// д0
			Delay_us(60);
			DS18B20_DQ_OUT_High;// �ͷ�����
			Delay_us(2);
		}
	}
}

//��ʼ��DS18B20��IO�ڣ�ͬʱ���DS�Ĵ���
u8 DS18B20_Init(void)
{
	uint8_t ret;
	DS18B20_GPIO_Config();
	DS18B20_Rst();
	ret = DS18B20_Answer_Check();
	DS18B20_Search_Rom();
	return ret;
}



//����ӿ�
int NS18B20StartConvert(void)
{
	int err = 0;
	
	//û�м�⵽18B20
	if(DS18B20_SensorNum<1)
		return -1;
		
	DisableINT();
	DS18B20_Rst();
	err = DS18B20_Answer_Check();
	if(err) 
	{
		EnableINT();  //�����ж�
		return err;
	}
	//��ʼת��
	DS18B20_Write_Byte(SKIP_ROM); 
	DS18B20_Write_Byte(CONVERT_T); 
	EnableINT();
	
	return 0;
}





// ��ds18b20�õ��¶�ֵ�����ȣ�0.1C�������¶�ֵ��-550~1250����Temperature1���ظ���ʵ���¶�
//-55~+125�ȣ�-880 ~ 2000
short DS18B20_Get_Temp(u8 i)
{
	int err = 0;
	u8 j;//ƥ����ֽ�
	u8 TL, TH;
	short Temperature;

	if(i>=DS18B20_SensorNum)
		return -2500;

	DisableINT();

	DS18B20_Rst();
	err = DS18B20_Answer_Check();
	if(err) 
	{
		EnableINT();  //�����ж�
		return -2600;
	}
	// DS18B20_Write_Byte(0xcc);// skip rom
	//ƥ��ID��iΪ�β�
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


//����ӿ�
int ns18b20_read(u8 i,short *temperature)
{
	short temp;
	
/*///////////////// ����ʹ��	
	static uint16_t n = 0;
	n++;
	if(n<150 && n>50)
	{
		
		*temperature = -320;   //-20��
		return 0;
	}
	else
	{
		*temperature = 0;   //-20��
		return 0;
	}
////////////////    
	*/

	temp = DS18B20_Get_Temp(i);
	if((temp > -900) && (temp <= 2000))	//��Ч��Χ	
	{
		*temperature = temp;
		return 0;
	}
	return -1;
}



// �Զ�����ROM
void DS18B20_Search_Rom(void)
{
	u8 k, l, chongtuwei, m, n, num;
	u8 zhan[5] = {0};   //��ʼ��
	u8 ss[64];
	u8 tempp;
	u8 i = 0;   //ͳ�Ƽ��� ��8 ���˳�
	
	l = 0;
	num = 0;
	do
	{
		DS18B20_Rst(); //ע�⣺��λ����ʱ����
		Delay_us(480); //480��720
		DS18B20_Write_Byte(0xf0);
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
					DS18B20_Write_Bit(0);
					ss[(m * 8 + n)] = 0;   //ss��Ҳ��λ������
				}
				else if (k == 0x02)//����������Ϊ1 д1 ��λΪ1��������Ӧ
				{
					s = s | 0x80;    //s�����������
					DS18B20_Write_Bit(1);
					ss[(m * 8 + n)] = 1;
				}
				else if (k == 0x00)//����������Ϊ00 �г�ͻλ �жϳ�ͻλ
				{
					//�����ͻλ����ջ��д0 С��ջ��д��ǰ���� ����ջ��д1
					chongtuwei = m * 8 + n + 1;    //��¼��ͻλ��1-64
					if (chongtuwei > zhan[l])   //��һ��������ͻ
					{
						DS18B20_Write_Bit(0);
						ss[(m * 8 + n)] = 0;
						zhan[++l] = chongtuwei;   //��¼��ͻλ
					}
					else if (chongtuwei < zhan[l])  //��ͻ�ȼ�¼��С��Ӧ�ò����ܷ����ɣ�����
					{
						s = s | ((ss[(m * 8 + n)] & 0x01) << 7);
						DS18B20_Write_Bit(ss[(m * 8 + n)]);
					}
					else if (chongtuwei == zhan[l])   //��¼���ͻλ��ͬ�������ǵڶ��γ�ͻ
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
					break;   //2021-12-23   ???
				}
			}
			tempp = s;
			DS18B20_ID[num][m] = tempp; // ������������ID
		}
		num = num + 1;// �����������ĸ���			
		if(++i> 20)
			break;
	} while (zhan[l] != 0 && (num < MaxSensorNum));
	DS18B20_SensorNum = num;
	//printf("DS18B20_SensorNum=%d\r\n",DS18B20_SensorNum);
}



