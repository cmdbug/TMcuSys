#ifndef __AUDIOSEL_H
#define __AUDIOSEL_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//音频选择器 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/14
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

//74HC4052控制设置端口
#define AUDIO_SELB	PBout(7)
#define AUDIO_SELA	PDout(7)
//声音通道选择
#define AUDIO_MP3 		0	//MP3通道
#define AUDIO_RADIO		1	//收音机通道
#define AUDIO_PWM		2	//PWM音频通道
#define AUDIO_NONE		3	//无声    						 
							    
void Audiosel_Init(void);	//初始化函数
void Audiosel_Set(u8 ch);   //设置函数

#endif
















