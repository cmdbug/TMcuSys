#include "sys.h"
#include "usart.h"
#include "pymb.h"
#include "pyinput.h"
#include "string.h"
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

//拼音输入法
pyinput t9=
{
	get_pymb,
	0,
};

//比较两个字符串的匹配情况
//返回值：0xff，表示完全匹配
//        其他，匹配的字符数
u8 str_match(u8 *str1, u8 *str2)
{
	u8 i=0;
	while(1)
	{
		if(*str1 != *str2)break;			//部分匹配
		if(*str1=='\0'){i=0XFF;break;}     //完全匹配
		i++;
		str1++;
		str2++;
	}
	return i;								//两个字符串相等
}

//获取匹配的拼音码表
//*strin，输入的字符串，形如:"726"
//*matchlist,输出的匹配表
//返回值:[7],0,表示完全匹配；1，表示部分匹配（仅在没有完全匹配的时候才会出现）
//		 [6:0],完全匹配的时候，表示完全匹配的拼音个数
//			   部分匹配的时候，表示有效匹配的位数	
u8 get_matched_pymb(u8 *strin,py_index **matchlist)
{
	py_index *bestmatch=0;	//最佳匹配
	u16 pyindex_len=0;
	u16 i=0;
	u8 temp,mcnt=0,bmcnt=0;
	bestmatch=(py_index*)&py_index3[0];	//默认为a的匹配
	pyindex_len=sizeof(py_index3)/sizeof(py_index3[0]);//得到py索引表的大小
	for(i=0;i<pyindex_len;i++)
	{
		temp=str_match(strin,(u8*)py_index3[i].py_input);
		if(temp)
		{
			if(temp==0XFF)matchlist[mcnt++]=(py_index*)&py_index3[i];	//完全匹配
			else if(temp>bmcnt) //找最佳匹配
			{
				bmcnt=temp;
				bestmatch=(py_index*)&py_index3[i];//最好的匹配
			}
		}
	}
	if(mcnt==0&&bmcnt)	//没有完全匹配的结果，但是有部分匹配的结果
	{
		matchlist[0]=bestmatch;
		mcnt=bmcnt|0x80;	//返回部分匹配的有效位数
	}
	return mcnt;			//返回匹配的个数
}

//得到拼音码表
//str:输入字符串
//返回值：匹配个数
u8 get_pymb(u8* str)
{
	return get_matched_pymb(str,t9.pymb);
}

//void test_py(u8 *inputstr)
//{
//	u8 t=0;
//	u8 i=0;
//	t=t9.getpymb(inputstr);
//	if(t&0x80)	//部分匹配
//	{
//		printf("\r\n输入数字为:%s\r\n",inputstr);
//		printf("部分匹配位数:%d\r\n",t&0x7F);
//		printf("部分匹配结果:%s,%s\r\n",t9.pymb[0]->py,t9.pymb[0]->pymb);
//	}else if(t)
//	{
//		printf("\r\n输入数字为:%s\r\n",inputstr);
//		printf("完全匹配个数:%d\r\n",t);
//		printf("完全匹配的结果:\r\n");
//		for(i=0;i<t;i++)
//		{
//			printf("%s,%s\r\n",t9.pymb[i]->py,t9.pymb[i]->pymb);
//		}
//	}else printf("没有任何匹配结果!\r\n");
//}

