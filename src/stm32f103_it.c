

#include "includes.h"


/*
	2021-09-18
	
	第六个移植的文件。
	
	已使用中断登记：
	1.systick  1ms中断，延时或者定时任务          systick_delay.c  中断优先级 0，2
//2021-09-30 外部中断2\3（按键中断）已取消
//	2.外部中断2  PC2   按键17                     btns_leds.c   中断优先级 1，1,双边沿触发  按下和松开产生中断 
//	3.外部中断3  PC3   按键18					  btns_leds.c   中断优先级 1，1,双边沿触发  按下和松开产生中断
	4.外部中断12 PB12  按键1-9 IIC控制器          Nca9555.c     中断优先级 1，1,下降沿触发  按下和松开产生中断
	5.串口2中断    接收数据中断和空闲中断         usart2.c      中断优先级 1，2,
//2021-09-30 增加串口1，3的外部中断，外部中断13	
	6.串口1中断    接收数据中断和空闲中断，发送缓存空中断         usart1.c      中断优先级 2，2, 调试使用，优先级最低
	7.串口3中断    接收数据中断和空闲中断					      usart3.c      中断优先级 1，2, 返回按键值，
	8.外部中断13 PB13 按键10-18 IIC控制器         Nca9555.c     中断优先级 1，1,下降沿触发  按下和松开产生中断

//2021-12-15
	18b20可能会关闭中断，并且导致不使能中断的！！！！！！
*/



/*!
	1ms定时器
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
    /* update the g_localtime by adding SYSTEMTICK_PERIOD_MS each SysTick interrupt */
    systick_int_update();
}


/*
	外部中断12，外部中断13，是0-16号按键的中断,暂不使用
*/
void EXTI15_10_IRQHandler(void)
{
	if(SET == EXTI_GetITStatus(EXTI_Line12))   //外部中断12，按键1-9触发
	{
		btn_start_scan = 1;   //开始扫描，左边按键 iic2
		
		//用定时器扫描吧，消抖
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
	else if(SET == EXTI_GetITStatus(EXTI_Line13))   //外部中断13，按键10-18触发
	{
		btn_start_scan = 10;   //开始扫描，右边按键 iic1
		
		//用定时器扫描吧，消抖
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
}


/*
	2021-12-09,
	外部中断0，是视频切换按键的中断
*/
void EXTI0_IRQHandler(void)
{
	exint0_handle();
	//用定时器扫描吧，消抖
	EXTI_ClearFlag(EXTI_Line0);

}


/*
	2021-12-09,
	外部中断1，是电源按键的中断
*/
void EXTI1_IRQHandler(void)
{
	exint1_handle();
	//用定时器扫描吧，消抖
	EXTI_ClearFlag(EXTI_Line1);	
}



/*!
    \brief      串口2的中断处理函数，开启了接收中断和空闲中断
				串口2主要是与cpu进行数据通信
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void USART2_IRQHandler(void)
//{	
//	if(USART_GetITStatus(USART2, USART_IT_RXNE))
//	{
//		com_rne_int_handle();
//		USART_ClearITPendingBit(USART2, USART_IT_RXNE);   //清中断标志
//	}
//	else if(USART_GetITStatus(USART2, USART_IT_IDLE))  //空闲中断，表示一帧数据已结束
//	{
//		//解析命令，并处理。
//		USART_ClearITPendingBit(USART2, USART_IT_IDLE);  //清中断标志
//	}
//}


//运行状态改变中断
void EXTI9_5_IRQHandler(void)
{
	if(SET == EXTI_GetITStatus(EXTI_Line8) || SET == EXTI_GetITStatus(EXTI_Line9))
	{	
		exint8_9_handle();
		EXTI_ClearITPendingBit(EXTI_Line8);
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
}




//void USART1_IRQHandler(void)
//{	
//	if(USART_GetITStatus(USART1, USART_IT_RXNE))
//	{
//	//	com1_rne_int_handle();
//		USART_ClearITPendingBit(USART1, USART_IT_RXNE);   //清中断标志
//	}
//	else if(USART_GetITStatus(USART1, USART_IT_IDLE))  //空闲中断，表示一帧数据已结束
//	{
//		//解析命令，并处理。
//		USART_ClearITPendingBit(USART1, USART_IT_IDLE);  //清中断标志
//	}
//#ifdef 	UART1_SEND_IRQ
//	else if(USART_GetITStatus(USART1, USART_IT_TXE))  //发送缓存空
//	{
//		com1_send_irq();
//		USART_ClearITPendingBit(USART1, USART_IT_TXE);  //清中断标志
//	}
//#endif	
//}



void USART3_IRQHandler(void)
{	
	if(USART_GetITStatus(USART3, USART_IT_RXNE))
	{
		com3_rne_int_handle();
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);   //清中断标志
		USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);   //允许空闲中断,不使用空闲中断
	}
	else if(USART_GetITStatus(USART3, USART_IT_IDLE))  //空闲中断，表示一帧数据已结束
	{
		USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);   //禁止空闲中断，只要一次空闲中断就好了！！！
		//解析命令，并处理。
		com3_frame_handle();
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);  //清中断标志		
	}
}


