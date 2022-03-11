

#include "includes.h"

/*
	2021-09-16
	
	1. 第一个移植的文件。
*/



static __IO uint32_t g_localtime = 0; /* for creating a time reference incremented by 1ms */

static uint32_t ticks = 0;
static uint32_t tflag = 0;




/*!
    \brief      insert a delay time
    \param[in]  ncount: number of 10ms periods to wait for
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t ncount)
{
    /* capture the current local time */
	uint32_t tEnd = g_localtime + ncount;
	
	/* wait until the desired delay finish */  
	while (tEnd != g_localtime);	
}


static u8  fac_us=72;	//systick多少频率		72MHz 没有8分频			//us延时倍乘数
//59652323ns，59652ms，59s
void delay_us(uint32_t nus)
{
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD的值
	ticks=nus*fac_us; 						//需要的节拍数
//	delay_osschedlock();					//阻止OS调度，防止打断us延时
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;
		if(tnow!=told)
		{
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}
	}
}



/*!
    \brief      updates the system local time
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_int_update(void)
{
    static uint16_t times = 0;
	g_localtime ++;
	
	//控制任务的执行
	if(g_localtime % TASK1_TICKS_INTERVAL == 0)
	{
		task_id |= 1;  //任务1，视频切换开关和上电开关扫描
	}
			
	if(g_localtime % TASK16_TICKS_INTERVAL == 10)
	{
		task_id |= 1<<15;   //任务16，433ms喂狗
	}
		
	if(cpu_run_status > LS3A_POWEROFF)   //非关机模式扫描
	{
		if(cpu_run_status == LS3A_RUN_OS)
		{
			if(g_localtime % TASK2_TICKS_INTERVAL == 2)
			{
				task_id |= 2;   //任务2，18个按键扫描
			}
					
			if(g_localtime % TASK5_TICKS_INTERVAL == 0)
			{
				task_id |= (1<<4);   //任务5，500ms扫描，定时汇报温度等任务
			}
		}		
		//读取温度，为之后的屏幕加热做准备，2021-12-15
		if(g_localtime % TASK4_TICKS_INTERVAL == 0)
		{
			task_id |= 8;   //任务4，200ms扫描，温湿度，电压监控读取任务
		}		
	}
	
	//开机状态扫描由2022-01-04修改
	if(SET == GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_13))   //上电后 2022-01-04增加。v1.2 屏蔽前1.8秒的状态
	{		
		if(times < 5000)	//屏蔽上电后1.5秒的状态 2022-01-04增加。v1.5 屏蔽上电后1.8秒的状态
		{				//屏蔽上电后5秒的状态 2022-01-27修改。v1.6 屏蔽上电后5秒的状态
			times++;
		}
		else if(g_localtime % TASK3_TICKS_INTERVAL == 4)
		{
			task_id |= 4;   //任务3，10ms扫描，检测关机和重启
		}		
	}
	else if(times) //屏蔽上电后1.5秒的状态，2022-01-17  v1.5增加。
	{
		times = 0;
	}
	
	
//	if(g_localtime % TASK16_TICKS_INTERVAL == 0)
//	{
//		task_id |= (1<<15);   //任务16，1s扫描，工作led闪烁任务
//	}
		
   
}



#if 0
/*!
    \brief      task ticks increase，
//风险：：如果执行的任务超过1ms，但是这个tick并没有及时增加。受执行任务的压力影响。
    \param[in]  none
    \param[out] none
    \retval     none
*/
void tasktick_increase(void)
{
    static uint32_t delay2tick = 0;

    if(delay2tick != g_localtime)
    {
        delay2tick = g_localtime;
        tflag = 1;
        ticks++;
    }
    else
    {
        tflag = 0;
    }
}

/*!
    \brief      loop a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
int8_t tasktick_1ms(uint32_t count)
{
    if(((ticks % count) == 0) && tflag)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     0 ： 成功 ； 非0： 失败
*/
int systick_config(void)
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);    //直接是HCLK时钟，
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U )){
        /* capture error */
        return 1;     //设置失败
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x02U);
	
	return 0;
}



