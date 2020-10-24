#ifndef __AUDIOPLAY_H
#define __AUDIOPLAY_H
#include "sys.h"
#include "includes.h" 	   	 
#include "common.h"
#include "ff.h"	
#include "gui.h"
//#include "lyric.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//APP-音乐播放器 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/7/10
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   

////////////////////////////////////////////////////////////////////////////////////////////
//各图标/图片路径  
extern u8*const AUDIO_BTN_PIC_TBL[2][5];  	//5个图片按钮的路径
extern u8*const AUDIO_BACK_PIC[3];			//三个背景图片
extern u8*const AUDIO_PLAYR_PIC;			//播放 松开
extern u8*const AUDIO_PLAYP_PIC;			//播放 按下
extern u8*const AUDIO_PAUSER_PIC;			//暂停 松开
extern u8*const AUDIO_PAUSEP_PIC;			//暂停 按下
////////////////////////////////////////////////////////////////////////////////////////////
#define AUDIO_TITLE_COLOR   	0XFFFF		//播放器标题颜色	
#define AUDIO_TITLE_BKCOLOR   	0X0000		//播放器标题背景色	

#define AUDIO_INFO_COLOR   		0X8410		//信息字体的颜色	
#define AUDIO_MAIN_BKCOLOR   	0X18E3		//主背景色	
#define AUDIO_BTN_BKCOLOR   	0XDF9F		//5个控制按钮背景色	
/////////////////////////////////////////////
#define AUDIO_LRC_MCOLOR       0XF810//0XFFE0		//当前歌词颜色为黄色
#define AUDIO_LRC_SCOLOR       0XFFFF//0X07FF		//前一句和后一句歌词颜色为青色			    
 
//音乐播放操作结果定义
typedef enum {
	AP_OK=0X00,				//正常播放完成
	AP_NEXT,				//播放下一曲
	AP_PREV,				//播放上一曲
	AP_ERR=0X80,			//播放有错误(没定义错误代码,仅仅表示出错) 
}APRESULT;

//音乐播放控制器
typedef __packed struct
{  
	//2个I2S解码的BUF
	FIL *file;			//音频文件指针 	
	u32(*file_seek)(u32);//文件快进快退函数 

	vu8 status;			//bit0:0,暂停播放;1,继续播放
						//bit1:0,结束播放;1,开启播放  
						//bit2~3:保留
						//bit4:0,无音乐播放;1,音乐播放中 (对外标记)		
						//bit5:0,无动作;1,执行了一次切歌操作(对外标记)
						//bit6:0,无动作;1,请求终止播放(但是不删除音频播放任务),处理完成后,播放任务自动清零该位
 						//bit7:0,音频播放任务已删除/请求删除;1,音频播放任务正在运行(允许继续执行)
	
	u8 mode;			//播放模式
						//0,全部循环;1,单曲循环;2,随机播放;
	
	u8 *path;			//当前文件夹路径
	u8 *name;			//当前播放的MP3歌曲名字
	u16 namelen;		//name所占的点数.
	u16 curnamepos;		//当前的偏移

    u32 totsec ;		//整首歌时长,单位:秒
    u32 cursec ;		//当前播放时长 
    u32 bitrate;	   	//比特率(位速)
	u32 samplerate;		//采样率 
	u16 bps;			//位数,比如16bit,24bit,32bit
	
	u16 curindex;		//当前播放的音频文件索引
	u16 mfilenum;		//音乐文件数目	    
	u16 *mfindextbl;	//音频文件索引表
	
}__audiodev; 
extern __audiodev audiodev;	//音乐播放控制器

//取2个值里面的较小值.
#ifndef AUDIO_MIN			
#define AUDIO_MIN(x,y)	((x)<(y)? (x):(y))
#endif

void audio_start(void);
void audio_stop(void); 

void music_play_task(void *pdata);
void audio_stop_req(__audiodev *audiodevx);
u8 audio_filelist(__audiodev *audiodevx);
void audio_load_ui(void);
void audio_show_vol(u8 pctx);
void audio_time_show(u16 sx,u16 sy,u16 sec); 
void audio_info_upd(__audiodev *audiodevx,_progressbar_obj* audioprgbx,_progressbar_obj* volprgbx,_lyric_obj* lrcx);
void audio_lrc_bkcolor_process(_lyric_obj* lrcx,u8 mode);
void audio_lrc_show(__audiodev *audiodevx,_lyric_obj* lrcx);
u8 audio_task_creat(void);
void audio_task_delete(void);
u8 audio_play(void);	    

#endif












