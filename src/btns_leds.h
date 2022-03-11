/*
    Copyright (C) 2016 GigaDevice

    2016-10-19, V1.0.0, firmware for GD32F450I
*/

#ifndef __BTNS_LEDS_H__
#define __BTNS_LEDS_H__

//#ifdef cplusplus
// extern "C" {
//#endif

#include <stm32f10x.h>
#include <stdint.h>

/* 按键检测：10ms中断检测3次 */

/* number of elements in an array */ 
#define NELEMENTS(array)	(sizeof(array) / sizeof((array)[0]))



#define GPIO_HIGH       1
#define GPIO_LOW        0

#define GPIO_INPUT    0
#define GPIO_OUTPUT   1

#define GPIO_DEFAULT_0  GPIO_LOW
#define GPIO_DEFAULT_1  GPIO_HIGH

//LED 低-亮
#define LED_ON   GPIO_LOW
#define LED_OFF  GPIO_HIGH
//按键 低-按下
#define BTN_PRESS    RESET
#define BTN_RELEASE  SET

#define BTN_CODE_START 0xE8     //根据技术文档修改的   2021-09-29
#define LED_CODE_START 0xD0     //自定义的，文档没有要求这个编码


//IIC 设备地址，只考虑低4位（与硬件设置有关），最低位保持为0,整个程序没有左移1位的操作！！！！
#define IIC_BTNS_ADDR (1<<3)   //按键 1000   
//#define IIC_BTNS2_ADDR (3<<1)   //按键
//#define IIC_LEDS1_ADDR 1   //led
#define IIC_LEDS_ADDR 0   //led    0000



//#define BTNS_USE_INT   //按键使用中断方式,还不行2021-12-07

#define TEST_BTN   //测试使用！！！！  按键控制灯


typedef enum{
	BTN1 = 1,    //2021-12-01  修改为1，原来为0
	BTN2,
	BTN3,
	BTN4,
	BTN5,
	BTN6,
	BTN7,
	BTN8,
	BTN9,
	BTN10,
	BTN11,
	BTN12,
	BTN13,
	BTN14,
	BTN15,
	BTN16,
	BTN17,
	BTN18,
    BTN_MAX = BTN18
}BUTTON_NUM;

typedef enum{
	LEDALL = 0,
	LED1,
	LED2,
	LED3,
	LED4,
	LED5,
	LED6,
	LED7,
	LED8,
	LED9,
	LED10,
	LED11,
	LED12,
	LED13,
	LED14,
	LED15,
	LED16,
	LED17,
	LED18,
    LED_MAX
}LED_NUM;

typedef enum{
	LED_FRONT_DVI = 0,
	LED_FRONT_LOCAL,
	LED_FRONT_ERR,
    LED_FRONT_MAX,
}LED_FRONT_NUM;


typedef enum 
{
    MODE_INPUT  = 0,
	MODE_OUTPUT = 1,
    MODE_EXTI   = 2,
	MODE_AF   = 3
} gpio_mode_enum;

typedef enum 
{
	TYPE_NONE = 0,
    TYPE_BTN  = 1,
	TYPE_LED  = 2,
    TYPE_LS7A,
    TYPE_LS3A,
    TYPE_FPGA,
    TYPE_PWR,
    TYPE_FP_LED,//前面板灯
    TYPE_HEAT,
    TYPE_FAN//作为PWM控制
} gpio_type_enum;


//IO端口配置结构体
typedef struct gpio_info{
	GPIO_TypeDef* port;
	uint16_t pin;
	uint8_t state_init;	
	gpio_mode_enum mode;	
}GPIO_INFO;


typedef struct btn_info{
	uint8_t  io;         //编号 0-17
	uint8_t  code;       //按键编码，0xe8开始
	uint8_t  value;	     //值，0表示松开，1表示按下
	uint8_t  reportEn;   //1，消抖检测到了，0没有检测到按键
	uint16_t  pressCnt;     //长按区分
}BTN_INFO;

typedef struct led_info{
	uint8_t  io;
	uint8_t  code;
	uint8_t  value;
}LED_INFO;







extern uint8_t btn_start_scan;




/*
	按键和led的初始化部分
*/
void btns_leds_init(void);


/*
	参数： ledn 参考枚举类型
	返回255是出错，
*/
uint8_t set_led_on(LED_NUM ledn);

/*
	参数： ledn 参考枚举类型
	返回255是出错，
*/
uint8_t set_led_off(LED_NUM ledn);



/*
	18个led同步设置,
	参数：
		ledn 表示设置对应的芯片，对应LED1-LED8表示第一个芯片 ，其他表示第二个芯片
		dat 包含两个字节，对应9个led

	每一位对应一个led
	每一个芯片控制其中9个led，0-8位有效
*/
//uint8_t led1_18_set(uint8_t ledn ,uint8_t* dat);
void led1_18_Set(uint32_t dat);
//uint8_t led1_9_set(void);
//uint8_t led10_18_set(void);


/*
	按键1-16的扫描
	中断触发扫描，10ms扫一次，30ms
*/
void btn_scan(BUTTON_NUM n); // 10ms 调用一次



/*
	main函数调用，按键处理任务
		就是通过串口上报到3A3000
*/
void btn_task_handle(void);



/*
	查询按键的状态
	返回255是出错，0，1表示正常
*/
uint8_t btns_query(BUTTON_NUM n);



uint8_t get_led_value(LED_NUM ledn);




void task1_btn_scan(void);




#endif 
