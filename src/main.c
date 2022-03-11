
#include "includes.h"

/*
2021-09-27
1.-40~-20度 开机前先加热 加热到-10再通电启动龙芯，大于0度不能加热。
2.关机后，屏幕不加热，尽量关闭所有的电源（lcden，led灯全部关闭）
3.开机按钮，点一下开机，点一下关机（发串口命令给龙芯，完成关机），长按直接断电关机
4.根据环境温度控制风扇的开和关
5.18个led的点亮或者熄灭由应用程序控制，通过串口3上传
6.温度电压监控由串口2上传
7.调试串口1
*/

//800ms 看门狗
static void iwdog_init(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_8);    //设置分配系数
	
	IWDG_Enable(); //使能看门狗
}


static  void iwdog_feed(void)
{
	if(mcu_reboot)  //设置mcu重启，不喂狗。2021-12-17增加
		return ;
	IWDG_ReloadCounter();
}




/*
	板级初始化
	
	返回
	0： 表示成功
	非零： 失败
*/
int boardInit(void)
{
	//1.配置中断分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    //2. 系统tik配置，需要确保这个函数最先调用，否则delay类似的函数不能使用！！！！
    systick_config();
    	
    /*4.  GPIO config */
	gpio_init_all();
	    
    //6. 初始化PWM并没有使能定时器输出，2021-12-07
	lcd_pwm_init(60);    //初始亮度60%  
    
	//6. 初始化I2C1 温度采集
	vol_temp_init();
	
#ifdef 	LCD_PWM_HEAT
	LcdHeat_pwm_init(0);   //初始化为io模式，禁止加热 2022-01-04
#endif	
		
	//关闭所有，//20210-12-16去除。。。进入低功耗模式，外部中断1唤醒
	cpu_poweroff_enable();    
	
	//7. 开启看门狗 2021-12-15
 	iwdog_init();    //2022-01-06  v1.3  调整到下面了。。
	
//	iwdog_feed();   //喂狗2022-0103 2022-01-06  v1.3删除
	return 0;
}








uint16_t task_id;   //每一个位对应一个任务，为1表示需要启动任务，在任务中清零该位
//2021-09-30增加task_allow,控制定时器，设置任务运行
//uint16_t task_allow; //每一个位对应一个任务，为1表示允许定时扫描该任务，关机后，可以将不必要的任务设置为不允许





typedef void(* task_t)(void);

int main(void)
{
	uint16_t i;
	
	const task_t task[TASK_MAX]={btn_power_change_scan    //任务1，上电按钮和视频切换按钮扫描								
								,[1] = task1_btn_scan       		//任务2，18个按键扫描，需要中断配合。没有中断就是空转
								,[2] = task_check_run_status    //任务3，运行状态检测，关机重启控制，这个优先级可以低一点
								,[3] = get_temp_vol_task       //任务4，温湿度，电压监控读取任务，2000ms一个周期
								,[4] = report_vol_temp_status  //任务5，定时向cpu汇报，500ms一次
								,[5] = com3_frame_handle    //串口接收到数据的处理！！这个由空闲中断调起
								,[15] = iwdog_feed         //最后一个任务喂狗
						//	,[15]=task_idle_workled       //任务16，最后一个任务，让工作led灯闪烁,1s调用一次
						//因为工作灯不能正常使用，所以删除该任务。2021-12-01
	};
	
	boardInit();
	
	
//	delay_1ms(200);
//	set_led_off(LEDALL);
	
	while(1)
	{
		for(i=0;i<TASK_MAX && task_id;i++){
			if(task_id & (1<<i))   //定时时间到，要执行
			{
				task_id &= ~(1<<i);  //对应的位置被清零，等待定时器设置
			
				if(task[i])  //指针不能为空
				{	
					task[i](); //执行该任务
					break;    //一次只执行一个任务，任务靠前的优先级高，任务靠后的优先级低
				}				
			}
		}//end for		
	}
}

