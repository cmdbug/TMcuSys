#include  "joypad.h"
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

//初始化手柄接口.	 
void JOYPAD_Init(void)
{
 	RCC->APB2ENR|=1<<4;//先使能外设PORTC时钟    	
	GPIOC->CRH&=0XFFF0FF00;
	GPIOC->CRH|=0X00030083;     
	GPIOC->ODR|=3<<8; 
	GPIOC->ODR|=1<<12; 
}

//读取手柄按键值.
//FC手柄数据输出格式:
//每给一个脉冲,输出一位数据,输出顺序:
//A->B->SELECT->START->UP->DOWN->LEFT->RIGHT.
//总共8位,对于有C按钮的手柄,按下C其实就等于A+B同时按下.
//按下是0,松开是1.
//返回值:
//[0]:右
//[1]:左
//[2]:下
//[3]:上
//[4]:Start
//[5]:Select
//[6]:B
//[7]:A
u8 JOYPAD_Read(void)
{
	u8 temp=0;
	u8 t;
	JOYPAD_LAT=1;					//锁存当前状态
 	JOYPAD_LAT=0;
	for(t=0;t<8;t++) {
		temp<<=1;	 
		if(JOYPAD_DAT)temp|=0x01;	//LOAD之后，就得到第一个数据
		JOYPAD_CLK=1;			   	//每给一次脉冲，收到一个数据
 		JOYPAD_CLK=0;	
	}
	return temp;
}





