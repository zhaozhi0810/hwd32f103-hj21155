

#include "stm32f10x.h"
#include "hard_iic.h"
//#include "systick_delay.h"  //ʹ����delay����

/*
	��������������������������������������
	
	���ĸ���ֲ���ļ���
	
	�������ײ�Ŀ��Ʋ��֣�ֻ��iic_app.c�ĺ������á���������Ҫ���á�

I2C1:
	PB6  SCL 
	PB7  SDA
I2c2:
	PB10  SCL 
	PB11  SDA
*/
static uint8_t iic_ready = 0; //��ʼ�����𣿣�


//iic��Ӳ����ʼ��
void iic_init(I2C_TypeDef* I2Cx)
{		
	GPIO_InitTypeDef GPIO_InitStruct;
	I2C_InitTypeDef I2C_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	if(I2Cx == I2C1)   //
	{	
		if((iic_ready & 1) == 0)  //��ֹ�ظ���ʼ��
		{
			//1.ʱ��ʹ��		
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
			
			//2.gpio�Ĺ������ã����ù���
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;		
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 ;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;  //	
			GPIO_Init(GPIOB, &GPIO_InitStruct);  //����������ýṹ����������õ��Ĵ�����
			
			
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;   //clk���		
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 ;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;  //	
			GPIO_Init(GPIOB, &GPIO_InitStruct);  //����������ýṹ����������õ��Ĵ�����
			iic_ready |= 1;
		}
		else{
			return;  //�Ѿ���ʼ���ˣ�ֱ�ӷ���
		}
	}
	else if(I2Cx == I2C2)
	{
		if((iic_ready & 2) == 0)
		{
			//1.ʱ��ʹ��
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
			
			//2.gpio�Ĺ������ã����ù���
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;		
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 ;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;  //	
			GPIO_Init(GPIOB, &GPIO_InitStruct);  //����������ýṹ����������õ��Ĵ�����
			
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;	//scl�������	
			GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 ;
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;  //	
			GPIO_Init(GPIOB, &GPIO_InitStruct);  //����������ýṹ����������õ��Ĵ�����
			
			
			iic_ready |= 2;
		}
		else{
			return; //�Ѿ���ʼ���ˣ�ֱ�ӷ���
		}
	}
	else  //��������
	{
		return;
	}
	
	//3.iic�������ĳ�ʼ��
	I2C_StructInit(&I2C_InitStruct);     //Ӧ���Ƿ�Ӧ��
	I2C_InitStruct.I2C_ClockSpeed = 100000;   //����ܳ���400k
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;        /*Ӧ��ʹ�� */
	I2C_Init(I2Cx, &I2C_InitStruct);
	
	I2C_Cmd(I2Cx, ENABLE);
	
//	delay_1ms(100);
	
}



//ȥ��ʼ����2021-12-10 ���ӡ�
static void deinit_iic(I2C_TypeDef* I2Cx)
{
//	GPIO_InitTypeDef GPIO_InitStruct;
//	I2C_InitTypeDef I2C_InitStruct;
	
	if(I2Cx == I2C1)   //
	{	
		//1.ʱ��ʹ��		
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);			
		iic_ready &= ~1;
	}
	else if(I2Cx == I2C2)
	{
		//1.ʱ��ʹ��
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, DISABLE);		
		iic_ready &= ~2;
	}
	else  //��������
	{
		return;
	}
	
	//3.iic��������ȥ��ʼ��
	I2C_DeInit(I2Cx);	
	I2C_Cmd(I2Cx, DISABLE);
}


static void iic_restart(I2C_TypeDef* I2Cx)
{
	deinit_iic(I2Cx);
	iic_init(I2Cx);
}


/*
	���Ϳ�ʼ�źţ�ǰ���������豸��ʼ�����
	����0�� ��ʾ�ɹ�
		1�� ��ʾʧ��
*/
int8_t iic_start(I2C_TypeDef* I2Cx)
{
	uint16_t times = 0;
	
	if(I2Cx == I2C1)   //�������
	{
		if((iic_ready & 1) == 0)  //δ��ʼ��������
		{
			iic_init(I2C1);
		}
	}
	else if(I2Cx == I2C2)
	{
		if((iic_ready & 2) == 0)  //δ��ʼ��������
		{
			iic_init(I2C2);
		}
	}
	else{
		return 1;    //ʧ��
	}
	
	//������ʼ�ź�
	I2C_GenerateSTART(I2Cx, ENABLE);
	
	//���EV5
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
	ǰ����������ʼ�ź��ѷ���
	����IIC �豸��ַ�����λ������д������
	����ֵ��
		0��ʾ�ɹ�
		��0 ��ʾʧ��
*/
int8_t iic_put_devaddr(I2C_TypeDef* I2Cx,uint8_t dev_addr)
{
	uint32_t envent;
	uint16_t times = 0;
	
	if(dev_addr&1)  //���λ��1����ʾ������
	{
		I2C_Send7bitAddress(I2Cx,dev_addr, I2C_Direction_Receiver);
		envent = I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED;
	}
	else
	{
		I2C_Send7bitAddress(I2Cx,dev_addr, I2C_Direction_Transmitter);
		envent = I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED;
	}

	//���EV6
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
	ǰ���������豸��ַ�ѷ���
	��������
*/
int8_t iic_put_byte_data(I2C_TypeDef* I2Cx,uint8_t dat)
{			
	I2C_SendData(I2Cx, dat);	
	return	0;	
}


/*
	���ӻ�IIC��Ӧ��λ
	
	����ֵ��
		0 ��ʾӦ��ɹ�
		1 ��ʾδӦ��
*/
uint8_t iic_get_ack(I2C_TypeDef* I2Cx)
{
	uint16_t times = 0;
	//���EV8_2,���������
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
	��һ���ֽڲ�����
		�Ⱦ������� ��ʼ�ź�������������λ�ѷ���

	����ֵ��
		��8λ�� 0 ��ʾ��ȡ�ɹ�
				��0 ��ʾʧ��
		��8λ�� �ɹ�ʱ����ʾ�յ�������
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
			return 1<<8;    //����
		}
	}
			
	dat = I2C_ReceiveData(I2Cx);
	
	return dat;
}


/*
	����Ӧ��
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




