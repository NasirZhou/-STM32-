#ifndef __WIFI_H
#define __WIFI_H	 

void wifi_GPIO_Init(void);
void rst_wifi(void);
void init_wifi(void);
u8 wifi_send_cmd(char *cmd,char *ack,u16 time);

#endif
