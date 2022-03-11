
#include "stm32f10x.h"
#include "i2c2.h"
#include "i2c1.h"
/*
	�������ֲ���ļ���
	
	������IIC��Ӧ�ò㣬ʵ����������
		��Ҫ���õײ��hard_iiC.c�е�ʵ�֣�����ʹ��ģ��ķ�ʽʵ��
		
	1. ����λ�ö�����  IICapp_read_bytes
	2. ����λ��д����  IICapp_write_bytes
	
*/

void IICapp_init(I2C_TypeDef* I2Cx)
{
//	iic_init(I2Cx);
	if(I2Cx == I2C1)	
		IIC_Init();
	else if (I2Cx == I2C2)
		IIC2_Init();
}

/*
	��ǰλ�ö�����ֽڡ����һ�ζ�ȡ256���ֽڣ�������
	����    0 ��ʾ�ɹ�����0��ʾʧ��
	!!!!!�������ṩ�ӿ�
*/
static uint8_t IICapp_read_byte_cur(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t *dat,uint8_t len)
{
	uint8_t i;
	
	if(I2Cx == I2C1)
	{
		IIC_Start();
				
		//2.�����豸��ַ
		IIC_Send_Byte(dev_addr | 1);	    //��������ַ
		if(IIC_Wait_Ack()!= 0) 
		//if(iic_put_devaddr(I2Cx,dev_addr | 1) != 0)   //���λ��1����ʾ������
		{
			//printf("i2c  read_byte_cur send dev addr error!\n");
			return 2;
		}
		
		for(i=0;i<len;i++)
		{					
			//3.���һ���ֽڵ�����
			//dat[i] = iic_get_byte_data(I2Cx);
		
			//4.ֻ�����һ�����ݷ��ͷ�Ӧ��
			if(i == len -1)
			{
				dat[i]=IIC_Read_Byte(0);
			}
			else
			{
				dat[i]=IIC_Read_Byte(1);
			}
				
		}		
		//.����stopʱ��
		IIC_Stop();//����һ��ֹͣ����
		
	}
	else if(I2Cx == I2C2)
	{
		IIC2_Start();
				
		//2.�����豸��ַ
		IIC2_Send_Byte(dev_addr | 1);	    //��������ַ
		if(IIC2_Wait_Ack()!= 0) 
		//if(iic_put_devaddr(I2Cx,dev_addr | 1) != 0)   //���λ��1����ʾ������
		{
			//printf("i2c  read_byte_cur send dev addr error!\n");
			return 2;
		}
		
		for(i=0;i<len;i++)
		{					
			//3.���һ���ֽڵ�����
			//dat[i] = iic_get_byte_data(I2Cx);
		
			//4.ֻ�����һ�����ݷ��ͷ�Ӧ��
			if(i == len -1)
			{
				dat[i]=IIC2_Read_Byte(0);
			}
			else
			{
				dat[i]=IIC2_Read_Byte(1);
			}
				
		}		
		//.����stopʱ��
		//iic_stop(I2Cx);     //����ֹͣ�źţ����߾Ϳ�����
		IIC2_Stop();//����һ��ֹͣ����	
	}
	
	return 0;
}

/*
	IICappд������ݣ����д��256���ֽڣ�����
	������
		word_addr �ռ��ַ������Ҫ���ֽ�datд��24c02����һ���洢��Ԫ��ȥ
		dat       ʵ�����ݵ��׵�ַ��
		len       ʵ����Ҫд�����ݵĸ���
	����ֵ��
		0  ��     �ɹ�
		��0��     ʧ��
*/
uint8_t IICapp_write_bytes(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t word_addr,const uint8_t *dat,uint8_t len)
{
	uint8_t i;
	
	if(I2Cx == I2C1)
	{
		IIC_Start();
		
		//2.�����豸��ַ
		IIC_Send_Byte(dev_addr & 0xfe);	    //��������ַ
		if(IIC_Wait_Ack()!= 0) 
		{
			//printf("i2c  read_byte_cur send dev addr error!\n");
			IIC_Stop();
			return 2;
		}

		//3.���Ϳռ��ַ
		IIC_Send_Byte(word_addr);     //�����ֽ�							    
		if(IIC_Wait_Ack()!= 0)  //���û��Ӧ��ֱ���˳�
		{
			//printf("send word addr error!\n");
			IIC_Stop(); //iic_stop(I2Cx);     //����ֹͣ�źţ����߾Ϳ�����
			return 3;
		}
		
		//len����0��ʱ������Ϊ���������һ����Ч����
		if(len == 0)
		{
			return 255;   //���Ǹ��������
		}
		
		for(i=0;i<len;i++)
		{		
			//4.��������
			IIC_Send_Byte(dat[i]);     //�����ֽ�							    
			if(IIC_Wait_Ack()!= 0)  //���û��Ӧ��ֱ���˳�
			//if(iic_put_byte_data(I2Cx,dat[i]))  //���û��Ӧ��ֱ���˳�
			{
				//printf("send data error!\n");
				IIC_Stop(); //iic_stop(I2Cx);     //����ֹͣ�źţ����߾Ϳ�����
				return 4;
			}
		}
		
		//5.�������������ߵ�ռ��
		IIC_Stop(); //iic_stop(I2Cx);
	}
	else if(I2Cx == I2C2)
	{
		IIC2_Start();
		
		//2.�����豸��ַ
		IIC2_Send_Byte(dev_addr & 0xfe);	    //��������ַ
		if(IIC2_Wait_Ack()!= 0) 
		{
			//printf("i2c  read_byte_cur send dev addr error!\n");
			return 2;
		}

		//3.���Ϳռ��ַ
		IIC2_Send_Byte(word_addr);     //�����ֽ�							    
		if(IIC2_Wait_Ack()!= 0)  //���û��Ӧ��ֱ���˳�
		{
			//printf("send word addr error!\n");
			IIC2_Stop(); //iic_stop(I2Cx);     //����ֹͣ�źţ����߾Ϳ�����
			return 3;
		}
		
		//len����0��ʱ������Ϊ���������һ����Ч����
		if(len == 0)
		{
			return 255;   //���Ǹ��������
		}
		
		for(i=0;i<len;i++)
		{		
			//4.��������
			IIC2_Send_Byte(dat[i]);     //�����ֽ�							    
			if(IIC2_Wait_Ack()!= 0)  //���û��Ӧ��ֱ���˳�
			{
				//printf("send data error!\n");
				IIC2_Stop(); //iic_stop(I2Cx);     //����ֹͣ�źţ����߾Ϳ�����
				return 4;
			}
		}		
		//5.�������������ߵ�ռ��
		IIC2_Stop(); //iic_stop(I2Cx);
	}	
	
	return 0;
}





/*
	���������
	���� �� word_addr ָ����Ҫ��ȡ��λ��
			dat ��ʾ�洢���ݻ�����׵�ַ
			len ��ʾ��Ҫ��ȡ���ݵĸ���
	����ֵ�� 0 ��ʾ�ɹ�
			��0 ��ʾʧ��
*/
uint8_t IICapp_read_bytes(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t word_addr,uint8_t *dat,uint8_t len)
{
	uint8_t ret;
	
	ret = IICapp_write_bytes(I2Cx,dev_addr,word_addr,(void*)0,0);
	
	if(ret == 255)	//��������Ĵ���
		return IICapp_read_byte_cur(I2Cx,dev_addr,dat,len);
	
	return ret;   //���ⷵ��
}

