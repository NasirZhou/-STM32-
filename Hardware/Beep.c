#include "stm32f10x.h"                  // Device header


void Beep_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//开启GPIOB的时钟
															//使用各个外设前必须开启时钟，否则对外设的操作无效
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;					//定义结构体变量
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		//GPIO模式，赋值为推挽输出模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				//GPIO引脚，赋值为第15号引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//GPIO速度，赋值为50MHz
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);					//将赋值后的构体变量传递给GPIO_Init函数
															//函数内部会自动根据结构体的参数配置相应寄存器
}
//实现GPIOB的初始化
void Beep_Commend(uint32_t com) 
{	
	if(com == 1)
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
	}
	else
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_15);	//将PB15引脚设置为高电平，蜂鸣器停止
	}	
}
