#ifndef __PS2_H
#define __PS2_H	 
#include "delay.h"	   
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//PS2 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/13
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////				   
//物理接口定义
//PS2输入 		  				    
#define PS2_SCL PCin(11)			//PC11
#define PS2_SDA PCin(10)			//PC10
//PS2输出
#define PS2_SCL_OUT PCout(11)		//PC11
#define PS2_SDA_OUT PCout(10)		//PC10

//设置PS2_SCL输入输出状态.		  
#define PS2_SET_SCL_IN()  {GPIOC->CRH&=0XFFFF0FFF;GPIOC->CRH|=0X00008000;}
#define PS2_SET_SCL_OUT() {GPIOC->CRH&=0XFFFF0FFF;GPIOC->CRH|=0X00003000;}	  
//设置PS2_SDA输入输出状态.		  
#define PS2_SET_SDA_IN()  {GPIOC->CRH&=0XFFFFF0FF;GPIOC->CRH|=0X00000800;}
#define PS2_SET_SDA_OUT() {GPIOC->CRH&=0XFFFFF0FF;GPIOC->CRH|=0X00000300;} 

#define MOUSE    0X20 //鼠标模式
#define KEYBOARD 0X10 //键盘模式
#define CMDMODE  0X00 //发送命令
//PS2_Status当前状态标志
//[5:4]:当前工作的模式;[7]:接收到一次数据
//[6]:校验错误;[3:0]:收到的数据长度;	 
extern u8 PS2_Status;       //定义为命令模式
extern u8 PS2_DATA_BUF[16]; //ps2数据缓存区
extern u8 MOUSE_ID;

void PS2_Init(void);
u8 PS2_Send_Cmd(u8 cmd);
void PS2_Set_Int(u8 en);
u8 PS2_Get_Byte(void);
void PS2_En_Data_Report(void);  
void PS2_Dis_Data_Report(void);		  				    
#endif





















