

#include "includes.h"

/*
	2021-09-16    !!!!!!!!!!本文件只包括18个 按键和led的处理，其他GPIO的处理，请参考gpios.c
	
	1. 第二个移植的文件。
		
	
		按键的处理使用 外部中断 + 定时器扫描处理（消抖 30ms）
		按键1-16 外部中断12
		按键17，18 使用外部中断2，3
	
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

	除了18个按键之外，还有两个按键：
		2.1 视频切换按键，参考6.GPB0
		2.2 面板电源开关  GPB1 IO 面板电源开关

	除了18个led，还有一个故障指示灯，工作指示灯
		2.3 GPB2 IO 故障指示灯 
		2.4 GPD1 ： 工作指示灯
		
	3.PWM风扇，PB3，PB4 需要同时工作。 
	
	GPIOA:
	4. UART2 --> 7A1000,PA2,PA3
	5. UART1 --> FPGA PA9,PA10   ,做GPIO使用
		（其他没有使用，13，14做调试端口）
	GPIOB:
	6. GPB0 IO 面板视频切换开关
	7. GPB1 IO 面板电源开关
	8. GPB2 IO 故障指示灯               (led)
	9. GPB3，4  ---》参考3PWM风扇
	10. GPB6，7 I2C接口，温度读取
	11. GPB8，9 IO 
	12. GPB10，11，12 ---》参考2. 按键LED说明  i2c接口
	13. GPB15  IO 加热使能
		（其他没有使用）
	GPIOC：
	14. GPC0-3 ---》参考2. 按键LED说明
	15. GPC4，5，6 IO 连接7A1000，启动关机等控制
	16. GPC7，8 IO 看门狗，使能，输入
	17. GPC9  lvds PWM 屏幕亮度 tim3通道4
	18. GPC10，IO，lvds背光使能 
	19. GPC11，12 IO  预留INDVI
	20. GPC13 IO 系统上电
	21. GPC14  CPU复位
	22. GPC15  7A复位
	
	GPIOD：
	23. GPD1 ： 工作指示灯
	
	
	
	2021-09-29 调整
	18个按键，分别用两个扩展芯片（NCA9555 国产替代NXP PCA9555PW）完成的 （I2C2完成，PB10，11，pb12（中断）pb13（中断））
	18个led灯 也是两个扩展芯片
	
	
	!!!!!
	扩展芯片，按键松开和按下都会触发外部中断
*/




static BTN_INFO btn[BTN_MAX];
//LED_INFO led[LED_MAX];	
static uint16_t led1_18_status[2] = {0xffff,0xffff};    //静态保持led的状态，每个变量的前9位对应led，其他位无效		
//static uint16_t key1_18_status[2];   //两个的低9位，分别对应左边的9个按键和右边的9个按键，1表示按下，0表示松开
static uint8_t btn_press_num = 0;  //0表示没有按键被按下，1-18表示对应的按键被按下
uint8_t btn_start_scan = 0;   //0表示没有按键被按下，1表示1-9触发中断，10表示9-18按键触发中断
//uint8_t btn_start_ticks = 0;

static uint16_t left_keys;
static uint16_t right_keys;
static uint16_t left_keys_old;   //上一次的值
static uint16_t right_keys_old;  //保存上一次的值


#define LEFT_IIC_CONTROLER  I2C2   //对应外部中断12
#define RIGHT_IIC_CONTROLER  I2C1   //对应外部中断13  2021-12-07


uint8_t sys_reboot_stat = 0;   //记录系统是重启还是关机，1表示重启，0表示关机
void led_record_poweroff(void);

//按键信息的初始化，是对数据结构的初始化
int32_t init_btn_info(void)
{
    uint32_t i;
    
    for(i=0; i<BTN_MAX; i++)
    {
        memset((void *)&btn[i], 0, sizeof(BTN_INFO));
        if(i<BTN9)
			btn[i].code = 0xf0 - i;   //因为按键是倒的，所以重新编码 //2021-12-13
		else
			btn[i].code = 0xf9 - i + 9;  //因为按键是倒的，所以重新编码 //2021-12-13
		btn[i].io = BTN1 + i;
    }
    
    return 0;
}


/*
	按键和led的初始化部分
*/
void btns_leds_init(void)
{
	uint8_t config;	
	uint8_t outcfg_dat[2]={0,0};   //IIC芯片GPIO输出模式，对应的位要设置为0
	
	//iic控制器初始化
	nca9555_init();   //扩展按键和led的控制器初始化，这个部分会对应4块9555芯片，led两片，btn两片
	
	
	//2022-01-03增加，用于判断重启还是关机
	if((0 == nca9555_read_2outport(LEFT_IIC_CONTROLER,IIC_LEDS_ADDR,outcfg_dat)) || 
		(0 == nca9555_read_2outport(LEFT_IIC_CONTROLER,IIC_LEDS_ADDR,outcfg_dat)))
	{
		config = outcfg_dat[1] >> 4;   //只保留高4位
		if(config == 5)  //是重启
		{
			sys_reboot_stat = 1;
		}
		else
			sys_reboot_stat = 0;
	}
	else
		sys_reboot_stat = 0;
	outcfg_dat[0] = 0;
	outcfg_dat[1] = 0;
	//0103增加结束。
	
	
	//配置leds1-16的输出模式(配置寄存器1表示输入(默认)，0表示输出模式)，输出数据寄存器默认为高，led熄灭
	nca9555_write_2config(LEFT_IIC_CONTROLER,IIC_LEDS_ADDR,outcfg_dat);   //每次配置16个引脚，默认引脚为输入模式
	nca9555_write_2config(RIGHT_IIC_CONTROLER,IIC_LEDS_ADDR,outcfg_dat);
	
	led_record_poweroff();//0103增加。消除记录
	
	//按键的，默认是输入，并且被上拉为高。省略就不设置了。
	outcfg_dat[0] = 0xff;
	outcfg_dat[1] = 0x1;
	nca9555_write_2config(LEFT_IIC_CONTROLER,IIC_BTNS_ADDR,outcfg_dat);
	nca9555_write_2config(RIGHT_IIC_CONTROLER,IIC_BTNS_ADDR,outcfg_dat);
	
	//数据结构初始化
	init_btn_info();
	
	//设置所有的led熄灭，主要也是改变全局变量的值，让其与led对应。
	//set_led_on(LEDALL);
	
//    init_led_info();
	
}


//用按键记录复位状态！！！
void led_record_reboot(void)
{
	uint8_t outcfg_dat[2]={0xff,0x5f};   //led全部熄灭，高4位记录重启模式
	nca9555_write_2outport(LEFT_IIC_CONTROLER,IIC_LEDS_ADDR,outcfg_dat);   //每次配置16个引脚，默认引脚为输入模式
	nca9555_write_2outport(RIGHT_IIC_CONTROLER,IIC_LEDS_ADDR,outcfg_dat);
}


//关机状态。
void led_record_poweroff(void)
{
	uint8_t outcfg_dat[2]={0xff,0xff};   //led全部熄灭，高4位记录重启模式
	nca9555_write_2outport(LEFT_IIC_CONTROLER,IIC_LEDS_ADDR,outcfg_dat);   //每次配置16个引脚，默认引脚为输入模式
	nca9555_write_2outport(RIGHT_IIC_CONTROLER,IIC_LEDS_ADDR,outcfg_dat);
}


/*
	18个led同步设置,

	参数：
		ledn 表示设置对应的芯片，对应LED1-LED8表示第一个芯片 ，其他表示第二个芯片
		dat 包含两个字节，对应9个led

	每一位对应一个led
	每一个芯片控制其中9个led，0-8位有效
*/
uint8_t led1_9_set(void) //const uint8_t* dat
{	
//	led1_18_status[0] = dat[1]<<8 | dat[0];   //保存到全局中
	send_leds_status_to_cpu(((led1_18_status[1]&0x1ff)<<9) | (led1_18_status[0]&0x1ff));
	return nca9555_write_2outport(LEFT_IIC_CONTROLER,IIC_LEDS_ADDR,(uint8_t*)&led1_18_status[0]);
}


uint8_t led10_18_set(void) //const uint8_t* dat
{	
//	led1_18_status[1] = dat[1]<<8 | dat[0];   //保存到全局中
	send_leds_status_to_cpu(((led1_18_status[1]&0x1ff)<<9) | (led1_18_status[0]&0x1ff));
	return nca9555_write_2outport(RIGHT_IIC_CONTROLER,IIC_LEDS_ADDR,(uint8_t*)&led1_18_status[1]);
}

#if 0
//设置
static uint8_t led1_18_set(uint8_t ledn ,uint8_t* dat)
{	
	uint8_t ret = 0;

	if(ledn == LEDALL)
	{}
	else if(ledn < LED10)   //1-9是这个地址
	{
		led1_18_status[0] = dat[1]<<8 | dat[0];   //保存到全局中
		ret = nca9555_write_2outport(LEFT_IIC_CONTROLER,IIC_LEDS_ADDR,dat);
	}
	else if(ledn < LED_MAX)  //10-18是这个地址
	{
		led1_18_status[1] = dat[1]<<8 | dat[0];   //保存到全局中
		ret = nca9555_write_2outport(RIGHT_IIC_CONTROLER,IIC_LEDS_ADDR,dat);
	}
		
	return ret;
}
#endif

//通过参数返回led的状态
//这个函数获得所有led的状态
void led1_18_get(uint16_t *dat)
{		
	dat[0] = led1_18_status[0]; 	 
	dat[1] = led1_18_status[1];   //记录在全局变量中
	return ;
}


void led1_18_Set(uint32_t dat)
{
	led1_18_status[0] = (dat & 0x1ff);
	led1_9_set();
	led1_18_status[1] = ((dat>>9)& 0x1ff);
	led10_18_set();
}



/*
	参数： ledn 参考枚举类型
	状态的改变全部在全局变量中。没有使用中间变量了
		低电平点亮，所以点亮的设置为0
*/
uint8_t  set_led_on(LED_NUM ledn)
{	
	uint8_t ret = 0;
	
	if(ledn < 0 || ledn >= LED_MAX )  //参数越界
		return 255;

	if(LED1 <= ledn && ledn <= LED9)
	{
		led1_18_status[0] &= ~(1<<(LED9-ledn));    //状态保存到全局变量中,//2021-12-13,键盘是倒的，编码修改
		return led1_9_set();
	}
	else if(LED10 <= ledn && ledn <= LED18)
	{
		led1_18_status[1] &= ~(1<<(LED18 - ledn)); //2021-12-13,键盘是倒的，编码修改
		return led10_18_set();
	}
	else  //全部点亮的情况
	{
		led1_18_status[0] = (LED_ON<<8)| LED_ON;
		led1_18_status[1] = (LED_ON<<8)| LED_ON;
		ret = led1_9_set();   //1-9全部熄灭了
		ret += led10_18_set();   //10-18全部熄灭了
	}
	
	return ret;   //返回值不为0表示出错
}



/*
	参数：
	ledn： 1-18 表示led1-18
			0 和其他 表示所有的led		
*/	
uint8_t  set_led_off(LED_NUM ledn)
{
	uint8_t ret = 0;
	
	if(ledn < 0 || ledn >= LED_MAX )  //参数越界
		return 255;

	if(LED1 <= ledn && ledn <= LED9)
	{
		led1_18_status[0] |= (1<<(LED9-ledn));    //状态保存到全局变量中//2021-12-13,键盘是倒的，编码修改
		return led1_9_set();
	}
	else if(LED10 <= ledn && ledn <= LED18)
	{
		led1_18_status[1] |= (1<<(LED18 - ledn));//2021-12-13,键盘是倒的，编码修改
		return led10_18_set();
	}
	else
	{
		led1_18_status[0] = ~LED_ON;
		led1_18_status[1] = ~LED_ON;
		ret = led1_9_set();   //1-9全部熄灭了
		ret += led10_18_set();   //10-18全部熄灭了
	}
	
	return ret;
}

//void gd32_led_toggle(uint32_t led)
//{
//    gd32_gpio_toggle(&gpio_config[led_index_of_gpio[led]]);
//}


/*
	获取某一个LED的状态。只能获取单个LED的状态
	返回值：	0 表示led点亮
				1 表示led熄灭
				255 表示超出范围
*/
uint8_t get_led_value(LED_NUM ledn)
{
	if((ledn < LED1) || (ledn > LED18))   //参数
		return 255;

    if(LED1 <= ledn && ledn <= LED9)
	{
		return led1_18_status[0] & (1<<(ledn-LED1));    //状态保存到全局变量中		
	}
	else if(LED10 <= ledn && ledn <= LED18)
	{
		return led1_18_status[1] & (1<<(ledn-LED9));		
	}
		
	return 0;
}




/*
	按键1-18的扫描
	中断触发扫描，10ms扫一次，30ms
*/
//void btn_scan(BUTTON_NUM n) // 10ms 调用一次
//{    
//	if(n > BTN18)   //参数检查
//		return;
//	
//	btn1_18_scan(n);
//}





/*
	函数调用，按键处理任务
		通过串口上报到3A3000
*/
static void btn_handle(void)
{
	if(btn_press_num)
	{
		if(btn[btn_press_num-1].reportEn == 1)
		{
			btn[btn_press_num-1].reportEn = 0;
			
			//只要按键改变了，就要通知cpu！！！！！！
			send_btn_change_to_cpu(btn[btn_press_num-1].code,btn[btn_press_num-1].value?1:0);
		}
		btn_press_num = 0;    //处理结束后清零
	}
}



/*
	查询按键的状态
	返回值：
		255是出错，0表示松开 1表示按下
*/
uint8_t btns_query(BUTTON_NUM n)
{
	if((n < 0) || (n > BTN18))   //参数
		return 255;
	
	return btn[n].value;
}






/*
	按键1-9的扫描
	中断触发扫描，10ms扫一次，30ms
	
	返回值：
		0 表示有按下
		1 表示松开
        254,255 表示出错
*/
static uint8_t btn1_18_scan(uint8_t btns) // 10ms 调用一次
{
    uint8_t i,ret = 0;
	uint16_t dat;
//	uint8_t btn_addr = IIC_BTNS_ADDR;
	uint8_t btn_index = 0;    //设置遍历数组的起始位置
		
	if(btns > BTN18)   //参数检查
		return 254;
	
	if(btns < BTN10)    //btn1-9 是一个芯片
	{
		if(nca9555_read_2inport(LEFT_IIC_CONTROLER,IIC_BTNS_ADDR,(uint8_t*)&dat))   //非0表示失败
			return 255;
		btn_index = 0;
		left_keys = dat & 0x1ff;
		
		if((left_keys != left_keys_old))
		{
			left_keys_old = left_keys & 0x1ff;  //把值保存起来
			//left_keys_old[1] = left_keys[1];
			
			led1_18_status[0] = left_keys_old;				
			led1_9_set();
		}	
	}
	else
	{
		if(nca9555_read_2inport(RIGHT_IIC_CONTROLER,IIC_BTNS_ADDR,(uint8_t*)&dat))   //非0表示失败
			return 255;
		btn_index = 9;
//		right_keys[0] = dat[0];
//		right_keys[1] = dat[1];
		right_keys = dat & 0x1ff;
		
		if((right_keys != right_keys_old))
		{
			right_keys_old = right_keys & 0x1ff;  //把值保存起来
			led1_18_status[1] = right_keys_old;
			led10_18_set();
		}
	}
		    
    for(i=0; i<9; i++)
    {
		if((dat & (1<<i)) == BTN_PRESS)   //按下是某一位为0
		{
			btn[i+btn_index].pressCnt++;
            if((btn[i+btn_index].pressCnt >= 3) && (btn[i+btn_index].value == 0))   //之前是松开的
            {
                btn[i+btn_index].value = 1;  //被按下了
                btn[i+btn_index].reportEn = 1;   //按下有效
				btn_press_num = i+btn_index+1;    //记录被按下的按键 1-18，加1的原因是去掉0
								
				return 0;   //松开被按下检测到。
            }
			else
				return 1;  //按下的时间不够，或者重复检测到
		}
		else //松开的
        {
            btn[i+btn_index].pressCnt = 0;
            if(btn[i+btn_index].value == 1)   //之前是按下的
            {
                btn[i+btn_index].value = 0;    //松开了
                btn[i+btn_index].reportEn = 1;  //松开有效
				btn_press_num = i+btn_index+1;    //记录被松开的按键
				return 0;    //按下后松开被检测到
            }
			else  //松开已经被检测到了，或者是抖动
				ret = 253;
        }
    }
	return ret;
}





#ifdef BTNS_USE_INT    //使用中断方式 宏在btns_leds.h中定义

/*
main函数周期扫描任务，10ms一次，由中断触发设置btn_start_scan进入，2021-12-07，不产生中断，暂时不用了

	btn_start_scan 由中断函数设置，按下和松开都会设置为非0
	抖动触发。
*/
void task1_btn_scan(void)
{
//	static uint8_t scan_num = 0;   //扫描计数，用于消抖
	uint8_t ret;
	
//	return;
	
	if(btn_start_scan) //外部（按下和松开都会触发）中断触发后，不为0.
	{
	//	btn_start_scan = 1;
		//scan_num ++;
		ret = btn1_18_scan(btn_start_scan);   //开始按键扫描
		if(ret == 0)  //检测到有效值了（按下或者松开）
		{
			btn_start_scan = 0;
			//scan_num = 0;
			btn_handle();  //有按键被按下或松开的处理
		}
		else if(ret > 250)  //返回值,出错了，不再扫描
		{
		//	MY_DEBUG("ERROR:task1_btn_scan ret = %d\n",ret);   //调试串口打印
			btn_start_scan = 0;
			//scan_num = 0;
		}		
	}
	else
		;//scan_num = 0;     //可以省略吧？？
}

#else

/*
	main函数周期扫描任务，10ms一次,查询方式

	btn_start_scan 由中断函数设置，按下和松开都会设置为非0
	抖动触发。
*/
void task1_btn_scan(void)
{
//	static uint8_t scan_num = 0;   //扫描计数，用于消抖
	uint8_t ret;

	
	if(btn_start_scan == 0)
		btn_start_scan =1;   //开始扫描
	
	if(btn_start_scan) //外部（按下和松开都会触发）中断触发后，不为0.
	{
	//	btn_start_scan = 1;
		//scan_num ++;
		ret = btn1_18_scan(btn_start_scan);   //开始按键扫描
		if(ret == 0)  //检测到有效值了（按下或者松开）
		{
		//	btn_start_scan = 0;
			if(btn_start_scan == 10)
				btn_start_scan =1;   //开始扫描左边
			else	
				btn_start_scan =10;   //开始扫描右边
			//scan_num = 0;
			btn_handle();  //有按键被按下或松开的处理
		}
		else if(ret > 250)  //返回值,出错了，不再扫描
		{
			if(btn_start_scan == 10)
				btn_start_scan =1;   //开始扫描左边
			else	
				btn_start_scan =10;   //开始扫描右边
		}
		
	}
	//else
		;//scan_num = 0;     //可以省略吧？？
}
#endif


