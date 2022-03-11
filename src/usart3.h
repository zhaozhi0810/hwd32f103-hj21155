/*
串口通信协议：
HW32 USART0   <-->	空
HW32 USART2   <-->	LS7A ttyS0
HW32 USART3   <-->	LS7A ttyS1

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


#ifndef __USART3_H__
#define __USART3_H__

#include <stm32f10x.h>

//
/* 协议 */
#define UART_PROTOCOL_FRAME_LENGTH    8  //帧长度，包括帧头2+命令字1+数据4+校验和1 约695us（115200时）
#define UART_PROTOCOL_HEAD_LENGTH     2  //帧中帧头的长度
#define UART3_PROTOCOL_FRAME_LENGTH    7  //帧长度，包括帧头1+命令字1+数据4+校验和1 约695us（115200时）
#define UART3_PROTOCOL_HEAD_LENGTH     1  //帧中帧头的长度

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







typedef enum
{	
	eMCU_LED_STATUS_TYPE=0,  //获得led的状态
	eMCU_KEY_STATUS_TYPE,    //获得按键的状态
	eMCU_FAN_DIV_BK_TYPE,    //获得风扇，视频源，故障灯的状态
	eMCU_CB_TEMP_TYPE,       //获得cpu和主板温度
	//eMCU_LCD_TYPE,			//获得lcd的温度，加热状态，亮度值
	eMCU_VOL_TYPE,			//获得三个电压值
	eMCU_CMD_TYPE,          //命令模式
	eMCU_LED_SETON_TYPE,    //设置对应的led亮
	eMCU_LED_SETOFF_TYPE,    //设置对应的led灭
	eMCU_DVI_SETSRC_TYPE,
	eMCU_LCD_SETPWM_TYPE,
	eMCU_KEY_CHANGE_TYPE,    //按键被修改上报
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
	eMCU_ANSWER_CMD,      //应答命令
	eMCU_SWITCH_DVI_SRC_CMD,
	eMCU_LCD_SETPWM_UP_CMD,    //背光增亮
	eMCU_LCD_SETPWM_DN_CMD     //背光变暗
}mcu_cmd_type;



typedef struct
{
	unsigned char dvi_src:1; 		//0本地，1外部
	unsigned char breakdownLed_status:1;   //0熄灭，1点亮
	unsigned char watch_dog_status:1; //硬件看门狗状态，0关闭，1开启
	unsigned char lcd_beat:1;   //lcd加热状态	
}bitstatus_t;

//2021-12-15,修改改结构体
typedef struct {
		unsigned char fan_pwm;      //0-100
		unsigned char lcd_pwm;
		bitstatus_t bstatus;		
	}fan_div_bk_t;


typedef struct {
		short cpu_t;
		short board_t;
	}cb_temp_t;

//2021-12-15,修改改结构体
typedef struct{
		short lcd_temp1;
		short lcd_temp2;
}lcd_temp_t;


typedef struct{
		unsigned int vol_core : 10;     //三个电压正常在1.1v左右，adc得出结果12位，去掉最高位，去掉最低位，保留中间10位
		unsigned int vol_7A : 10;
		unsigned int vol_12V : 10;
	}vol_t;


typedef struct {
		unsigned char cmd;      //串口关机，重启命令，视频切换，应答
		unsigned char param_len;      //有几个字节参数，0-3，0表示没有参数
		unsigned char param1;   //有参数的话，设置参数值，无参数时忽略。！！！！应答时，表示应答结果，0表示正常，非零表示错误
		unsigned char param2;
//		unsigned char param3;
	}cmd_t;


typedef struct
{
		unsigned char whichkey; //哪一个按键
		unsigned char status;   //什么状态，0松开，1按下
}keys_status_t;


typedef union 
{
	unsigned int leds_status;   //低18位  0熄灭，1点亮
	unsigned int keys_status;   //低18位  0松开，1按下
	keys_status_t key_change;   //按键被修改
	fan_div_bk_t fan_div_bk;
	cb_temp_t cb_temp;
	lcd_temp_t lcd;
	vol_t vol;
	cmd_t cmd;
}mcu_data_t;


//#pragma pack(1) 这个会设置全局的，注释掉

typedef struct
{
	unsigned char data_type;
	mcu_data_t data;
	unsigned char crc;     //校验和
}__attribute__((packed))com_frame_t;    //注意对齐方式



#define FRAME_HEAD 0xa5







//队列中插入一个字节
int32_t QueueUARTDataInsert(Queue_UART_STRUCT *Queue,uint8_t data);
uint32_t QueueUARTDataLenGet(Queue_UART_STRUCT *Queue);
void QueueUARTDataIndexRecover(Queue_UART_STRUCT *Queue);
int32_t QueueUARTDataDele(Queue_UART_STRUCT *Queue,uint8_t *data);


//串口初始化
//void  usart2_init_all(uint32_t bandrate);


/*
	串口接收中断：(中断函数中被调用)
		前提：每一帧都是8个字节。
		队列中没有保存帧头，只有后面的数据和校验和（共6个字节）
*/
//void com_rne_int_handle(void);

//第二参数是数据总长度
uint8_t checksum(uint8_t *buf, uint8_t len);
//第二参数是数据总长度（包括帧头到最后的校验和），适应不同的帧长计算校验和
int32_t verify_data(uint8_t *data,uint8_t len);


/*
	串口收到命令后的处理。
		前提： 收到完整的数据包，校验和正确。

*/
//void com_message_handle(void);
//void com1_message_handle(void);
//void com3_message_handle(void);




//串口3发送led或者按键的信息到龙芯3A
int32_t uart3_protocol_send(UART_PROTOCOL_TYPE type, uint8_t code, uint8_t value);

void com3_rne_int_handle(void);

//串口3初始化，参数为波特率
void  usart3_init_all(uint32_t bandrate);



//发送dvi视频被切换的数据到cpu
//whichkey 0xe8 - 0xf9  keycode
//status 0 or 1 松开或者按下
void send_btn_change_to_cpu(int whichkey,int status);

//发送命令数据到cpu
//cmd请参考usart3.h中宏定义
//param 参数。
void send_cmd_to_cpu(int cmd,int param);


//发送主板和cpu温度
void send_bc_temp_to_cpu(short cpu_temp,short board_temp);
//发送3个电压
void send_vol_to_cpu(short vol_core,short vol_7A,short vol_12v);

//发送lcd的温度，pwm，加热使能状态
//void send_lcd_status_to_cpu(short lcd_t,uint8_t lcd_pwm,uint8_t lcd_beat);
//定时发送风扇状态，dvi和故障灯状态
void send_fan_bk_div_status_to_cpu(bitstatus_t b_status,uint8_t fan_pwm,uint8_t lcd_pwm);

//发送led状态的数据到cpu
//status表示18个led，(低18位有效)
void send_leds_status_to_cpu(int status);

void com3_frame_handle(void);

//发送lcd温度
void send_lcd_temp_to_cpu(short lcd_t1,short lcd_t2);


//uart3去初始化，两个引脚输出低电平 2021-12-17 增加 关机时调用
void usart3_deinit(void);
#endif


