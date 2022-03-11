
#include "includes.h"


/*
	2021-09-16
	
	1.��������ֲ���ļ� ��������led��Ҫ�õ����������
	
	��Ƭ �����Ҹ����飬һ������9����������һ������9��led��
	һƬ����9��led�����ģʽ
	һƬ����9������������ʹ���ⲿ�жϡ�
	
	������ͬ��
	
	��NCA9555 �������NXP PCA9555PW����ɵ� ��I2C2��ɣ�PB10��11��12���жϣ���
		������ I2C ��ַ 0x4������λ 100��
		LED �� I2C ��ַ 0x0������λ 000��

	����iic��������һ��������ߣ�һ�������ұ�


Command Register
0         Input port 0   //�˿�0����������
1         Input port 1
2         Output port 0   //�˿�0���������
3         Output port 1
4         Polarity inversion port 0  //����������ݼ���ȡ��
5         Polarity inversion port 1
6         Configuration port 0     //�������ŵĹ��ܣ�1 ���루Ĭ�ϣ� 0 ���
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
	
	IICapp_init(I2C2);  //��������ʼ��
	IICapp_init(I2C1);  //��������ʼ��,�������������ֱ�������ҵĵƺͰ���

#ifdef 	BTNS_USE_INT   //����btns_leds.h�ж���
	//�ж����ų�ʼ��
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_IN_FLOATING;//GPIO_Mode_IPU;    //�ж��������ţ��ʹ����ж�
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;   //12��13�������Ŷ����ж���
	GPIO_Init(GPIOB, &GPIO_InitStruct);
		
	//�ⲿ�ж�����
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line12;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  //�ж��ǵ͵�ƽ������ѡ���½��ش���
	EXTI_Init(&EXTI_InitStruct);     //���������ŵı仯������жϣ������ɿ��Ͱ��¶�������ж�
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line13;    //�ⲿ�ж�13Ҳʹ��
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  //�ж��ǵ͵�ƽ������ѡ���½��ش���
	EXTI_Init(&EXTI_InitStruct); 
	
	
	//�жϿ�����ʹ�ܣ�ʹ�õ����ⲿ�ж�12��13
	NVIC_InitStruct.NVIC_IRQChannel= EXTI15_10_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;    //ʹ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
#endif	
}




//�����������ɲ�������
/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�

	����dev_addr ֻ���ǵ�4λ�����λ����Ϊ0���ɡ�ֻ��Ҫ��7λ��������
*/
uint8_t nca9555_read_2inport(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_read_bytes(IICn,dev_addr,INPORT0,dat,DATA_LENGTH);
}



/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�

	����dev_addr ֻ���ǵ�4λ�����λ����Ϊ0���ɡ�ֻ��Ҫ��7λ��������
*/
uint8_t nca9555_read_2outport(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[])
{	
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_read_bytes(IICn,dev_addr,OUTPORT0,dat,DATA_LENGTH);
}

/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2config(I2C_TypeDef*IICn,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_write_bytes(IICn,dev_addr,CFGPORT0,dat,DATA_LENGTH);
}

//2022-01-03 ��ȡ���üĴ�����ֵ�������жϵ�Ƭ����Ҫ�������Ǹ�λ
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

//2022-01-03���ӽ���

/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2outport(I2C_TypeDef*IICn,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_write_bytes(IICn,dev_addr,OUTPORT0,dat,DATA_LENGTH);
}

/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2inv_cfg(I2C_TypeDef*IICn,uint8_t dev_addr,const uint8_t dat[])
{
	dev_addr &= 0xe;
	dev_addr |= PCA9555_DEVADDR;
	return IICapp_write_bytes(IICn,dev_addr,INVPORT0,dat,DATA_LENGTH);
}

