#ifndef _PYINPUT_H
#define _PYINPUT_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//拼音输入法 代码	   
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

#define MAX_MATCH_PYMB 	10	//最大匹配数

//拼音码表与拼音的对应表
typedef struct
{
	u8 *py_input;	//输入的字符串
	u8 *py;			//输入的拼音
	u8 *pymb;		//码表
}py_index;

//拼音输入法
typedef struct
{
	u8(*getpymb)(u8 *instr);		//字符串到码表获取函数
	py_index *pymb[MAX_MATCH_PYMB];	//码表存放位置
}pyinput;

extern pyinput t9;
u8 str_match(u8*str1,u8*str2);
u8 get_matched_pymb(u8 *strin,py_index **matchlist);
u8 get_pymb(u8* str);
//void test_py(u8 *inputstr);
#endif
