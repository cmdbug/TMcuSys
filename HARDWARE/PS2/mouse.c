#include "mouse.h"
#include "usart.h"
#include "lcd.h"
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
   	 
u8 MOUSE_ID;//用来标记鼠标ID 
PS2_Mouse MouseX;
//处理MOUSE的数据	 
void Mouse_Data_Pro(void)
{			    			    
    MouseX.x_pos+=(signed char)PS2_DATA_BUF[1];
    MouseX.y_pos+=(signed char)PS2_DATA_BUF[2];
	//MouseX.y_pos=-MouseX.y_pos;//Y方向反了,纠正过来
    MouseX.z_pos+=(signed char)PS2_DATA_BUF[3];		  
	MouseX.bt_mask=PS2_DATA_BUF[0]&0X07;//取出掩码
}	 
//初始化鼠标
//返回:0,初始化成功
//其他:错误代码
//CHECK OK 2010/5/2
u8 Init_Mouse(void)
{
	u8 t;		 
	PS2_Init();							     
	delay_ms(800);            //等待上电复位完成		 	 
	PS2_Status=CMDMODE;       //进入命令模式
	t=PS2_Send_Cmd(PS_RESET); //复位鼠标		 
	if(t!=0)return 1;
	t=PS2_Get_Byte();			  
    if(t!=0XFA)return 2;
	t=0;
	while((PS2_Status&0x80)==0)//等待复位完毕 
	{
		t++;
		delay_ms(10);      
		if(t>50)return 3;
	}
	PS2_Get_Byte();//得到0XAA
	PS2_Get_Byte();//得到ID 0X00		 
	//进入滚轮模式的特殊初始化序列
	PS2_Send_Cmd(SET_SAMPLE_RATE);//进入设置采样率
    if(PS2_Get_Byte()!=0XFA)return 4;//传输失败
	PS2_Send_Cmd(0XC8);//采样率200
    if(PS2_Get_Byte()!=0XFA)return 5;//传输失败
	PS2_Send_Cmd(SET_SAMPLE_RATE);//进入设置采样率
    if(PS2_Get_Byte()!=0XFA)return 6;//传输失败
	PS2_Send_Cmd(0X64);//采样率100
    if(PS2_Get_Byte()!=0XFA)return 7;//传输失败
	PS2_Send_Cmd(SET_SAMPLE_RATE);//进入设置采样率
    if(PS2_Get_Byte()!=0XFA)return 8;//传输失败
	PS2_Send_Cmd(0X50);//采样率80
    if(PS2_Get_Byte()!=0XFA)return 9;//传输失败
	//序列完成		   
	PS2_Send_Cmd(GET_DEVICE_ID); //读取ID
    if(PS2_Get_Byte()!=0XFA)return 10;//传输失败
	MOUSE_ID=PS2_Get_Byte();//得到MOUSE ID	 

	PS2_Send_Cmd(SET_SAMPLE_RATE);//再次进入设置采样率
    if(PS2_Get_Byte()!=0XFA)return 11;//传输失败
	PS2_Send_Cmd(0X0A);//采样率10
    if(PS2_Get_Byte()!=0XFA)return 12;//传输失败		 
	PS2_Send_Cmd(GET_DEVICE_ID); //读取ID
    if(PS2_Get_Byte()!=0XFA)return 13;//传输失败
	MOUSE_ID=PS2_Get_Byte();//得到MOUSE ID		 

	PS2_Send_Cmd(SET_RESOLUTION);  //设置分辨率
    if(PS2_Get_Byte()!=0XFA)return 14;//传输失败   
 	PS2_Send_Cmd(0X03);//8点/mm
    if(PS2_Get_Byte()!=0XFA)return 15;//传输失败 
	PS2_Send_Cmd(SET_SCALING11);   //设置缩放比率为1:1
    if(PS2_Get_Byte()!=0XFA)return 16;//传输失败 
	  
 	PS2_Send_Cmd(SET_SAMPLE_RATE); //设置采样率
    if(PS2_Get_Byte()!=0XFA)return 17;//传输失败   
 	PS2_Send_Cmd(0X28);//40
    if(PS2_Get_Byte()!=0XFA)return 18;//传输失败 
	   
	PS2_Send_Cmd(EN_DATA_REPORT);   //使能数据报告
    if(PS2_Get_Byte()!=0XFA)return 19;//传输失败

	PS2_Status=MOUSE;//进入鼠标模式
	return 0;//无错误,初始化成功   
} 











