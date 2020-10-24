#ifndef __SMS_RENDER_H
#define __SMS_RENDER_H
#include "sms_vdp.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的NES模拟器工程
//ALIENTEK STM32F407开发板   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/10/12
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 


// formato de cor usado pelo gens (16 bits 555 ou 565)
#define MAKE_PIXEL(r, g, b) ((r << 8) & 0xF800) | ((g << 3) & 0x07E0) | ((b >> 3) & 0x001F) 

#define BACKDROP_COLOR      (0x10 | (SMS_VDP->reg[7] & 0x0F))

void  render_init(void);
void  render_reset(void);
//void  render_bg_gg(int line);
void  render_bg_sms(int line);
void  render_obj(int line);
void  update_cache(void);
void  palette_sync(int index);
void render_line(int line);

#endif // _SMSREND_H_
