

/*
	2021-09-26
	第十个移植的文件。
	
	这一层封装在sm2990上。

	主要是
	1.电压的监控读取
	2.cpu和主板温度的读取
	3.lcd液晶屏温度的读取
	
	//温度转换一次需要的最大延时 是 167ms 
	//每一路温度的转换需要37-55ms
	//每一路电压的转换需要1.2-1.8ms
	
	
	2021-11-30
	修改，sm2990芯片无货取消
	改为HCE75MS（cpu温度和主板温度）
	和18B20（lcd温度）
	三个ADC通道采集电压
	
*/


#include "includes.h"
//#include "sm2990.h"      //只对vol_temp提供接口，不对其他文件提供接口

static uint8_t ns18b20_answer = 0;   //18b20有应答吗，1表示有应答，0表示没有

void vol_temp_init(void)
{
	//18b20初始化
//	ns18b20_init();
	DS18B20_Init();
	//hce75ms初始化
	hce75ms_init();
		
	//adc控制初始化
	Vol_Adc_Init();	
}




//获得电压值
/*
	通过参数返回
				uint16_t *mem_vol,  内存电压已取消
	返回值表示成功与否
		0表示成功
		非零表示失败
*/
uint8_t get_vol_monitor(uint16_t *cpu_vol,uint16_t *ls7a_vol,uint16_t *p12_vol)
{
	*cpu_vol  = ADCgetCoreVol()>>1;
	*ls7a_vol = ADCget7AVol()>>1;		
	*p12_vol =  ADCget12VVol()>>1;	

	return 0;	
}

#if 0
//计算出温度来，只保留整数部分
short calc_temperature_int(short temp)
{
	short t = temp*0.0625;
	
	return t;
}
#endif

static uint16_t g_cpu_vol,g_ls7a_vol, g_p12_vol;
static int16_t g_cpu_temp,g_board_temp;
static int16_t g_lcd1_temp=-2000,g_lcd2_temp=-2000;  //给个比较小的初始值
static uint8_t g_lcd_temp_active = 0;   //第0位和第1位表示温度有效，0表示无效，1表示有效


//停止液晶加热
/*
	1. 关机状态，并且没有进入准备启动状态
	2. 两个温度都大于-5度
	3. 其中一个大于0度
	4. 18b20没有应答
*/
static void stop_lcd_heat(void)
{
	if(cpu_run_status == LS3A_POWEROFF)
		set_heat_status(HOT_DISABLE);  //不加热;
		
	if((g_lcd_temp_active & 3))   //值非0，两个传感器有效或者其中一个有效
	{
		if(g_lcd1_temp>-160 && g_lcd2_temp>-160 )  //两个都大于-10度
			set_heat_status(HOT_DISABLE);  //不加热;
		
		if(g_lcd1_temp >-80 || g_lcd2_temp>-80)    //有一个大于-5度
			set_heat_status(HOT_DISABLE);  //不加热;
	}
	else  //两个传感器都无效了，不加热
		set_heat_status(HOT_DISABLE);  //不加热;
	
	if(!ns18b20_answer)
		set_heat_status(HOT_DISABLE);  //不加热;
	
	if(g_board_temp > 0)   //2022-01-03 主板温度高于0 不加热
		set_heat_status(HOT_DISABLE);  //不加热;
	
	if(g_cpu_temp > 0) //2022-01-13 v1.4 主板上的另一个传感器
		set_heat_status(HOT_DISABLE);  //不加热;
}




/*
	周期任务，建议200ms一个周期，
	温度的转换时间周期是167ms
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
		//获取监控的电压值
		ack = get_vol_monitor(&cpu_vol,&ls7a_vol,&p12_vol);
		if(ack == 0) //只有对了才更新
		{
			g_cpu_vol = cpu_vol;
			g_ls7a_vol = ls7a_vol;
		//	g_mem_vol = mem_vol;
			g_p12_vol = p12_vol;
		}
		else{
		}
		
		//启动18b20转换！！！2021-12-11		
		ns18b20_answer = !NS18B20StartConvert();   //返回值为0，结果为1，否则为0

	}
	else if(n%5 ==1)
	{
		//获得主板和cpu的温度
		ack = read_cpu_temp(&cpu_temp);    //实际温度值应该乘0.0625
		if(ack == 0) //只有对了才更新
		{
			g_cpu_temp = cpu_temp;
			if(g_board_temp == -2000) //如果主板温度传感器失效了，cpu温度传感器有效 2022-01-13 增加 v1.4
			{
				if(g_cpu_temp > 560) //环境温度大于35，开启风扇  //560表示35度，35/0.0625 = 560
				{
					if(cpu_run_status == LS3A_RUN_OS)    //进入系统后打开风扇 2022-01-13 增加 v1.4
						set_fan_enable();  //风扇打开
				}
				else if(g_cpu_temp < 320)   //环境温度小于20，关闭风扇 //320表示20度，160表示10度，10/0.0625 = 560
				{
					set_fan_disable();  //风扇不开
				}
			}
		}
		else{
			g_cpu_temp = -2000;
		}
	}
	else if(n%5 ==2)
	{
		//获得主板和cpu的温度
		ack = read_board_temp(&board_temp);
		if(ack == 0) //只有对了才更新
		{
			g_board_temp = board_temp;
			if(board_temp > 560) //环境温度大于35，开启风扇  //560表示35度，35/0.0625 = 560
			{
//				set_heat_status(HOT_DISABLE);  //不加热
				if(cpu_run_status == LS3A_RUN_OS)    //进入系统后打开风扇 2022-01-13 增加 v1.4
					set_fan_enable();  //风扇打开
			}
			else if(board_temp < 320)   //环境温度小于20，关闭风扇 //320表示20度，160表示10度，10/0.0625 = 560
			{
				set_fan_disable();  //风扇不开
			}
		}
		else{
			g_board_temp = -2000;   //读取不到温度。 2022-01-13 增加 v1.4
		}
	}
	else if(n%5 ==3)
	{
		if(ns18b20_answer) //2021-12-15 转换已经开始，这里考虑到必须采集到温度才能控制加热，所以要确保能读到温度
		{
			//获得lcd的温度1   //2021-12-11 18b20分开采集
			ack = ns18b20_read(0,&lcd1_temp);
			if(ack == 0) //只有对了才更新
			{
				g_lcd1_temp = lcd1_temp;
				
				if(lcd1_temp < -240) //开启加热，温度低于-15度
				{				
					set_heat_status(HOT_ENABLE);  //加热
					set_fan_disable();  //风扇不开					
				}
				g_lcd_temp_active |= 1;  //第一个温度传感器有效
			}
			else{  //没有读到温度不加热
				g_lcd_temp_active &= ~1;  //第一个温度传感器无效
			}
		}
	}
	else if(n%5 ==4)
	{
		if(ns18b20_answer)  //2021-12-15 转换已经开始，这里考虑到必须采集到温度才能控制加热，所以要确保能读到温度
		{
			//获得lcd的温度2  //2021-12-11 18b20分开采集
			ack = ns18b20_read(1,&lcd2_temp);
			if(ack == 0) //只有对了才更新
			{
				g_lcd2_temp = lcd2_temp;
				
				if(lcd2_temp < -240) //开启加热，温度低于-15度//2021-12-11这里没有乘0.0625，就直接算了
				{				
					set_heat_status(HOT_ENABLE);  //加热
					set_fan_disable();  //风扇不开					
				}
				g_lcd_temp_active |= 2;  //第二个温度传感器有效，加热凭证
			}
			else{ //没有读到温度不加热	
				g_lcd_temp_active &= ~2;  //第二个温度传感器无效
			}
		}		
	}
	n++;

#ifdef LCD_PWM_HEAT	  //2022-0104   增加pwm的处理
	if(g_lcdHeat_pwm)
#else	
	if(get_lcd_heat_status() == SET)   //如果屏幕在加热的话，2021-12-15
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
		
		stop_lcd_heat();    //停止屏幕加热
	}
}







/*
	增加一个任务，定时上报温度，电压等状态信息
	每500ms进入一次，每一次汇报一种信息
*/
void report_vol_temp_status(void)
{
	static uint8_t n = 0;
	bitstatus_t b_status;
	
	if(n%4 == 0)   //上报cpu温度
	{
		send_bc_temp_to_cpu(g_cpu_temp,g_board_temp);   //上报温度
	}
	else if(n%4 == 1)  //上报lcd情况
	{
		b_status.lcd_beat = !!get_lcd_heat_status();
		b_status.breakdownLed_status = !get_bug_led_status();   //返回1点亮，0熄灭
		b_status.dvi_src = dvi_switch_get() - 1;
		b_status.watch_dog_status = get_watchdog_status();

		send_fan_bk_div_status_to_cpu(b_status,g_fan_pwm,g_lcd_pwm);
	}
	else if(n%4 == 2) //上报电压
	{
		send_vol_to_cpu(g_cpu_vol,g_ls7a_vol, g_p12_vol);
	}
	else if(n%4 == 3)  //上报lcd温度
	{
		send_lcd_temp_to_cpu(g_lcd1_temp,g_lcd2_temp);
	}
	n++;
}





