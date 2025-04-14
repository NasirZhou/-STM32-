#ifndef __GPS_H
#define __GPS_H	 
#include "sys.h"  
#define USART_MAX_SEND_LEN 1024
#define USART_MAX_RECV_LEN 1024
 									   						    
extern uint16_t RX_len;//接收字节计数
//卫星信息
__packed typedef struct  
{										    
 	uint8_t num;		//卫星编号
	uint8_t eledeg;	//卫星仰角
	uint16_t azideg;	//卫星方位角
	uint8_t sn;		//信噪比		   
}nmea_slmsg;  
//UTC时间信息
__packed typedef struct  
{										    
 	uint16_t year;	//年份
	uint8_t month;	//月份
	uint8_t date;	//日期
	uint8_t hour; 	//小时
	uint8_t min; 	//分钟
	uint8_t sec; 	//秒钟
}nmea_utc_time;   	   
//NMEA 0183 协议解析后数据存放结构体
__packed typedef struct  
{										    
 	uint8_t svnum;					//可见卫星数
	nmea_slmsg slmsg[12];		//最多12颗卫星
	nmea_utc_time utc;			//UTC时间
	uint32_t latitude;				//纬度 分扩大100000倍,实际要除以100000
	uint8_t nshemi;					//北纬/南纬,N:北纬;S:南纬				  
	uint32_t longitude;			    //经度 分扩大100000倍,实际要除以100000
	uint8_t ewhemi;					//东经/西经,E:东经;W:西经
	uint8_t gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.				  
 	uint8_t posslnum;				//用于定位的卫星数,0~12.
 	uint8_t possl[12];				//用于定位的卫星编号
	uint8_t fixmode;					//定位类型:1,没有定位;2,2D定位;3,3D定位
	uint16_t pdop;					//位置精度因子 0~500,对应实际值0~50.0
	uint16_t hdop;					//水平精度因子 0~500,对应实际值0~50.0
	uint16_t vdop;					//垂直精度因子 0~500,对应实际值0~50.0 

	int altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m	 
	uint16_t speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时	 
}nmea_msg; 
//////////////////////////////////////////////////////////////////////////////////////////////////// 	
//UBLOX NEO-6M 配置(清除,保存,加载等)结构体
__packed typedef struct  
{										    
 	uint16_t header;					//cfg header,固定为0X62B5(小端模式)
	uint16_t id;						//CFG CFG ID:0X0906 (小端模式)
	uint16_t dlength;				//数据长度 12/13
	uint32_t clearmask;				//子区域清除掩码(1有效)
	uint32_t savemask;				//子区域保存掩码
	uint32_t loadmask;				//子区域加载掩码
	uint8_t  devicemask; 		  	//目标器件选择掩码	b0:BK RAM;b1:FLASH;b2,EEPROM;b4,SPI FLASH
	uint8_t  cka;		 			//校验CK_A 							 	 
	uint8_t  ckb;			 		//校验CK_B							 	 
}_ublox_cfg_cfg; 

//UBLOX NEO-6M 消息设置结构体
__packed typedef struct  
{										    
 	uint16_t header;					//cfg header,固定为0X62B5(小端模式)
	uint16_t id;						//CFG MSG ID:0X0106 (小端模式)
	uint16_t dlength;				//数据长度 8
	uint8_t  msgclass;				//消息类型(F0 代表NMEA消息格式)
	uint8_t  msgid;					//消息 ID 
								//00,GPGGA;01,GPGLL;02,GPGSA;
								//03,GPGSV;04,GPRMC;05,GPVTG;
								//06,GPGRS;07,GPGST;08,GPZDA;
								//09,GPGBS;0A,GPDTM;0D,GPGNS;
	uint8_t  iicset;					//IIC消输出设置    0,关闭;1,使能.
	uint8_t  uart1set;				//UART1输出设置	   0,关闭;1,使能.
	uint8_t  uart2set;				//UART2输出设置	   0,关闭;1,使能.
	uint8_t  usbset;					//USB输出设置	   0,关闭;1,使能.
	uint8_t  spiset;					//SPI输出设置	   0,关闭;1,使能.
	uint8_t  ncset;					//未知输出设置	   默认为1即可.
 	uint8_t  cka;			 		//校验CK_A 							 	 
	uint8_t  ckb;			    	//校验CK_B							 	 
}_ublox_cfg_msg; 

//UBLOX NEO-6M UART端口设置结构体
__packed typedef struct  
{										    
 	uint16_t header;					//cfg header,固定为0X62B5(小端模式)
	uint16_t id;						//CFG PRT ID:0X0006 (小端模式)
	uint16_t dlength;				//数据长度 20
	uint8_t  portid;					//端口号,0=IIC;1=UART1;2=UART2;3=USB;4=SPI;
	uint8_t  reserved;				//保留,设置为0
	uint16_t txready;				//TX Ready引脚设置,默认为0
	uint32_t mode;					//串口工作模式设置,奇偶校验,停止位,字节长度等的设置.
 	uint32_t baudrate;				//波特率设置
 	uint16_t inprotomask;		 	//输入协议激活屏蔽位  默认设置为0X07 0X00即可.
 	uint16_t outprotomask;		 	//输出协议激活屏蔽位  默认设置为0X07 0X00即可.
 	uint16_t reserved4; 				//保留,设置为0
 	uint16_t reserved5; 				//保留,设置为0 
 	uint8_t  cka;			 		//校验CK_A 							 	 
	uint8_t  ckb;			    	//校验CK_B							 	 
}_ublox_cfg_prt; 

//UBLOX NEO-6M 时钟脉冲配置结构体
__packed typedef struct  
{										    
 	uint16_t header;					//cfg header,固定为0X62B5(小端模式)
	uint16_t id;						//CFG TP ID:0X0706 (小端模式)
	uint16_t dlength;				//数据长度
	uint32_t interval;				//时钟脉冲间隔,单位为us
	uint32_t length;				 	//脉冲宽度,单位为us
	signed char status;			//时钟脉冲配置:1,高电平有效;0,关闭;-1,低电平有效.			  
	uint8_t timeref;			   		//参考时间:0,UTC时间;1,GPS时间;2,当地时间.
	uint8_t flags;					//时间脉冲设置标志
	uint8_t reserved;				//保留			  
 	signed short antdelay;	 	//天线延时
 	signed short rfdelay;		//RF延时
	signed int userdelay; 	 	//用户延时	
	uint8_t cka;						//校验CK_A 							 	 
	uint8_t ckb;						//校验CK_B							 	 
}_ublox_cfg_tp; 

//UBLOX NEO-6M 刷新速率配置结构体
__packed typedef struct  
{										    
 	uint16_t header;					//cfg header,固定为0X62B5(小端模式)
	uint16_t id;						//CFG RATE ID:0X0806 (小端模式)
	uint16_t dlength;				//数据长度
	uint16_t measrate;				//测量时间间隔，单位为ms，最少不能小于200ms（5Hz）
	uint16_t navrate;				//导航速率（周期），固定为1
	uint16_t timeref;				//参考时间：0=UTC Time；1=GPS Time；
 	uint8_t  cka;					//校验CK_A 							 	 
	uint8_t  ckb;					//校验CK_B							 	 
}_ublox_cfg_rate; 
				 
int NMEA_Str2num(uint8_t *buf,uint8_t*dx);
void GPS_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGGA_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPGSA_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPRMC_Analysis(nmea_msg *gpsx,uint8_t *buf);
void NMEA_GPVTG_Analysis(nmea_msg *gpsx,uint8_t *buf);
uint8_t Ublox_Cfg_Cfg_Save(void);
uint8_t Ublox_Cfg_Msg(uint8_t msgid,uint8_t uart1set);
uint8_t Ublox_Cfg_Prt(uint32_t baudrate);
uint8_t Ublox_Cfg_Tp(uint32_t interval,uint32_t length,signed char status);
uint8_t Ublox_Cfg_Rate(uint16_t measrate,uint8_t reftime);

void GpsDataRead(void);
#endif  

 



