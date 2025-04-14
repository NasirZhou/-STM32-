#include "stm32f10x.h"                  // Device header
#include "wifi.h"
#include "delay.h"
#include "usart.h"
#include "usart2.h"  // 修改为使用USART1
#include "string.h"

/*
			ESP01s       STM32
			 3V3----------3.3V
			 GND----------GND
			 RX-----------PA9
			 TX-----------PA10
			 RST----------PA11
*/

//第一步、wifi模块上电先重启一下

void wifi_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;                     
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);  

	// 配置PA11为推挽输出，用于ESP01s的复位控制
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;                
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   		 
	GPIO_Init(GPIOA, &GPIO_InitStructure);            		 

	// 设置ESP01s复位引脚为高电平，重置模块
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
}

void rst_wifi(void)
{
	// 将PA11拉低，复位ESP01s模块
	GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	Delay_ms(500);
	// 将PA11拉高，解除复位
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
//	printf("SetBits");
//	printf("Delay");
}

//判断串口一收到的数据是不是前面定义的ack（期待的应答结果）
u8* wifi_check_cmd(char *str)
{
	char *strx = 0;
	if(USART1_RX_STA & 0X8000)  // 检查USART1接收状态
	{
		USART1_RX_BUF[USART1_RX_STA & 0X7FFF] = 0;  // 确保接收到的数据是以'\0'结尾的
		strx = strstr((const char*)USART1_RX_BUF, (const char*)str);  // 查找是否包含期望的字符串
//		printf("BUF:%s \r\n", (const char*)USART1_RX_BUF);
	}
	return (u8*)strx;
}

//发送AT指令并等待应答
u8 wifi_send_cmd(char *cmd, char *ack, u16 time)
{
	u8 res = 0;
	USART1_RX_STA = 0;  // 清空接收状态
	u1_printf("%s\r\n", cmd);  // 发送指令
	if(time)
	{
		while(--time)
		{
			Delay_ms(10);  // 等待10ms
			if(USART1_RX_STA & 0X8000) // 判断是否接收到数据
			{
				//判断接受的数据是不是想要的
				if(wifi_check_cmd(ack))
				{
					break;
				}
				USART1_RX_STA = 0;  // 重置接收状态
			}
		}
		if(time == 0) res = 1;  // 超时未接收到预期数据
	}
	return res;
}

//初始化Wi-Fi模块
void init_wifi(void)
{
	//1 AT
	while(wifi_send_cmd("AT", "OK", 50))
	{
//		printf("AT响应失败\r\n");
	}
//	printf("AT响应成功\r\n");

	//2 将Wi-Fi模块设置为Station（STA）模式
	while(wifi_send_cmd("AT+CWMODE=1", "OK", 50))
	{
//		printf("STA模式设置失败\r\n");
	}
	
	//3 连接WIFI的用户名和密码
//	while(wifi_send_cmd("AT+CWJAP=\"xiaomi\",\"123456789\"", "OK", 500))
	while(wifi_send_cmd("AT+CWJAP=\"JXAU-Stu\",\"20030504\"", "OK", 500))
	{
//		printf("连接WIFI失败\r\n");
	}
//	printf("连接WIFI成功\r\n");

	//4 设置MQTT相关属性
	while(wifi_send_cmd("AT+MQTTUSERCFG=0,1,\"MQTT\",\"zhouz0504\",\"030504\",0,0,\"\"", "OK", 500))
	{
//		printf("设置属性失败\r\n");
	}

	//5 连接MQTT的ip
	while(wifi_send_cmd("AT+MQTTCONN=0,\"broker-cn.emqx.io\",1883,1", "OK", 1500))
	{
//		printf("连接MQTT服务器失败\r\n");
	}	

	//6 订阅主题
//	printf("连接MQTT服务器成功\r\n");
	while(wifi_send_cmd("AT+MQTTSUB=0,\"zhouz54test\",0", "OK", 50))
	{
//		printf("订阅主题失败\r\n");
	}
//	printf("订阅主题成功\r\n");

	//7发送信息
	while(wifi_send_cmd("AT+MQTTPUB=0,\"zhouz54test\",\"fuck8266\",0,0", "OK", 50))
	{
//		printf("发送信息失败\r\n");
	}
//	printf("发送信息成功\r\n");
}
