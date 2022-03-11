

#include "includes.h"


/*
	2021-09-18
	
	��������ֲ���ļ���
	
	��ʹ���жϵǼǣ�
	1.systick  1ms�жϣ���ʱ���߶�ʱ����          systick_delay.c  �ж����ȼ� 0��2
//2021-09-30 �ⲿ�ж�2\3�������жϣ���ȡ��
//	2.�ⲿ�ж�2  PC2   ����17                     btns_leds.c   �ж����ȼ� 1��1,˫���ش���  ���º��ɿ������ж� 
//	3.�ⲿ�ж�3  PC3   ����18					  btns_leds.c   �ж����ȼ� 1��1,˫���ش���  ���º��ɿ������ж�
	4.�ⲿ�ж�12 PB12  ����1-9 IIC������          Nca9555.c     �ж����ȼ� 1��1,�½��ش���  ���º��ɿ������ж�
	5.����2�ж�    ���������жϺͿ����ж�         usart2.c      �ж����ȼ� 1��2,
//2021-09-30 ���Ӵ���1��3���ⲿ�жϣ��ⲿ�ж�13	
	6.����1�ж�    ���������жϺͿ����жϣ����ͻ�����ж�         usart1.c      �ж����ȼ� 2��2, ����ʹ�ã����ȼ����
	7.����3�ж�    ���������жϺͿ����ж�					      usart3.c      �ж����ȼ� 1��2, ���ذ���ֵ��
	8.�ⲿ�ж�13 PB13 ����10-18 IIC������         Nca9555.c     �ж����ȼ� 1��1,�½��ش���  ���º��ɿ������ж�

//2021-12-15
	18b20���ܻ�ر��жϣ����ҵ��²�ʹ���жϵģ�����������
*/



/*!
	1ms��ʱ��
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
	�ⲿ�ж�12���ⲿ�ж�13����0-16�Ű������ж�,�ݲ�ʹ��
*/
void EXTI15_10_IRQHandler(void)
{
	if(SET == EXTI_GetITStatus(EXTI_Line12))   //�ⲿ�ж�12������1-9����
	{
		btn_start_scan = 1;   //��ʼɨ�裬��߰��� iic2
		
		//�ö�ʱ��ɨ��ɣ�����
		EXTI_ClearITPendingBit(EXTI_Line12);
	}
	else if(SET == EXTI_GetITStatus(EXTI_Line13))   //�ⲿ�ж�13������10-18����
	{
		btn_start_scan = 10;   //��ʼɨ�裬�ұ߰��� iic1
		
		//�ö�ʱ��ɨ��ɣ�����
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
}


/*
	2021-12-09,
	�ⲿ�ж�0������Ƶ�л��������ж�
*/
void EXTI0_IRQHandler(void)
{
	exint0_handle();
	//�ö�ʱ��ɨ��ɣ�����
	EXTI_ClearFlag(EXTI_Line0);

}


/*
	2021-12-09,
	�ⲿ�ж�1���ǵ�Դ�������ж�
*/
void EXTI1_IRQHandler(void)
{
	exint1_handle();
	//�ö�ʱ��ɨ��ɣ�����
	EXTI_ClearFlag(EXTI_Line1);	
}



/*!
    \brief      ����2���жϴ������������˽����жϺͿ����ж�
				����2��Ҫ����cpu��������ͨ��
    \param[in]  none
    \param[out] none
    \retval     none
*/
//void USART2_IRQHandler(void)
//{	
//	if(USART_GetITStatus(USART2, USART_IT_RXNE))
//	{
//		com_rne_int_handle();
//		USART_ClearITPendingBit(USART2, USART_IT_RXNE);   //���жϱ�־
//	}
//	else if(USART_GetITStatus(USART2, USART_IT_IDLE))  //�����жϣ���ʾһ֡�����ѽ���
//	{
//		//�������������
//		USART_ClearITPendingBit(USART2, USART_IT_IDLE);  //���жϱ�־
//	}
//}


//����״̬�ı��ж�
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
//		USART_ClearITPendingBit(USART1, USART_IT_RXNE);   //���жϱ�־
//	}
//	else if(USART_GetITStatus(USART1, USART_IT_IDLE))  //�����жϣ���ʾһ֡�����ѽ���
//	{
//		//�������������
//		USART_ClearITPendingBit(USART1, USART_IT_IDLE);  //���жϱ�־
//	}
//#ifdef 	UART1_SEND_IRQ
//	else if(USART_GetITStatus(USART1, USART_IT_TXE))  //���ͻ����
//	{
//		com1_send_irq();
//		USART_ClearITPendingBit(USART1, USART_IT_TXE);  //���жϱ�־
//	}
//#endif	
//}



void USART3_IRQHandler(void)
{	
	if(USART_GetITStatus(USART3, USART_IT_RXNE))
	{
		com3_rne_int_handle();
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);   //���жϱ�־
		USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);   //��������ж�,��ʹ�ÿ����ж�
	}
	else if(USART_GetITStatus(USART3, USART_IT_IDLE))  //�����жϣ���ʾһ֡�����ѽ���
	{
		USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);   //��ֹ�����жϣ�ֻҪһ�ο����жϾͺ��ˣ�����
		//�������������
		com3_frame_handle();
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);  //���жϱ�־		
	}
}


