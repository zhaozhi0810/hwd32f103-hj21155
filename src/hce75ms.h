
#ifndef __HCE75MS_H__
#define __HCE75MS_H__

int hce75ms_init(void);


//��cpu���¶�(�����¶ȷ��غ�Ӧ��*0.0625����ʵ���¶�)
//ͨ�����������¶�ֵ
//�ɹ�����0�����򷵻�-1
int read_cpu_temp(short* temp);


//��board���¶�(�����¶ȷ��غ�Ӧ��*0.0625����ʵ���¶�)
//ͨ�����������¶�ֵ
//�ɹ�����0�����򷵻�-1
int read_board_temp(short* temp);


#endif



