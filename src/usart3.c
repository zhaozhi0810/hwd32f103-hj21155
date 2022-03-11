/*
	2021-09-28
	
	第十三个移植的文件。
		
	中断处理函数统一放在stm32f103_it.c中
	
	2021-09-28  stm32 3个串口都打开使用
	uart1-调试使用
	uart2/uart3-连接3A3000，一个用于上传温度电压等信息，一个用于上传18个按键的信息，以及18个led的控制部分
	
	usart2 -> 连接3A3000，主要是用于上传18个按键的信息，以及18个led的控制部分
	
	
	帧头：
		3A3000 -> 单片机 A5   //单字节
		单片机 -> 3A3000 A5   //单字节
	
	帧长：
		共7个字节
		帧头1+类型1+数据4+校验和1
	
	类型：
		发送的数据
		1.定时上报，3个电压（核电，7A，12V）,cpu温度，主板温度，lcd温度，风扇pwm
		2.触发上报，18个按键，视频切换按键，lcd亮度，lcd加热状态（被修改即上报），故障灯状态，关机（重启）命令
				
		接收的数据：
		3.设置（cpu端设置mcu），视频切换，18个led状态，故障灯状态
		4.

*/


#include "includes.h"
//#include "usart3.h"


#define RECV_BUF_LEN 64
static Queue_UART_STRUCT Queue_UART3_Recv;   //接收队列，用于接收中断
//static uint8_t com3_recv_buf[RECV_BUF_LEN];
//static uint8_t com3_recv_len = 0;  //接收数据的长度
static uint8_t com3_data[8];   //接收缓存
uint8_t uart_inited = 0;   // 0表示没有初始化，1表示初始化了。



#define UART_BUFF_FULL_COVER
//队列插入数据
int32_t QueueUARTDataInsert(Queue_UART_STRUCT *Queue,uint8_t data)
{
	if(MAX_QUEUE_UART_LEN == Queue->length)
	{
#ifdef UART_BUFF_FULL_COVER
		Queue->sendIndex = (Queue->sendIndex + 1) % MAX_QUEUE_UART_LEN;
		Queue->length--;
#else
		return -1;
#endif
	}
	
	Queue->dataBuf[Queue->recvIndex] = data;
	
	Queue->recvIndex = (Queue->recvIndex + 1) % MAX_QUEUE_UART_LEN;
	
	Queue->length++;
	
	return 0;
}

int32_t QueueUARTDataDele(Queue_UART_STRUCT *Queue,uint8_t *data)
{
	if(0 == Queue->length)
		return -1;
	
	*data = Queue->dataBuf[Queue->sendIndex];
	
	Queue->sendIndex = (Queue->sendIndex + 1) % MAX_QUEUE_UART_LEN;
	
	Queue->length--;
	
	return 0;
}

void QueueUARTDataIndexRecover(Queue_UART_STRUCT *Queue)
{
	Queue->sendIndex = (Queue->sendIndex - 1) % MAX_QUEUE_UART_LEN;
	
	Queue->length++;
}

uint32_t QueueUARTDataLenGet(Queue_UART_STRUCT *Queue)
{
	return Queue->length;
}

uint8_t checksum(uint8_t *buf, uint8_t len)
{
	uint8_t sum;
	uint8_t i;

	for(i=0,sum=0; i<len; i++)
		sum += buf[i];

	return sum;
}



//校验数据
int32_t verify_data(uint8_t *data,uint8_t len)
{
	uint8_t check;
	int32_t ret = -1;
	
	if(data == NULL)
		return -1;
	
	//读取原数据中的校验值
	check = data[len - 1];
	
	//重新计算校验值
	if(check==checksum(data,len - 1))
		ret = 0;
	
	return ret;
}





/*!
    \brief      configure COM port
    \param[in]  
      \arg        bandrate: 波特率
    \param[out] none
    \retval     none
*/
static void usart3_init(uint32_t bandrate)
{    
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	//1.时钟使能，（不使能，外部设备不能使用）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);    //PC10，11
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   //IO复用功能模块
	
	//2.配置IO端口的模式（复用功能）
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 ;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; //发送引脚
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; //接收引脚
	GPIO_Init(GPIOC, &GPIO_InitStruct);	
	
	//GPIO_SetBits(GPIOC, GPIO_Pin_11);   //拉高
	
	//2.1 端口重映射功能
	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);   //默认是PB10，PB11，部分映射是PC10，PC11
	
		
	//3.初始化串口控制器
	USART_StructInit(& USART_InitStruct);  //初始化结构体
	USART_InitStruct.USART_BaudRate = bandrate;
	USART_Init(USART3, &USART_InitStruct);

	//4.开启串口控制器
	USART_Cmd(USART3, ENABLE);
	
	//5.开启中断允许
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);   //接收数据中断
//	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);   //接收空闲中断
	
	//6.设置nvic，允许中断
	NVIC_InitStruct.NVIC_IRQChannel= USART3_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;    //使能
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);

	uart_inited = 1; //已经初始化了。

}

//uart3去初始化，两个引脚输出低电平
void usart3_deinit(void)
{    
	GPIO_InitTypeDef GPIO_InitStruct;
//	USART_InitTypeDef USART_InitStruct;
//	NVIC_InitTypeDef NVIC_InitStruct;
		
	//2.配置IO端口的模式（复用功能）
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 |GPIO_Pin_11;  //0,13 是背光和电源使能
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_ResetBits(GPIOC, GPIO_Pin_10);    //串口3在断电的情况输出低
	GPIO_ResetBits(GPIOC, GPIO_Pin_11);

	//2.1 端口重映射功能
	//GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, DISABLE);   //默认是PB10，PB11，部分映射是PC10，PC11
		
	//3.去初始化串口控制器
	USART_DeInit(USART3);
	//4.开启串口控制器
	USART_Cmd(USART3, DISABLE);
	
	//5.开启中断允许
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);   //接收数据中断
	USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);   //接收空闲中断
	
	//6.设置nvic，允许中断
	NVIC_DisableIRQ(USART3_IRQn);

	uart_inited = 0; //已经初始化了。
}



/*
	串口3的初始化
*/
void  usart3_init_all(uint32_t bandrate)
{
	memset((void *)&Queue_UART3_Recv, 0, sizeof(Queue_UART_STRUCT));

	usart3_init(bandrate);   //UART3 跟LS7A uart1连接
}



static void UART3_TX(uint8_t ch)
{
	if(uart_inited)   //cpu断电之后没有初始化
	{
		while(RESET == USART_GetFlagStatus(USART3, USART_FLAG_TXE))  //等待前面的数据发送结束
			;
		USART_SendData(USART3, ch); 
	}	
}


static void UART3_TX_STRING(uint8_t *str, uint8_t length)
{
	uint8_t i;
	
	for(i = 0; i < length; i++)
		UART3_TX(str[i]);
}



//单片机发送数据给cpu，不由单独的线程处理了。data只需要包含数据类型和数据。头部和crc由该函数完成。
/*
 * data 用于发送的数据，不需要包括帧头和校验和，只要包括数据类型和数据（共5个字节）
 * 返回值
 * 	0表示成功，其他表示失败
 * */
int send_mcu_data_tocpu(const void* data)
{	
	unsigned char buf[8];  	
	
	if(!uart_inited)
		return -1;
	
	buf[0] = FRAME_HEAD;  //帧头	
	memcpy(buf+1,data,sizeof(com_frame_t)-1);    //拷贝

	//crc重新计算
	buf[sizeof(com_frame_t)] = checksum(buf,sizeof(com_frame_t));  //校验和，存储在第7个字节上，数组下标6.
	
	UART3_TX_STRING(buf, sizeof(com_frame_t)+1);   //com_frame_t并没有包含数据头，所以加1个字节	

	//发送成功，等待应答
	return 0;   //暂时没有等待应答2021-11-23

}



//发送命令数据到cpu
//cmd请参考usart3.h中宏定义
//param 参数。
void send_cmd_to_cpu(int cmd,int param)
{
	com_frame_t data;
	data.data_type = eMCU_CMD_TYPE;
	data.data.cmd.cmd = cmd;
	data.data.cmd.param_len = 1;     //带一个参数
	data.data.cmd.param1 = param;   //带一个参数
	send_mcu_data_tocpu(&data);
}

//发送dvi视频被切换的数据到cpu
//source 1（本地）或者2（外部）
void send_dvi_change_to_cpu(int source)
{
	com_frame_t data;
	data.data_type = eMCU_DIV_CHANGE_TYPE;
    data.data.fan_div_bk.bstatus.dvi_src = source-1;
	send_mcu_data_tocpu(&data);
}

//发送dvi视频被切换的数据到cpu
//whichkey 0xe8 - 0xf9
//status 0 or 1 松开或者按下
void send_btn_change_to_cpu(int whichkey,int status)
{
	com_frame_t data;
	data.data_type = eMCU_KEY_CHANGE_TYPE;    
	data.data.key_change.whichkey =whichkey;
	data.data.key_change.status = status;	
	send_mcu_data_tocpu(&data);
}

//发送led状态的数据到cpu
//status表示18个led，(低18位有效)
void send_leds_status_to_cpu(int status)
{
	com_frame_t data;
	data.data_type = eMCU_LED_STATUS_TYPE;
	data.data.leds_status = status & 0x3ffff;
	send_mcu_data_tocpu(&data);
}

//发送主板和cpu温度
void send_bc_temp_to_cpu(short cpu_temp,short board_temp)
{
	com_frame_t data;
	data.data_type = eMCU_CB_TEMP_TYPE;
	data.data.cb_temp.cpu_t = cpu_temp;
	data.data.cb_temp.board_t = board_temp;
	send_mcu_data_tocpu(&data);
}

//发送3个电压
void send_vol_to_cpu(short vol_core,short vol_7A,short vol_12v)
{
	com_frame_t data;
	
	data.data_type = eMCU_VOL_TYPE;
	data.data.vol.vol_core = vol_core;
	data.data.vol.vol_7A = vol_7A;
	data.data.vol.vol_12V = vol_12v;

	send_mcu_data_tocpu(&data);
}

//发送lcd的温度，pwm，加热使能状态
void send_lcd_temp_to_cpu(short lcd_t1,short lcd_t2)
{
	com_frame_t data;
	
	data.data_type = eMCU_LCD_TEMP_TYPE;
	data.data.lcd.lcd_temp1 = lcd_t1;  //加热状态
	data.data.lcd.lcd_temp2 = lcd_t2;    //lcdpwm

	send_mcu_data_tocpu(&data);
}



//2021-12-15调整
//定时发送风扇状态，dvi和故障灯状态
void send_fan_bk_div_status_to_cpu(bitstatus_t b_status,uint8_t fan_pwm,uint8_t lcd_pwm)
{
	com_frame_t data;
	
	data.data_type = eMCU_FAN_DIV_BK_TYPE;
	data.data.fan_div_bk.bstatus = b_status;  //故障灯状态
	data.data.fan_div_bk.fan_pwm = fan_pwm;    //风扇pwm值
	data.data.fan_div_bk.lcd_pwm = lcd_pwm;    //lcdpwm值

	send_mcu_data_tocpu(&data);
}



static uint8_t head_recv = 0;  //等于0表示没有接收到帧头，1，表示收到帧头1，2表示收到帧头2,3表示收到第一个数据。。。。

/*
	串口数据接收中断：
		前提：每一帧都是7个字节。
		队列中没有保存帧头，只有后面的数据和校验和（共6个字节）
*/
void com3_rne_int_handle(void)
{
	uint8_t dat;
	dat = (uint8_t)USART_ReceiveData(USART3);   //接收就存到队列中！！！！！2021-12-02

	
	if(head_recv == 0)  //没有收到帧头
	{		
		if(dat == FRAME_HEAD)    //3A发送的帧头是A5
		{
			QueueUARTDataInsert(&Queue_UART3_Recv,dat);   //接收的数据存入队列中。
			head_recv = 1;    //只有一个帧头了！！！！！2021-12-01
		}
	}
	else if(head_recv == 1)   //考虑到数据乱序，就不再寻找
	{
		QueueUARTDataInsert(&Queue_UART3_Recv,dat);   //接收的数据存入队列中。
//		head_recv++;   //收到的字节数增加
//		if(head_recv >= UART3_PROTOCOL_FRAME_LENGTH)  //一帧已经接收完了，验证下一帧
//		{
//			head_recv = 0;
//		}
	}
//	else 
//		head_recv = 0;   //没有收到帧头，数据不保存
}


/*
	串口收到命令后的处理。com3_idle_int_handle中断处理函数调用
		前提： 收到完整的数据包，校验和正确。


	单片机能够收到的命令：
	1.设置视频源
	2.设置18个led灯状态
	3.设置故障灯状态
	4.设置lcd的pwm（亮度）
	4.关机或重启命令。

*/

static void com3_message_handle(void)
{	
	com_frame_t* pdata = (com_frame_t*)(com3_data+1);    //+1是跳过帧头，使用结构体初始化
	int t;
//	uint8_t dat[2];
	
	switch(pdata->data_type)
    {
        case eMCU_LED_SETON_TYPE:  //设置led开
        //        printf("led = %#x\n",data);   //调试信息
            t = pdata->data.leds_status;  //t中读取到是哪一个(1-18数字表示)led被设置，0表示全部
            //    leds_control(t,1);
			set_led_on(t);
        break;
        case eMCU_LED_SETOFF_TYPE:   //设置led关
            t = pdata->data.leds_status;  //t中读取到是哪一个(1-18数字表示)led被设置，0表示全部
        //    leds_control(t,0);
			set_led_off(t);
            break;

        case   eMCU_LEDSETALL_TYPE:    //设置所有的led
            t = pdata->data.leds_status;   //t表示所有的led的开关情况
			led1_18_Set(t);
        break;
        case eMCU_DVI_SETSRC_TYPE:   //设置视频源
            t = pdata->data.fan_div_bk.bstatus.dvi_src;
            if(t)
				dvi_switch_set(DVI_OTHER);   //设置后会上报给cpu
            else
				dvi_switch_set(DVI_LOONGSON);   //本地视频
            break;
//        case eMCU_CB_TEMP_TYPE:   
        //    pdata->data.cb_temp = *(cb_temp_t*)&data;
                //pdata->data.cb_temp.cpu_t;
                //pdata->data.cb_temp.board_t;
//        break;
        case eMCU_BKLEDSET_TYPE:    //设置故障灯的命令
            t = pdata->data.fan_div_bk.bstatus.breakdownLed_status;  //t中读到的是故障灯的开关情况
            if(t)
				set_bug_led_on();
            else
				set_bug_led_off();
        break;
        case eMCU_LCD_SETPWM_TYPE:    //设置屏幕亮度
            t = pdata->data.fan_div_bk.lcd_pwm;
            if(t>=0 && t <101)
            {
				Lcd_pwm_out(t);
            }
        break;		
        case eMCU_CMD_TYPE:    //单片机与计算机有一些命令处理！！
            t = pdata->data.cmd.cmd;
            switch(t)
            {
                case eMCU_ANSWER_CMD:
                //    if(pdata->data.cmd.param_len)
                //		return pdata->data.cmd.param1;
                //	return -1;   //应答错误
                break;
                case eMCU_REBOOT_CMD:   //计算机重启命令
                break;
                case eMCU_SHUTDOWN_CMD:  //计算机关机命令
                break;
                case eMCU_SWITCH_DVI_SRC_CMD:  ////计算机切换视频源命令
                break;
				case eMCU_LCD_SETPWM_UP_CMD:    //设置屏幕亮度
					t = g_lcd_pwm + 10;
					if(t >100)
					{
						t = 100;
					}
					Lcd_pwm_out(t);
				break;
				case eMCU_LCD_SETPWM_DN_CMD:    //设置屏幕亮度
					t = g_lcd_pwm - 10;
					if(t < 0)
					{
						t = 0;
					}
					Lcd_pwm_out(t);
				break;
                default:
                break;
            }

        break;
        default:
        break;
    }
}










/*
	串口空闲中断的处理

	1.判断接收到的字节数，>=7 表示正常
	2.正常就继续处理，读出7个字节，计算校验和，
	3.校验和正确，则处理命令

*/

#define FRAME_LENGHT (sizeof(com_frame_t)+1)    //数据帧的字节数

void com3_frame_handle(void)
{
//	int32_t err;
	uint8_t length,i,j;
	static uint8_t datalen =FRAME_LENGHT,offset = 0;  //这两个变量用于帧错误的情况
	
	while(1)  //考虑到可能接收到多帧要处理的情况
	{		
		length = QueueUARTDataLenGet(&Queue_UART3_Recv);	
		if(length < datalen)   //（包括帧头）小于7个字节，不是完整的一帧
		{	
			//com3_answer_3A3000(RECV_NO);   //应答接收到的长度不对
			return ;   //继续等待
		}	
		length = datalen;   //计算需要读出的字节数
		for(i=0;i<length;i++)
		{
			//这里不判断错误了，前面已经检测确实有这么多字节。
			QueueUARTDataDele(&Queue_UART3_Recv,com3_data+i+offset) ;  //com_data 空出1个字节，为了兼容之前的校验和算法，及数据解析算法
		}
	//	com3_data[0] = FRAME_HEAD;  //加入帧头进行校验和计算
		if((com3_data[0] == FRAME_HEAD) && (0 == verify_data(com3_data,FRAME_LENGHT)))   //第二参数是数据总长度，包括校验和共7个字节
		{
			//校验和正确。
			//com3_answer_3A3000(RECV_OK);
			com3_message_handle();		
		}	
		else  //校验和不正确，可能是帧有错误。
		{
			for(i=1;i<FRAME_LENGHT;i++)   //前面的判断出问题，考虑是帧错误，寻找下一个帧头！！！
			{
				if(com3_data[i] == FRAME_HEAD)   //中间找到一个帧头
				{
					break;
				}
			}		
			if(i != FRAME_LENGHT) //在数据中间找到帧头！！！
			{
				datalen = i;   //下一次需要读的字节数
				offset = FRAME_LENGHT-i;  //存储的偏移位置的计算

				for(j=0;i<FRAME_LENGHT;i++,j++)   //有可能帧头不对，所以第一个字节还是要拷贝一下
				{
					com3_data[j] = com3_data[i];   //把剩下的拷贝过去
				}
			}
			else  //在数据中间没有找到帧头
			{
				datalen = FRAME_LENGHT;  //	下一次需要读的字节数
				offset = 0;
			}
		}	
	}//end while 1
}


