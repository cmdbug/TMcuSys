#ifndef _BMPDISPLAY_H
#define _BMPDISPLAY_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103战舰开发板
//STemwin BMP图片显示 
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/4/10
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


//使用GUI_BMP_Draw()函数绘制BMP图片的话
//图片是加载到RAM中的，因此不能大于BMPMEMORYSIZE
//注意：显示BMP图片时内存申请使用的EMWIN的内存申请函数，因此
//BMPMEMORYSIZE不能大于我们给EMWIN分配的内存池大小
#define BMPMEMORYSIZE	500*1024	//图片大小不大于500kb

//绘制无需加载到RAM中的BMP图片时，图片每行的字节数
#define BMPPERLINESIZE	2*1024		

int dispbmp(u8 *BMPFileName,u8 mode,u32 x,u32 y,int member,int denom);
int dispbmpex(u8 *BMPFileName,u8 mode,u32 x,u32 y,int member,int denom);
void create_bmppicture(u8 *filename,int x0,int y0,int Xsize,int Ysize);
void bmpdisplay_demo(void);
#endif
