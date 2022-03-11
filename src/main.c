
#include "includes.h"

/*
2021-09-27
1.-40~-20�� ����ǰ�ȼ��� ���ȵ�-10��ͨ��������о������0�Ȳ��ܼ��ȡ�
2.�ػ�����Ļ�����ȣ������ر����еĵ�Դ��lcden��led��ȫ���رգ�
3.������ť����һ�¿�������һ�¹ػ����������������о����ɹػ���������ֱ�Ӷϵ�ػ�
4.���ݻ����¶ȿ��Ʒ��ȵĿ��͹�
5.18��led�ĵ�������Ϩ����Ӧ�ó�����ƣ�ͨ������3�ϴ�
6.�¶ȵ�ѹ����ɴ���2�ϴ�
7.���Դ���1
*/

//800ms ���Ź�
static void iwdog_init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_8);    //���÷���ϵ��
	
	IWDG_Enable(); //ʹ�ܿ��Ź�
}


static  void iwdog_feed(void)
{
	if(mcu_reboot)  //����mcu��������ι����2021-12-17����
		return ;
	IWDG_ReloadCounter();
}




/*
	�弶��ʼ��
	
	����
	0�� ��ʾ�ɹ�
	���㣺 ʧ��
*/
int boardInit(void)
{
	//1.�����жϷ���
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    //2. ϵͳtik���ã���Ҫȷ������������ȵ��ã�����delay���Ƶĺ�������ʹ�ã�������
    systick_config();
    	
    /*4.  GPIO config */
	gpio_init_all();
	    
    //6. ��ʼ��PWM��û��ʹ�ܶ�ʱ�������2021-12-07
	lcd_pwm_init(60);    //��ʼ����60%  
    
	//6. ��ʼ��I2C1 �¶Ȳɼ�
	vol_temp_init();
	
#ifdef 	LCD_PWM_HEAT
	LcdHeat_pwm_init(0);   //��ʼ��Ϊioģʽ����ֹ���� 2022-01-04
#endif	
		
	//�ر����У�//20210-12-16ȥ������������͹���ģʽ���ⲿ�ж�1����
	cpu_poweroff_enable();    
	
	//7. �������Ź� 2021-12-15
 	iwdog_init();    //2022-01-06  v1.3  �����������ˡ���
	
//	iwdog_feed();   //ι��2022-0103 2022-01-06  v1.3ɾ��
	return 0;
}








uint16_t task_id;   //ÿһ��λ��Ӧһ������Ϊ1��ʾ��Ҫ���������������������λ
//2021-09-30����task_allow,���ƶ�ʱ����������������
//uint16_t task_allow; //ÿһ��λ��Ӧһ������Ϊ1��ʾ����ʱɨ������񣬹ػ��󣬿��Խ�����Ҫ����������Ϊ������





typedef void(* task_t)(void);

int main(void)
{
	uint16_t i;
	
	const task_t task[TASK_MAX]={btn_power_change_scan    //����1���ϵ簴ť����Ƶ�л���ťɨ��								
								,[1] = task1_btn_scan       		//����2��18������ɨ�裬��Ҫ�ж���ϡ�û���жϾ��ǿ�ת
								,[2] = task_check_run_status    //����3������״̬��⣬�ػ��������ƣ�������ȼ����Ե�һ��
								,[3] = get_temp_vol_task       //����4����ʪ�ȣ���ѹ��ض�ȡ����2000msһ������
								,[4] = report_vol_temp_status  //����5����ʱ��cpu�㱨��500msһ��
								,[5] = com3_frame_handle    //���ڽ��յ����ݵĴ���������ɿ����жϵ���
								,[15] = iwdog_feed         //���һ������ι��
						//	,[15]=task_idle_workled       //����16�����һ�������ù���led����˸,1s����һ��
						//��Ϊ�����Ʋ�������ʹ�ã�����ɾ��������2021-12-01
	};
	
	boardInit();
	
	
//	delay_1ms(200);
//	set_led_off(LEDALL);
	
	while(1)
	{
		for(i=0;i<TASK_MAX && task_id;i++){
			if(task_id & (1<<i))   //��ʱʱ�䵽��Ҫִ��
			{
				task_id &= ~(1<<i);  //��Ӧ��λ�ñ����㣬�ȴ���ʱ������
			
				if(task[i])  //ָ�벻��Ϊ��
				{	
					task[i](); //ִ�и�����
					break;    //һ��ִֻ��һ����������ǰ�����ȼ��ߣ����񿿺�����ȼ���
				}				
			}
		}//end for		
	}
}

