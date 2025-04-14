#ifndef __TIMERALARM_H
#define __TIMERALARM_H
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

extern QueueHandle_t alarmQueue_Handle;
extern SemaphoreHandle_t beepSemaphore;
void Alarm_Init(void);


#endif
