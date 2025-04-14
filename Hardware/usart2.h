#ifndef __USART2_H
#define __USART2_H	  
#include "sys.h"
#include "FreeRTOS.h"
#include "queue.h"

//////////////////////////////////////////////////////////////////////////////////	 
// 本程序只供学习使用，未经作者许可，不得用于其它任何用途
// ALIENTEK STM32开发板
// 串口1驱动代码	   
// 正点原子@ALIENTEK
// 技术论坛:www.openedv.com
// 修改日期:2024/4/09
// 版本：V1.0
// 版权所有，盗版必究。
// Copyright(C) 广州市星翼电子科技有限公司 2009-2019
// All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define USART1_MAX_RECV_LEN     200            // 最大接收缓存字节数
#define USART1_MAX_SEND_LEN     200            // 最大发送缓存字节数
#define USART1_RX_EN            1               // 0, 不接收; 1, 接收.

extern u8  USART1_RX_BUF[USART1_MAX_RECV_LEN];    // 接收缓冲, 最大USART1_MAX_RECV_LEN字节
extern u8  USART1_TX_BUF[USART1_MAX_SEND_LEN];    // 发送缓冲, 最大USART1_MAX_SEND_LEN字节
extern u16 USART1_RX_STA;                          // 接收数据状态

void USART1_Init(u32 bound);                       // 串口1初始化 
void TIM4_Set(u8 sta);
void TIM4_Init(u16 arr, u16 psc);
//void UART_DMA_Config(DMA_Channel_TypeDef*DMA_CHx, u32 cpar, u32 cmar);
//void UART_DMA_Enable(DMA_Channel_TypeDef*DMA_CHx, u16 len);
void u1_printf(char* fmt, ...);

#endif
