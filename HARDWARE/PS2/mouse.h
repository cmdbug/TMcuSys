#ifndef __MOUSE_H
#define __MOUSE_H	 
#include "ps2.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//鼠标 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/13
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////


//HOST->DEVICE的命令集				  				   
#define PS_RESET    		0XFF //复位命令 回应0XFA
#define RESEND	     		0XFE //再次发送
#define SET_DEFAULT	 		0XF6 //使用默认设置 回应0XFA
#define DIS_DATA_REPORT   	0XF5 //禁用数据报告 回应0XFA
#define EN_DATA_REPORT    	0XF4 //使能数据报告 回应0XFA
#define SET_SAMPLE_RATE		0XF3 //设置采样速率 回应0XFA
#define GET_DEVICE_ID       0XF2 //得到设备ID   回应0XFA+ID
#define SET_REMOTE_MODE     0XF0 //设置到REMOTE模式 回应OXFA
#define SET_WRAP_MODE       0XEE //设置到WRAP模式 回应0XFA
#define RST_WRAP_MODE       0XEC //回到WRAP之前的模式 回应0XFA
#define READ_DATA           0XEB //读取数据 回应0XFA+位移数据包
#define SET_STREAM_MODE     0XEA //设置到STREAM模式 回应0XFA
#define STATUS_REQUEST      0XE9 //请求得到状态 回应0XFA+3个字节
#define SET_RESOLUTION      0XE8 //设置分辨率 回应OXFA+读取1个字节+应带0XFA
#define SET_SCALING21       0XE7 //设置缩放比率为2:1 回应0XFA
#define SET_SCALING11       0XE6 //设置缩放比率为1:1 回应0XFA
//DEVICE->HOST的指令
#define ERROR	     		0XFC //错误
//#define RESEND	     		0XFE //再次发送

#define LEFT_DOWN  0X01//左键按下
#define MID_DOWN   0X04//中间键按下
#define RIGHT_DOWN 0X02//右键按下
//鼠标结构体
typedef struct
{
	short x_pos;//横坐标
	short y_pos;//纵坐标
	short z_pos;//滚轮坐标
	u8  bt_mask;//按键标识,bit2中间键;bit1,右键;bit0,左键
} PS2_Mouse;
extern PS2_Mouse MouseX;	   
extern u8 MOUSE_ID;//鼠标ID,0X00,表示标准鼠标(3字节);0X03表示扩展鼠标(4字节)

u8 Init_Mouse(void); 
void Mouse_Data_Pro(void);
	   	 			    
#endif













