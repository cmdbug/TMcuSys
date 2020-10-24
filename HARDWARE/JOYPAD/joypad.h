#ifndef __JOYPAD_H
#define __JOYPAD_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//游戏手柄驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/12
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

//手柄连接引脚
#define JOYPAD_CLK PCout(12)  	//时钟		PC9
#define JOYPAD_LAT PCout(8)  	//锁存     	PC8
#define JOYPAD_DAT PCin(9)	 	//数据     	PC12    

void JOYPAD_Init(void);			//初始化	
u8 JOYPAD_Read(void);			//读取键值	 

#endif


