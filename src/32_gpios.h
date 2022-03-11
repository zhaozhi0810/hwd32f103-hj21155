

#ifndef __32_GPIOS_H__
#define __32_GPIOS_H__

#include <stm32f10x.h>

#define DVI_LOONGSON  1
#define DVI_OTHER     2


typedef enum {
	LS3A_POWEROFF = 1,
	LS3A_REBOOT  =  2,
	LS3A_RUNNING  = 3,
	LS3A_RUN_OS = 4,
	LS3A_PRERUN  =  5
}system_run_status;

#define SWITCH_VIDEO_INTERVAL  1000   //1000即10s，增加对视频切换按键的时间间隔要求，两次需要间隔10s才行。


#define HOT_ENABLE  1    //串口命令，允许加热，不代表开始加热，
#define HOT_DISABLE 2    //不允许加热





extern uint8_t g_fan_pwm;
extern system_run_status cpu_run_status;
extern uint8_t mcu_reboot; //用于单片机重启一次。1重启（不喂狗），0不重启。
/*
	除了18个按键和led外，其他的IO部分，初始化函数			
*/		
//void others_gpio_init(void);


/*
	按键1-16的扫描
	扫描，10ms扫一次，30ms
*/
void btn_power_change_scan(void); // 10ms 调用一次

/*
	任务3，10ms扫描，检测关机和重启
*/
void task_check_run_status(void);


//设置加热状态
void set_heat_status(uint8_t enable);

void dvi_switch_set(uint8_t dvi);
uint8_t dvi_switch_get(void);

//	返回加热允许状态
//uint8_t get_heat_status(void);


int32_t gpio_init_all(void);


//故障灯的控制
void set_bug_led_on(void);
void set_bug_led_off(void);

/* 开启风扇，无调速功能 */
void set_fan_enable(void);
void set_fan_disable(void);

//lcd背光
//void set_LcdLight_disable(void);


//做一个任务，让工作led灯闪烁,1s调用一次
void task_idle_workled(void);

//发送dvi视频被切换的数据到cpu
//source 1（本地）或者2（外部）
void send_dvi_change_to_cpu(int source);


//获得加热状态
uint8_t get_lcd_heat_status(void);

//获得故障灯的状态
uint8_t get_bug_led_status(void);

//获得看门狗的状态
uint8_t get_watchdog_status(void);

void cpu_poweron_enable(void);


//void set_LcdLight_enable(void);

//控制显卡输出 2021-12-08增加
void FPGA_dvi_switch(uint8_t dvi);

//外部中断8，9的处理函数
void exint8_9_handle(void);
void exint0_handle(void);
void exint1_handle(void);

//获取cpu是否上电
//uint8_t get_cpu_power_status(void);


/*通过控制PC13(PG10)来控制CPU关机 -- 低并保持-关机，高-不关机 */
//关机后，只有单片机在运行，其他任务关闭。
void cpu_poweroff_enable(void);
void gpio_init_power_btn(void);
#endif 
