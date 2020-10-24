#ifndef __AVI_H
#define __AVI_H 
//#include <stm32f4xx.h>
#include "includes.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//AVI视频格式解析 代码	   
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

//错误类型
typedef enum {
	AVI_OK=0,				//0,成功
	AVI_RIFF_ERR,			//1,RIFF ID读取失败
	AVI_AVI_ERR,			//2,AVI ID读取失败
	AVI_LIST_ERR,			//3,LIST ID读取失败
	AVI_HDRL_ERR,			//4,HDRL ID读取失败
	AVI_AVIH_ERR,			//5,AVIH ID读取失败
	AVI_STRL_ERR,			//6,STRL ID读取失败
	AVI_STRH_ERR,			//7,STRH ID读取失败
	AVI_STRF_ERR,			//8,STRF ID读取失败
	AVI_MOVI_ERR,			//9,MOVI ID读取失败
	AVI_FORMAT_ERR,			//10,格式错误
	AVI_STREAM_ERR,			//11,流错误
}AVISTATUS;



#define AVI_RIFF_ID			0X46464952  
#define AVI_AVI_ID			0X20495641
#define AVI_LIST_ID			0X5453494C  
#define AVI_HDRL_ID			0X6C726468		//信息块标志
#define AVI_MOVI_ID			0X69766F6D 		//数据块标志
#define AVI_STRL_ID			0X6C727473		//strl标志

#define AVI_AVIH_ID			0X68697661 		//avih子块∈AVI_HDRL_ID
#define AVI_STRH_ID			0X68727473		//strh(流头)子块∈AVI_STRL_ID
#define AVI_STRF_ID			0X66727473 		//strf(流格式)子块∈AVI_STRL_ID
#define AVI_STRD_ID			0X64727473 		//strd子块∈AVI_STRL_ID (可选的)

#define AVI_VIDS_STREAM		0X73646976		//视频流
#define AVI_AUDS_STREAM		0X73647561 		//音频流


#define AVI_VIDS_FLAG		0X6463			//视频流标志
#define AVI_AUDS_FLAG		0X7762 			//音频流标志
//////////////////////////////////////////////////////////////////////////////////////////

#define AVI_FORMAT_MJPG		0X47504A4D  


//AVI 信息结构体
//将一些重要的数据,存放在这里,方便解码
typedef __packed struct
{	  
	u32 SecPerFrame;		//视频帧间隔时间(单位为us)
	u32 TotalFrame;			//文件总帧数
	u32 Width;				//图像宽
	u32 Height;				//图像高
	u32 SampleRate; 		//音频采样率
	u16 Channels;	  		//声道数,一般为2,表示立体声 
	u16 AudioBufSize;		//音频缓冲区大小
	u16 AudioType;	  		//音频类型:0X0001=PCM;0X0050=MP2;0X0055=MP3;0X2000=AC3;
	u16 StreamID;			//流类型ID,StreamID=='dc'==0X6463 /StreamID=='wb'==0X7762
	u32 StreamSize;			//流大小,必须是偶数,如果读取到为奇数,则加1.补为偶数.
	u8* VideoFLAG;			//视频帧标记,VideoFLAG="00dc"/"01dc"
	u8* AudioFLAG;			//音频帧标记,AudioFLAG="00wb"/"01wb"
}AVI_INFO;

extern AVI_INFO avix;		//avi文件相关信息

//////////////////////////////////////////////////////////////////////////////////////////
//AVI 块信息
typedef struct
{	
	u32 RiffID;				//RiffID=='RIFF'==0X61766968
	u32 FileSize;			//AVI文件大小(不包含最初的8字节,也RIFFID和FileSize不计算在内)
	u32 AviID;				//AviID=='AVI '==0X41564920 
}AVI_HEADER;

//AVI 块信息
typedef struct
{	
	u32 FrameID;			//帧ID,FrameID=='RIFF'==0X61766968
	u32 FrameSize;			//帧大小 
}FRAME_HEADER;


//LIST 块信息
typedef struct
{	
	u32 ListID;				//ListID=='LIST'==0X4c495354
	u32 BlockSize;			//块大小(不包含最初的8字节,也ListID和BlockSize不计算在内)
	u32 ListType;			//LIST子块类型:hdrl(信息块)/movi(数据块)/idxl(索引块,非必须,是可选的)
}LIST_HEADER;

//avih 子块信息
typedef struct
{	
	u32 BlockID;			//块标志:avih==0X61766968
	u32 BlockSize;			//块大小(不包含最初的8字节,也就是BlockID和BlockSize不计算在内)
	u32 SecPerFrame;		//视频帧间隔时间(单位为us)
	u32 MaxByteSec; 		//最大数据传输率,字节/秒
	u32 PaddingGranularity; //数据填充的粒度
	u32 Flags;				//AVI文件的全局标记，比如是否含有索引块等
	u32 TotalFrame;			//文件总帧数
	u32 InitFrames;  		//为交互格式指定初始帧数（非交互格式应该指定为0）
	u32 Streams;			//包含的数据流种类个数,通常为2
	u32 RefBufSize;			//建议读取本文件的缓存大小（应能容纳最大的块）默认可能是1M字节!!!
	u32 Width;				//图像宽
	u32 Height;				//图像高
	u32 Reserved[4];		//保留
}AVIH_HEADER;

//strh 流头子块信息(strh∈strl)
typedef struct
{	
	u32 BlockID;			//块标志:strh==0X73747268
	u32 BlockSize;			//块大小(不包含最初的8字节,也就是BlockID和BlockSize不计算在内)
	u32 StreamType;			//数据流种类，vids(0X73646976):视频;auds(0X73647561):音频
	u32 Handler;			//指定流的处理者，对于音视频来说就是解码器,比如MJPG/H264之类的.
	u32 Flags;  			//标记：是否允许这个流输出？调色板是否变化？
	u16 Priority;			//流的优先级（当有多个相同类型的流时优先级最高的为默认流）
	u16 Language;			//音频的语言代号
	u32 InitFrames;  		//为交互格式指定初始帧数
	u32 Scale;				//数据量, 视频每桢的大小或者音频的采样大小
	u32 Rate; 				//Scale/Rate=每秒采样数
	u32 Start;				//数据流开始播放的位置，单位为Scale
	u32 Length;				//数据流的数据量，单位为Scale
 	u32 RefBufSize;  		//建议使用的缓冲区大小
    u32 Quality;			//解压缩质量参数，值越大，质量越好
	u32 SampleSize;			//音频的样本大小
	struct					//视频帧所占的矩形 
	{				
	   	short Left;
		short Top;
		short Right;
		short Bottom;
	}Frame;				
}STRH_HEADER;

//BMP结构体
typedef struct
{
	u32	 BmpSize;			//bmp结构体大小,包含(BmpSize在内)
 	long Width;				//图像宽
	long Height;			//图像高
	u16  Planes;			//平面数，必须为1
	u16  BitCount;			//像素位数,0X0018表示24位
	u32  Compression;		//压缩类型，比如:MJPG/H264等
	u32  SizeImage;			//图像大小
	long XpixPerMeter;		//水平分辨率
	long YpixPerMeter;		//垂直分辨率
	u32  ClrUsed;			//实际使用了调色板中的颜色数,压缩格式中不使用
	u32  ClrImportant;		//重要的颜色
}BMP_HEADER;

//颜色表
typedef struct 
{
	u8  rgbBlue;			//蓝色的亮度(值范围为0-255)
	u8  rgbGreen; 			//绿色的亮度(值范围为0-255)
	u8  rgbRed; 			//红色的亮度(值范围为0-255)
	u8  rgbReserved;		//保留，必须为0
}AVIRGBQUAD;

//对于strh,如果是视频流,strf(流格式)使STRH_BMPHEADER块
typedef struct 
{
	u32 BlockID;			//块标志,strf==0X73747266
	u32 BlockSize;			//块大小(不包含最初的8字节,也就是BlockID和本BlockSize不计算在内)
	BMP_HEADER bmiHeader;  	//位图信息头
	AVIRGBQUAD bmColors[1];	//颜色表
}STRF_BMPHEADER;  

//对于strh,如果是音频流,strf(流格式)使STRH_WAVHEADER块
typedef struct 
{
	u32 BlockID;			//块标志,strf==0X73747266
	u32 BlockSize;			//块大小(不包含最初的8字节,也就是BlockID和本BlockSize不计算在内)
   	u16 FormatTag;			//格式标志:0X0055
	u16 Channels;	  		//声道数,一般为2,表示立体声
	u32 SampleRate; 		//音频采样率
	u32 BaudRate;   		//波特率 
	u16 BlockAlign; 		//数据块对齐标志
	u16 Size;				//该结构大小
}STRF_WAVHEADER;

#define	 MAKEWORD(ptr)	(u16)(((u16)*((u8*)(ptr))<<8)|(u16)*(u8*)((ptr)+1))
#define  MAKEDWORD(ptr)	(u32)(((u16)*(u8*)(ptr)|(((u16)*(u8*)(ptr+1))<<8)|\
						(((u16)*(u8*)(ptr+2))<<16)|(((u16)*(u8*)(ptr+3))<<24))) 


AVISTATUS avi_init(u8 *buf,u16 size);		//初始化avi解码器
u16 avi_srarch_id(u8* buf,u16 size,u8* id);	//查找ID,ID必须是4个字节长度
AVISTATUS avi_get_streaminfo(u8* buf);		//获取流信息


u8 Avih_Parser(u8 *buffer);
u8 Strl_Parser(u8 *buffer);
u8 Strf_Parser(u8 *buffer);



u16 Search_Movi(u8* buffer);
u16 Search_Fram(u8* buffer);
u32 ReadUnit(u8 *buffer,u8 index,u8 Bytes,u8 Format);

#endif

