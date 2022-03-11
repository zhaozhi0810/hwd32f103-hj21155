/*
	2021-09-28
	
	第十二个移植的文件。
		
	中断处理函数统一放在stm32f103_it.c中
	
	2021-09-28  stm32 3个串口都打开使用
	uart1-调试使用
	uart2/uart3-连接3A3000，一个用于上传温度电压等信息，一个用于上传18个按键的信息，以及18个led的控制部分
	
	usart1 -> 连接pc，主要是用于调试
	
	
	2021-12-01.
	串口1无外部连接！！目前全部注释
	
*/


#if 0 //2021-12-01

#include "includes.h"
#include <stdio.h>

#ifdef  DEBUG

static uint8_t txe_enable = 0 ;  //txe中断开启？ 0表示没有开启，1表示开启


static Queue_UART_STRUCT Queue_UART1_Recv;
#ifdef UART1_SEND_IRQ
static Queue_UART_STRUCT Queue_UART1_Send;   //发送
#endif
static uint8_t com1_data[8];   //接收缓存


/*!
    \brief      configure COM port
    \param[in]  
      \arg        bandrate: 波特率
    \param[out] none
    \retval     none
*/
static void usart1_init(uint32_t bandrate)
{    
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	//1.时钟使能，（不使能，外部设备不能使用）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	
	//2.配置IO端口的模式（复用功能）
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
		
	//3.初始化串口控制器
	USART_StructInit(& USART_InitStruct);  //初始化结构体
	USART_InitStruct.USART_BaudRate = bandrate;
	USART_Init(USART1, &USART_InitStruct);

	//4.开启串口控制器
	USART_Cmd(USART1, ENABLE);
	
	//5.开启中断允许
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);   //接收数据中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);   //接收空闲中断
#ifdef UART1_SEND_IRQ
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);   //发送缓存空中断
#endif
	//6.设置nvic，允许中断
	NVIC_InitStruct.NVIC_IRQChannel= USART1_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;    //使能
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);

}


void  usart1_init_all(uint32_t bandrate)
{
	memset((void *)&Queue_UART1_Recv, 0, sizeof(Queue_UART_STRUCT));

	usart1_init(bandrate);   //UART2 跟LS7A uart0连接
}



static void UART1_TX(uint8_t ch)
{
#ifdef UART1_SEND_IRQ	
	USART_SendData(USART1, ch); 
	if(txe_enable == 0){
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);   //开启中断
		txe_enable = 1;  //中断已开启
	}
#else	
	while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE))  //等待前面的数据发送结束
		;
    USART_SendData(USART1, ch);  
#endif	
}


void UART1_TX_STRING(uint8_t *str, uint8_t length)
{
	uint8_t i;
	
	for(i = 0; i < length; i++)
		UART1_TX(str[i]);
}


/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    UART1_TX(ch);
    return ch;
}

#ifdef UART1_SEND_IRQ    //中断的方式发送
void com1_send_irq(void)
{
	uint8_t len;
	uint8_t dat;
	len = QueueUARTDataLenGet(&Queue_UART1_Send);
	if(len)
	{
		QueueUARTDataDele(&Queue_UART1_Send,&dat);
		UART1_TX(dat);
	}
	else{
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);   //只要是空，就一直有中断。没数据的时候先关闭
		txe_enable  = 0; //中断已关闭
	}
}
#endif

void com1_idle_int_handle(void)
{
	int32_t err;
	uint8_t length,i;
	
	length = QueueUARTDataLenGet(&Queue_UART1_Recv);	
	if(length < UART_PROTOCOL_FRAME_LENGTH - UART_PROTOCOL_HEAD_LENGTH)   //小于8个字节，不是完整的一帧
		return ;   //继续等待
		
	length = UART_PROTOCOL_FRAME_LENGTH - UART_PROTOCOL_HEAD_LENGTH;   //读出6个字节来
	for(i=0;i<length;i++)
	{
		//这里不判断错误了，前面已经检测确实有这么多字节。
		QueueUARTDataDele(&Queue_UART1_Recv,com1_data+i+2) ;  //com_data 空出两个字节，为了兼容之前的校验和算法，及数据解析算法
	}
	
	if(0 == verify_data(com1_data,UART_PROTOCOL_FRAME_LENGTH))
	{
		com1_message_handle();
		//校验和正确。
	}	
	else  //即使收到错误的帧，也应该回复错误。
	{
		printf("check sum error!");    //prinf发送到uart1
	}	
}


/*
	串口收到命令后的处理。
		前提： 收到完整的数据包，校验和正确。

*/
void com1_message_handle(void)
{
	UART_PROTOCOL_TYPE type = (UART_PROTOCOL_TYPE)com1_data[2];
//	uint8_t i;
	uint8_t ret;
	uint8_t *data = com1_data+3;   //指向有效数据，4个字节。
	//根据命令类型作出处理
	
	switch(type)
	{
		case BTN_QUERY:   //查询按键的状态，不管正确与否，串口都会返回。！！！！！！
			ret = btns_query(data[0] - BTN_CODE_START);  //参数获得按键的位置
		//	lf_uart_protocol_send(BTN_REPLY, data[0], ret);		//返回值可能是255，串口同样返回数据，告诉主机表示出错了
			break;
		case LED_CTL:  //led控制			
			if(data[1] == 1)
				ret = set_led_on(data[0] - LED_CODE_START);
			else
				ret = set_led_off(data[0] - LED_CODE_START);
			//不管正确与否，单片机对命令作出应答
		//	lf_uart_protocol_send(LED_CTL, data[0], ret);	
			break;

		case LED_QUERY:   //led状态查询
			ret = get_led_value(data[0] - LED_CODE_START);
			if(ret != 255)
				ret = (ret==RESET) ? 1 : 0;//低电平表示亮，高电平灭
		//	lf_uart_protocol_send(LED_REPLY, data[0], ret);			
			break;
						
		case PWM_CTL:   //pwm控制
//			if(data[0] == CODE_PWM)
//			{
//				pwm = data[1];
//				if(pwm > 100) pwm=100;
//				
//				gd32_pwm_out(pwm);
//			}
			break;
		case PWM_QUERY:  //pwm查询
//			lf_uart_protocol_send(PWM_REPLY, CODE_PWM, pwm);
			break;
		case DVI_CTL:    //视频切换控制
			if(data[0] == CODE_DVI)
			{
				if(DVI_LOONGSON == data[1])
					dvi_switch_set(DVI_LOONGSON);
				else
					dvi_switch_set(DVI_OTHER);
			}
			break;
		case DVI_QUERY:  //显示视频查询
			ret = dvi_switch_get();
		//	lf_uart_protocol_send(DVI_REPLY, CODE_DVI, ret);
			break;	
		case FAN_CTL:   //风扇控制
//			if(data[0] == CODE_FAN)
//			{
//				switch(data[1])
//				{
//					case FAN_ON_1:
//						fan_enable(0);
//						break;
//					case FAN_ON_2:
//						fan_enable(1);
//						break;
//					case FAN_ON_ALL:
//						fan_enable(0);
//						fan_enable(1);
//						break;
//					case FAN_OFF_1:
//						fan_disable(0);
//						break;
//					case FAN_OFF_2:
//						fan_disable(1);
//						break;
//					case FAN_OFF_ALL:
//						fan_disable(0);
//						fan_disable(1);
//						break;
//					default:
//						break;
//				}
//			}
			break;
		case FAN_QUERY:  //风扇状态查询
//			if(fan_get_value(0) == ENABLE)
//			{
//				if(fan_get_value(1) == ENABLE)
//				{
//					fan = FAN_ON_ALL;
//				}
//				else
//					fan = FAN_ON_1;
//			}
//			else
//			{
//				if(fan_get_value(1) == ENABLE)
//				{
//					fan = FAN_ON_2;
//				}
//				else
//					fan = FAN_OFF_ALL;
//			}
//				
//			lf_uart_protocol_send(FAN_REPLY, CODE_FAN, fan);
			break;	
		case HOT_CTL:   //加热控制
			if(data[0] == CODE_HOT)
			{
				set_heat_status(data[1]);
			}			
			break;
		case HOT_QUERY:  //加热状态控制
			ret = get_heat_status();
		//	lf_uart_protocol_send(HOT_REPLY, CODE_HOT, ret);
			break;		
		case TMP_CTL:   //温度控制
//			if(data[0] == CODE_TMP)
//			{
//				if(1 == data[1])
//					temperat_send = 1;
//				else
//					temperat_send = 0;
//			}
			break;
		case TMP_QUERY:  //温度查询
		//	lf_uart_protocol_send(TMP_REPLY, CODE_TMP, temperat_send);
			break;				
		default:
			break;
	}
}


#endif


#endif
