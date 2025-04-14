/**
	**********************************************************
	*FreeRTOS的头文件
	**********************************************************
*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/**
	**********************************************************
	*STM32的头文件
	**********************************************************
*/
#include "stm32f10x.h" //设备头文件
#include "OLED.h"
#include "RTOSPRO.h"
#include "MyRTC.h"
#include "AD.h"
#include "MPU6050.h"
#include "math.h"
#include "queue.h"
#include "semphr.h"
#include "usart.h"
#include "delay.h"
#include "TimerKey.h"
#include "TimerAlarm.h"
#include "beep.h"
#include "max30102.h"
#include "usart2.h"
#include "gps.h"
#include "wifi.h"
int main()
{
	Delay_Init();
	MyRTC_Init();
	OLED_Init();
//	uart_init(9600);
	AD_Init();
	Beep_Init();
	Beep_Commend(0);
	Alarm_Init();
	MAX30102_Init();
	MPU6050_Init();
	Key_Init();
	USART1_Init(115200);
	wifi_GPIO_Init();
	rst_wifi();
	init_wifi();
	RTOS_Process();   
	
	while(1)
	{

	}
}



 

