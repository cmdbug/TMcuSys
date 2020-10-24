#include "fontupd.h"
#include "ff.h"	  
#include "w25qxx.h"   
#include "ILI93xx.h"  
#include "string.h"
#include "malloc.h"
#include "delay.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//字库更新 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/18
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
						   
u32 FONTINFOADDR=(1024*6+500)*1024;//默认是6M的地址
//字库信息结构体. 
//用来保存字库基本信息，地址，大小等
_font_info ftinfo;

//在sd卡中的路径
const u8 *GBK16_SDPATH="0:/SYSTEM/FONT/GBK16.FON";		//GBK16的存放位置
const u8 *GBK12_SDPATH="0:/SYSTEM/FONT/GBK12.FON";		//GBK12的存放位置
const u8 *UNIGBK_SDPATH="0:/SYSTEM/FONT/UNIGBK.BIN";	//UNIGBK.BIN的存放位置
//在25Qxx中的路径
const u8 *GBK16_25QPATH="1:/SYSTEM/FONT/GBK16.FON";		//GBK16的存放位置
const u8 *GBK12_25QPATH="1:/SYSTEM/FONT/GBK12.FON";		//GBK12的存放位置
const u8 *UNIGBK_25QPATH="1:/SYSTEM/FONT/UNIGBK.BIN";	//UNIGBK.BIN的存放位置

//显示当前字体更新进度
//x,y:坐标
//size:字体大小
//fsize:整个文件大小
//pos:当前文件指针位置
u32 fupd_prog(u16 x,u16 y,u8 size,u32 fsize,u32 pos)
{
	float prog;
	u8 t=0XFF;
	prog=(float)pos/fsize;
	prog*=100;
	if(t!=prog)
	{
		LCD_ShowString(x+3*size/2,y,240,320,size,"%", 0, POINT_COLOR);		
		t=prog;
		if(t>100)t=100;
		LCD_ShowNum(x,y,t,3,size, 0, POINT_COLOR);//显示数值
	}
	return 0;					    
} 
//更新某一个
//x,y:坐标
//size:字体大小
//fxpath:路径
//fx:更新的内容 0,ungbk;1,gbk12;2,gbk16;
//返回值:0,成功;其他,失败.
u8 updata_fontx(u16 x,u16 y,u8 size,u8 *fxpath,u8 fx)
{
	u32 flashaddr=0;								    
	FIL * fftemp;
	u8 *tempbuf;
 	u8 res;	
	u16 bread;
	u32 offx=0;
	u8 rval=0;	     
	fftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));	//分配内存	
	if(fftemp==NULL)rval=1;
	tempbuf=mymalloc(SRAMIN,4096);	//分配4096个字节空间
	if(tempbuf==NULL)rval=1;
 	res=f_open(fftemp,(const TCHAR*)fxpath,FA_READ); 
 	if(res)rval=2;//打开文件失败  
 	if(rval==0)	 
	{
		if(fx==0)		//更新UNIGBK.BIN
		{
			ftinfo.ugbkaddr=FONTINFOADDR+sizeof(ftinfo);//信息头之后，紧跟UNIGBK转换码表
  			ftinfo.ugbksize=fftemp->fsize;				//UNIGBK大小
 			flashaddr=ftinfo.ugbkaddr;
		}else if(fx==1)	//GBK12
		{				  
			ftinfo.f12addr=ftinfo.ugbkaddr+ftinfo.ugbksize;		//UNIGBK之后，紧跟GBK12字库
			ftinfo.gbk12size=fftemp->fsize;						//GBK12字库大小
			flashaddr=ftinfo.f12addr;							//GBK12的起始地址
		}else			//GBK16
		{
			ftinfo.f16addr=ftinfo.f12addr+ftinfo.gbk12size;		//GBK12之后，紧跟GBK16字库
			ftinfo.gbk16size=fftemp->fsize;						//GBK16字库大小
			flashaddr=ftinfo.f16addr;							//GBK16的起始地址
		}	   
		while(res==FR_OK)//死循环执行
		{
	 		res=f_read(fftemp,tempbuf,4096,(UINT *)&bread);		//读取数据	 
			if(res!=FR_OK)break;								//执行错误
			W25QXX_Write(tempbuf,offx+flashaddr,4096);		//从0开始写入4096个数据  
	  		offx+=bread;	  
			fupd_prog(x,y,size,fftemp->fsize,offx);	 			//进度显示
			if(bread!=4096)break;								//读完了.
	 	} 	
		f_close(fftemp);		
	}			 
	myfree(SRAMIN,fftemp);	//释放内存
	myfree(SRAMIN,tempbuf);	//释放内存
	return res;
}

//更新字体文件,UNIGBK,GBK12,GBK16一起更新
//x,y:提示信息的显示地址
//size:字体大小
//提示信息字体大小
//src:0,从SD卡更新.
//	  1,从25QXX更新											  
//返回值:0,更新成功;
//		 其他,错误代码.	  
u8 update_font(u16 x,u16 y,u8 size,u8 src)
{	
	u8 *gbk16_path;
	u8 *gbk12_path;
	u8 *unigbk_path;
	u8 res;		  
	if(src)//从25qxx更新
	{
		unigbk_path=(u8*)UNIGBK_25QPATH;
		gbk12_path=(u8*)GBK12_25QPATH;
		gbk16_path=(u8*)GBK16_25QPATH;
	}else//从sd卡更新
	{
		unigbk_path=(u8*)UNIGBK_SDPATH;
		gbk12_path=(u8*)GBK12_SDPATH;
		gbk16_path=(u8*)GBK16_SDPATH;
	}   
 	res=0XFF;		
	ftinfo.fontok=0XFF;
  	W25QXX_Write((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));	//清除之前字库成功的标志.防止更新到一半重启,导致的字库部分数据丢失.
 	W25QXX_Read((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));	//重新读出ftinfo结构体数据
 	LCD_ShowString(x,y,240,320,size,"Updating UNIGBK.BIN", 0, POINT_COLOR);		
	res=updata_fontx(x+20*size/2,y,size,unigbk_path,0);			//更新UNIGBK.BIN
	if(res)return 1;
 	LCD_ShowString(x,y,240,320,size,"Updating GBK12.FON  ", 0, POINT_COLOR);
	res=updata_fontx(x+20*size/2,y,size,gbk12_path,1);			//更新GBK12.FON
	if(res)return 2;
	LCD_ShowString(x,y,240,320,size,"Updating GBK16.FON  ", 0, POINT_COLOR);
	res=updata_fontx(x+20*size/2,y,size,gbk16_path,2);			//更新GBK16.FON
	if(res)return 3;	   
	//全部更新好了
	ftinfo.fontok=0XAA;
  	W25QXX_Write((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));	//保存字库信息
	return 0;//无错误.		 
} 
//初始化字体
//返回值:0,字库完好.
//		 其他,字库丢失
u8 font_init(void)
{			  												 
	W25QXX_Init();
	FONTINFOADDR=(1024*6+500)*1024;			//W25Q64,6M以后	 
	ftinfo.ugbkaddr=FONTINFOADDR+25;		//UNICODEGBK 表存放首地址固定地址
	W25QXX_Read((u8*)&ftinfo,FONTINFOADDR,sizeof(ftinfo));//读出ftinfo结构体数据
	if(ftinfo.fontok!=0XAA)return 1;		//字库错误. 
	return 0;		    
}






