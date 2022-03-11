

/*
	2021-09-26
	��ʮ����ֲ���ļ���
	
	��һ���װ��sm2990�ϡ�

	��Ҫ��
	1.��ѹ�ļ�ض�ȡ
	2.cpu�������¶ȵĶ�ȡ
	3.lcdҺ�����¶ȵĶ�ȡ
	
	//�¶�ת��һ����Ҫ�������ʱ �� 167ms 
	//ÿһ·�¶ȵ�ת����Ҫ37-55ms
	//ÿһ·��ѹ��ת����Ҫ1.2-1.8ms
	
	
	2021-11-30
	�޸ģ�sm2990оƬ�޻�ȡ��
	��ΪHCE75MS��cpu�¶Ⱥ������¶ȣ�
	��18B20��lcd�¶ȣ�
	����ADCͨ���ɼ���ѹ
	
*/


#include "includes.h"
//#include "sm2990.h"      //ֻ��vol_temp�ṩ�ӿڣ����������ļ��ṩ�ӿ�

static uint8_t ns18b20_answer = 0;   //18b20��Ӧ����1��ʾ��Ӧ��0��ʾû��

void vol_temp_init(void)
{
	//18b20��ʼ��
//	ns18b20_init();
	DS18B20_Init();
	//hce75ms��ʼ��
	hce75ms_init();
		
	//adc���Ƴ�ʼ��
	Vol_Adc_Init();	
}




//��õ�ѹֵ
/*
	ͨ����������
				uint16_t *mem_vol,  �ڴ��ѹ��ȡ��
	����ֵ��ʾ�ɹ����
		0��ʾ�ɹ�
		�����ʾʧ��
*/
uint8_t get_vol_monitor(uint16_t *cpu_vol,uint16_t *ls7a_vol,uint16_t *p12_vol)
{
	*cpu_vol  = ADCgetCoreVol()>>1;
	*ls7a_vol = ADCget7AVol()>>1;		
	*p12_vol =  ADCget12VVol()>>1;	

	return 0;	
}

#if 0
//������¶�����ֻ������������
short calc_temperature_int(short temp)
{
	short t = temp*0.0625;
	
	return t;
}
#endif

static uint16_t g_cpu_vol,g_ls7a_vol, g_p12_vol;
static int16_t g_cpu_temp,g_board_temp;
static int16_t g_lcd1_temp=-2000,g_lcd2_temp=-2000;  //�����Ƚ�С�ĳ�ʼֵ
static uint8_t g_lcd_temp_active = 0;   //��0λ�͵�1λ��ʾ�¶���Ч��0��ʾ��Ч��1��ʾ��Ч


//ֹͣҺ������
/*
	1. �ػ�״̬������û�н���׼������״̬
	2. �����¶ȶ�����-5��
	3. ����һ������0��
	4. 18b20û��Ӧ��
*/
static void stop_lcd_heat(void)
{
	if(cpu_run_status == LS3A_POWEROFF)
		set_heat_status(HOT_DISABLE);  //������;
		
	if((g_lcd_temp_active & 3))   //ֵ��0��������������Ч��������һ����Ч
	{
		if(g_lcd1_temp>-160 && g_lcd2_temp>-160 )  //����������-10��
			set_heat_status(HOT_DISABLE);  //������;
		
		if(g_lcd1_temp >-80 || g_lcd2_temp>-80)    //��һ������-5��
			set_heat_status(HOT_DISABLE);  //������;
	}
	else  //��������������Ч�ˣ�������
		set_heat_status(HOT_DISABLE);  //������;
	
	if(!ns18b20_answer)
		set_heat_status(HOT_DISABLE);  //������;
	
	if(g_board_temp > 0)   //2022-01-03 �����¶ȸ���0 ������
		set_heat_status(HOT_DISABLE);  //������;
	
	if(g_cpu_temp > 0) //2022-01-13 v1.4 �����ϵ���һ��������
		set_heat_status(HOT_DISABLE);  //������;
}




/*
	�������񣬽���200msһ�����ڣ�
	�¶ȵ�ת��ʱ��������167ms
*/
void get_temp_vol_task(void)
{
	uint16_t cpu_vol,ls7a_vol, p12_vol;
	int16_t cpu_temp,board_temp;
	int16_t lcd1_temp,lcd2_temp;
	uint8_t ack;
	static uint8_t n = 0;

	if(n%5 == 0)
	{
		//��ȡ��صĵ�ѹֵ
		ack = get_vol_monitor(&cpu_vol,&ls7a_vol,&p12_vol);
		if(ack == 0) //ֻ�ж��˲Ÿ���
		{
			g_cpu_vol = cpu_vol;
			g_ls7a_vol = ls7a_vol;
		//	g_mem_vol = mem_vol;
			g_p12_vol = p12_vol;
		}
		else{
		}
		
		//����18b20ת��������2021-12-11		
		ns18b20_answer = !NS18B20StartConvert();   //����ֵΪ0�����Ϊ1������Ϊ0

	}
	else if(n%5 ==1)
	{
		//��������cpu���¶�
		ack = read_cpu_temp(&cpu_temp);    //ʵ���¶�ֵӦ�ó�0.0625
		if(ack == 0) //ֻ�ж��˲Ÿ���
		{
			g_cpu_temp = cpu_temp;
			if(g_board_temp == -2000) //��������¶ȴ�����ʧЧ�ˣ�cpu�¶ȴ�������Ч 2022-01-13 ���� v1.4
			{
				if(g_cpu_temp > 560) //�����¶ȴ���35����������  //560��ʾ35�ȣ�35/0.0625 = 560
				{
					if(cpu_run_status == LS3A_RUN_OS)    //����ϵͳ��򿪷��� 2022-01-13 ���� v1.4
						set_fan_enable();  //���ȴ�
				}
				else if(g_cpu_temp < 320)   //�����¶�С��20���رշ��� //320��ʾ20�ȣ�160��ʾ10�ȣ�10/0.0625 = 560
				{
					set_fan_disable();  //���Ȳ���
				}
			}
		}
		else{
			g_cpu_temp = -2000;
		}
	}
	else if(n%5 ==2)
	{
		//��������cpu���¶�
		ack = read_board_temp(&board_temp);
		if(ack == 0) //ֻ�ж��˲Ÿ���
		{
			g_board_temp = board_temp;
			if(board_temp > 560) //�����¶ȴ���35����������  //560��ʾ35�ȣ�35/0.0625 = 560
			{
//				set_heat_status(HOT_DISABLE);  //������
				if(cpu_run_status == LS3A_RUN_OS)    //����ϵͳ��򿪷��� 2022-01-13 ���� v1.4
					set_fan_enable();  //���ȴ�
			}
			else if(board_temp < 320)   //�����¶�С��20���رշ��� //320��ʾ20�ȣ�160��ʾ10�ȣ�10/0.0625 = 560
			{
				set_fan_disable();  //���Ȳ���
			}
		}
		else{
			g_board_temp = -2000;   //��ȡ�����¶ȡ� 2022-01-13 ���� v1.4
		}
	}
	else if(n%5 ==3)
	{
		if(ns18b20_answer) //2021-12-15 ת���Ѿ���ʼ�����￼�ǵ�����ɼ����¶Ȳ��ܿ��Ƽ��ȣ�����Ҫȷ���ܶ����¶�
		{
			//���lcd���¶�1   //2021-12-11 18b20�ֿ��ɼ�
			ack = ns18b20_read(0,&lcd1_temp);
			if(ack == 0) //ֻ�ж��˲Ÿ���
			{
				g_lcd1_temp = lcd1_temp;
				
				if(lcd1_temp < -240) //�������ȣ��¶ȵ���-15��
				{				
					set_heat_status(HOT_ENABLE);  //����
					set_fan_disable();  //���Ȳ���					
				}
				g_lcd_temp_active |= 1;  //��һ���¶ȴ�������Ч
			}
			else{  //û�ж����¶Ȳ�����
				g_lcd_temp_active &= ~1;  //��һ���¶ȴ�������Ч
			}
		}
	}
	else if(n%5 ==4)
	{
		if(ns18b20_answer)  //2021-12-15 ת���Ѿ���ʼ�����￼�ǵ�����ɼ����¶Ȳ��ܿ��Ƽ��ȣ�����Ҫȷ���ܶ����¶�
		{
			//���lcd���¶�2  //2021-12-11 18b20�ֿ��ɼ�
			ack = ns18b20_read(1,&lcd2_temp);
			if(ack == 0) //ֻ�ж��˲Ÿ���
			{
				g_lcd2_temp = lcd2_temp;
				
				if(lcd2_temp < -240) //�������ȣ��¶ȵ���-15��//2021-12-11����û�г�0.0625����ֱ������
				{				
					set_heat_status(HOT_ENABLE);  //����
					set_fan_disable();  //���Ȳ���					
				}
				g_lcd_temp_active |= 2;  //�ڶ����¶ȴ�������Ч������ƾ֤
			}
			else{ //û�ж����¶Ȳ�����	
				g_lcd_temp_active &= ~2;  //�ڶ����¶ȴ�������Ч
			}
		}		
	}
	n++;

#ifdef LCD_PWM_HEAT	  //2022-0104   ����pwm�Ĵ���
	if(g_lcdHeat_pwm)
#else	
	if(get_lcd_heat_status() == SET)   //�����Ļ�ڼ��ȵĻ���2021-12-15
#endif
	{
		if(n%4 ==1)
		{
			set_bug_led_on();
		}
		else if(n%4 ==3)
		{
			set_bug_led_off();
		}
		
		stop_lcd_heat();    //ֹͣ��Ļ����
	}
}







/*
	����һ�����񣬶�ʱ�ϱ��¶ȣ���ѹ��״̬��Ϣ
	ÿ500ms����һ�Σ�ÿһ�λ㱨һ����Ϣ
*/
void report_vol_temp_status(void)
{
	static uint8_t n = 0;
	bitstatus_t b_status;
	
	if(n%4 == 0)   //�ϱ�cpu�¶�
	{
		send_bc_temp_to_cpu(g_cpu_temp,g_board_temp);   //�ϱ��¶�
	}
	else if(n%4 == 1)  //�ϱ�lcd���
	{
		b_status.lcd_beat = !!get_lcd_heat_status();
		b_status.breakdownLed_status = !get_bug_led_status();   //����1������0Ϩ��
		b_status.dvi_src = dvi_switch_get() - 1;
		b_status.watch_dog_status = get_watchdog_status();

		send_fan_bk_div_status_to_cpu(b_status,g_fan_pwm,g_lcd_pwm);
	}
	else if(n%4 == 2) //�ϱ���ѹ
	{
		send_vol_to_cpu(g_cpu_vol,g_ls7a_vol, g_p12_vol);
	}
	else if(n%4 == 3)  //�ϱ�lcd�¶�
	{
		send_lcd_temp_to_cpu(g_lcd1_temp,g_lcd2_temp);
	}
	n++;
}





