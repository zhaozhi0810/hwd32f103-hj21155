

#ifndef __NCA9555_H__
#define __NCA9555_H__

#include <stm32f10x.h>

typedef enum  
{
	INPORT0 = 0,
	INPORT1 = 1,
	OUTPORT0 = 2,
	OUTPORT1 = 3,
	INVPORT0 = 4,
	INVPORT1 = 5,
	CFGPORT0 = 6,
	CFGPORT1 = 7
}Nca9555_Cmd_t;
	


#define PCA9555_DEVADDR 0x40    //��4λ��Ч


//#define PCA9555_DEV1 (0<<1)
//#define PCA9555_DEV2 (4<<1)

void nca9555_init(void);


//�����������ɲ�������
/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_read_2inport(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[]);


/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2config(I2C_TypeDef*IICn,uint8_t dev_addr,const uint8_t dat[]);



/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�

	����dev_addr ֻ���ǵ�4λ�����λ����Ϊ0���ɡ�ֻ��Ҫ��7λ��������
*/
uint8_t nca9555_read_2outport(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[]);


/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2outport(I2C_TypeDef*IICn,uint8_t dev_addr,const uint8_t dat[]);

/*
	����ֵ��0��ʾ����
		0��ʾ�ɹ�
*/
uint8_t nca9555_write_2inv_cfg(I2C_TypeDef*IICn,uint8_t dev_addr,const uint8_t dat[]);



//2022-01-03 ����
uint8_t nca9555_read_2outport(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[]);
//2022-01-03 ��ȡ���üĴ�����ֵ�������жϵ�Ƭ����Ҫ�������Ǹ�λ
uint8_t nca9555_read_2config(I2C_TypeDef*IICn,uint8_t dev_addr,uint8_t dat[]);


#endif
