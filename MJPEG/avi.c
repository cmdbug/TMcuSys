#include "avi.h"
#include "mjpeg.h"
#include "usart.h"
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

AVI_INFO avix;					//avi文件相关信息 
u8*const AVI_VIDS_FLAG_TBL[2]={"00dc","01dc"};//视频编码标志字符串,00dc/01dc
u8*const AVI_AUDS_FLAG_TBL[2]={"00wb","01wb"};//视频编码标志字符串,00dc/01dc

//avi解码初始化
//buf:输入缓冲区
//size:缓冲区大小
//返回值:AVI_OK,avi文件解析成功
//         其他,错误代码
AVISTATUS avi_init(u8 *buf,u16 size)		 
{
	u16 offset;
	u8 *tbuf;
	AVISTATUS res=AVI_OK;
	AVI_HEADER *aviheader;
	LIST_HEADER *listheader;
	AVIH_HEADER *avihheader; 
	STRH_HEADER *strhheader; 
	
	STRF_BMPHEADER *bmpheader; 
	STRF_WAVHEADER *wavheader; 
	
	tbuf=buf;
	aviheader=(AVI_HEADER*)buf; 
	if(aviheader->RiffID!=AVI_RIFF_ID)return AVI_RIFF_ERR;		//RIFF ID错误
	if(aviheader->AviID!=AVI_AVI_ID)return AVI_AVI_ERR;			//AVI ID错误
	buf+=sizeof(AVI_HEADER);									//偏移
	listheader=(LIST_HEADER*)(buf);						
	if(listheader->ListID!=AVI_LIST_ID)return AVI_LIST_ERR;		//LIST ID错误 
	if(listheader->ListType!=AVI_HDRL_ID)return AVI_HDRL_ERR;	//HDRL ID错误 
	buf+=sizeof(LIST_HEADER);									//偏移
	avihheader=(AVIH_HEADER*)(buf);
	if(avihheader->BlockID!=AVI_AVIH_ID)return AVI_AVIH_ERR;	//AVIH ID错误 
	avix.SecPerFrame=avihheader->SecPerFrame;					//得到帧间隔时间
	avix.TotalFrame=avihheader->TotalFrame;						//得到总帧数  
	buf+=avihheader->BlockSize+8;								//偏移
	listheader=(LIST_HEADER*)(buf);
	if(listheader->ListID!=AVI_LIST_ID)return AVI_LIST_ERR;		//LIST ID错误 
	if(listheader->ListType!=AVI_STRL_ID)return AVI_STRL_ERR;	//STRL ID错误  
	strhheader=(STRH_HEADER*)(buf+12);
	if(strhheader->BlockID!=AVI_STRH_ID)return AVI_STRH_ERR;	//STRH ID错误 
 	if(strhheader->StreamType==AVI_VIDS_STREAM)					//视频帧在前
	{
		if(strhheader->Handler!=AVI_FORMAT_MJPG)return AVI_FORMAT_ERR;	//非MJPG视频流,不支持
		avix.VideoFLAG=(u8*)AVI_VIDS_FLAG_TBL[0];					//视频流标记  "00dc"
		avix.AudioFLAG=(u8*)AVI_AUDS_FLAG_TBL[1];					//音频流标记  "01wb"
		bmpheader=(STRF_BMPHEADER*)(buf+12+strhheader->BlockSize+8);//strf
		if(bmpheader->BlockID!=AVI_STRF_ID)return AVI_STRF_ERR;		//STRF ID错误  
		avix.Width=bmpheader->bmiHeader.Width;
		avix.Height=bmpheader->bmiHeader.Height; 
		buf+=listheader->BlockSize+8;								//偏移
		listheader=(LIST_HEADER*)(buf);
		if(listheader->ListID!=AVI_LIST_ID)//是不含有音频帧的视频文件
		{
			avix.SampleRate=0;						//音频采样率
			avix.Channels=0;						//音频通道数
			avix.AudioType=0;						//音频格式
			
		}else
		{			
			if(listheader->ListType!=AVI_STRL_ID)return AVI_STRL_ERR;	//STRL ID错误   
			strhheader=(STRH_HEADER*)(buf+12);
			if(strhheader->BlockID!=AVI_STRH_ID)return AVI_STRH_ERR;	//STRH ID错误 
			if(strhheader->StreamType!=AVI_AUDS_STREAM)return AVI_FORMAT_ERR;//格式错误
			wavheader=(STRF_WAVHEADER*)(buf+12+strhheader->BlockSize+8);//strf
			if(wavheader->BlockID!=AVI_STRF_ID)return AVI_STRF_ERR;		//STRF ID错误   
			avix.SampleRate=wavheader->SampleRate;						//音频采样率
			avix.Channels=wavheader->Channels;							//音频通道数
			avix.AudioType=wavheader->FormatTag;						//音频格式
		}
	}else if(strhheader->StreamType==AVI_AUDS_STREAM)		 		//音频帧在前
	{ 
		avix.VideoFLAG=(u8*)AVI_VIDS_FLAG_TBL[1];					//视频流标记  "01dc"
		avix.AudioFLAG=(u8*)AVI_AUDS_FLAG_TBL[0];					//音频流标记  "00wb"
		wavheader=(STRF_WAVHEADER*)(buf+12+strhheader->BlockSize+8);//strf
		if(wavheader->BlockID!=AVI_STRF_ID)return AVI_STRF_ERR;		//STRF ID错误 
		avix.SampleRate=wavheader->SampleRate;						//音频采样率
		avix.Channels=wavheader->Channels;							//音频通道数
		avix.AudioType=wavheader->FormatTag;						//音频格式
		buf+=listheader->BlockSize+8;								//偏移
		listheader=(LIST_HEADER*)(buf);
		if(listheader->ListID!=AVI_LIST_ID)return AVI_LIST_ERR;		//LIST ID错误 
		if(listheader->ListType!=AVI_STRL_ID)return AVI_STRL_ERR;	//STRL ID错误   
		strhheader=(STRH_HEADER*)(buf+12);
		if(strhheader->BlockID!=AVI_STRH_ID)return AVI_STRH_ERR;	//STRH ID错误 
		if(strhheader->StreamType!=AVI_VIDS_STREAM)return AVI_FORMAT_ERR;//格式错误  
		bmpheader=(STRF_BMPHEADER*)(buf+12+strhheader->BlockSize+8);//strf
		if(bmpheader->BlockID!=AVI_STRF_ID)return AVI_STRF_ERR;		//STRF ID错误  
		if(bmpheader->bmiHeader.Compression!=AVI_FORMAT_MJPG)return AVI_FORMAT_ERR;//格式错误  
		avix.Width=bmpheader->bmiHeader.Width;
		avix.Height=bmpheader->bmiHeader.Height; 	
	}
	offset=avi_srarch_id(tbuf,size,"movi");					//查找movi ID
	if(offset==0)return AVI_MOVI_ERR;						//MOVI ID错误
	if(avix.SampleRate)//有音频流,才查找
	{
		tbuf+=offset;
		offset=avi_srarch_id(tbuf,size,avix.AudioFLAG);			//查找音频流标记
		if(offset==0)return AVI_STREAM_ERR;						//流错误
		tbuf+=offset+4;
		avix.AudioBufSize=*((u16*)tbuf);						//得到音频流buf大小.
	}		
	printf("avi init ok\r\n");
	printf("avix.SecPerFrame:%d\r\n",avix.SecPerFrame);
	printf("avix.TotalFrame:%d\r\n",avix.TotalFrame);
	printf("avix.Width:%d\r\n",avix.Width);
	printf("avix.Height:%d\r\n",avix.Height);
	printf("avix.AudioType:%d\r\n",avix.AudioType);
	printf("avix.SampleRate:%d\r\n",avix.SampleRate);
	printf("avix.Channels:%d\r\n",avix.Channels);
	printf("avix.AudioBufSize:%d\r\n",avix.AudioBufSize);
	printf("avix.VideoFLAG:%s\r\n",avix.VideoFLAG); 
	printf("avix.AudioFLAG:%s\r\n",avix.AudioFLAG); 
	return res;
}

//查找 ID
//buf:待查缓存区
//size:缓存大小
//id:要查找的id,必须是4字节长度
//返回值:0,查找失败,其他:movi ID偏移量
u16 avi_srarch_id(u8* buf,u16 size,u8 *id)
{
	u16 i;
	size-=4;
	for(i=0;i<size;i++)
	{
	   	if(buf[i]==id[0])
			if(buf[i+1]==id[1])
				if(buf[i+2]==id[2])	
					if(buf[i+3]==id[3])return i;//找到"id"所在的位置	
	}
	return 0;		
}
//得到stream流信息
//buf:流开始地址(必须是01wb/00wb/01dc/00dc开头)
AVISTATUS avi_get_streaminfo(u8* buf)
{
	avix.StreamID=MAKEWORD(buf+2);			//得到流类型
	avix.StreamSize=MAKEDWORD(buf+4);		//得到流大小 
	if(avix.StreamSize%2)avix.StreamSize++;	//奇数加1(avix.StreamSize,必须是偶数)
	if(avix.StreamID==AVI_VIDS_FLAG||avix.StreamID==AVI_AUDS_FLAG)return AVI_OK;
	return AVI_STREAM_ERR;	
}




















