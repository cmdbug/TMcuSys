#ifndef __SMS_SN76496_H
#define __SMS_SN76496_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的NES模拟器工程
//ALIENTEK STM32F407开发板   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/10/12
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 

#define MAX_76496      4
#define MASTER_CLOCK   3579545
#define Sound_Rate     22050 //采样率
#define SNBUF_size     Sound_Rate/60   //每帧数据量		// 44100/60=735		22050/60=367
#define	sms_Frame      2			//帧音频数据缓存数目
 

typedef struct 
{
	int  Channel;
	int  SampleRate; 
	int  VolTable[16];
	int  Register[8];  
	int  Volume[4];
	int  LastRegister;
	int  NoiseFB;
	int  Period[4];
	int  Count[4];
	int  Output[4];
	unsigned int RNG_A;
	unsigned int UpdateStep;
} t_SN76496;
extern t_SN76496 *SN76496;
extern u16 * psg_buffer;		//音频数据缓存,大小为:SNBUF_size*2字节

void SN76496Write(int data);
void SN76496Update(short *buffer,int length,unsigned char mask);
void SN76496_set_clock(int clock);
void SN76496_set_gain(int gain);
int  SN76496_init(int clock,int volume,int sample_rate);

u8 sms_audio_init(void);
void sms_update_Sound(void);
#endif
