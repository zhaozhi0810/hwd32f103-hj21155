/*
	2021-09-28
	
	��ʮ������ֲ���ļ���
		
	�жϴ�����ͳһ����stm32f103_it.c��
	
	2021-09-28  stm32 3�����ڶ���ʹ��
	uart1-����ʹ��
	uart2/uart3-����3A3000��һ�������ϴ��¶ȵ�ѹ����Ϣ��һ�������ϴ�18����������Ϣ���Լ�18��led�Ŀ��Ʋ���
	
	usart2 -> ����3A3000����Ҫ�������ϴ�18����������Ϣ���Լ�18��led�Ŀ��Ʋ���
	
	
	֡ͷ��
		3A3000 -> ��Ƭ�� A5   //���ֽ�
		��Ƭ�� -> 3A3000 A5   //���ֽ�
	
	֡����
		��7���ֽ�
		֡ͷ1+����1+����4+У���1
	
	���ͣ�
		���͵�����
		1.��ʱ�ϱ���3����ѹ���˵磬7A��12V��,cpu�¶ȣ������¶ȣ�lcd�¶ȣ�����pwm
		2.�����ϱ���18����������Ƶ�л�������lcd���ȣ�lcd����״̬�����޸ļ��ϱ��������ϵ�״̬���ػ�������������
				
		���յ����ݣ�
		3.���ã�cpu������mcu������Ƶ�л���18��led״̬�����ϵ�״̬
		4.

*/


#include "includes.h"
//#include "usart3.h"


#define RECV_BUF_LEN 64
static Queue_UART_STRUCT Queue_UART3_Recv;   //���ն��У����ڽ����ж�
//static uint8_t com3_recv_buf[RECV_BUF_LEN];
//static uint8_t com3_recv_len = 0;  //�������ݵĳ���
static uint8_t com3_data[8];   //���ջ���
uint8_t uart_inited = 0;   // 0��ʾû�г�ʼ����1��ʾ��ʼ���ˡ�



#define UART_BUFF_FULL_COVER
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
	if(check==checksum(data,len - 1))
		ret = 0;
	
	return ret;
}





/*!
    \brief      configure COM port
    \param[in]  
      \arg        bandrate: ������
    \param[out] none
    \retval     none
*/
static void usart3_init(uint32_t bandrate)
{    
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	//1.ʱ��ʹ�ܣ�����ʹ�ܣ��ⲿ�豸����ʹ�ã�
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);    //PC10��11
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   //IO���ù���ģ��
	
	//2.����IO�˿ڵ�ģʽ�����ù��ܣ�
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 ;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP; //��������
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;	
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
	GPIO_Init(GPIOC, &GPIO_InitStruct);	
	
	//GPIO_SetBits(GPIOC, GPIO_Pin_11);   //����
	
	//2.1 �˿���ӳ�书��
	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE);   //Ĭ����PB10��PB11������ӳ����PC10��PC11
	
		
	//3.��ʼ�����ڿ�����
	USART_StructInit(& USART_InitStruct);  //��ʼ���ṹ��
	USART_InitStruct.USART_BaudRate = bandrate;
	USART_Init(USART3, &USART_InitStruct);

	//4.�������ڿ�����
	USART_Cmd(USART3, ENABLE);
	
	//5.�����ж�����
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);   //���������ж�
//	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);   //���տ����ж�
	
	//6.����nvic�������ж�
	NVIC_InitStruct.NVIC_IRQChannel= USART3_IRQn;  
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;    //ʹ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);

	uart_inited = 1; //�Ѿ���ʼ���ˡ�

}

//uart3ȥ��ʼ����������������͵�ƽ
void usart3_deinit(void)
{    
	GPIO_InitTypeDef GPIO_InitStruct;
//	USART_InitTypeDef USART_InitStruct;
//	NVIC_InitTypeDef NVIC_InitStruct;
		
	//2.����IO�˿ڵ�ģʽ�����ù��ܣ�
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 |GPIO_Pin_11;  //0,13 �Ǳ���͵�Դʹ��
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_ResetBits(GPIOC, GPIO_Pin_10);    //����3�ڶϵ����������
	GPIO_ResetBits(GPIOC, GPIO_Pin_11);

	//2.1 �˿���ӳ�书��
	//GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, DISABLE);   //Ĭ����PB10��PB11������ӳ����PC10��PC11
		
	//3.ȥ��ʼ�����ڿ�����
	USART_DeInit(USART3);
	//4.�������ڿ�����
	USART_Cmd(USART3, DISABLE);
	
	//5.�����ж�����
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);   //���������ж�
	USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);   //���տ����ж�
	
	//6.����nvic�������ж�
	NVIC_DisableIRQ(USART3_IRQn);

	uart_inited = 0; //�Ѿ���ʼ���ˡ�
}



/*
	����3�ĳ�ʼ��
*/
void  usart3_init_all(uint32_t bandrate)
{
	memset((void *)&Queue_UART3_Recv, 0, sizeof(Queue_UART_STRUCT));

	usart3_init(bandrate);   //UART3 ��LS7A uart1����
}



static void UART3_TX(uint8_t ch)
{
	if(uart_inited)   //cpu�ϵ�֮��û�г�ʼ��
	{
		while(RESET == USART_GetFlagStatus(USART3, USART_FLAG_TXE))  //�ȴ�ǰ������ݷ��ͽ���
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



//��Ƭ���������ݸ�cpu�����ɵ������̴߳����ˡ�dataֻ��Ҫ�����������ͺ����ݡ�ͷ����crc�ɸú�����ɡ�
/*
 * data ���ڷ��͵����ݣ�����Ҫ����֡ͷ��У��ͣ�ֻҪ�����������ͺ����ݣ���5���ֽڣ�
 * ����ֵ
 * 	0��ʾ�ɹ���������ʾʧ��
 * */
int send_mcu_data_tocpu(const void* data)
{	
	unsigned char buf[8];  	
	
	if(!uart_inited)
		return -1;
	
	buf[0] = FRAME_HEAD;  //֡ͷ	
	memcpy(buf+1,data,sizeof(com_frame_t)-1);    //����

	//crc���¼���
	buf[sizeof(com_frame_t)] = checksum(buf,sizeof(com_frame_t));  //У��ͣ��洢�ڵ�7���ֽ��ϣ������±�6.
	
	UART3_TX_STRING(buf, sizeof(com_frame_t)+1);   //com_frame_t��û�а�������ͷ�����Լ�1���ֽ�	

	//���ͳɹ����ȴ�Ӧ��
	return 0;   //��ʱû�еȴ�Ӧ��2021-11-23

}



//�����������ݵ�cpu
//cmd��ο�usart3.h�к궨��
//param ������
void send_cmd_to_cpu(int cmd,int param)
{
	com_frame_t data;
	data.data_type = eMCU_CMD_TYPE;
	data.data.cmd.cmd = cmd;
	data.data.cmd.param_len = 1;     //��һ������
	data.data.cmd.param1 = param;   //��һ������
	send_mcu_data_tocpu(&data);
}

//����dvi��Ƶ���л������ݵ�cpu
//source 1�����أ�����2���ⲿ��
void send_dvi_change_to_cpu(int source)
{
	com_frame_t data;
	data.data_type = eMCU_DIV_CHANGE_TYPE;
    data.data.fan_div_bk.bstatus.dvi_src = source-1;
	send_mcu_data_tocpu(&data);
}

//����dvi��Ƶ���л������ݵ�cpu
//whichkey 0xe8 - 0xf9
//status 0 or 1 �ɿ����߰���
void send_btn_change_to_cpu(int whichkey,int status)
{
	com_frame_t data;
	data.data_type = eMCU_KEY_CHANGE_TYPE;    
	data.data.key_change.whichkey =whichkey;
	data.data.key_change.status = status;	
	send_mcu_data_tocpu(&data);
}

//����led״̬�����ݵ�cpu
//status��ʾ18��led��(��18λ��Ч)
void send_leds_status_to_cpu(int status)
{
	com_frame_t data;
	data.data_type = eMCU_LED_STATUS_TYPE;
	data.data.leds_status = status & 0x3ffff;
	send_mcu_data_tocpu(&data);
}

//���������cpu�¶�
void send_bc_temp_to_cpu(short cpu_temp,short board_temp)
{
	com_frame_t data;
	data.data_type = eMCU_CB_TEMP_TYPE;
	data.data.cb_temp.cpu_t = cpu_temp;
	data.data.cb_temp.board_t = board_temp;
	send_mcu_data_tocpu(&data);
}

//����3����ѹ
void send_vol_to_cpu(short vol_core,short vol_7A,short vol_12v)
{
	com_frame_t data;
	
	data.data_type = eMCU_VOL_TYPE;
	data.data.vol.vol_core = vol_core;
	data.data.vol.vol_7A = vol_7A;
	data.data.vol.vol_12V = vol_12v;

	send_mcu_data_tocpu(&data);
}

//����lcd���¶ȣ�pwm������ʹ��״̬
void send_lcd_temp_to_cpu(short lcd_t1,short lcd_t2)
{
	com_frame_t data;
	
	data.data_type = eMCU_LCD_TEMP_TYPE;
	data.data.lcd.lcd_temp1 = lcd_t1;  //����״̬
	data.data.lcd.lcd_temp2 = lcd_t2;    //lcdpwm

	send_mcu_data_tocpu(&data);
}



//2021-12-15����
//��ʱ���ͷ���״̬��dvi�͹��ϵ�״̬
void send_fan_bk_div_status_to_cpu(bitstatus_t b_status,uint8_t fan_pwm,uint8_t lcd_pwm)
{
	com_frame_t data;
	
	data.data_type = eMCU_FAN_DIV_BK_TYPE;
	data.data.fan_div_bk.bstatus = b_status;  //���ϵ�״̬
	data.data.fan_div_bk.fan_pwm = fan_pwm;    //����pwmֵ
	data.data.fan_div_bk.lcd_pwm = lcd_pwm;    //lcdpwmֵ

	send_mcu_data_tocpu(&data);
}



static uint8_t head_recv = 0;  //����0��ʾû�н��յ�֡ͷ��1����ʾ�յ�֡ͷ1��2��ʾ�յ�֡ͷ2,3��ʾ�յ���һ�����ݡ�������

/*
	�������ݽ����жϣ�
		ǰ�᣺ÿһ֡����7���ֽڡ�
		������û�б���֡ͷ��ֻ�к�������ݺ�У��ͣ���6���ֽڣ�
*/
void com3_rne_int_handle(void)
{
	uint8_t dat;
	dat = (uint8_t)USART_ReceiveData(USART3);   //���վʹ浽�����У���������2021-12-02

	
	if(head_recv == 0)  //û���յ�֡ͷ
	{		
		if(dat == FRAME_HEAD)    //3A���͵�֡ͷ��A5
		{
			QueueUARTDataInsert(&Queue_UART3_Recv,dat);   //���յ����ݴ�������С�
			head_recv = 1;    //ֻ��һ��֡ͷ�ˣ���������2021-12-01
		}
	}
	else if(head_recv == 1)   //���ǵ��������򣬾Ͳ���Ѱ��
	{
		QueueUARTDataInsert(&Queue_UART3_Recv,dat);   //���յ����ݴ�������С�
//		head_recv++;   //�յ����ֽ�������
//		if(head_recv >= UART3_PROTOCOL_FRAME_LENGTH)  //һ֡�Ѿ��������ˣ���֤��һ֡
//		{
//			head_recv = 0;
//		}
	}
//	else 
//		head_recv = 0;   //û���յ�֡ͷ�����ݲ�����
}


/*
	�����յ������Ĵ���com3_idle_int_handle�жϴ���������
		ǰ�᣺ �յ����������ݰ���У�����ȷ��


	��Ƭ���ܹ��յ������
	1.������ƵԴ
	2.����18��led��״̬
	3.���ù��ϵ�״̬
	4.����lcd��pwm�����ȣ�
	4.�ػ����������

*/

static void com3_message_handle(void)
{	
	com_frame_t* pdata = (com_frame_t*)(com3_data+1);    //+1������֡ͷ��ʹ�ýṹ���ʼ��
	int t;
//	uint8_t dat[2];
	
	switch(pdata->data_type)
    {
        case eMCU_LED_SETON_TYPE:  //����led��
        //        printf("led = %#x\n",data);   //������Ϣ
            t = pdata->data.leds_status;  //t�ж�ȡ������һ��(1-18���ֱ�ʾ)led�����ã�0��ʾȫ��
            //    leds_control(t,1);
			set_led_on(t);
        break;
        case eMCU_LED_SETOFF_TYPE:   //����led��
            t = pdata->data.leds_status;  //t�ж�ȡ������һ��(1-18���ֱ�ʾ)led�����ã�0��ʾȫ��
        //    leds_control(t,0);
			set_led_off(t);
            break;

        case   eMCU_LEDSETALL_TYPE:    //�������е�led
            t = pdata->data.leds_status;   //t��ʾ���е�led�Ŀ������
			led1_18_Set(t);
        break;
        case eMCU_DVI_SETSRC_TYPE:   //������ƵԴ
            t = pdata->data.fan_div_bk.bstatus.dvi_src;
            if(t)
				dvi_switch_set(DVI_OTHER);   //���ú���ϱ���cpu
            else
				dvi_switch_set(DVI_LOONGSON);   //������Ƶ
            break;
//        case eMCU_CB_TEMP_TYPE:   
        //    pdata->data.cb_temp = *(cb_temp_t*)&data;
                //pdata->data.cb_temp.cpu_t;
                //pdata->data.cb_temp.board_t;
//        break;
        case eMCU_BKLEDSET_TYPE:    //���ù��ϵƵ�����
            t = pdata->data.fan_div_bk.bstatus.breakdownLed_status;  //t�ж������ǹ��ϵƵĿ������
            if(t)
				set_bug_led_on();
            else
				set_bug_led_off();
        break;
        case eMCU_LCD_SETPWM_TYPE:    //������Ļ����
            t = pdata->data.fan_div_bk.lcd_pwm;
            if(t>=0 && t <101)
            {
				Lcd_pwm_out(t);
            }
        break;		
        case eMCU_CMD_TYPE:    //��Ƭ����������һЩ�������
            t = pdata->data.cmd.cmd;
            switch(t)
            {
                case eMCU_ANSWER_CMD:
                //    if(pdata->data.cmd.param_len)
                //		return pdata->data.cmd.param1;
                //	return -1;   //Ӧ�����
                break;
                case eMCU_REBOOT_CMD:   //�������������
                break;
                case eMCU_SHUTDOWN_CMD:  //������ػ�����
                break;
                case eMCU_SWITCH_DVI_SRC_CMD:  ////������л���ƵԴ����
                break;
				case eMCU_LCD_SETPWM_UP_CMD:    //������Ļ����
					t = g_lcd_pwm + 10;
					if(t >100)
					{
						t = 100;
					}
					Lcd_pwm_out(t);
				break;
				case eMCU_LCD_SETPWM_DN_CMD:    //������Ļ����
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
	���ڿ����жϵĴ���

	1.�жϽ��յ����ֽ�����>=7 ��ʾ����
	2.�����ͼ�����������7���ֽڣ�����У��ͣ�
	3.У�����ȷ����������

*/

#define FRAME_LENGHT (sizeof(com_frame_t)+1)    //����֡���ֽ���

void com3_frame_handle(void)
{
//	int32_t err;
	uint8_t length,i,j;
	static uint8_t datalen =FRAME_LENGHT,offset = 0;  //��������������֡��������
	
	while(1)  //���ǵ����ܽ��յ���֡Ҫ��������
	{		
		length = QueueUARTDataLenGet(&Queue_UART3_Recv);	
		if(length < datalen)   //������֡ͷ��С��7���ֽڣ�����������һ֡
		{	
			//com3_answer_3A3000(RECV_NO);   //Ӧ����յ��ĳ��Ȳ���
			return ;   //�����ȴ�
		}	
		length = datalen;   //������Ҫ�������ֽ���
		for(i=0;i<length;i++)
		{
			//���ﲻ�жϴ����ˣ�ǰ���Ѿ����ȷʵ����ô���ֽڡ�
			QueueUARTDataDele(&Queue_UART3_Recv,com3_data+i+offset) ;  //com_data �ճ�1���ֽڣ�Ϊ�˼���֮ǰ��У����㷨�������ݽ����㷨
		}
	//	com3_data[0] = FRAME_HEAD;  //����֡ͷ����У��ͼ���
		if((com3_data[0] == FRAME_HEAD) && (0 == verify_data(com3_data,FRAME_LENGHT)))   //�ڶ������������ܳ��ȣ�����У��͹�7���ֽ�
		{
			//У�����ȷ��
			//com3_answer_3A3000(RECV_OK);
			com3_message_handle();		
		}	
		else  //У��Ͳ���ȷ��������֡�д���
		{
			for(i=1;i<FRAME_LENGHT;i++)   //ǰ����жϳ����⣬������֡����Ѱ����һ��֡ͷ������
			{
				if(com3_data[i] == FRAME_HEAD)   //�м��ҵ�һ��֡ͷ
				{
					break;
				}
			}		
			if(i != FRAME_LENGHT) //�������м��ҵ�֡ͷ������
			{
				datalen = i;   //��һ����Ҫ�����ֽ���
				offset = FRAME_LENGHT-i;  //�洢��ƫ��λ�õļ���

				for(j=0;i<FRAME_LENGHT;i++,j++)   //�п���֡ͷ���ԣ����Ե�һ���ֽڻ���Ҫ����һ��
				{
					com3_data[j] = com3_data[i];   //��ʣ�µĿ�����ȥ
				}
			}
			else  //�������м�û���ҵ�֡ͷ
			{
				datalen = FRAME_LENGHT;  //	��һ����Ҫ�����ֽ���
				offset = 0;
			}
		}	
	}//end while 1
}


