#include "stm32f10x.h"                  // Device header
#include "FreeRTOS.h"
#include "queue.h"

QueueHandle_t keyQueue_Handle;

void Key_Init(void)
{
	//使能时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	//配置GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	//配置一个用来Debug的led外设
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period =   750-1;//周期，计数器
	TIM_TimeBaseStructure.TIM_Prescaler =  720-1;//预分频，大齿轮
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; //高级定时器才设置
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	
	//打开定时器中断，每1ms执行检测程序一次
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	//开始配置中断
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2,ENABLE);
}

void TIM2_IRQHandler(void)
{
	int key_event;
	static uint8_t key1_shark = 0; //消抖变量
	static uint8_t key2_shark = 0; //消抖变量
	static uint8_t key3_shark = 0; //消抖变量
	static uint8_t key4_shark = 0; //消抖变量
	
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)!= RESET)
	{
		
		//按键1按下的判断
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5) == 1)
		{
			if(key1_shark<15)//检测15次（检测15ms）消抖
			{key1_shark ++;}
			else    //第16ms确定按键被按下
			{
				key1_shark = 0;
				key_event = 1;
				xQueueSendToBackFromISR(keyQueue_Handle,&key_event,NULL); //中断服务程序向队列发送一个数据：按键的值
				key_event = 0;
			}
		}
		
		//按键2按下的判断
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6) == 1)
		{
			if(key2_shark<15)//检测15次（检测15ms）消抖
			{key2_shark ++;}
			else    //第16ms确定按键被按下
			{
				key2_shark = 0;
				key_event = 2;
//				GPIO_SetBits(GPIOA,GPIO_Pin_2);
				xQueueSendToBackFromISR(keyQueue_Handle,&key_event,NULL); //中断服务程序向队列发送一个数据：按键的值
				key_event = 0;
			}
		}
		
		//按键3按下的判断
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7) == 1)
		{
			if(key3_shark<15)//检测15次（检测15ms）消抖
			{key3_shark ++;}
			else    //第16ms确定按键被按下
			{
				key3_shark = 0;
				key_event = 3;
//				GPIO_SetBits(GPIOA,GPIO_Pin_2);
				xQueueSendToBackFromISR(keyQueue_Handle,&key_event,NULL); //中断服务程序向队列发送一个数据：按键的值
				key_event = 0;
			}
		}
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0) == 1)
		{
			if(key4_shark<15)//检测15次（检测15ms）消抖
			{key4_shark ++;}
			else    //第16ms确定按键被按下
			{
				key4_shark = 0;
				key_event = 4;
//				GPIO_SetBits(GPIOA,GPIO_Pin_2);
				xQueueSendToBackFromISR(keyQueue_Handle,&key_event,NULL); //中断服务程序向队列发送一个数据：按键的值
				key_event = 0;
			}
		}
	}
	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //清除中断标志位
}



