#ifndef __MJPEG_H
#define __MJPEG_H 
#include "stdio.h" 
#include <cdjpeg.h> 
#include <sys.h> 
#include <setjmp.h>
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//MJPEG视频处理 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/7/20
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   



struct my_error_mgr {
  struct jpeg_error_mgr pub;	
  jmp_buf setjmp_buffer;		//for return to caller 
}; 
typedef struct my_error_mgr * my_error_ptr;

u8 mjpegdec_init(u16 offx,u16 offy);
void mjpegdec_free(void);
u8 mjpegdec_decode(u8* buf,u32 bsize);

#endif

