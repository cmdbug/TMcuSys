#ifndef __SMS_MAIN_H
#define __SMS_MAIN_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的SMS模拟器工程
//ALIENTEK STM32F407开发板
//SMS主函数 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/7/1
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 
 

extern u8* SMS_romfile;//ROM指针


void sms_sram_free(void);
u8 sms_sram_malloc(u32 romsize);
void sms_set_window(void);

void sms_start(u8 bank_mun);
void sms_update_pad(void);
u8 sms_load(u8* pname);

void sms_i2s_dma_tx_callback(void);
int sms_sound_open(int sample_rate);
void sms_sound_close(void);
void sms_apu_fill_buffer(int samples,u16* wavebuf);
 
#endif
