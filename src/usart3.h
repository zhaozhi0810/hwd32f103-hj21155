/*
����ͨ��Э�飺
HW32 USART0   <-->	��
HW32 USART2   <-->	LS7A ttyS0
HW32 USART3   <-->	LS7A ttyS1

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


#ifndef __USART3_H__
#define __USART3_H__

#include <stm32f10x.h>

//
/* Э�� */
#define UART_PROTOCOL_FRAME_LENGTH    8  //֡���ȣ�����֡ͷ2+������1+����4+У���1 Լ695us��115200ʱ��
#define UART_PROTOCOL_HEAD_LENGTH     2  //֡��֡ͷ�ĳ���
#define UART3_PROTOCOL_FRAME_LENGTH    7  //֡���ȣ�����֡ͷ1+������1+����4+У���1 Լ695us��115200ʱ��
#define UART3_PROTOCOL_HEAD_LENGTH     1  //֡��֡ͷ�ĳ���

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







typedef enum
{	
	eMCU_LED_STATUS_TYPE=0,  //���led��״̬
	eMCU_KEY_STATUS_TYPE,    //��ð�����״̬
	eMCU_FAN_DIV_BK_TYPE,    //��÷��ȣ���ƵԴ�����ϵƵ�״̬
	eMCU_CB_TEMP_TYPE,       //���cpu�������¶�
	//eMCU_LCD_TYPE,			//���lcd���¶ȣ�����״̬������ֵ
	eMCU_VOL_TYPE,			//���������ѹֵ
	eMCU_CMD_TYPE,          //����ģʽ
	eMCU_LED_SETON_TYPE,    //���ö�Ӧ��led��
	eMCU_LED_SETOFF_TYPE,    //���ö�Ӧ��led��
	eMCU_DVI_SETSRC_TYPE,
	eMCU_LCD_SETPWM_TYPE,
	eMCU_KEY_CHANGE_TYPE,    //�������޸��ϱ�
    eMCU_BKLEDSET_TYPE,
    eMCU_LEDSETALL_TYPE,
    eMCU_DIV_CHANGE_TYPE,
    eMCU_COREVOL_CHANGE_TYPE,
    eMCU_7AVOL_CHANGE_TYPE,
    eMCU_12VVOL_CHANGE_TYPE,
	eMCU_LCD_TEMP_TYPE
}data_type;



typedef enum
{
	eMCU_REBOOT_CMD = 0,
	eMCU_SHUTDOWN_CMD,
	eMCU_ANSWER_CMD,      //Ӧ������
	eMCU_SWITCH_DVI_SRC_CMD,
	eMCU_LCD_SETPWM_UP_CMD,    //��������
	eMCU_LCD_SETPWM_DN_CMD     //����䰵
}mcu_cmd_type;



typedef struct
{
	unsigned char dvi_src:1; 		//0���أ�1�ⲿ
	unsigned char breakdownLed_status:1;   //0Ϩ��1����
	unsigned char watch_dog_status:1; //Ӳ�����Ź�״̬��0�رգ�1����
	unsigned char lcd_beat:1;   //lcd����״̬	
}bitstatus_t;

//2021-12-15,�޸ĸĽṹ��
typedef struct {
		unsigned char fan_pwm;      //0-100
		unsigned char lcd_pwm;
		bitstatus_t bstatus;		
	}fan_div_bk_t;


typedef struct {
		short cpu_t;
		short board_t;
	}cb_temp_t;

//2021-12-15,�޸ĸĽṹ��
typedef struct{
		short lcd_temp1;
		short lcd_temp2;
}lcd_temp_t;


typedef struct{
		unsigned int vol_core : 10;     //������ѹ������1.1v���ң�adc�ó����12λ��ȥ�����λ��ȥ�����λ�������м�10λ
		unsigned int vol_7A : 10;
		unsigned int vol_12V : 10;
	}vol_t;


typedef struct {
		unsigned char cmd;      //���ڹػ������������Ƶ�л���Ӧ��
		unsigned char param_len;      //�м����ֽڲ�����0-3��0��ʾû�в���
		unsigned char param1;   //�в����Ļ������ò���ֵ���޲���ʱ���ԡ���������Ӧ��ʱ����ʾӦ������0��ʾ�����������ʾ����
		unsigned char param2;
//		unsigned char param3;
	}cmd_t;


typedef struct
{
		unsigned char whichkey; //��һ������
		unsigned char status;   //ʲô״̬��0�ɿ���1����
}keys_status_t;


typedef union 
{
	unsigned int leds_status;   //��18λ  0Ϩ��1����
	unsigned int keys_status;   //��18λ  0�ɿ���1����
	keys_status_t key_change;   //�������޸�
	fan_div_bk_t fan_div_bk;
	cb_temp_t cb_temp;
	lcd_temp_t lcd;
	vol_t vol;
	cmd_t cmd;
}mcu_data_t;


//#pragma pack(1) ���������ȫ�ֵģ�ע�͵�

typedef struct
{
	unsigned char data_type;
	mcu_data_t data;
	unsigned char crc;     //У���
}__attribute__((packed))com_frame_t;    //ע����뷽ʽ



#define FRAME_HEAD 0xa5







//�����в���һ���ֽ�
int32_t QueueUARTDataInsert(Queue_UART_STRUCT *Queue,uint8_t data);
uint32_t QueueUARTDataLenGet(Queue_UART_STRUCT *Queue);
void QueueUARTDataIndexRecover(Queue_UART_STRUCT *Queue);
int32_t QueueUARTDataDele(Queue_UART_STRUCT *Queue,uint8_t *data);


//���ڳ�ʼ��
//void  usart2_init_all(uint32_t bandrate);


/*
	���ڽ����жϣ�(�жϺ����б�����)
		ǰ�᣺ÿһ֡����8���ֽڡ�
		������û�б���֡ͷ��ֻ�к�������ݺ�У��ͣ���6���ֽڣ�
*/
//void com_rne_int_handle(void);

//�ڶ������������ܳ���
uint8_t checksum(uint8_t *buf, uint8_t len);
//�ڶ������������ܳ��ȣ�����֡ͷ������У��ͣ�����Ӧ��ͬ��֡������У���
int32_t verify_data(uint8_t *data,uint8_t len);


/*
	�����յ������Ĵ���
		ǰ�᣺ �յ����������ݰ���У�����ȷ��

*/
//void com_message_handle(void);
//void com1_message_handle(void);
//void com3_message_handle(void);




//����3����led���߰�������Ϣ����о3A
int32_t uart3_protocol_send(UART_PROTOCOL_TYPE type, uint8_t code, uint8_t value);

void com3_rne_int_handle(void);

//����3��ʼ��������Ϊ������
void  usart3_init_all(uint32_t bandrate);



//����dvi��Ƶ���л������ݵ�cpu
//whichkey 0xe8 - 0xf9  keycode
//status 0 or 1 �ɿ����߰���
void send_btn_change_to_cpu(int whichkey,int status);

//�����������ݵ�cpu
//cmd��ο�usart3.h�к궨��
//param ������
void send_cmd_to_cpu(int cmd,int param);


//���������cpu�¶�
void send_bc_temp_to_cpu(short cpu_temp,short board_temp);
//����3����ѹ
void send_vol_to_cpu(short vol_core,short vol_7A,short vol_12v);

//����lcd���¶ȣ�pwm������ʹ��״̬
//void send_lcd_status_to_cpu(short lcd_t,uint8_t lcd_pwm,uint8_t lcd_beat);
//��ʱ���ͷ���״̬��dvi�͹��ϵ�״̬
void send_fan_bk_div_status_to_cpu(bitstatus_t b_status,uint8_t fan_pwm,uint8_t lcd_pwm);

//����led״̬�����ݵ�cpu
//status��ʾ18��led��(��18λ��Ч)
void send_leds_status_to_cpu(int status);

void com3_frame_handle(void);

//����lcd�¶�
void send_lcd_temp_to_cpu(short lcd_t1,short lcd_t2);


//uart3ȥ��ʼ����������������͵�ƽ 2021-12-17 ���� �ػ�ʱ����
void usart3_deinit(void);
#endif


