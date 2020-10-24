#include "audiosel.h"	  
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

//声音初始化
void Audiosel_Init(void)
{
	RCC->APB2ENR|=1<<3;    //使能PORTB时钟	   	 
	RCC->APB2ENR|=1<<5;    //使能PORTD时钟	
 	GPIOB->CRL&=0X0FFFFFFF; 
	GPIOB->CRL|=0X30000000;//PB.7 推挽输出 
 	GPIOD->CRL&=0X0FFFFFFF; 
	GPIOD->CRL|=0X30000000;//PD.7 推挽输出 
}
//设置4052的选择通道
//声音通道选择
//0	//MP3通道
//1	//收音机通道
//2	//PWM音频通道
//3	//无声  
void Audiosel_Set(u8 ch)
{
	AUDIO_SELA=ch&0X01;
 	AUDIO_SELB=(ch>>1)&0X01;	 
}




