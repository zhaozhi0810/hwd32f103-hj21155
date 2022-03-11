/*
����ͨ��Э�飺
GD32 USART2   <-->	LS7A ttyS1

GD32�ϵĹ���:
1.�ϴ�������Ϣ: btn����-ָʾ����λ��,����ֵ-ָʾ���»����ɿ�
2.ʹ��LED:	    led����-ָʾledλ��,ledֵ-ָʾ��������
3.������Ļ����: pwm����,pwmֵ-0-100
4.�л�DVIԴ:       dvi����,dviֵ-1-2
5.ʹ�ܷ���:     	fan����,fanֵ-1-3,6-8
6.ʹ����Ļ����: hot����,hotֵ-1-2
7.ʹ����Ļ�¶��ϱ�:tmp����,tmpֵ-0-1

����ͨ��Э��:16����,ÿ֡8�ֽ�:֡ͷ2�ֽ�,��������1�ֽ�,��������4�ֽ�,У��ֵ1�ֽ�
֡ͷ(2�ֽ�)  
	A5 5A		  

��������(1�ֽ�)	 
	A1 - �ϴ�����״̬    -- ��Ƭ����
	A3 - ��ѯ������Ϣ    -- ��CPU��
	A4 - �ظ�����״̬    -- ��Ƭ����
	B1 - ����LED״̬	 -- ��CPU��
	B3 - ��ѯLED״̬	 -- ��CPU��
	B4 - �ظ�LED״̬	 -- ��Ƭ����

	C1 - ����PWM
	C3 - ��ѯPWM

	D1 - ����DVI
	D3 - ��ѯDVI

	E1 - ����fan
	E3 - ��ѯfan

	F1 - ����HOT
	F3 - ��ѯHOT

	F9 - ����TMP
	FA - ��ѯTMP
	...

��������(4�ֽ�,δ����0)
	 CMD		A1		A3	  A4	   B1	  B3	B4
	BYTE3	   code    code  code	  code	 code  code
	BYTE4	   value		 value	  value 	   value
	BYTE5
	BYTE6
У��ֵ(1�ֽ�)
	������֡ͷ�ͱ��ֽڵ�ǰ5�ֽ�У���
	
	������
	code:��0x20��ʼ ����0x20-����0x29,����0x2a-����0x31
	value:0-�ɿ���1-����
	
	LED:
	code:��0x60��ʼ ����0x40-����0x49,����0x4a-����0x51,ERR_LED-0x60,LOCAL_LED-0x61
	value:0-��1-��

	PWM:
	code:0x80
	value:0-100
	
	DVI:
	code:0x81
	value:1-local,2-other
	
	FAN: 
	code:0x82
	value:1-enable fan1,2-enable fan2,3-enable all,6-disable fan1,7-disable fan2,8-disable all
	
	HOT:
	code:0x83
	value:1-enable hot,other-disable hot
	
	TMP:
	code:0x84
	value:0-disable temperature report,1-enable temperature report
����ͨ�Ű��շ��ͷ���ɷ�Ϊ����:
GD32->LS:
	�������º���:	 A5 5A A1 XX XX 00 00 YY
			 

LS->GD32
	����LED:	   A5 5A B1 XX XX 00 00 YY
*/


#ifndef __USART_H__
#define __USART_H__

#include <stm32f10x.h>

//
/* Э�� */
#define UART_PROTOCOL_FRAME_LENGTH    8  //֡���ȣ�����֡ͷ2+������1+����4+У���1 Լ695us��115200ʱ��
#define UART_PROTOCOL_HEAD_LENGTH     2  //֡��֡ͷ�ĳ���
#define UART3_PROTOCOL_FRAME_LENGTH    6  //֡���ȣ�����֡ͷ2+������1+����4+У���1 Լ695us��115200ʱ��
#define UART3_PROTOCOL_HEAD_LENGTH     2  //֡��֡ͷ�ĳ���

#define UART_PROTOCOL_HEAD0  0xA5        //֡ͷ
#define UART_PROTOCOL_HEAD1  0x5A

#define UART1_SEND_IRQ    //����1����ʹ���ж�


#define UART_BUFF_FULL_COVER
#define MAX_QUEUE_UART_LEN (64)  //ÿ�����л���ռ�

typedef struct{
	uint8_t sendIndex;
	uint8_t recvIndex;
	uint8_t length;      //���յ������ݳ��ȣ����͸����ٵĿռ��й�
	uint8_t dataBuf[MAX_QUEUE_UART_LEN];
}Queue_UART_STRUCT;


typedef enum{
	BTN_REPORT  = 0xA1,
    BTN_QUERY   = 0xA6,
    BTN_REPLY   = 0xAA,
    
    LED_CTL    = 0xB1,
    LED_QUERY  = 0xB6,
    LED_REPLY  = 0xBA,

	PWM_CTL    = 0xC1,
    PWM_QUERY  = 0xC3,
    PWM_REPLY  = 0xC4,

	DVI_CTL    = 0xD1,
    DVI_QUERY  = 0xD3,
    DVI_REPLY  = 0xD4,

	FAN_CTL    = 0xE1,
    FAN_QUERY  = 0xE3,
    FAN_REPLY  = 0xE4,

	HOT_CTL    = 0xF1,//������ʾ��
    HOT_QUERY  = 0xF3,
    HOT_REPLY  = 0xF4,

	TMP_CTL    = 0xF9,//�¶Ȳ�ѯ
    TMP_QUERY  = 0xFA,
    TMP_REPLY  = 0xFB,
    
	ANSWER_3A = 0xcc,    //��Ƭ��Ӧ��3A�Ĵ�������
	
    PROTOCOL_NONE,
}UART_PROTOCOL_TYPE;


#define CODE_PWM  		0x80
#define CODE_DVI  		0x81
#define CODE_FAN  		0x82
#define CODE_HOT  		0x83
#define CODE_TMP  		0x84





//�����в���һ���ֽ�
int32_t QueueUARTDataInsert(Queue_UART_STRUCT *Queue,uint8_t data);
uint32_t QueueUARTDataLenGet(Queue_UART_STRUCT *Queue);
void QueueUARTDataIndexRecover(Queue_UART_STRUCT *Queue);
int32_t QueueUARTDataDele(Queue_UART_STRUCT *Queue,uint8_t *data);


//���ڳ�ʼ��
void  usart2_init_all(uint32_t bandrate);


/*
	���ڽ����жϣ�(�жϺ����б�����)
		ǰ�᣺ÿһ֡����8���ֽڡ�
		������û�б���֡ͷ��ֻ�к�������ݺ�У��ͣ���6���ֽڣ�
*/
void com_rne_int_handle(void);

//�ڶ������������ܳ���
uint8_t checksum(uint8_t *buf, uint8_t len);
//�ڶ������������ܳ��ȣ�����֡ͷ������У��ͣ�����Ӧ��ͬ��֡������У���
int32_t verify_data(uint8_t *data,uint8_t len);


/*
	�����յ������Ĵ���
		ǰ�᣺ �յ����������ݰ���У�����ȷ��

*/
void com_message_handle(void);
void com1_message_handle(void);
void com3_message_handle(void);




#ifdef UART1_SEND_IRQ    //�жϵķ�ʽ����
void com1_send_irq(void);
#endif	


//����3����led���߰�������Ϣ����о3A
int32_t uart3_protocol_send(UART_PROTOCOL_TYPE type, uint8_t code, uint8_t value);


#endif


