#include "usart2.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_tim.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "MyRTC.h"
#include "semphr.h"


// 串口发送缓存区
__align(8) u8 USART1_TX_BUF[USART1_MAX_SEND_LEN];  // 发送缓冲,最大USART1_MAX_SEND_LEN字节

#ifdef USART1_RX_EN  // 如果使能了接收
// 串口接收缓存区
u8 USART1_RX_BUF[USART1_MAX_RECV_LEN];  // 接收缓冲,最大USART1_MAX_RECV_LEN个字节
u16 USART1_RX_STA = 0;  // 接收状态

void USART1_IRQHandler(void) {
    u8 res;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  // 接收到数据
    {
        res = USART_ReceiveData(USART1);
        if (USART1_RX_STA < USART1_MAX_RECV_LEN)  // 还可以接收数据
        {
            TIM_SetCounter(TIM4, 0);  // 计数器清空
            if (USART1_RX_STA == 0) TIM4_Set(1);  // 使能定时器4的中断
            USART1_RX_BUF[USART1_RX_STA++] = res;  // 记录接收到的值
        } else {
            USART1_RX_STA |= 1 << 15;  // 强制标记接收完成
        }
    }
}

// 初始化USART1
void USART1_Init(u32 bound) {
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  // GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  // USART1时钟

    USART_DeInit(USART1);  // 复位USART1

    // USART1_TX PA9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);  // 初始化PA9

    // USART1_RX PA10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  // 浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  // 初始化PA10

    USART_InitStructure.USART_BaudRate = bound;  // 一般设置为9600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  // 字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  // 一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;  // 无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  // 无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;  // 收发模式
    USART_Init(USART1, &USART_InitStructure);  // 初始化串口1

    USART_Cmd(USART1, ENABLE);  // 使能USART1

#ifdef USART1_RX_EN
    // 如果使能了接收
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // 开启接收中断

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;  // 中断配置
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);  // 初始化NVIC

    TIM4_Init(99, 7199);  // 10ms定时器中断
    USART1_RX_STA = 0;  // 清零
    TIM4_Set(0);  // 关闭定时器4
#endif
}

// USART1 printf函数，确保一次发送数据不超过USART1_MAX_SEND_LEN字节
void u1_printf(char* fmt, ...) {
    unsigned int i, length;
    va_list ap;
    va_start(ap, fmt);
    vsprintf((char*)USART1_TX_BUF, fmt, ap);
    va_end(ap);

    length = strlen((const char*)USART1_TX_BUF);
    while ((USART1->SR & 0X40) == 0);  // 等待发送完成
    for (i = 0; i < length; i++) {
        USART1->DR = USART1_TX_BUF[i];
        while ((USART1->SR & 0X40) == 0);  // 等待发送完成
    }
}

// 定时器4中断服务程序
void TIM4_IRQHandler(void) {
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  // 是更新中断
    {
        USART1_RX_STA |= 1 << 15;  // 标记接收完成
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  // 清除TIM4更新中断标志
        TIM4_Set(0);  // 关闭定时器4
    }
}

// 设置TIM4的开关
void TIM4_Set(u8 sta) {
    if (sta) {
        TIM_SetCounter(TIM4, 0);  // 计数器清空
        TIM_Cmd(TIM4, ENABLE);  // 使能TIM4
    } else {
        TIM_Cmd(TIM4, DISABLE);  // 关闭定时器4
    }
}

// 初始化定时器4
void TIM4_Init(u16 arr, u16 psc) {
    NVIC_InitTypeDef NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);  // 时钟使能

    TIM_TimeBaseStructure.TIM_Period = arr;  // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = psc;  // 时钟预分频
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  // 设置时钟分割
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);  // 初始化定时器

    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);  // 使能更新中断

    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);  // 初始化NVIC
}
#endif
