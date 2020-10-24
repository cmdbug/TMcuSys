#include "pngdisplay.h"
#include "EmWinHZFont.h"
#include "GUI.h"
#include "malloc.h"
#include "ff.h"
#include "ili93xx.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103战舰开发板
//STemwin PNG图片显示 
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/4/14
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////////	

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif

static FIL PNGFile;
//static char pngBuffer[PNGPERLINESIZE];
/*******************************************************************
*
*       Static functions
*
********************************************************************
*/
/*********************************************************************
*
*       PngGetData
*
* Function description
*   This routine is called by GUI_PNG_DrawEx(). The routine is responsible
*   for setting the data pointer to a valid data location with at least
*   one valid byte.
*
* Parameters:
*   p           - Pointer to application defined data.
*   NumBytesReq - Number of bytes requested.
*   ppData      - Pointer to data pointer. This pointer should be set to
*                 a valid location.
*   StartOfFile - If this flag is 1, the data pointer should be set to the
*                 beginning of the data stream.
*
* Return value:
*   Number of data bytes available.
*/
static int PngGetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
{
	static int readaddress=0;
	FIL * phFile;
	U8 *pData;
	UINT NumBytesRead;
#if SYSTEM_SUPPORT_OS
    CPU_SR_ALLOC();
#endif
	
	pData = (U8*)*ppData;
	phFile = (FIL *)p;
	
	//移动指针到应该读取的位置
	if(Off == 1) readaddress = 0;
	else readaddress=Off;
	
#if SYSTEM_SUPPORT_OS
    OS_CRITICAL_ENTER();	//进入临界区
#endif
	f_lseek(phFile,readaddress); 
	
	//读取数据到缓冲区中
	f_read(phFile,pData,NumBytesReq,&NumBytesRead);
	
		
#if SYSTEM_SUPPORT_OS
    OS_CRITICAL_EXIT();	//退出临界区
#endif
	return NumBytesRead;//返回读取到的字节数
}

//在指定位置显示加载到RAM中的PNG图片(PNG图片不能缩放！)
//PNGFileName:图片在SD卡或者其他存储设备中的路径(需文件系统支持！)
//mode:显示模式
//		0 在指定位置显示，有参数x,y确定显示位置
//		1 在LCD中间显示图片，当选择此模式的时候参数x,y无效。
//x:图片左上角在LCD中的x轴位置(当参数mode为1时，此参数无效)
//y:图片左上角在LCD中的y轴位置(当参数mode为1时，此参数无效)
//返回值:0 显示正常,其他 失败
int displaypng(char *PNGFileName,u8 mode,u32 x,u32 y)
{
	u16 bread;
	char *pngbuffer;
	char result;
	int XSize,YSize;
	
#if SYSTEM_SUPPORT_OS
    CPU_SR_ALLOC();
#endif

	result = f_open(&PNGFile,(const TCHAR*)PNGFileName,FA_READ);	//打开文件
	//文件打开错误或者文件大于JPEGMEMORYSIZE
	if((result != FR_OK) || (PNGFile.fsize>PNGMEMORYSIZE)) 	return 1;

	pngbuffer=mymalloc(SRAMEX,PNGFile.fsize);
	if(pngbuffer == NULL) return 2;
	
#if SYSTEM_SUPPORT_OS
    OS_CRITICAL_ENTER();	//进入临界区
#endif
		
	result = f_read(&PNGFile,pngbuffer,PNGFile.fsize,(UINT *)&bread); //读取数据
//	if(result != FR_OK) return 3;
	
#if SYSTEM_SUPPORT_OS
    OS_CRITICAL_EXIT();	//退出临界区
#endif
    if(result != FR_OK) return 3;
	
	XSize = GUI_PNG_GetXSize(pngbuffer,PNGFile.fsize);	//获取PNG图片的X轴大小
	YSize = GUI_PNG_GetYSize(pngbuffer,PNGFile.fsize);	//获取PNG图片的Y轴大小
	switch(mode)
	{
		case 0:	//在指定位置显示图片
			GUI_PNG_Draw(pngbuffer,PNGFile.fsize,x,y);//显示PNG图片
			break;
		case 1:	//在LCD中间显示图片
			GUI_PNG_Draw(pngbuffer,PNGFile.fsize,(lcddev.width - 10 -XSize)/2-1,(lcddev.height - 55 -YSize)/2-1);	
			break;
	}
	f_close(&PNGFile);		//关闭PNGFile文件
	myfree(SRAMEX,pngbuffer);
	return 0;
}

//在指定位置显示无需加载到RAM中的PNG图片(需文件系统支持！对于小RAM，推荐使用此方法！PNG图片不能缩放！)
//PNGFileName:图片在SD卡或者其他存储设备中的路径
//mode:显示模式
//		0 在指定位置显示，有参数x,y确定显示位置
//		1 在LCD中间显示图片，当选择此模式的时候参数x,y无效。
//x:图片左上角在LCD中的x轴位置(当参数mode为1时，此参数无效)
//y:图片左上角在LCD中的y轴位置(当参数mode为1时，此参数无效)
//返回值:0 显示正常,其他 失败
int displaypngex(char *PNGFileName,u8 mode,u32 x,u32 y)
{
	char result;
	int XSize,YSize;

	result = f_open(&PNGFile,(const TCHAR*)PNGFileName,FA_READ);	//打开文件
	//文件打开错误
	if(result != FR_OK)	return 1;
	
	XSize = GUI_PNG_GetXSizeEx(PngGetData,&PNGFile);//PNG图片X大小
	YSize = GUI_PNG_GetYSizeEx(PngGetData,&PNGFile);//PNG图片Y大小
	switch(mode)
	{
		case 0:	//在指定位置显示图片
			GUI_PNG_DrawEx(PngGetData,&PNGFile,x,y);
			break;
		case 1:	//在LCD中间显示图片
			GUI_PNG_DrawEx(PngGetData,&PNGFile,(lcddev.width - 10 -XSize)/2-1,(lcddev.height - 55 -YSize)/2-1);
			break;
	}
	f_close(&PNGFile);	//关闭PNGFile文件
	return 0;
}

//PNG图片显示例程
void pngdisplay_demo(void)
{
	GUI_SetBkColor(GUI_BLUE);
	GUI_SetFont(&GUI_FontHZ16);
	GUI_SetColor(GUI_RED);
	GUI_Clear();
	while(1)
	{
		GUI_DispStringHCenterAt("在指定位置显示一张加载到RAM中的PNG图片",400,0);
		displaypng("0:/PICTURE/PNG/皇冠.png",0,200,100);
		GUI_Delay(1000);
		GUI_Clear();

		displaypng("0:/PICTURE/PNG/CS.png",1,0,0);
		GUI_Delay(1000);
		GUI_Clear();	
		
		displaypng("0:/PICTURE/PNG/香蕉.png",1,0,0);
		GUI_Delay(1000);
		GUI_Clear();
		
		GUI_DispStringHCenterAt("在指定位置显示一张无需加载的PNG图片",400,0);
		displaypngex("0:/PICTURE/PNG/驴.png",0,200,100);
		GUI_Delay(1000);
		GUI_Clear();
		
		displaypngex("0:/PICTURE/PNG/文件夹.png",1,0,0);
		GUI_Delay(1000);
		GUI_Clear();
		
		displaypngex("0:/PICTURE/PNG/风扇.png",1,0,0);
		GUI_Delay(1000);
		GUI_Clear();
	}
}
