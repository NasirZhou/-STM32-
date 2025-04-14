#include "stm32f10x.h"                  // Device header
#include "MyRTC.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "beep.h"

QueueHandle_t alarmQueue_Handle;
SemaphoreHandle_t beepSemaphore;

//定时器3每0.5s检测一次设定时间是否和当前时间重合
void Alarm_Init(void)
{
//	//使能时钟
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	
//	//配置GPIO
//	GPIO_InitTypeDef GPIO_InitStructure;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA,&GPIO_InitStructure);
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//	GPIO_Init(GPIOB,&GPIO_InitStructure);
//	
//	//配置一个用来Debug的led外设
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOA,&GPIO_InitStructure);
//	GPIO_ResetBits(GPIOA,GPIO_Pin_2);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period =   5000-1;//周期，计数器
	TIM_TimeBaseStructure.TIM_Prescaler =  7200-1;//预分频，大齿轮
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0; //高级定时器才设置
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);
	
	//打开定时器中断，每1ms执行检测程序一次
	//TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	//开始配置中断
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM3,ENABLE);
}


void TIM3_IRQHandler(void)
{
	int i;
	static int Alarm_time[3];
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        MyRTC_ReadTime();
        if(alarmQueue_Handle !=NULL)
        {
            for(i=0;i<3;i++)
            {
                xQueueReceiveFromISR(alarmQueue_Handle,&Alarm_time[i],NULL); //中断服务程序向队列接收三个数据，闹钟的时间
            }
        }
        if(Alarm_time[0] ==  MyRTC_Time[3] && Alarm_time[1] ==  MyRTC_Time[4] && Alarm_time[2] ==  MyRTC_Time[5])
        {
            //如果设定时间对比成功则蜂鸣器报警(传输了一个信号量)
			xSemaphoreGiveFromISR( beepSemaphore, &xHigherPriorityTaskWoken ); 
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}
