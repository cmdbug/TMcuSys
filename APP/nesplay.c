#include "nesplay.h"
#include "nes_main.h"	 
#include "ff.h"//teng
#include "malloc.h"//teng
#include "ili93xx.h"//teng
#include "tpad.h"//teng
//#include "spb.h"	  
//#include "audioplay.h"	
//#include "usart3.h"	
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//APP-NES模拟器 代码	   
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
  
u8 *rom_file;
extern u8 nesruning;

//加载游戏界面
void nes_load_ui(void)
{	
//	app_filebrower((u8*)nes_caption_tbl[gui_phy.language],0X05);//显示标题 
//  	gui_fill_rectangle(0,20,lcddev.width,lcddev.height-20,BLACK);//填充底色 		    	  
} 
//NES游戏
u8 nes_play(u8 *path)
{
  	DIR nesdir;		//nesdir专用	  
	FILINFO nesinfo;	 	   
	u8 res;
	u8 rval=0;			//返回值	  
	u8 *pname=0;
	u8 *fn;	
  
	//为长文件名申请缓存区
 	nesinfo.lfsize=_MAX_LFN*2+1;
	nesinfo.lfname=mymalloc(SRAMEX, nesinfo.lfsize);
	if(nesinfo.lfname==NULL) {
        rval=1;//申请内存失败 
    } else {
        memset((u8*)nesinfo.lfname,0,nesinfo.lfsize); 
    }
    fn=(u8*)(*nesinfo.lfname?nesinfo.lfname:nesinfo.fname);
    pname=mymalloc(SRAMEX, strlen((const char*)fn)+strlen((const char*)path)+2);//申请内存
    if(pname==NULL) {
        rval=1;//申请失败
    } else {
        pname=path;//文件名加入路径  
//				if(audiodev.status&(1<<7))		//当前在放歌??
//				{
//					audio_stop_req(&audiodev);	//停止音频播放
//					audio_task_delete();		//删除音乐播放任务.
//					delay_ms(1010);
//				}  				
        //非1963/9341/5510/5310驱动LCD的LCD,降速
        if(lcddev.id!=0X1963&&lcddev.id!=0X9341&&lcddev.id!=0X5510&&lcddev.id!=0X5310) {
            FSMC_Bank1E->BWTR[6]&=0XFFFFFFF0;//清除之前的设置
            FSMC_Bank1E->BWTR[6]|=3<<0;	//地址建立时间（ADDSET）为4个HCLK  	 	 
        }
        //SRAM设置
        FSMC_Bank1->BTCR[5]&=0XFFFF00FF;//清除之前的设置
        FSMC_Bank1->BTCR[5]|=4<<8;		//数据保持时间（DATAST）为8个HCLK 8/128M=62.5ns	 	 
//        USART3->CR1&=~(1<<5);			//禁止串口3接收（此时不再处理SIM900A相关信息）
        //开始nes游戏
        nesruning = 1;
        rval = nes_load(pname);			//开始nes游戏
        nesruning = 0;
        //非1963/9341/5510/5310驱动LCD的LCD,恢复
        if(lcddev.id!=0X1963&&lcddev.id!=0X9341&&lcddev.id!=0X5510&&lcddev.id!=0X5310) {
            FSMC_Bank1E->BWTR[6]&=0XFFFFFFF0;//清除之前的设置
            FSMC_Bank1E->BWTR[6]|=0<<0;	//地址建立时间（ADDSET）为1个HCLK  	 	 
        }
        //SRAM设置
        FSMC_Bank1->BTCR[5]&=0XFFFF00FF;//清除之前的设置
        FSMC_Bank1->BTCR[5]|=3<<8;		//数据保持时间（DATAST）为7个HCLK 7/128M=55ns	 	 
//				usart3_init(36,115200);			//恢复串口3功能
    }	       
    myfree(SRAMEX, pname);				//释放内存		  
    pname=NULL;
//    TPAD_Init(6);			//重新设置TPAD值
	myfree(SRAMEX, pname);			//释放内存		  
 	myfree(SRAMEX, nesinfo.lfname);		 
	return rval;  								  
}


