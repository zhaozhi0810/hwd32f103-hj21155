

/*
	2021-09-22
	
	第八个移植的文件。
	
	目前单片机仅使用串口2，跟cpu进行数据通信。波特率暂定115200（跟原来的一致）
	
	串口命令请参考usart2.h文件
	
	中断处理函数统一放在stm32f103_it.c中
	
	2021-09-28  stm32 3个串口都打开使用
	uart1-调试使用
	uart2/uart3-连接3A3000，一个用于上传温度电压等信息，一个用于上传18个按键的信息，以及18个led的控制部分
	
	usart2 -> 连接3A3000，主要是用于上传温度电压等信息
	

	2021-12-01.
	串口2做冗余设计，可替代串口3的功能！！目前全部注释
*/

#if 0 //2021-12-01

#include "includes.h"
#include <stdio.h>



static Queue_UART_STRUCT Queue_UART2_Recv;
static uint8_t com_data[UART_PROTOCOL_FRAME_LENGTH];   //接收缓存


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
	if(check==checksum(data+2,len - 3))
		ret = 0;
	
	return ret;
}



static void UART2_TX(uint8_t ch)
{
	while(RESET == USART_GetFlagStatus(USART2, USART_FLAG_TXE))  //等待前面的数据发送结束
		;
    USART_SendData(USART2, ch);    
}

void UART2_TX_STRING(uint8_t *str, uint8_t length)
{
	uint8_t i;
	
	for(i = 0; i < length; i++)
		UART2_TX(str[i]);
}

int32_t lf_uart_send(char *str)
{
    uint8_t data[UART_PROTOCOL_FRAME_LENGTH];
    
    data[0] = UART_PROTOCOL_HEAD0;
    data[1] = UART_PROTOCOL_HEAD1;
    memcpy(&data[2], str, 6);
    
    UART2_TX_STRING(data, UART_PROTOCOL_FRAME_LENGTH);
    
    return 0;
}

/*!
    \brief      configure COM port
    \param[in]  
      \arg        bandrate: 波特率
    \param[out] none
    \retval     none
*/
static void usart2_init(uint32_t bandrate)
{    
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	//1.时钟使能，（不使能，外部设备不能使用）
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	
	//2.配置IO端口的模式（复用功能）
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
		
	//3.初始化串口控制器
	USART_StructInit(& USART_InitStruct);  //初始化结构体
	USART_InitStruct.USART_BaudRate = bandrate;
	USART_Init(USART2, &USART_InitStruct);

	//4.开启串口控制器
	USART_Cmd(USART2, ENABLE);
	
	//5.开启中断允许
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);   //接收数据中断
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);   //接收空闲中断
	
	//6.设置nvic，允许中断
	NVIC_InitStruct.NVIC_IRQChannel= USART2_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;    //使能
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);

}

void  usart2_init_all(uint32_t bandrate)
{
	memset((void *)&Queue_UART2_Recv, 0, sizeof(Queue_UART_STRUCT));

	usart2_init(bandrate);   //UART2 跟LS7A uart0连接
}



int32_t lf_uart_protocol_send(UART_PROTOCOL_TYPE type, uint8_t code, uint8_t value)
{
    uint8_t data[UART_PROTOCOL_FRAME_LENGTH];
    
    data[0] = UART_PROTOCOL_HEAD0;
    data[1] = UART_PROTOCOL_HEAD1;
    data[2] = type;
    data[3] = code;
    data[4] = value;
    data[5] = 0;
    data[6] = 0;
    data[7] = checksum(data+2, UART_PROTOCOL_FRAME_LENGTH - 3);
    
    UART2_TX_STRING(data, UART_PROTOCOL_FRAME_LENGTH);
    
    return 0;
}



static uint8_t head_recv = 0;  //等于0表示没有接收到帧头，1，表示收到帧头1，2表示收到帧头2,3表示收到第一个数据。。。。

/*
	串口接收中断：
		前提：每一帧都是8个字节。
		队列中没有保存帧头，只有后面的数据和校验和（共6个字节）
*/
void com_rne_int_handle(void)
{
	uint8_t dat;
	dat = (uint8_t)USART_ReceiveData(USART2);
	if(head_recv == 0)  //没有收到帧头
	{		
		if(dat == UART_PROTOCOL_HEAD0)
		{
			head_recv = 1;
		}
	}
	else if(head_recv == 1)  //已经收到了第一个帧头
	{
		if(dat == UART_PROTOCOL_HEAD1)  //收到第一个帧头之后，立马收到第二个帧头
		{
			head_recv = 2;
		}
		else
			head_recv = 0;   //第二个帧头不对
	}
	else if(head_recv >= 2)
	{
		QueueUARTDataInsert(&Queue_UART2_Recv,dat);   //接收的数据存入队列中。
		head_recv++;   //收到的字节数增加
		if(head_recv >= UART_PROTOCOL_FRAME_LENGTH)  //一帧已经接收完了，验证下一帧
		{
			head_recv = 0;
		}
	}
	else 
		head_recv = 0;   //没有收到帧头，数据不保存
}



/*
	串口空闲中断的处理
*/

void com_idle_int_handle(void)
{
	int32_t err;
	uint8_t length,i;
	
	length = QueueUARTDataLenGet(&Queue_UART2_Recv);	
	if(length < UART_PROTOCOL_FRAME_LENGTH - UART_PROTOCOL_HEAD_LENGTH)   //小于8个字节，不是完整的一帧
		return ;   //继续等待
		
	length = UART_PROTOCOL_FRAME_LENGTH - UART_PROTOCOL_HEAD_LENGTH;   //读出6个字节来
	for(i=0;i<length;i++)
	{
		//这里不判断错误了，前面已经检测确实有这么多字节。
		QueueUARTDataDele(&Queue_UART2_Recv,com_data+i+2) ;  //com_data 空出两个字节，为了兼容之前的校验和算法，及数据解析算法
	}
	
	if(0 == verify_data(com_data,UART_PROTOCOL_FRAME_LENGTH))
	{
		com_message_handle();
		//校验和正确。
	}	
	else  //即使收到错误的帧，也应该回复错误。
	{
		lf_uart_send("check sum error!");
	}	
}




UART_PROTOCOL_TYPE lf_uart_protocol_receive(uint8_t *data)
{
//    static uint8_t to_search_frame=1;
//    static int32_t frame_i=0;
//    static uint8_t frame_buff[UART_PROTOCOL_FRAME_LENGTH];
//	int32_t err;
//    uint8_t length;
//    int8_t i;

//    length = QueueUARTDataLenGet();
//    if(length <= 0)
//        return PROTOCOL_NONE;
//    
//    //正在查找帧头
//    if(to_search_frame == 1)
//    {
//        //读取缓存数据，查找帧头
//        for(i=0;i<length;i++)
//        {
//			err = QueueUARTDataDele(&frame_buff[frame_i]);//读1字节
//			if(!err)
//			{
//				if(frame_buff[frame_i] == UART_PROTOCOL_HEAD0)//找到帧头1,判断是否是第一个字节,如果不是,赋值给第一个字节,索引加1
//				{
//					if(frame_i != 0)
//					{
//						frame_buff[0] = frame_buff[frame_i];
//					}
//					
//					frame_i = 1;
//				}
//				else if(frame_buff[frame_i] == UART_PROTOCOL_HEAD1)//找到帧头2,判断是否是第二个字节,如果不是,重新搜索,如果是,则表明找到了正确的帧头,索引等于2
//				{
//					if(frame_i == 1)
//					{
//						to_search_frame = 0;
//						frame_i = 2;
//						length -= i;
//						break;//找到帧头后，跳出
//					}
//					else
//					{
//						frame_i = 0;
//					}
//				}
//				else
//				{}
//			}		
//			else
//			{
//				length -= i;
//				break;
//			}
//        }
//    }
// 	
//    if(to_search_frame == 0)//找到了帧头,继续读取,直到一共读到8个字节(包括帧头2字节)
//    {
//        for(i=0;i<length;i++)
//        {
//            err = QueueUARTDataDele(&frame_buff[frame_i]);
//            if(!err)//读到一个字节,索引加1
//            {
//                frame_i++;
//                if(frame_i == UART_PROTOCOL_FRAME_LENGTH)//索引加到8表明读到了8个字节,一帧完整的数据读取完毕,进行解析和数据状态复位
//                {
//                    if(0 == verify_data(frame_buff))//校验数据,如果校验通过,则进行类型判断
//                    {
//                        memcpy(&data[0], &frame_buff[3], 4);

//						/* 找到帧后，重置状态 */
//	                    to_search_frame = 1;
//	                    frame_i = 0;

//                        return (UART_PROTOCOL_TYPE)(frame_buff[2]);
//                    }
//                    else
//                    {
//                    	/* 帧校验和不正确，重置状态 */
//	                    to_search_frame = 1;
//	                    frame_i = 0;
//                    }
//                }
//                else
//                {}
//            }
//			else
//			{
//				break;
//			}
//        }
//    }
//    
//    return PROTOCOL_NONE;
}


/*
	串口收到命令后的处理。
		前提： 收到完整的数据包，校验和正确。

*/
void com_message_handle(void)
{
	UART_PROTOCOL_TYPE type = (UART_PROTOCOL_TYPE)com_data[2];
//	uint8_t i;
	uint8_t ret;
	uint8_t *data = com_data+3;   //指向有效数据，4个字节。
	//根据命令类型作出处理
	
	switch(type)
	{
		case BTN_QUERY:   //查询按键的状态，不管正确与否，串口都会返回。！！！！！！
			ret = btns_query(data[0] - BTN_CODE_START);  //参数获得按键的位置
			lf_uart_protocol_send(BTN_REPLY, data[0], ret);		//返回值可能是255，串口同样返回数据，告诉主机表示出错了
			break;
		case LED_CTL:  //led控制			
			if(data[1] == 1)
				ret = set_led_on(data[0] - LED_CODE_START);
			else
				ret = set_led_off(data[0] - LED_CODE_START);
			//不管正确与否，单片机对命令作出应答
			lf_uart_protocol_send(LED_CTL, data[0], ret);	
			break;

		case LED_QUERY:   //led状态查询
			ret = get_led_value(data[0] - LED_CODE_START);
			if(ret != 255)
				ret = (ret==RESET) ? 1 : 0;//低电平表示亮，高电平灭
			lf_uart_protocol_send(LED_REPLY, data[0], ret);			
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
			lf_uart_protocol_send(DVI_REPLY, CODE_DVI, ret);
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
			lf_uart_protocol_send(HOT_REPLY, CODE_HOT, ret);
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

