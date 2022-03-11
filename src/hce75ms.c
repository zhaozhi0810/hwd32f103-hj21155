
/*
	2021-11-30 hce75ms���ڲ��������cpu�¶�
	
	iic1ͨ�ŷ�ʽ
	
	cpu�¶ȵĵ�ַ 0x91/0x90  ��Ӧ��д
	�����¶ȵĵ�ַ 0x93/0x92  ��Ӧ��д
	
*/
#include "includes.h"



#define HCE75_CPU_TEMP_ADDR 0x90
#define HCE75_BOARD_TEMP_ADDR 0x92

/*
	������iic��������ʼ����������Ϊ13λģʽ������
	�ɹ�����0�����򷵻�-1
*/
int hce75ms_init(void)
{
	int ret = 0;
//	uint8_t dat[2] = {0x00,0x30}; //����Ϊ13λģʽ
	//��������ʼ��
	IICapp_init(I2C1);   //
		
	//����hceģʽ13λģʽ
	//ret = IICapp_write_bytes(I2C1,HCE75_CPU_TEMP_ADDR,1,dat,2);
	//ret += IICapp_write_bytes(I2C1,HCE75_BOARD_TEMP_ADDR,1,dat,2);   //����Ҳ���ã��������㷽���	
	return ret;
}

//��cpu���¶�(�����¶ȷ��غ�Ӧ��*0.0625����ʵ���¶�)
//ͨ�����������¶�ֵ
//�ɹ�����0�����򷵻�-1
int read_cpu_temp(short* temp)
{
	uint8_t dat[2];
	if(0 == IICapp_read_bytes(I2C1,HCE75_CPU_TEMP_ADDR,0,dat,2))   //�ֲ���û˵�ǿ��Խ���������ݶ�ȡ�ģ���֪�Ƿ���У�����
	{
		*temp= dat[0]<<8 | dat[1];   //
		*temp >>= 4;   //��4λ���������� ���Ƶĺô����ܹ���������λ����
		return 0;
	}
	return -1;
}

//��board���¶�(�����¶ȷ��غ�Ӧ��*0.0625����ʵ���¶�)
//ͨ�����������¶�ֵ
//�ɹ�����0�����򷵻�-1
int read_board_temp(short* temp)
{
	uint8_t dat[2];
	
	if(0 == IICapp_read_bytes(I2C1,HCE75_BOARD_TEMP_ADDR,0,dat,2))
	{
		*temp= dat[0]<<8 | dat[1];   //
		*temp >>= 4;   //��4λ���������� ���Ƶĺô����ܹ���������λ����
		return 0;
	}
	return -1;
}
