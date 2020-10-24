#include "jpegdisplay.h"
#include "EmWinHZFont.h"
#include "GUI.h"
#include "malloc.h"
#include "ff.h"
#include "ili93xx.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103战舰开发板
//STemwin JPEG图片显示 
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/4/10
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos 使用	  
#endif

static FIL JPEGFile;
static char jpegBuffer[JPEGPERLINESIZE];
/*******************************************************************
*
*       Static functions
*
********************************************************************
*/
/*********************************************************************
*
*       _GetData
*
* Function description
*   This routine is called by GUI_JPEG_DrawEx(). The routine is responsible
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
static int JpegGetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) 
{
	static int readaddress=0;
	FIL * phFile;
	UINT NumBytesRead;
#if SYSTEM_SUPPORT_OS
    CPU_SR_ALLOC();
#endif

	phFile = (FIL *)p;
	
	if (NumBytesReq > sizeof(jpegBuffer)) 
	{
		NumBytesReq = sizeof(jpegBuffer);
	}

	//移动指针到应该读取的位置
	if(Off == 1) readaddress = 0;
	else readaddress=Off;
	
#if SYSTEM_SUPPORT_OS
    OS_CRITICAL_ENTER();	//进入临界区
#endif
		
	f_lseek(phFile,readaddress); 
	
	//读取数据到缓冲区中
	f_read(phFile,jpegBuffer,NumBytesReq,&NumBytesRead);
	
#if SYSTEM_SUPPORT_OS
    OS_CRITICAL_EXIT();	//退出临界区
#endif
	
	*ppData = (U8 *)jpegBuffer;
	return NumBytesRead;//返回读取到的字节数
}

//在指定位置显示加载到RAM中的JPEG图片
//JPEGFileName:图片在SD卡或者其他存储设备中的路径(需文件系统支持！)
//mode:显示模式
//		0 在指定位置显示，有参数x,y确定显示位置
//		1 在LCD中间显示图片，当选择此模式的时候参数x,y无效。
//x:图片左上角在LCD中的x轴位置(当参数mode为1时，此参数无效)
//y:图片左上角在LCD中的y轴位置(当参数mode为1时，此参数无效)
//member:  缩放比例的分子项
//denom:缩放比例的分母项
//返回值:0 显示正常,其他 失败
int displayjpeg(u8 *JPEGFileName,u8 mode,u32 x,u32 y,int member,int denom)
{
	u16 bread;
	char *jpegbuffer;
	char result;
	int XSize,YSize;
	GUI_JPEG_INFO JpegInfo;
	float Xflag,Yflag;
	
#if SYSTEM_SUPPORT_OS
    CPU_SR_ALLOC();
#endif

	result = f_open(&JPEGFile,(const TCHAR*)JPEGFileName,FA_READ);	//打开文件
	//文件打开错误或者文件大于JPEGMEMORYSIZE
	if((result != FR_OK) || (JPEGFile.fsize>JPEGMEMORYSIZE)) 	return 1;
	
	jpegbuffer=mymalloc(SRAMEX,JPEGFile.fsize);	//申请内存
	if(jpegbuffer == NULL) return 2;
	
#if SYSTEM_SUPPORT_OS
    OS_CRITICAL_ENTER();	//进入临界区
#endif
		
	result = f_read(&JPEGFile,jpegbuffer,JPEGFile.fsize,(UINT *)&bread); //读取数据
//	if(result != FR_OK) return 3;
	
#if SYSTEM_SUPPORT_OS
    OS_CRITICAL_EXIT();	//退出临界区
#endif
    if(result != FR_OK) return 3;
	
	GUI_JPEG_GetInfo(jpegbuffer,JPEGFile.fsize,&JpegInfo); //获取JEGP图片信息
	XSize = JpegInfo.XSize;	//获取JPEG图片的X轴大小
	YSize = JpegInfo.YSize;	//获取JPEG图片的Y轴大小
	switch(mode)
	{
		case 0:	//在指定位置显示图片
			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
			{
				GUI_JPEG_Draw(jpegbuffer,JPEGFile.fsize,x,y);	//在指定位置显示JPEG图片
			}else //否则图片需要缩放
			{
				GUI_JPEG_DrawScaled(jpegbuffer,JPEGFile.fsize,x,y,member,denom);
			}
			break;
		case 1:	//在LCD中间显示图片
			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
			{
				//在LCD中间显示图片
				GUI_JPEG_Draw(jpegbuffer,JPEGFile.fsize,(lcddev.width-XSize)/2-1,(lcddev.height-YSize)/2-1);
			}else //否则图片需要缩放
			{
				Xflag = (float)XSize*((float)member/(float)denom);
				Yflag = (float)YSize*((float)member/(float)denom);
				XSize = (lcddev.width - 10 -(int)Xflag)/2-1;
				YSize = (lcddev.height - 55 - (int)Yflag)/2-1;
				GUI_JPEG_DrawScaled(jpegbuffer,JPEGFile.fsize,XSize,YSize,member,denom);
			}
			break;
	}
	f_close(&JPEGFile);			//关闭JPEGFile文件
	myfree(SRAMEX,jpegbuffer);	//释放内存
	return 0;
}

//在指定位置显示无需加载到RAM中的BMP图片(需文件系统支持！对于小RAM，推荐使用此方法！)
//JPEGFileName:图片在SD卡或者其他存储设备中的路径
//mode:显示模式
//		0 在指定位置显示，有参数x,y确定显示位置
//		1 在LCD中间显示图片，当选择此模式的时候参数x,y无效。
//x:图片左上角在LCD中的x轴位置(当参数mode为1时，此参数无效)
//y:图片左上角在LCD中的y轴位置(当参数mode为1时，此参数无效)
//member:  缩放比例的分子项
//denom:缩放比例的分母项
//返回值:0 显示正常,其他 失败
int displayjpegex(u8 *JPEGFileName,u8 mode,u32 x,u32 y,int member,int denom)
{
	char result;
	int XSize,YSize;
	float Xflag,Yflag;
	GUI_JPEG_INFO JpegInfo;
	
	result = f_open(&JPEGFile,(const TCHAR*)JPEGFileName,FA_READ);	//打开文件
	//文件打开错误
	if(result != FR_OK) 	return 1;
	
	GUI_JPEG_GetInfoEx(JpegGetData,&JPEGFile,&JpegInfo);
	XSize = JpegInfo.XSize;	//JPEG图片X大小
	YSize = JpegInfo.YSize;	//JPEG图片Y大小
	switch(mode)
	{
		case 0:	//在指定位置显示图片
			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
			{
				GUI_JPEG_DrawEx(JpegGetData,&JPEGFile,x,y);//在指定位置显示BMP图片
			}else //否则图片需要缩放
			{
				GUI_JPEG_DrawScaledEx(JpegGetData,&JPEGFile,x,y,member,denom);
			}
			break;
		case 1:	//在LCD中间显示图片
			if((member == 1) && (denom == 1)) //无需缩放，直接绘制
			{
				//在LCD中间显示图片
				GUI_JPEG_DrawEx(JpegGetData,&JPEGFile,(lcddev.width-XSize)/2-1,(lcddev.height-YSize)/2-1);
			}else //否则图片需要缩放
			{
				Xflag = (float)XSize*((float)member/(float)denom);
				Yflag = (float)YSize*((float)member/(float)denom);
				XSize = (lcddev.width - 10 -(int)Xflag)/2-1;
				YSize = (lcddev.height - 55 -(int)Yflag)/2-1;
				GUI_JPEG_DrawScaledEx(JpegGetData,&JPEGFile,XSize,YSize,member,denom);
			}
			break;
	}
	f_close(&JPEGFile);		//关闭BMPFile文件
	return 0;
}	

void jpegdisplay_demo(void)
{
	GUI_SetBkColor(GUI_BLUE);
	GUI_SetColor(GUI_RED);
	GUI_SetFont(&GUI_FontHZ16);
	GUI_Clear();
	
	while(1)
	{
		GUI_DispStringHCenterAt("在指定位置显示一张加载到RAM中的JPEG图片",240,0);
		displayjpeg("0:/PICTURE/JPEG/实例图片.jpg",0,0,0,	1,1);
		GUI_Delay(1000);
		GUI_Clear();
	
		GUI_DispStringHCenterAt("在LCD中间显示一张扩大2倍的加载到RAM中的JPEG图片",240,0);
		displayjpeg("0:/PICTURE/JPEG/jpeg0.jpg",1,0,	0,2,1);
		GUI_Delay(1000);
		GUI_Clear();
	
		GUI_DispStringHCenterAt("在LCD中间显示一张缩小1/2的加载到RAM中的JPEG图片",240,0);
		displayjpeg("0:/PICTURE/JPEG/SIM900A.jpg",1,0,0,1,2);
		GUI_Delay(1000);
		GUI_Clear();
		
		GUI_DispStringHCenterAt("在指定位置显示一张无需加载的JPEG图片",240,0);
		displayjpegex("0:/PICTURE/JPEG/实例图片.jpg",0,0,0,1,1);
		GUI_Delay(1000);
		GUI_Clear();
	
		GUI_DispStringHCenterAt("在LCD中间显示一张扩大2倍的无需加载的JPEG图片",240,0);
		displayjpegex("0:/PICTURE/JPEG/jpeg0.jpg",1,0,0,2,1);
		GUI_Delay(1000);
		GUI_Clear();
	
		GUI_DispStringHCenterAt("在LCD中间显示一张缩小1/2的无需加载的JPEG图片",240,0);
		displayjpegex("0:/PICTURE/JPEG/SIM900A.jpg",1,0,0,1,2);
		GUI_Delay(1000);
		GUI_Clear();
	}
}
