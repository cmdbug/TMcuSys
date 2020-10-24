#ifndef __RDA5820_H
#define __RDA5820_H	 
#include "sys.h"
#include "myiic.h"   
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//RDA5820 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/14
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

#define RDA5820_R00		0X00  //读出16个位的ID =0X5820
#define RDA5820_R02		0X02  //DHIZ[15],DMUTE[14],MONO[13],BASS[12],SEEKUP[9],SEEK[8],SKMODE[7],CLK_MODE[6:4],ANTENNA[2],SOFTRESET[1],ENABLE[0]
#define RDA5820_R03		0X03  //CHAN[15:6],TUNE[4],BAND[3:2],SPACE[1:0]
#define RDA5820_R04		0X04  //STCIEN[14](搜索完成中断),DE[11],I2S_ENABLE[6],GPIO3~1[5:0]
#define RDA5820_R05		0X05  //INT_MODE[15],SEEKTH[14:8](设定信号强度),LNA_PORT_SEL[7:6],LNA_ICSEL_BIT[5:4],VOLUME[3:0];
#define RDA5820_R0A		0X0A  //STC[14](搜索完成标志),SF[13](搜索失败标志),ST[10](立体声标志),READCHAN[9:0](频率).
#define RDA5820_R0B		0X0B  //RSSI[15:9](信号强度),FM_TRUE[8](站点标志),FM_READY[7](fm就绪标志)
#define RDA5820_R40		0X40  //AUTO_SEEK[15](全/半自动搜台),CHAN_SCAN[14](搜空/忙台),CHIP_FUN[3:0](工作模式)
#define RDA5820_R41		0X41  //MEM_CLR[15](内存清除),RPT_TIME[14:9](RDS应答次数),MEM_DEPTH[8:0](FIFO大小,最大256)
#define RDA5820_R42		0X42  //TX_SIG_GAIN[10:8],TX_PA_GAIN[5:0](发射功率).
#define RDA5820_R4A		0X4A  //MEM_INTMOD[4:0]
#define RDA5820_R4B		0X4B  //CHAN_NUM[7:0](仅在全自动搜台有效,代表电台数量).
#define RDA5820_R4C		0X4C  //TX_AUDIO_DEV[15:0]
#define RDA5820_R4D		0X4D  //TX_PILOT_DEV[15:0]
#define RDA5820_R4E		0X4E  //TX_RDS_DEV[15:0]
#define RDA5820_R53		0X53  //CHAN_BOTTON[10:0](通道最低频率,100kHZ为单位)
#define RDA5820_R54		0X54  //CHAN_TOP[10:0](通道最高频率,100kHZ为单位)
#define RDA5820_R64		0X64  //RDS_RXMOD[4:3](FIFO模式[10]或者寄存器模式[11])		 

#define RDA5820_READ    0X23  //读RDA5820
#define RDA5820_WRITE	0X22  //写RDA5820



u8 RDA5820_Init(void);					//初始化		 				    
void RDA5820_WR_Reg(u8 addr,u16 val);	//写RDA5820寄存器
u16 RDA5820_RD_Reg(u8 addr);			//读RDA5820寄存器
void RDA5820_RX_Mode(void);				//设置RDA5820为RX模式
void RDA5820_TX_Mode(void);				//设置RDA5820为TX模式
u8 RDA5820_Rssi_Get(void);
void RDA5820_Mute_Set(u8 mute);
void RDA5820_Rssi_Set(u8 rssi);
void RDA5820_Vol_Set(u8 vol);
void RDA5820_TxPAG_Set(u8 gain);
void RDA5820_TxPGA_Set(u8 gain);
void RDA5820_Band_Set(u8 band);
void RDA5820_Space_Set(u8 spc);
void RDA5820_Freq_Set(u16 freq);
u16 RDA5820_Freq_Get(void);





		 				    
#endif































