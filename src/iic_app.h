
#ifndef __IIC_APP_H__
#define __IIC_APP_H__

#include <stm32f10x.h>


void IICapp_init(I2C_TypeDef* I2Cx);

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
uint8_t IICapp_write_bytes(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t word_addr,const uint8_t *dat,uint8_t len);



/*
	���������
	���� �� word_addr ָ����Ҫ��ȡ��λ��
			dat ��ʾ�洢���ݻ�����׵�ַ
			len ��ʾ��Ҫ��ȡ���ݵĸ���
	����ֵ�� 0 ��ʾ�ɹ�
			��0 ��ʾʧ��
*/
uint8_t IICapp_read_bytes(I2C_TypeDef* I2Cx,uint8_t dev_addr,uint8_t word_addr,uint8_t *dat,uint8_t len);


#endif
