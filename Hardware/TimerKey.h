#ifndef __TIMERKEY_H
#define __TIMERKEY_H
#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t keyQueue_Handle;
void Key_Init(void);


#endif
