#include "stm32f10x.h" //设备头文件
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "OLED.h"
#include "MyRTC.h"
#include "MPU6050.h"
#include "math.h"
#include "queue.h"
#include "semphr.h"
#include "usart.h"
#include "AD.h"
#include "TimerKey.h"
#include "TimerAlarm.h"
#include "Beep.h"
#include "max30102.h"
#include "gps.h"  
#include "delay.h"  
#include "usart2.h"
#include <string.h>
#include "wifi.h"
#define MAX_BRIGHTNESS 255
#define INTERRUPT_REG 0X00


#define M_PI (3.14159265358979323846264338327950288)

QueueHandle_t newsQueue_Handle;
QueueHandle_t StepQueue_Handle;

//以下内容是各个任务配置的定义列表,方便进行任务参数管理
//开始任务1
#define START_TASK_STACK_SIZE 64
#define START_TASK_PRIO 1
TaskHandle_t StartTask_Handle;
void StartTask(void *arg);

//手表桌面显示任务2
#define Watch_Home_STACK_SIZE 128
#define Watch_Home_PRIO 2
TaskHandle_t Watch_Home_Handle;
void Watch_Home(void *arg);

//抬腕检测任务3
#define Hand_Detect_STACK_SIZE 128
#define Hand_Detect_PRIO 1
TaskHandle_t Hand_Detect_Handle;
void Hand_Detect(void *arg);

//菜单任务4
#define Watch_Menu_STACK_SIZE 128
#define Watch_Menu_PRIO 2
TaskHandle_t Watch_Menu_Handle;
void Watch_Menu(void *arg);

//设置闹钟时间任务5
#define Watch_Alarm_STACK_SIZE 128
#define Watch_Alarm_PRIO 2
TaskHandle_t Watch_Alarm_Handle;
void Watch_Alarm(void *arg);

//设置闹钟角标任务6
#define Mini_Alarm_STACK_SIZE 32
#define Mini_Alarm_PRIO 1
TaskHandle_t Mini_Alarm_Handle;
void Mini_Alarm(void *arg);

//设置血氧检测任务7
#define Watch_Heart_STACK_SIZE 400
#define Watch_Heart_PRIO 2
TaskHandle_t Watch_Heart_Handle;
void Watch_Heart(void *arg);

//设置手表步数任务8
#define Watch_Step_STACK_SIZE 128
#define Watch_Step_PRIO 2
TaskHandle_t Watch_Step_Handle;
void Watch_Step(void *arg);

//设置GPS定位任务9
#define Watch_GPS_STACK_SIZE 512
#define Watch_GPS_PRIO 2
TaskHandle_t Watch_GPS_Handle;
void Watch_GPS(void *arg);

//设置手电筒任务10
#define Watch_Light_STACK_SIZE 32
#define Watch_Light_PRIO 2
TaskHandle_t Watch_Light_Handle;
void Watch_Light(void *arg);

//蜂鸣器发生11
#define BEEP_STACK_SIZE 32
#define BEEP_PRIO 1
TaskHandle_t BEEP_Handle;
void BEEP(void *arg);

//设置步数检测任务12
#define Step_detect_STACK_SIZE 128
#define Step_detect_PRIO 1
TaskHandle_t Step_detect_Handle;
void Step_detect(void *arg);

//设置串口2检测任务13
#define usart2_Detect_STACK_SIZE 256
#define usart2_Detect_PRIO 2
TaskHandle_t usart2_Detect_Handle;
void usart2_Detect(void *arg);

//设置显示数据任务14
#define Watch_News_STACK_SIZE 128
#define Watch_News_PRIO 2
TaskHandle_t Watch_News_Handle;
void Watch_News(void *arg);

//其他函数声明
void OLED_SHOW_TIME(void);
void OLED_SHOW_BAT(void);
void OLED_SHOW_TEMP(void);
void MUNE_FLAG(void);
void UpdateAttitude(int16_t AX, int16_t AY, int16_t AZ, int16_t GX, int16_t GY, int16_t GZ,float *pitch,float *roll);

void GPIO_Config_Init(void);
void RCC_Configuration(void);
void USART_Config_Init(void);
void USART_BaudRate_Init(uint32_t Data);
void NVIC_Configuration(void);

//中间层入口函数，用于创建开始任务，打开调度器
void RTOS_Process(void)
{
	xTaskCreate( 	StartTask,   			//任务函数
					"StartTask", 			//任务名字
					START_TASK_STACK_SIZE,	//任务堆栈大小
					NULL,					//任务函数参数
					START_TASK_PRIO,		//任务优先级
					&StartTask_Handle	);	//任务句柄
	
	vTaskStartScheduler();   //开始任务调度
}

//用开始任务创建其他任务.以及信号量
void StartTask(void *arg)
{
	taskENTER_CRITICAL();
	xTaskCreate(Watch_Home,"Watch_Home",Watch_Home_STACK_SIZE,NULL,Watch_Home_PRIO,&Watch_Home_Handle);
	xTaskCreate(Hand_Detect,"Hand_Detect",Hand_Detect_STACK_SIZE,NULL,Hand_Detect_PRIO,&Hand_Detect_Handle);
	xTaskCreate(Watch_Menu,"Watch_Menu",Watch_Menu_STACK_SIZE,NULL,Watch_Menu_PRIO,&Watch_Menu_Handle);
	xTaskCreate(Watch_Alarm,"Watch_Alarm",Watch_Alarm_STACK_SIZE,NULL,Watch_Alarm_PRIO,&Watch_Alarm_Handle);
	xTaskCreate(BEEP,"BEEP",BEEP_STACK_SIZE,NULL,BEEP_PRIO,&BEEP_Handle);
	xTaskCreate(Mini_Alarm,"Mini_Alarm",Mini_Alarm_STACK_SIZE,NULL,Mini_Alarm_PRIO,&Mini_Alarm_Handle);
	xTaskCreate(Watch_Heart,"Watch_Heart",Watch_Heart_STACK_SIZE,NULL,Watch_Heart_PRIO,&Watch_Heart_Handle);
	xTaskCreate(Watch_Light,"Watch_Light",Watch_Light_STACK_SIZE,NULL,Watch_Light_PRIO,&Watch_Light_Handle);
	xTaskCreate(Watch_GPS,"Watch_GPS",Watch_GPS_STACK_SIZE,NULL,Watch_GPS_PRIO,&Watch_GPS_Handle);
	xTaskCreate(Watch_Step,"Watch_Step",Watch_Step_STACK_SIZE,NULL,Watch_Step_PRIO,&Watch_Step_Handle);
	xTaskCreate(Step_detect,"Step_detect",Step_detect_STACK_SIZE,NULL,Step_detect_PRIO,&Step_detect_Handle);
	xTaskCreate(usart2_Detect,"usart2_Detect",usart2_Detect_STACK_SIZE,NULL,usart2_Detect_PRIO,&usart2_Detect_Handle);
	xTaskCreate(Watch_News,"Watch_News",Watch_News_STACK_SIZE,NULL,Watch_News_PRIO,&Watch_News_Handle);
	
	keyQueue_Handle = xQueueCreate(1,8);   //中断->任务之间传按键值
	alarmQueue_Handle = xQueueCreate(3,8);	//任务->中断之间传设定的时间
	StepQueue_Handle = xQueueCreate(1,16);	
	newsQueue_Handle = xQueueCreate(1,32);
	beepSemaphore = xSemaphoreCreateBinary();
	
	vTaskSuspend(Watch_News_Handle);
	vTaskSuspend(Watch_Step_Handle);
	vTaskSuspend(Watch_GPS_Handle);
	vTaskSuspend(Watch_Light_Handle);
	vTaskSuspend(Watch_Menu_Handle);
	vTaskSuspend(Watch_Alarm_Handle);
	vTaskSuspend(Mini_Alarm_Handle);
	vTaskSuspend(Watch_Heart_Handle);
	vTaskDelete(NULL);
    taskEXIT_CRITICAL();
	
}

void Watch_Home(void *arg)  //桌面显示任务的函数
{
	int key_event;
	while(1)
	{
		OLED_SHOW_TIME();
		OLED_SHOW_BAT();
		OLED_SHOW_TEMP();
		vTaskDelay(10);
		if(keyQueue_Handle !=NULL)
		{
			xQueueReceive(keyQueue_Handle,&key_event,0);
		}
		if(key_event == 3)
		{
			OLED_Clear();
			key_event = 0;
			vTaskResume(Watch_Menu_Handle);
			vTaskSuspend(Hand_Detect_Handle);  //进入菜单记得挂起抬手检测
			vTaskSuspend(Watch_Home_Handle);
		}
	}
}

void Hand_Detect(void *arg)  //抬手检测的函数
{
	int16_t AX, AY, AZ, GX, GY, GZ;
	float pitch = 0.0; // 俯仰角
	float roll = 0.0;  // 横滚角
	BaseType_t taskSuspended_Flag = pdFALSE; // 用于记录任务是否已挂起 pdFALSE：恢复态 pdTRUE：挂起态
	
    while(1)
    {
		taskENTER_CRITICAL();  //为了保障iic的getdata，在此处进入临界区
		MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
        UpdateAttitude(AX, AY, AZ, GX, GY, GZ ,&pitch ,&roll);
		//printf("pitch:%f",pitch);
		taskEXIT_CRITICAL();  //退出临界区
		
		if(pitch>20 || pitch<-20)
		{
			if(taskSuspended_Flag == pdFALSE)  //该任务未挂起，则可以执行挂起
			{	
				taskENTER_CRITICAL();   //硬件ii2，在此处进入临界区
				OLED_Clear();
				OLED_Update(); //OLED驱动层写法规定
				taskEXIT_CRITICAL();
				vTaskSuspend(Watch_Home_Handle);
				taskSuspended_Flag = pdTRUE; //更改flag从而避免任务重复挂起
			}
		}
		
		else
		{
			if(taskSuspended_Flag == pdTRUE)  //该任务已挂起，则可以执行恢复
			{	
				vTaskResume(Watch_Home_Handle);
				taskSuspended_Flag = pdFALSE; //该任务已恢复的情况下，不能重复恢复
			}
		}
        vTaskDelay(10);
	} 
}

void OLED_SHOW_TIME(void)
{
	MyRTC_ReadTime();	//RTC读取时间，最新的时间存储到MyRTC_Time数组中
	OLED_ShowString(52, 16, "-   -", OLED_8X16); //日期分隔符
	OLED_ShowString(44, 32, ":   :", OLED_8X16); //时间分割符
	//OLED_ShowImage(96, 0, 32, 16, Signal);
	OLED_ShowNum(16, 16, MyRTC_Time[0], 4, OLED_8X16);
	OLED_ShowNum(64, 16, MyRTC_Time[1], 2,OLED_8X16);  //月份
	OLED_ShowNum(96, 16, MyRTC_Time[2], 2, OLED_8X16);
	OLED_ShowNum(24, 32, MyRTC_Time[3], 2, OLED_8X16); //小时
	OLED_ShowNum(56, 32, MyRTC_Time[4], 2,OLED_8X16);
	OLED_ShowNum(88, 32, MyRTC_Time[5], 2, OLED_8X16);
	OLED_Update();//更新
}

void OLED_SHOW_BAT(void)
{

	float Voltage;
	uint16_t bat_ad;
	bat_ad = AD_GetValue();
	Voltage = (float)bat_ad/4095*3.3;
	//printf("bat_ad:%d\r\n,Voltage:%f\r\n",bat_ad,Voltage);
	if(Voltage>3.0)
	{
		OLED_ShowImage(111,0,16,8,Battery_1);
	}
	else if(Voltage<3.0 && Voltage>2.0)
	{
		OLED_ShowImage(111,0,16,8,Battery_2);
	}
	else if(Voltage<2.0 && Voltage>1.0)
	{
		OLED_ShowImage(111,0,16,8,Battery_3);
	}
	else
	{
		OLED_ShowImage(111,0,16,8,Battery_4);
	}
	OLED_Update();//更新
	
}

void OLED_SHOW_TEMP(void)
{
	float TEMP;
	taskENTER_CRITICAL(); 
	TEMP =  MPU6050_GetTEMP();
	taskEXIT_CRITICAL(); 
	OLED_ShowNum(10,0,TEMP,2,OLED_6X8);
	OLED_ShowImage(24,0,8,8,Temp_Sign);
	OLED_Update();//更新
}

// 定义函数进行姿态检测
void UpdateAttitude(int16_t AX, int16_t AY, int16_t AZ, int16_t GX, int16_t GY, int16_t GZ ,float *pitch,float *roll) 
{
    // 加速度计的角度计算
    float accPitch = atan2(AY, sqrt(AX * AX + AZ * AZ)) * (180.0 / M_PI);
    float accRoll = atan2(-AX, AZ) * (180.0 / M_PI);
 
    // 陀螺仪积分得到角速度
    float gyroPitch = *pitch + (float)GY / 131.0; // 131 根据陀螺仪灵敏度调整
    float gyroRoll = *roll + (float)GX / 131.0;
 
    // 综合加速度计和陀螺仪数据，使用互补滤波
    float alpha = 0.98; // 互补滤波系数，根据需要调整
    *pitch = alpha * gyroPitch + (1.0 - alpha) * accPitch;
    *roll = alpha * gyroRoll + (1.0 - alpha) * accRoll;
}

void Watch_Menu(void *arg)
{
	int key_event;
	int MenuRoll;
	MenuRoll = 4;
	while(1)
	{
		if(keyQueue_Handle !=NULL)
		{
			xQueueReceive(keyQueue_Handle,&key_event,0);
		}
		if( key_event == 1)
		{
			MenuRoll++;
			key_event = 0;
		}
		if( key_event == 2)
		{
			MenuRoll--;
			key_event = 0;
		}
		
		//越界判断
		if(MenuRoll>6)MenuRoll=1;
		if(MenuRoll<1)MenuRoll=6;
		if( key_event == 4)
		{
			key_event = 0;   //特别强调一下栈空间变量的初始化
			OLED_Clear();
			vTaskResume(Watch_Home_Handle);
			vTaskResume(Hand_Detect_Handle);
			vTaskSuspend(Watch_Menu_Handle);
			//由于任务已经挂起，Suspend之后的代码都执行不到
		}
		
		taskENTER_CRITICAL();
		switch(MenuRoll)
		{
			case 1:  MUNE_FLAG();	OLED_DrawCircle(39,60,3,1); OLED_ShowString(49,4,"Heart",OLED_6X8);   		OLED_ShowImage(49,20,30,30,Heart);       	 OLED_Update();	break;
			case 2:  MUNE_FLAG();	OLED_DrawCircle(49,60,3,1); OLED_ShowString(49,4,"Light",OLED_6X8);   		OLED_ShowImage(49,20,30,30,Light);       	 OLED_Update();	break;
			case 3:  MUNE_FLAG();	OLED_DrawCircle(59,60,3,1); OLED_ShowString(49,4,"Alarm",OLED_6X8);   		OLED_ShowImage(49,20,30,30,Alarm_Clock);	 OLED_Update();	break;
			case 4:	 MUNE_FLAG();	OLED_DrawCircle(69,60,3,1); OLED_ShowString(54,4,"Step",OLED_6X8); 			OLED_ShowImage(49,20,30,30,Step);			 OLED_Update();	break;
			case 5:  MUNE_FLAG();	OLED_DrawCircle(79,60,3,1); OLED_ShowString(54,4,"News",OLED_6X8);  		OLED_ShowImage(49,20,30,30,News);			 OLED_Update();	break;	
			case 6:  MUNE_FLAG();	OLED_DrawCircle(89,60,3,1); OLED_ShowString(54,4,"Gps",OLED_6X8);  	 		OLED_ShowImage(49,20,30,30,Gps);			 OLED_Update();	break;			
			default:break;
		}
		taskEXIT_CRITICAL();
		
		if( key_event == 3)
		{
			OLED_Clear();
			key_event =0;
			switch(MenuRoll)
			{
				case 1:  vTaskResume(Watch_Heart_Handle); vTaskSuspend(Watch_Menu_Handle); break;
				case 2:  vTaskResume(Watch_Light_Handle); vTaskSuspend(Watch_Menu_Handle);break;
				case 3:  vTaskResume(Watch_Alarm_Handle); vTaskSuspend(Watch_Menu_Handle); break;
				case 4:  vTaskResume(Watch_Step_Handle); vTaskSuspend(Watch_Menu_Handle); break;
				case 5:  vTaskResume(Watch_News_Handle); vTaskSuspend(Watch_Menu_Handle); break;
				case 6:  vTaskResume(Watch_GPS_Handle); vTaskSuspend(Watch_Menu_Handle);break;
				default:break;
			}
		}
			
		vTaskDelay(30);
	}
}


void MUNE_FLAG(void)

{
	/*显示进度点*/
//	OLED_Clear();
	OLED_ClearArea(30,0,68,64);
	OLED_DrawCircle(39,60,3,0);
	OLED_DrawCircle(49,60,3,0);
	OLED_DrawCircle(59,60,3,0);
	OLED_DrawCircle(69,60,3,0);
	OLED_DrawCircle(79,60,3,0);
	OLED_DrawCircle(89,60,3,0);
}

void Watch_Alarm(void *arg) //最主要的功能还是把设定的时间传出去（队列）
{
	int set_position =0;
	int key_event;
	int Alarm_time[3] = {12,0,0};
	int i;
	BaseType_t taskAlarm_Flag = pdFALSE;
	
	while(1)
	{	
		//按键检测
		if(keyQueue_Handle !=NULL)
		{
			xQueueReceive(keyQueue_Handle,&key_event,0);
		}
		
		if( key_event == 3)
		{
			key_event =0;
			set_position++;
		}
		if (set_position>2)set_position=0; //位置变量的越界判断
		
		if( key_event == 1)
		{
			key_event =0;
			Alarm_time[set_position]+=1;
		}
		
		if( key_event == 2)
		{
			key_event =0;
			Alarm_time[set_position]-=1;
		}
		
		//设定时间的越界判断
		if(Alarm_time[0]>23)Alarm_time[0]=0;
		if(Alarm_time[0]<0)Alarm_time[0]=23;
		if(Alarm_time[1]>59)Alarm_time[1]=0;
		if(Alarm_time[1]<0)Alarm_time[1]=59;
		if(Alarm_time[2]>59)Alarm_time[2]=0;
		if(Alarm_time[2]<0)Alarm_time[2]=59;
		
		OLED_ShowString(0, 16, "Set Alarm", OLED_8X16); //时间分割符
		OLED_ShowString(44, 32, ":   :", OLED_8X16); //时间分割符
		OLED_ShowNum(24, 32, Alarm_time[0], 2, OLED_8X16); //小时
		OLED_ShowNum(56, 32, Alarm_time[1], 2,OLED_8X16);
		OLED_ShowNum(88, 32, Alarm_time[2], 2, OLED_8X16);
		OLED_ClearArea(23,48,82,16);
		OLED_ShowString(24 + set_position*32, 48, "--", OLED_8X16);
		OLED_Update();//更新
		
		if(key_event == 4)
		{
			OLED_Clear();
			key_event =0;
			if(taskAlarm_Flag == pdFALSE)
			{
			    //显示一个闹钟的小图标，告诉用户开启了闹钟功能
				vTaskResume(Mini_Alarm_Handle);
				TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//打开中断，使得中断源能够进入中断服务程序
				taskAlarm_Flag = pdTRUE;
			}
			else
			{
				//擦除一个闹钟的小图标，告诉用户关闭了闹钟功能
				vTaskSuspend(Mini_Alarm_Handle);
				TIM_ITConfig(TIM3,TIM_IT_Update,DISABLE);//关闭中断，使得中断源不能够进入中断服务程序
				taskAlarm_Flag = pdFALSE;
			}
			for(i=0;i<3;i++)
			{
				xQueueSendToBack(alarmQueue_Handle,&Alarm_time[i],NULL);
			}
			vTaskResume(Watch_Menu_Handle);
			vTaskSuspend(Watch_Alarm_Handle);
		}
		vTaskDelay(20);
	}
}

void Mini_Alarm(void *arg)
{
	while(1)
	{
		taskENTER_CRITICAL();  				  	//在此处进入临界区
		OLED_ShowImage(2,50,10,10,Alarm_Clock_Min);
		OLED_Update(); 					    	//OLED驱动层写法规定
		taskEXIT_CRITICAL();
		vTaskDelay(20);
	}
}
	

void BEEP(void *arg)
{
	BaseType_t xFlag;
	while(1)
	{
		xFlag = xSemaphoreTake(beepSemaphore,0);
		if(xFlag==pdTRUE)
		{
			Beep_Commend(1);
			vTaskDelay(1500);
			Beep_Commend(0);
		}
		vTaskDelay(20);
	}	
}

void Watch_Light(void *arg) //最主要的功能还是把设定的时间传出去（队列）
{
	int key_event;
	int Light_Flag;
	while(1)
	{	
		//按键检测
		OLED_ShowString(0, 0, "Watch_Light:", OLED_8X16);
		if(Light_Flag == 1)
		{
			OLED_ClearArea(30,16,68,32);
			OLED_ShowString(64, 16, "OFF", OLED_8X16); 
		}
		else
		{
			OLED_ClearArea(30,16,68,32);
			OLED_ShowString(64, 16, "ON", OLED_8X16);
		}			
		OLED_Update();
		if(keyQueue_Handle !=NULL)
		{
			xQueueReceive(keyQueue_Handle,&key_event,0);
		}
		
		if( key_event == 1)
		{
			key_event =0;
			GPIO_SetBits(GPIOA,GPIO_Pin_4);
			Light_Flag = 1; 
			OLED_Update(); 
		}
		
		if( key_event == 2)
		{
			key_event =0;
			GPIO_ResetBits(GPIOA,GPIO_Pin_4);
			Light_Flag = 0;
			OLED_Update(); 
		}

		if(key_event == 4)
		{
			OLED_Clear();
			key_event =0;
			vTaskResume(Watch_Menu_Handle);
			vTaskSuspend(Watch_Light_Handle);
		}
		vTaskDelay(20);
	}
}
void Watch_Heart(void *arg)  //桌面显示任务的函数
{
    int key_event;
    uint32_t aun_ir_buffer[100];        // 缓冲区大小缩减为100
    int32_t n_ir_buffer_length = 100;   // 缓冲区长度
    uint32_t aun_red_buffer[100];       // 缓冲区大小缩减为100
    int32_t n_sp02;                     // SpO2值
    int8_t ch_spo2_valid;               // SPO2有效性
    int32_t n_heart_rate;               // 心率值
    int8_t ch_hr_valid;                 // 心率有效性
    uint8_t Temp[6];                    // Temp改为数组，以接收6个字节
    uint32_t un_min = 0x3FFFF, un_max = 0;  
    int i;
    uint8_t dis_hr = 0, dis_spo2 = 0;

    // 初始化显示
    OLED_Clear();
    OLED_ShowString(0, 0, "Wait a moment...", OLED_8X16);
    OLED_Printf(0, 16, OLED_8X16, "心率:  BMP      ");
    OLED_Printf(0, 32, OLED_8X16, "血氧:  %%        ");
    OLED_Update();

    // 读取前100个样本
    for (i = 0; i < n_ir_buffer_length; i++) {
        while (MAX30102_INT == 1);   // 等待中断引脚断言
        max30102_FIFO_ReadBytes(REG_FIFO_DATA, Temp);  // 读取6个字节数据到Temp数组
        aun_red_buffer[i] =  (long)((long)((long)Temp[0] & 0x03) << 16) | (long)Temp[1] << 8 | (long)Temp[2];
        aun_ir_buffer[i] = (long)((long)((long)Temp[3] & 0x03) << 16) | (long)Temp[4] << 8 | (long)Temp[5];

        // 更新最大值和最小值
        if (un_min > aun_red_buffer[i]) un_min = aun_red_buffer[i];
        if (un_max < aun_red_buffer[i]) un_max = aun_red_buffer[i];
    }

    // 计算初步的心率和血氧
    maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

    // 循环读取数据并更新显示
    while (1) {
        // 更新缓存区并计算心率与血氧
        for (i = 0; i < n_ir_buffer_length - 1; i++) {
            aun_red_buffer[i] = aun_red_buffer[i + 1];    // 移动缓冲区
            aun_ir_buffer[i] = aun_ir_buffer[i + 1];      // 移动缓冲区

            // 更新最大值和最小值
            if (un_min > aun_red_buffer[i]) un_min = aun_red_buffer[i];
            if (un_max < aun_red_buffer[i]) un_max = aun_red_buffer[i];
        }

        // 添加新的样本
        while (MAX30102_INT == 1);  // 等待中断
        max30102_FIFO_ReadBytes(REG_FIFO_DATA, Temp);  // 读取传感器数据到Temp数组
        aun_red_buffer[n_ir_buffer_length - 1] =  (long)((long)((long)Temp[0] & 0x03) << 16) | (long)Temp[1] << 8 | (long)Temp[2];
        aun_ir_buffer[n_ir_buffer_length - 1] = (long)((long)((long)Temp[3] & 0x03) << 16) | (long)Temp[4] << 8 | (long)Temp[5];

        // 计算心率和血氧
        maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);

        // 显示数据
        if (ch_hr_valid == 1 && n_heart_rate < 120) {
            dis_hr = n_heart_rate;
            dis_spo2 = n_sp02;
        } else {
            dis_hr = 0;
            dis_spo2 = 0;
        }

        // 更新OLED显示
        if (dis_hr == 0) {
            OLED_ShowString(0, 0, "Press                  ", OLED_8X16);
            OLED_Update();
        } else if (dis_spo2 <= 70) {
            OLED_Clear();
            OLED_ShowString(0, 0, "Press                  ", OLED_8X16);
            OLED_Printf(0, 16, OLED_8X16, "心率:%d BMP      ", dis_hr - 20);
            OLED_Printf(0, 32, OLED_8X16, "血氧:%d %%        ", dis_spo2 + 70);
            OLED_Update();
            printf("心率= %d BPM 血氧= %d\r\n", dis_hr, dis_spo2);
        } else {
            OLED_Clear();
            OLED_ShowString(0, 0, "Press                  ", OLED_8X16);
            OLED_Printf(0, 16, OLED_8X16, "心率:%d BMP      ", dis_hr - 20);
            OLED_Printf(0, 32, OLED_8X16, "血氧:%d %%        ", dis_spo2);
            OLED_Update();
            printf("心率= %d BPM 血氧= %d\r\n", dis_hr, dis_spo2);
        }

        // 处理按键事件
        if (keyQueue_Handle != NULL) {
            xQueueReceive(keyQueue_Handle, &key_event, 0);
        }
        if (key_event == 3) {
            OLED_Clear();
            key_event = 0;
        }
        if (key_event == 4) {
            OLED_Clear();
            key_event = 0;
            vTaskResume(Watch_Menu_Handle);
            vTaskSuspend(Watch_Heart_Handle);
        }

        // 延时
        vTaskDelay(20);
    }
}


void Watch_Step(void *arg) 
{
    int steps;
	int key_event;
    while (1) 
	{
		OLED_ShowString(32,0,"STEP:                  ",OLED_8X16);
		OLED_Update();
		if(keyQueue_Handle !=NULL)
		{
			xQueueReceive(keyQueue_Handle,&key_event,0);
		}
		if (xQueueReceive(StepQueue_Handle ,&steps,0) == pdTRUE) 
		{
			// 这里应该是实际显示步数的代码，此处为打印
			OLED_ShowNum(40,16,steps,5,OLED_8X16);
			OLED_Update();
		}
		
		if(key_event == 4)
		{
			key_event = 0;
			vTaskResume(Watch_Menu_Handle);
			vTaskSuspend(Watch_Step_Handle);
		}
		vTaskDelay(20);
    }
}	


void Step_detect(void *arg)
{
	int16_t AX, AY, AZ, GX, GY, GZ;
	float pitch = 0.0; 
	float roll = 0.0; 
	int step_state;
	int steps;
	
    while(1)
    {
		taskENTER_CRITICAL();  
		MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
      UpdateAttitude(AX, AY, AZ, GX, GY, GZ ,&pitch ,&roll);
		taskEXIT_CRITICAL();  
		
		if(roll>30 || roll<-30)
		{
			step_state = 1;			
		}
		
		if(step_state == 1&&roll < 5&& roll >-10)  
		{	
			steps+=1;
			step_state = 0;
			xQueueOverwrite(StepQueue_Handle ,&steps);
		}		
        vTaskDelay(10);
	} 
}

void usart2_Detect(void *arg)
{
	char buf1[200];
	while(1)
	{
		if(USART1_RX_STA&0X8000) //检测sta的最高位是否为1（检测有没有收到一个数据包）
		{
			strncpy(buf1,(char*)USART1_RX_BUF+31,5);
//			printf("串口二收到:%s\r\n",buf1);
			xQueueOverwrite(newsQueue_Handle,&buf1);
			USART1_RX_STA = 0; //接收完信息之后清楚sta标志位，准备下一次接收
		}
	vTaskDelay(20);
	}
}

void Watch_News(void *arg)
{
	char mesg;
	int key_event;
	while(1)
	{
		if (xQueueReceive(newsQueue_Handle ,&mesg,0) == pdTRUE) 
		{
			//这里显示收到的信息
//			printf("%s\r\n",&mesg);
			OLED_ShowString(0,0,&mesg,OLED_8X16);
			OLED_Update();
			
		}
		if(keyQueue_Handle !=NULL)
		{
			xQueueReceive(keyQueue_Handle,&key_event,0);
		}		
		if(key_event == 4)
		{
			OLED_Clear();
			key_event = 0;
			vTaskResume(Watch_Menu_Handle);
			vTaskSuspend(Watch_Step_Handle);
		}
		vTaskDelay(20);
	}

}


//USART2管脚初始化
void GPIO_Config_Init(void)
{
 
	GPIO_InitTypeDef GPIO_InitStructure;
  /* Configure USART1 Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure USART1 Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

}

void RCC_Configuration(void)
{
  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  /* Enable USART1 Clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  
}

void USART_Config_Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	
  USART_InitStructure.USART_BaudRate = 38400;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);
  /* Enable USARTy Receive  interrupts */
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);//使能空闲中断
  /* Enable the USART2 */
  USART_Cmd(USART2, ENABLE);
		 delay_ms(10);//等待10ms

}

void USART_BaudRate_Init(uint32_t Data)
{
	USART_InitTypeDef USART_InitStructure;
	
  USART_InitStructure.USART_BaudRate = Data;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);
  /* Enable USARTy Receive  interrupts */
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);//使能空闲中断
  /* Enable the USART2 */
  USART_Cmd(USART2, ENABLE);
	delay_ms(10);//等待10ms
	
}

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

}

//主函数，采用外部8M晶振，72M系统主频，可以在void SetSysClock(void)函数中选择主频率设置
void Watch_GPS(void *arg)
 {	
	uint8_t key=0XFF;


	RCC_Configuration();//USART2时钟使能，GPIO时钟使能
	NVIC_Configuration();//中断接收使能
	GPIO_Config_Init();	//初始化串口2,PA2->USART2_TX，PA3->USART2_RX
	USART_Config_Init();//USART2配置，38400波特率，8位数据，无校验，1位停止
	OLED_Clear();//清屏
	OLED_ShowString(0,0,"GPS                   ",OLED_8X16);
	OLED_Update();
	 delay_ms(500);//等待500ms
	if(Ublox_Cfg_Rate(1000,1)!=0)	//设置定位信息更新速度为1000ms,顺便判断GPS模块是否在位. 
	{
		OLED_ShowString(0,0,"Connecting             ",OLED_8X16);
		OLED_Update();
		while((Ublox_Cfg_Rate(1000,1)!=0)&& key)	//持续判断,直到可以检查到NEO-6M,且数据保存成功，如果时初次上电使用，先用9600波特率再重新配置
		{
			USART_BaudRate_Init(9600);//初始化串口2波特率为9600(EEPROM没有保存数据的时候,波特率为9600.)

	  	Ublox_Cfg_Prt(38400);			//重新设置模块的波特率为38400
			Ublox_Cfg_Tp(1000000,100000,1);	//设置PPS为1秒钟输出1次,脉冲宽度为100ms	    
			key=Ublox_Cfg_Cfg_Save();		//保存配置  
		}	  					 
		delay_ms(500);//等待500ms
	
	}
  while (1)
  {
		printf("GpsDataRead\r\n");
		OLED_Clear();//清屏
		GpsDataRead();//读取经纬度，时间信息，并在液晶上显示
		delay_ms(800);//等待800ms
  }
 }
