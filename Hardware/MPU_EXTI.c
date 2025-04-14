#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "MPU6050.h"
#include "Delay.h"


//uint8_t Spcount = 0;


void MPU_EXTI_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//AFIO用于配置外部中断引脚
	
	/*GPIO配置*/
	GPIO_InitTypeDef GPIO_InitStructure;				//GPIO结构体配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//上拉输入模式(默认高电平)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;			//PB14口
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);				//初始化GPIOB外设
	
	/*AFIO外部中断引脚配置*/
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);//选择MPU的INT接的PB14作为外部中断引脚
	
	/*EXTI配置*/
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	/*NVIC配置*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//分组2,即2位抢占2位响应
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;//10到15的通道合并为此通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//中断通道使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =2;//抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;//响应优先级2
	NVIC_Init(&NVIC_InitStructure);
		
}

/**
  * @brief 中断线14,PB14中断函数,当MPU6050数据变化时,进入中断
  * @param  无
  * @retval 无
  */
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line14) == SET)//确保是PB14进来的中断
	{
		/*数据接收*/
//		Spcount = Step_Detection();
//		if(Fall_Detection() == 1)
//		{
//			
//		}
		EXTI_ClearITPendingBit(EXTI_Line14);//清除中断标志位
	}	
}


