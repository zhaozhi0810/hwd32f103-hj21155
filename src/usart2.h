/*
串口通信协议：
GD32 USART2   <-->	LS7A ttyS1

GD32上的功能:
1.上传按键信息: btn代码-指示按键位置,按键值-指示按下或者松开
2.使能LED:	    led代码-指示led位置,led值-指示亮或者灭
3.调节屏幕亮度: pwm代码,pwm值-0-100
4.切换DVI源:       dvi代码,dvi值-1-2
5.使能风扇:     	fan代码,fan值-1-3,6-8
6.使能屏幕加热: hot代码,hot值-1-2
7.使能屏幕温度上报:tmp代码,tmp值-0-1

串口通信协议:16进制,每帧8字节:帧头2字节,命令类型1字节,命令数据4字节,校验值1字节
帧头(2字节)  
	A5 5A		  

命令类型(1字节)	 
	A1 - 上传按键状态    -- 单片机发
	A3 - 查询按键信息    -- 主CPU发
	A4 - 回复按键状态    -- 单片机发
	B1 - 控制LED状态	 -- 主CPU发
	B3 - 查询LED状态	 -- 主CPU发
	B4 - 回复LED状态	 -- 单片机发

	C1 - 设置PWM
	C3 - 查询PWM

	D1 - 设置DVI
	D3 - 查询DVI

	E1 - 设置fan
	E3 - 查询fan

	F1 - 设置HOT
	F3 - 查询HOT

	F9 - 设置TMP
	FA - 查询TMP
	...

命令数据(4字节,未用填0)
	 CMD		A1		A3	  A4	   B1	  B3	B4
	BYTE3	   code    code  code	  code	 code  code
	BYTE4	   value		 value	  value 	   value
	BYTE5
	BYTE6
校验值(1字节)
	不包括帧头和本字节的前5字节校验和
	
	按键：
	code:从0x20开始 左上0x20-左下0x29,右上0x2a-右下0x31
	value:0-松开，1-按下
	
	LED:
	code:从0x60开始 左上0x40-左下0x49,右上0x4a-右下0x51,ERR_LED-0x60,LOCAL_LED-0x61
	value:0-灭，1-亮

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
串口通信按照发送方向可分为两类:
GD32->LS:
	按键按下后发送:	 A5 5A A1 XX XX 00 00 YY
			 

LS->GD32
	控制LED:	   A5 5A B1 XX XX 00 00 YY
*/


#ifndef __USART_H__
#define __USART_H__

#include <stm32f10x.h>

//
/* 协议 */
#define UART_PROTOCOL_FRAME_LENGTH    8  //帧长度，包括帧头2+命令字1+数据4+校验和1 约695us（115200时）
#define UART_PROTOCOL_HEAD_LENGTH     2  //帧中帧头的长度
#define UART3_PROTOCOL_FRAME_LENGTH    6  //帧长度，包括帧头2+命令字1+数据4+校验和1 约695us（115200时）
#define UART3_PROTOCOL_HEAD_LENGTH     2  //帧中帧头的长度

#define UART_PROTOCOL_HEAD0  0xA5        //帧头
#define UART_PROTOCOL_HEAD1  0x5A

#define UART1_SEND_IRQ    //串口1发送使用中断


#define UART_BUFF_FULL_COVER
#define MAX_QUEUE_UART_LEN (64)  //每个队列缓存空间

typedef struct{
	uint8_t sendIndex;
	uint8_t recvIndex;
	uint8_t length;      //接收到的数据长度，类型跟开辟的空间有关
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

	HOT_CTL    = 0xF1,//加热显示屏
    HOT_QUERY  = 0xF3,
    HOT_REPLY  = 0xF4,

	TMP_CTL    = 0xF9,//温度查询
    TMP_QUERY  = 0xFA,
    TMP_REPLY  = 0xFB,
    
	ANSWER_3A = 0xcc,    //单片机应答3A的串口数据
	
    PROTOCOL_NONE,
}UART_PROTOCOL_TYPE;


#define CODE_PWM  		0x80
#define CODE_DVI  		0x81
#define CODE_FAN  		0x82
#define CODE_HOT  		0x83
#define CODE_TMP  		0x84





//队列中插入一个字节
int32_t QueueUARTDataInsert(Queue_UART_STRUCT *Queue,uint8_t data);
uint32_t QueueUARTDataLenGet(Queue_UART_STRUCT *Queue);
void QueueUARTDataIndexRecover(Queue_UART_STRUCT *Queue);
int32_t QueueUARTDataDele(Queue_UART_STRUCT *Queue,uint8_t *data);


//串口初始化
void  usart2_init_all(uint32_t bandrate);


/*
	串口接收中断：(中断函数中被调用)
		前提：每一帧都是8个字节。
		队列中没有保存帧头，只有后面的数据和校验和（共6个字节）
*/
void com_rne_int_handle(void);

//第二参数是数据总长度
uint8_t checksum(uint8_t *buf, uint8_t len);
//第二参数是数据总长度（包括帧头到最后的校验和），适应不同的帧长计算校验和
int32_t verify_data(uint8_t *data,uint8_t len);


/*
	串口收到命令后的处理。
		前提： 收到完整的数据包，校验和正确。

*/
void com_message_handle(void);
void com1_message_handle(void);
void com3_message_handle(void);




#ifdef UART1_SEND_IRQ    //中断的方式发送
void com1_send_irq(void);
#endif	


//串口3发送led或者按键的信息到龙芯3A
int32_t uart3_protocol_send(UART_PROTOCOL_TYPE type, uint8_t code, uint8_t value);


#endif


