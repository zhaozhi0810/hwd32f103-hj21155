
#include "includes.h"

/*
	2021-09-18   ！！！！！这个文件包含除了18个按键和led之外的其他io操作
	
	1. 第七个移植的文件。
	
	2. 按键LED说明
	新的开发板   按键和LED调整比较大   （与旧的相比）
	旧：
	都是单独GPIO对应
	新：	
	1-16 是扩展芯片（NCA9555 国产替代NXP PCA9555PW）完成的 （I2C2完成，PB10，11，12（中断））
		按键： I2C 地址 0x4（低三位 100）
		LED ： I2C 地址 0x0（低三位 000）
	按键：17，18是gpio实现的 （PC2，PC3）
	LED ：17，18是gpio实现的 （PC0，PC1）

	3.风扇，PB3，PB4 需要同时工作。  GPO             ✔15 (0926增加)
	
	GPIOA:
	4. UART2 --> 7A1000,PA2,PA3
	5. UART1 --> FPGA PA9,PA10   ,做GPIO使用            ✔1
		（其他没有使用，13，14做调试端口）
	GPIOB:
	6. GPB0 IO 面板视频切换开关            				✔2
	7. GPB1 IO 面板电源开关            					✔3
	8. GPB2 IO 故障指示灯            					✔4
	9. GPB3，4  ---》参考3PWM风扇
	10. GPB6，7 I2C接口，温度读取
	11. GPB8，9 IO 输出                            		✔5
	12. GPB10，11，12 ---》参考2. 按键LED说明
	13. GPB15  IO 加热使能                          	✔6
		（其他没有使用）
	GPIOC：
	14. GPC0-3 ---》参考2. 按键LED说明
	15. GPC4(22)，5(14)，6(15) IO 连接7A1000，启动关机等控制         ✔7
	16. GPC7，8 IO 看门狗，使能，输出				     ✔8
	17. GPC9  lvds PWM 屏幕亮度 tim3通道4		
	18. GPC10，IO，lvds背光使能 						 ✔9
	19. GPC11，12 IO  预留INDVI						     ✔10
	20. GPC13 IO 系统上电								 ✔11
	21. GPC14  CPU复位				            		 ✔12
	22. GPC15  7A复位							         ✔13
	
	GPIOD：
	23. GPD1 ： 工作指示灯								  ✔14
	
	
	本文件对应除了18个led和18个按键之外的其他io控制
	包括：
	1.开关机按键
	2.视频切换按键
	
	
	5.故障指示灯
	6.工作指示灯
	7.lcd加热使能
	8.lcd背光使能
	9.cpu电源上电控制
	10.视频切换引脚---到fpga
	11.cpu运行状态获取，两个io引脚

2021-12-08
PA6 低输出龙芯
	高输出DVI


*/

system_run_status cpu_run_status = LS3A_POWEROFF;  //3A3000 cpu运行状态，默认是关机状态
static uint16_t time_count = 0;    //3A3000状态切换计时
//static uint8_t hot_en;   //全局标志，为1表示允许加热，为0表示不允许加热	
static BTN_INFO btn2[2];   //记录按键信息
static uint8_t btn_press_num = 0;  //记录哪一个按键被按下，0表示没有按键被按下
uint8_t g_fan_pwm=0;     //风扇控制，2021-12-09 不使用pwm
//static uint8_t enter_system; //监控是否进入系统
uint8_t mcu_reboot = 0;   //单片机重启 2021-12-17增加，cpu关机后，单片机重启一次。

//检测cpu运行状态的改为中断方式
//2021-12-09
static void PB8PB9_set_int(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
//中断引脚初始化
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_IN_FLOATING;//GPIO_Mode_IPU;    //中断输入引脚，低触发中断
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;   //12和13两个引脚都是中断了
	GPIO_Init(GPIOB, &GPIO_InitStruct);
		
	//外部中断配置
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource9);
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line8;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  //选择双边沿触发
	EXTI_Init(&EXTI_InitStruct);     //由于是引脚的变化会产生中断，所以松开和按下都会产生中断
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line9;    //外部中断9也使能
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  //选择双边沿触发
	EXTI_Init(&EXTI_InitStruct); 
	
	
	//中断控制器使能，使用的是外部中断8，9
	NVIC_InitStruct.NVIC_IRQChannel= EXTI9_5_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;    //使能
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
}

//电源按键和视频切换按键改为中断方式
//2021-12-09
static void PB0PB1_set_int(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef EXTI_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
//中断引脚初始化
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;//GPIO_Mode_IN_FLOATING;//GPIO_Mode_IPU;    //中断输入引脚，低触发中断
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;   //12和13两个引脚都是中断了
	GPIO_Init(GPIOB, &GPIO_InitStruct);
		
	//外部中断配置
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line0;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  //选择下降沿触发
	EXTI_Init(&EXTI_InitStruct);     //由于是引脚的变化会产生中断，所以松开和按下都会产生中断
	
	EXTI_InitStruct.EXTI_Line = EXTI_Line1;    //外部中断9也使能
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;  //选择下降沿触发
	EXTI_Init(&EXTI_InitStruct); 
	
	
	//中断控制器使能，使用的是外部中断0，1
	NVIC_InitStruct.NVIC_IRQChannel= EXTI1_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;    //使能
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel= EXTI0_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_Init(&NVIC_InitStruct);
}


/*
	除了18个按键和led外，其他的IO部分，初始化函数			
*/		
static void others_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	//1.io引脚涉及的时钟不再初始化，前面都统一初始化了。
	
	//pc14，15需当作gpio，需要关闭LSE控制器  2021-12-02
	//RCC_LSEConfig(RCC_LSE_OFF);  //2021-12-09 不再使用这个引脚
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);  //2021-12-08  低
	//2. 输出功能引脚的初始化(PA10,PB2,PB8,PB9,PB15,PC4,PC7,PC8,PC10-PC15,PD1，PB3，PB4)
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;    
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;             //PA7用于与fpga通信，进行视频切换 2021-12-08增加PA6也用于通信
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 |GPIO_Pin_4  | GPIO_Pin_15;  //| GPIO_Pin_8 | GPIO_Pin_9
	GPIO_Init(GPIOB, &GPIO_InitStruct);
			
	//lcd 背光使能。调试完后去除
//	GPIO_SetBits(GPIOC, GPIO_Pin_0);    //背光设置为高
//	GPIO_SetBits(GPIOC, GPIO_Pin_13);    //控制启动系统引脚
	//以上两条调试完后去除
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_4 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_13;  //
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	//2021-12-15,两个引脚改为输入模式，不再使用！！！！
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;   //
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	//2.1 改输出高电平（其他需要高电平的注意增加！！！！！！！）
	GPIO_SetBits(GPIOB, GPIO_Pin_2);    //故障指示灯，高电平熄灭
	GPIO_SetBits(GPIOD, GPIO_Pin_1);    //工作指示灯，高电平熄灭
	
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	//PB8，9设置为中断模式
	PB8PB9_set_int();
	
	//电源按键和视频切换按键设置为中断方式
	PB0PB1_set_int();
}	

#if 0

//系统没有上电之前的初始化
void gpio_init_power_btn(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	//电源按键和视频切换按键设置为中断方式
	PB0PB1_set_int();
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_13 |GPIO_Pin_10 |GPIO_Pin_11;  //0,13 是背光和电源使能
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_ResetBits(GPIOC, GPIO_Pin_10);    //串口3在断电的情况输出低
	GPIO_ResetBits(GPIOC, GPIO_Pin_11);
	GPIO_ResetBits(GPIOC, GPIO_Pin_0);    //串口3在断电的情况输出低
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;  //
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	GPIO_SetBits(GPIOB, GPIO_Pin_2);    //故障指示灯，高电平熄灭
	
}
#endif



int32_t gpio_init_all(void)
{	
	//1.只有4组GPIO,16+16+16+3 = 51个
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   //IO复用功能模块
	/* enable the gpio clock */
	
	//2. 其他io引脚的初始化
	others_gpio_init();
	
	//3. 18个 led和按键的初始化
	btns_leds_init();
						
	return 0;
}



/*
	故障灯。
	参数： ledn 参考枚举类型
*/
void set_bug_led_on(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_2);
}

void set_bug_led_off(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_2);
}

uint8_t get_bug_led_status(void)
{
	return GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_2);
}

#if 0
//工作指示灯
static void set_work_led_on(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_1);
}

static void set_work_led_off(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_1);
}

static uint8_t get_work_led_status(void)
{
	return GPIO_ReadOutputDataBit(GPIOD, GPIO_Pin_1);
}
#endif


/*通过控制PB15来控制屏幕加热 -- 低-禁用，高-使能 */
static void heat_enable(void)
{
//	hot_en = 1;  //保存到全局状态中，开启加热
#ifdef 	LCD_PWM_HEAT
	LcdHeat_pwm_Enable();
#else	
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
#endif
}

static void heat_disable(void)
{
//	hot_en = 0;  //保存到全局状态中，开启加热
#ifdef 	LCD_PWM_HEAT
	LcdHeat_pwm_Disable();
#else		
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);
#endif
	//如果进入加热状态，由测温任务再次启动
//	lowtemp_ready_run = 0;
	set_bug_led_off();
}


uint8_t get_lcd_heat_status(void)
{
#ifdef LCD_PWM_HEAT	  //2022-0104   增加pwm的处理
	return g_lcdHeat_pwm;
#else		
	return GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_15);
#endif
}


/* 开启Lcd背光 */
static void set_LcdLight_enable(void)
{
	GPIO_SetBits(GPIOC, GPIO_Pin_0);
}


/* 关闭Lcd背光 */
static void set_LcdLight_disable(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_0);
}




#if 1
/*
	设置加热控制
*/
void set_heat_status(uint8_t enable)
{	
	if(enable == HOT_ENABLE)  
	{		
		heat_enable();
	}
	else//停止加热
	{
		heat_disable();
	}
}
#endif

/*
	返回加热允许状态
*/
//uint8_t get_heat_status(void)
//{
//	return hot_en ;
//}


/* 开启风扇，无调速功能 */
void set_fan_enable(void)
{
	if(cpu_run_status > LS3A_POWEROFF)   //没有开机风扇不开，2021-12-15
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_3);
		GPIO_SetBits(GPIOB, GPIO_Pin_4);
		g_fan_pwm = 100;  //用于返回到应用层
	}
}


/* 关闭风扇，无调速功能 */
void set_fan_disable(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);
	GPIO_ResetBits(GPIOB, GPIO_Pin_4);
	g_fan_pwm = 0;  //用于返回到应用层
}





static  uint8_t get_cpu_power_status(void)
{
	return GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13);
}



/* LS7A GPIO14 GPIO15 接单片机，(PC5,PC6)
 * GPIO14-低， GPIO15-低，则LS上电默认配置（pmon阶段） 
 * GPIO14-高， GPIO15-低，则开始关机动作 
 * GPIO14-低， GPIO15-高，则开始CPU重启动作 
 * GPIO14-高， GPIO15-高，则进入操作系统

2021-12-07
因为tim3端口复用，改引脚为PB8，PB9检测

*/
static system_run_status cpu_ctrl_get_cmd(void)
{
    int8_t gpio_ls7a[2];
    		
    gpio_ls7a[0] = (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == GPIO_LOW);
    gpio_ls7a[1] = (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == GPIO_LOW);    //为0，得到的值是1
    
	if(!gpio_ls7a[0] && gpio_ls7a[1])  //gpio14为高，gpio15为低
	{
		return LS3A_POWEROFF;
	}
	else if(gpio_ls7a[0] && !gpio_ls7a[1])
	{
		return LS3A_REBOOT;
	}
	else if(gpio_ls7a[0] && gpio_ls7a[1])    //两个都为低.
	{
		return LS3A_RUNNING;
	}
	else
	{
		if(RESET == get_cpu_power_status())   //没有上电的时候，就不进行以下判断了。2021-12-10
		{
			cpu_run_status = LS3A_POWEROFF;
			return LS3A_POWEROFF;
		}		
		return LS3A_RUN_OS;
	}
        
//	return 0;
}

/*通过控制PA7(PF7) 拉低-当作FPGA 切换DIV为龙芯信号 
 *通过控制PA7(PF7) 拉高-视频切换到其他输出 */
void dvi_switch_set(uint8_t dvi)
{
	if(dvi == DVI_LOONGSON)    //如果原来是本地，切换为dvi
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_7);
		send_dvi_change_to_cpu(DVI_OTHER);     //视频源被修改，上报给cpu
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_7);
		send_dvi_change_to_cpu(DVI_LOONGSON);
	}
}

#if 0
/*通过控制PA6(PF7) 拉低-为龙芯信号 
 *通过控制PA6(PF7) 拉高-DVI通道输出 
2021-12-08 增加该引脚
*/
static void FPGA_dvi_switch(uint8_t dvi)
{
	if(dvi == 0)    //如果原来是本地，切换为dvi
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_6);
	//	send_dvi_change_to_cpu(DVI_OTHER);     //视频源被修改，上报给cpu
	}
	else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_6);
	//	send_dvi_change_to_cpu(DVI_LOONGSON);
	}
}
#endif


/*通过获取PA7(PF7) 低-目前DIV为龙芯信号 
 *通过获取PA7(PF7) 高-目前DIV为其他输出 */
uint8_t dvi_switch_get(void)
{
	return GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_7)? DVI_OTHER:DVI_LOONGSON;
}



//获得看门狗的状态！！！
uint8_t get_watchdog_status(void)
{
	return 0;
}




extern uint8_t sys_reboot_stat;   //2022-0103 用于识别开机



/*通过控制PC13(PG10)来控制CPU关机 -- 低并保持-关机，高-不关机 */
//关机后，只有单片机在运行，其他任务关闭。
void cpu_poweroff_enable(void)
{
	int i = 0;
	
	//1.电源控制引脚拉低，停止给cpu供电。
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	
	//7. 系统的运行状态改为关机状态。
	cpu_run_status = LS3A_POWEROFF;
	
	//2.
	set_fan_disable();  //风扇不开
	set_heat_status(HOT_DISABLE);  //lcd不加热
	
	//3.设置所有的led熄灭。
	set_led_off(LEDALL);
	
	//4.led都关闭
	//set_work_led_off();
	set_bug_led_off();
	
	//5. lcd 背光关闭
	set_LcdLight_disable();
			
	usart3_deinit();	
	
	//8. 外部中断（视频切换）0关闭，串口中断关闭
	NVIC_DisableIRQ(EXTI0_IRQn);
	NVIC_DisableIRQ(USART3_IRQn);	
	
	//9. 判断是否是重启
	if(sys_reboot_stat) //系统要求重启   2022-0103 增加
	{
		delay_1ms(2000);   //等待两秒 //2022-01-06  v1.3
		
		cpu_poweron_enable();  //开机
		sys_reboot_stat = 0;
	}
}






/*通过控制PC13(PG10)来控制CPU关机 -- 低并保持-关机，高-不关机 */
void cpu_poweron_enable(void)
{		
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
//	cpu_run_status = LS3A_RUNNING;    //注释2022-01-04 v1.2
	delay_1ms(100);
	
	//1.开启定时0器	，LCD背光PWM开启
	TIM_Cmd(TIM3, ENABLE);	
	delay_1ms(100);
		
	//2.熄灭按键灯
	set_led_off(LEDALL);
		
	//3.开启lcd背光使能
	set_LcdLight_enable();
			
	//4. 开启使能
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(USART3_IRQn);
	
	//5.视频切换为龙芯本地
	dvi_switch_set(DVI_LOONGSON);
	
}



void led_record_reboot(void);


/*通过控制PC15(PI6)来控制CPU重启 -- 拉低持续160ms-重启，高-不重启 */
void cpu_reboot_enable(void)
{	
	led_record_reboot();
	mcu_reboot = 1;
	while(1);  //等待重启
	
//	GPIO_ResetBits(GPIOC, GPIO_Pin_13); //2021-12-02 改为13  GPIO_Pin_15
	cpu_poweroff_enable();
	set_led_on(LEDALL);   //按键灯点亮	
	IWDG_ReloadCounter();  //喂狗
	delay_1ms(200);		
	cpu_poweron_enable();
}






//外部中断1，对应电源按键
void exint1_handle(void)
{
	btn_press_num = 1;
}

//外部中断0，对应视频切换按键
void exint0_handle(void)
{
	btn_press_num = 2;
}


/*
	按键扫描
	扫描，10ms扫一次，30ms
	电源按钮 区分 短按和长按
*/
void btn_power_change_scan(void) // 10ms 调用一次
{   
	if(btn_press_num == 1)  //关机之后继续扫描
	{
		if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)== BTN_PRESS)
		{
			btn2[0].pressCnt++;
			if(btn2[0].pressCnt>500)
			{
				btn2[0].value = 1;      //长按
				cpu_run_status = cpu_ctrl_get_cmd();
				if(cpu_run_status > LS3A_POWEROFF)  //已经开机
				{
					mcu_reboot = 1;
					while(1);   //等待重启  2022-01-07  v1.3
//					__NVIC_SystemReset(); //单片机复位
				//	cpu_poweroff_enable();   //直接断电啦,2021-12-10改为单片机复位
				}
				if(btn2[0].pressCnt > 10000)
					btn2[0].pressCnt = 10000; //防止长按越界
			}
			else if(btn2[0].pressCnt>150) //1S以上，用于关机和开机
			{
				if(cpu_run_status == LS3A_RUN_OS)   //系统在运行
				{
					//1. 串口2发送指令，等待关机
					send_cmd_to_cpu(eMCU_SHUTDOWN_CMD,0);   //发送
				}
				btn2[0].value = 3; 				
			}
			else if(btn2[0].pressCnt>1)  //20ms 用于开机
			{
				btn2[0].value = 2; 				
			}
			else	
			{
				btn2[0].value = 0;
			}				
		}
		else{
			btn2[0].pressCnt = 0;   //松开
			if(btn2[0].value > 1) //检测到短按
			{
				if(RESET == get_cpu_power_status())   //系统已经关闭
					cpu_poweron_enable();  //开机				
			}			
			btn2[0].value = 0;
			btn_press_num = 0;
		}
	}
	else if(btn_press_num == 2)  //关机之后不再扫描,中断被关闭了
	{
		if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0)== BTN_PRESS)   //按下
		{
			btn2[1].pressCnt++;
			if(btn2[1].pressCnt>3)
			{
				btn2[1].value = 1; 
				if(btn2[1].pressCnt > 10000)
					btn2[1].pressCnt = 10000; //防止长按越界
			}	
		}
		else{ //松开
			btn2[1].pressCnt = 0;   //松开
			if(btn2[1].value == 1) //检测到短按
			{
				if(dvi_switch_get() == DVI_LOONGSON)    //最好要限制切换的速度 10s内不得多次切换
					dvi_switch_set(DVI_OTHER);
				else{
					dvi_switch_set(DVI_LOONGSON);
				}
			}
			btn2[1].value = 0;
			btn_press_num = 0;
		}
	}
}






//中断后调用
void exint8_9_handle(void)
{
	time_count = 20;
//	enter_system = 0;   //修改bug，进入中断，即取消进入系统的判断  2022-01-04 v1.2 增加
}


/*
	2021-12-09 改为中断方式
	任务3.10ms调用一次，检测系统运行状态
	2022-01-04 改为上电后开始执行
*/
void task_check_run_status(void)
{
	system_run_status cur_status;   //现在的状态
	static uint16_t times = 0;   //重启计时
		
//	if(RESET == get_cpu_power_status())   //系统没上电的
//	{	
//		if(time_count)
//			time_count = 0;
//		return;
//	}
	//2022-01-17 v1.5 更新软件看门狗重启策略
	if(cpu_run_status != LS3A_RUN_OS)   //系统上电后，没进入系统之前
	{
		if(times < 800)//8秒计时
		{
			times++;
		}
		else
		{
			//8秒没有进入系统，就会重启
			cpu_reboot_enable();
		}
	}
	else if(times)
		times = 0;
	
	//系统上电后，如果状态是关机，而定时器还是0，则需要查询io引脚，及时更新系统状态
	//2022-01-17 删除
//	if((cpu_run_status == LS3A_POWEROFF) && (time_count == 0)) //2022-01-04 v1.2 增加
//	{
//		time_count = 20;
//	}
		
	if(time_count)
	{
		time_count --;		
		if(time_count == 0)
		{
			cur_status = cpu_ctrl_get_cmd();    //改为局部变量保存 2022-01-04 v1.2 增加
			
			if(cpu_run_status == LS3A_POWEROFF) // 2022-01-04 v1.2 增加
			{
				if(cur_status == LS3A_RUNNING)  // 2022-01-04 v1.2 增加,开机进入pmon
				{	
					cpu_run_status = cur_status;
				//	time_count = 800;  //等待8s
				//	enter_system = 1;   //用于判断是否进入系统
				}
				else if(cur_status != LS3A_POWEROFF)  //当前状态不是关机 // 2022-01-04 v1.2 增加
				{
					cpu_run_status = LS3A_RUNNING;  //系统进入运行状态
					time_count = 20;   //再等待200ms，如果还是重启或关机，即执行关机或重启
				}
				//如果还是关机状态，则不修改系统状态 2022-01-04 v1.2 
			}
			else  //现在系统的状态不是关机了。// 2022-01-04 v1.2 增加
			{
//				if(enter_system)//8秒没有进入系统，就会重启
//				{
//					if(cur_status != LS3A_RUN_OS)  //10秒没有进入系统，就会重启
//						cpu_reboot_enable();
//					enter_system = 0;
//					return;
//				}
				
				if(cur_status == LS3A_REBOOT) // 获得的状态是重启
				{									 
					cpu_reboot_enable();							
				}
				else if(cur_status == LS3A_POWEROFF)  // 获得的状态是关机
				{
					mcu_reboot = 1;
					while(1);  //等待重启   //2022-01-06  v1.3
				//	cpu_poweroff_enable();  //这句其实没有意义了！！！！
	//				__NVIC_SystemReset(); //单片机复位									
				}
				else if(cur_status == LS3A_RUN_OS)  // 获得的状态是进入系统了
				{
				//	enter_system = 0;   //不用判断是否进入系统了 // 2022-01-04 v1.2 增加
					usart3_init_all(115200);   //2021-12-17,cpu系统启动时，开启串口
				}
//				else if(cur_status == LS3A_RUNNING)  // 获得的状态是进入pmon了
//				{	
//					time_count = 800;  //等待8s
//					enter_system = 1;   //用于判断是否进入系统
//				}
				cpu_run_status = cur_status;  //状态保存到全局中  //2022-01-04 v1.2 增加
			}			
		}
	}
}






#if 0
//2021-12-10 任务不再需要，工作灯因为引脚复用问题无法闪烁
//做一个任务，让工作led灯闪烁,1s调用一次
void task_idle_workled(void)
{
	//工作指示灯
//	static int8_t n = 0;
	
	//if(n++%2)
	if(get_work_led_status() == SET)
		set_work_led_on();	
	else
		set_work_led_off();
}
#endif



