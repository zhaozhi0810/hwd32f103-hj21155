

/*
	2021-09-22
	
	�ڰ˸���ֲ���ļ���
	
	Ŀǰ��Ƭ����ʹ�ô���2����cpu��������ͨ�š��������ݶ�115200����ԭ����һ�£�
	
	����������ο�usart2.h�ļ�
	
	�жϴ�����ͳһ����stm32f103_it.c��
	
	2021-09-28  stm32 3�����ڶ���ʹ��
	uart1-����ʹ��
	uart2/uart3-����3A3000��һ�������ϴ��¶ȵ�ѹ����Ϣ��һ�������ϴ�18����������Ϣ���Լ�18��led�Ŀ��Ʋ���
	
	usart2 -> ����3A3000����Ҫ�������ϴ��¶ȵ�ѹ����Ϣ
	

	2021-12-01.
	����2��������ƣ����������3�Ĺ��ܣ���Ŀǰȫ��ע��
*/

#if 0 //2021-12-01

#include "includes.h"
#include <stdio.h>



static Queue_UART_STRUCT Queue_UART2_Recv;
static uint8_t com_data[UART_PROTOCOL_FRAME_LENGTH];   //���ջ���


//���в�������
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



//У������
int32_t verify_data(uint8_t *data,uint8_t len)
{
	uint8_t check;
	int32_t ret = -1;
	
	if(data == NULL)
		return -1;
	
	//��ȡԭ�����е�У��ֵ
	check = data[len - 1];
	
	//���¼���У��ֵ
	if(check==checksum(data+2,len - 3))
		ret = 0;
	
	return ret;
}



static void UART2_TX(uint8_t ch)
{
	while(RESET == USART_GetFlagStatus(USART2, USART_FLAG_TXE))  //�ȴ�ǰ������ݷ��ͽ���
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
      \arg        bandrate: ������
    \param[out] none
    \retval     none
*/
static void usart2_init(uint32_t bandrate)
{    
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	//1.ʱ��ʹ�ܣ�����ʹ�ܣ��ⲿ�豸����ʹ�ã�
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	
	//2.����IO�˿ڵ�ģʽ�����ù��ܣ�
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
		
	//3.��ʼ�����ڿ�����
	USART_StructInit(& USART_InitStruct);  //��ʼ���ṹ��
	USART_InitStruct.USART_BaudRate = bandrate;
	USART_Init(USART2, &USART_InitStruct);

	//4.�������ڿ�����
	USART_Cmd(USART2, ENABLE);
	
	//5.�����ж�����
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);   //���������ж�
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);   //���տ����ж�
	
	//6.����nvic�������ж�
	NVIC_InitStruct.NVIC_IRQChannel= USART2_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;    //ʹ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStruct);

}

void  usart2_init_all(uint32_t bandrate)
{
	memset((void *)&Queue_UART2_Recv, 0, sizeof(Queue_UART_STRUCT));

	usart2_init(bandrate);   //UART2 ��LS7A uart0����
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



static uint8_t head_recv = 0;  //����0��ʾû�н��յ�֡ͷ��1����ʾ�յ�֡ͷ1��2��ʾ�յ�֡ͷ2,3��ʾ�յ���һ�����ݡ�������

/*
	���ڽ����жϣ�
		ǰ�᣺ÿһ֡����8���ֽڡ�
		������û�б���֡ͷ��ֻ�к�������ݺ�У��ͣ���6���ֽڣ�
*/
void com_rne_int_handle(void)
{
	uint8_t dat;
	dat = (uint8_t)USART_ReceiveData(USART2);
	if(head_recv == 0)  //û���յ�֡ͷ
	{		
		if(dat == UART_PROTOCOL_HEAD0)
		{
			head_recv = 1;
		}
	}
	else if(head_recv == 1)  //�Ѿ��յ��˵�һ��֡ͷ
	{
		if(dat == UART_PROTOCOL_HEAD1)  //�յ���һ��֡ͷ֮�������յ��ڶ���֡ͷ
		{
			head_recv = 2;
		}
		else
			head_recv = 0;   //�ڶ���֡ͷ����
	}
	else if(head_recv >= 2)
	{
		QueueUARTDataInsert(&Queue_UART2_Recv,dat);   //���յ����ݴ�������С�
		head_recv++;   //�յ����ֽ�������
		if(head_recv >= UART_PROTOCOL_FRAME_LENGTH)  //һ֡�Ѿ��������ˣ���֤��һ֡
		{
			head_recv = 0;
		}
	}
	else 
		head_recv = 0;   //û���յ�֡ͷ�����ݲ�����
}



/*
	���ڿ����жϵĴ���
*/

void com_idle_int_handle(void)
{
	int32_t err;
	uint8_t length,i;
	
	length = QueueUARTDataLenGet(&Queue_UART2_Recv);	
	if(length < UART_PROTOCOL_FRAME_LENGTH - UART_PROTOCOL_HEAD_LENGTH)   //С��8���ֽڣ�����������һ֡
		return ;   //�����ȴ�
		
	length = UART_PROTOCOL_FRAME_LENGTH - UART_PROTOCOL_HEAD_LENGTH;   //����6���ֽ���
	for(i=0;i<length;i++)
	{
		//���ﲻ�жϴ����ˣ�ǰ���Ѿ����ȷʵ����ô���ֽڡ�
		QueueUARTDataDele(&Queue_UART2_Recv,com_data+i+2) ;  //com_data �ճ������ֽڣ�Ϊ�˼���֮ǰ��У����㷨�������ݽ����㷨
	}
	
	if(0 == verify_data(com_data,UART_PROTOCOL_FRAME_LENGTH))
	{
		com_message_handle();
		//У�����ȷ��
	}	
	else  //��ʹ�յ������֡��ҲӦ�ûظ�����
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
//    //���ڲ���֡ͷ
//    if(to_search_frame == 1)
//    {
//        //��ȡ�������ݣ�����֡ͷ
//        for(i=0;i<length;i++)
//        {
//			err = QueueUARTDataDele(&frame_buff[frame_i]);//��1�ֽ�
//			if(!err)
//			{
//				if(frame_buff[frame_i] == UART_PROTOCOL_HEAD0)//�ҵ�֡ͷ1,�ж��Ƿ��ǵ�һ���ֽ�,�������,��ֵ����һ���ֽ�,������1
//				{
//					if(frame_i != 0)
//					{
//						frame_buff[0] = frame_buff[frame_i];
//					}
//					
//					frame_i = 1;
//				}
//				else if(frame_buff[frame_i] == UART_PROTOCOL_HEAD1)//�ҵ�֡ͷ2,�ж��Ƿ��ǵڶ����ֽ�,�������,��������,�����,������ҵ�����ȷ��֡ͷ,��������2
//				{
//					if(frame_i == 1)
//					{
//						to_search_frame = 0;
//						frame_i = 2;
//						length -= i;
//						break;//�ҵ�֡ͷ������
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
//    if(to_search_frame == 0)//�ҵ���֡ͷ,������ȡ,ֱ��һ������8���ֽ�(����֡ͷ2�ֽ�)
//    {
//        for(i=0;i<length;i++)
//        {
//            err = QueueUARTDataDele(&frame_buff[frame_i]);
//            if(!err)//����һ���ֽ�,������1
//            {
//                frame_i++;
//                if(frame_i == UART_PROTOCOL_FRAME_LENGTH)//�����ӵ�8����������8���ֽ�,һ֡���������ݶ�ȡ���,���н���������״̬��λ
//                {
//                    if(0 == verify_data(frame_buff))//У������,���У��ͨ��,����������ж�
//                    {
//                        memcpy(&data[0], &frame_buff[3], 4);

//						/* �ҵ�֡������״̬ */
//	                    to_search_frame = 1;
//	                    frame_i = 0;

//                        return (UART_PROTOCOL_TYPE)(frame_buff[2]);
//                    }
//                    else
//                    {
//                    	/* ֡У��Ͳ���ȷ������״̬ */
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
	�����յ������Ĵ���
		ǰ�᣺ �յ����������ݰ���У�����ȷ��

*/
void com_message_handle(void)
{
	UART_PROTOCOL_TYPE type = (UART_PROTOCOL_TYPE)com_data[2];
//	uint8_t i;
	uint8_t ret;
	uint8_t *data = com_data+3;   //ָ����Ч���ݣ�4���ֽڡ�
	//��������������������
	
	switch(type)
	{
		case BTN_QUERY:   //��ѯ������״̬��������ȷ��񣬴��ڶ��᷵�ء�������������
			ret = btns_query(data[0] - BTN_CODE_START);  //������ð�����λ��
			lf_uart_protocol_send(BTN_REPLY, data[0], ret);		//����ֵ������255������ͬ���������ݣ�����������ʾ������
			break;
		case LED_CTL:  //led����			
			if(data[1] == 1)
				ret = set_led_on(data[0] - LED_CODE_START);
			else
				ret = set_led_off(data[0] - LED_CODE_START);
			//������ȷ��񣬵�Ƭ������������Ӧ��
			lf_uart_protocol_send(LED_CTL, data[0], ret);	
			break;

		case LED_QUERY:   //led״̬��ѯ
			ret = get_led_value(data[0] - LED_CODE_START);
			if(ret != 255)
				ret = (ret==RESET) ? 1 : 0;//�͵�ƽ��ʾ�����ߵ�ƽ��
			lf_uart_protocol_send(LED_REPLY, data[0], ret);			
			break;
						
		case PWM_CTL:   //pwm����
//			if(data[0] == CODE_PWM)
//			{
//				pwm = data[1];
//				if(pwm > 100) pwm=100;
//				
//				gd32_pwm_out(pwm);
//			}
			break;
		case PWM_QUERY:  //pwm��ѯ
//			lf_uart_protocol_send(PWM_REPLY, CODE_PWM, pwm);
			break;
		case DVI_CTL:    //��Ƶ�л�����
			if(data[0] == CODE_DVI)
			{
				if(DVI_LOONGSON == data[1])
					dvi_switch_set(DVI_LOONGSON);
				else
					dvi_switch_set(DVI_OTHER);
			}
			break;
		case DVI_QUERY:  //��ʾ��Ƶ��ѯ
			ret = dvi_switch_get();
			lf_uart_protocol_send(DVI_REPLY, CODE_DVI, ret);
			break;	
		case FAN_CTL:   //���ȿ���
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
		case FAN_QUERY:  //����״̬��ѯ
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
		case HOT_CTL:   //���ȿ���
			if(data[0] == CODE_HOT)
			{
				set_heat_status(data[1]);
			}			
			break;
		case HOT_QUERY:  //����״̬����
			ret = get_heat_status();
			lf_uart_protocol_send(HOT_REPLY, CODE_HOT, ret);
			break;		
		case TMP_CTL:   //�¶ȿ���
//			if(data[0] == CODE_TMP)
//			{
//				if(1 == data[1])
//					temperat_send = 1;
//				else
//					temperat_send = 0;
//			}
			break;
		case TMP_QUERY:  //�¶Ȳ�ѯ
		//	lf_uart_protocol_send(TMP_REPLY, CODE_TMP, temperat_send);
			break;				
		default:
			break;
	}
}

#endif

