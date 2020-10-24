#include "sys.h"
#include "ff.h"
#include "exfuns.h" 
#include "ili93xx.h"
//#include "audioplay.h"
//#include "spb.h"

#include "sms_vdp.h"
#include "sms_main.h"
#include "sms_sn76496.h"
#include "sms_z80a.h"

#include "nes_main.h" 
#include "malloc.h"	            //内存管理 
//#include "usbh_hid_gamepad.h"
//#include "i2s.h" 
#include "timer.h"
#include "audiosel.h"
#include "nes_apu.h"
#include "vs10xx.h"
#include "string.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序移植自网友ye781205的SMS模拟器工程
//ALIENTEK STM32F407开发板
//SMS主函数 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/10/12
//版本：V1.0  			  
////////////////////////////////////////////////////////////////////////////////// 	 
extern u8 nesruning;
extern u8 *nesapusbuf[NES_APU_BUF_NUM];		//音频缓冲帧

u16 *smsi2sbuf1; 			//音频缓冲帧,占用内存数 367*2 字节@22050Hz
u16 *smsi2sbuf2; 			//音频缓冲帧,占用内存数 367*2 字节@22050Hz

u8* sms_rom;				//SMS ROM存储地址指针
u8* SMS_romfile;			//ROM指针=sms_rom/sms_rom+512
u8* S_RAM;					//internal SMS RAM	16k  [0x4000] 	 
u8* E_RAM;					//external cartridge RAM (2 X 16K Bank Max) [0x8000] 


//释放SMS申请的所有内存
void sms_sram_free(void)
{
    u8 i;
	myfree(SRAMEX,sms_rom);
	myfree(SRAMEX,E_RAM);
	myfree(SRAMEX,cache);
	
	myfree(SRAMEX,S_RAM);
	myfree(SRAMEX,SMS_VDP);
	myfree(SRAMEX,psg_buffer);
	myfree(SRAMEX,SN76496);
	myfree(SRAMEX,Z80A);
	myfree(SRAMEX,VRam);
	
	myfree(SRAMEX,smsi2sbuf1);
	myfree(SRAMEX,smsi2sbuf2);
	myfree(SRAMEX,lut);
    for (i=0;i<NES_APU_BUF_NUM;i++) {
        myfree(SRAMEX,nesapusbuf[i]);//释放APU BUFs
        nesapusbuf[i] = 0;
    }
}

//为SMS运行申请内存
//返回值:0,申请成功
//      1,申请失败
u8 sms_sram_malloc(u32 romsize)
{
    u8 i;
	E_RAM=mymalloc(SRAMEX,0X8000);				//申请2*16K字节
	cache=mymalloc(SRAMEX,0x20000);				//128K
	sms_rom=mymalloc(SRAMEX,romsize);			//开辟romsize字节的内存区域	
	if (sms_rom==NULL) {
//		spb_delete();//释放SPB占用的内存
		sms_rom=mymalloc(SRAMEX,romsize);		//重新申请
	}	
	
	S_RAM=mymalloc(SRAMEX,0X4000);				//申请16K字节
 	SMS_VDP=mymalloc(SRAMEX,sizeof(SVDP));		
	psg_buffer=mymalloc(SRAMEX,SNBUF_size*2);	//申请内存SNBUF_size
	SN76496=mymalloc(SRAMEX,sizeof(t_SN76496));//申请内存184
	Z80A=mymalloc(SRAMEX,sizeof(SMS_CPU80));		//申请内存 
	VRam=mymalloc(SRAMEX,0x4000); 				//申请16K字节
	
	smsi2sbuf1=mymalloc(SRAMEX,SNBUF_size*4+10);
	smsi2sbuf2=mymalloc(SRAMEX,SNBUF_size*4+10);  
	lut=mymalloc(SRAMEX,0x10000); 				//64K
    for (i=0;i<NES_APU_BUF_NUM;i++) {
		nesapusbuf[i]=mymalloc(SRAMEX,APU_PCMBUF_SIZE+10);//申请内存
	}
	if(sms_rom&&cache&&VRam&&lut)
	{ 
		memset(E_RAM,0,0X8000);					//清零
		memset(cache,0,0x20000);				//清零
		memset(S_RAM,0,0X4000);					//清零
		memset(SMS_VDP,0,sizeof(SVDP));			//清零
		
		memset(psg_buffer,0,SNBUF_size*2);		//清零
		memset(SN76496,0,sizeof(t_SN76496));	//清零
		memset(Z80A,0,sizeof(SMS_CPU80));			//清零 
		memset(VRam,0,0X4000);					//清零
		
		memset(smsi2sbuf1,0,SNBUF_size*4+10);	//清零
		memset(smsi2sbuf2,0,SNBUF_size*4+10);	//清零
		memset(lut,0,0x10000);					//清零
        for (i=0;i<NES_APU_BUF_NUM;i++) {
            memset(nesapusbuf[i],0,APU_PCMBUF_SIZE+10);//清零
        }
		return 0;
	} else {
		sms_sram_free();						//释放所有内存.
		return 1;
	}
} 

u8 sms_xoff=0;	//显示在x轴方向的偏移量(实际显示宽度=256-2*sms_xoff)
//设置游戏显示窗口
void sms_set_window(void)
{	
	u16 xoff=0,yoff=0; 
	u16 lcdwidth,lcdheight;
	if(lcddev.width==240)
	{
		lcdwidth=240;
		lcdheight=192;
		sms_xoff=(256-lcddev.width)/2;	//得到x轴方向的偏移量
 		xoff=0; 
	}else if(lcddev.width==320) 
	{
		lcdwidth=256;
		lcdheight=192; 
		sms_xoff=0;
		xoff=(lcddev.width-256)/2;
	}else if(lcddev.width==480)
	{
		lcdwidth=480;
		lcdheight=192*2; 
		sms_xoff=(256-(lcddev.width/2))/2;//得到x轴方向的偏移量
 		xoff=0;  
	}	
	yoff=(lcddev.height-lcdheight)/2;//屏幕高度 
	LCD_Set_Window(xoff,yoff,lcdwidth,lcdheight);//让NES始终在屏幕的正中央显示
	LCD_SetCursor(xoff,yoff);
	LCD_WriteRAM_Prepare();//写入LCD RAM的准备  
}

extern u8 framecnt; 

//模拟器启动，各种初始化,然后循环运行模拟器
void sms_start(u8 bank_mun)
{
	u8 zhen;
	u8 res=0;  
	res=VDP_init(); 	
    res+=SMS_CPU_Init(S_RAM, E_RAM,SMS_romfile,bank_mun);//0x8080000,0x0f,"Sonic the Hedgehog '91"
    res+=sms_audio_init(); 
	if (res==0) {
//		TIM3_Int_Init(10000-1,8400-1);//启动TIM3 ,1s中断一次
        TIM7_Int_Init(10000-1,7200-1);    
        EXTIX_Init();//teng使用中断来退出游戏 
        JOYPAD_Init();
        Audiosel_Set(AUDIO_MP3);        
		sms_set_window();			//设置窗口
        nesruning=1;
		while (nesruning) {				
			SMS_frame(zhen);		//+FB_OFS  (24+240*32)	
			nes_get_gamepadval();	//借用NES的手柄数据获取函数
			sms_update_Sound();
			sms_update_pad();		//获取手柄值
			zhen++;
			framecnt++; 
			if(zhen>2)zhen=0; 		//跳2帧
		}
//		TIM3->CR1&=~(1<<0);//关闭定时器3
        TIM_Cmd(TIM7,DISABLE); //关闭定时器7
        EXTI->IMR &= ~(EXTI_Line2);// 关闭中断吧
//        Audiosel_Set(AUDIO_NONE);
		LCD_Set_Window(0,0,lcddev.width,lcddev.height);//恢复屏幕窗口
	}
	sms_sound_close();//关闭音频输出
} 

//更新手柄数据
//SMS键值 1111 1111 全1表示没按键
//	     D7  D6  D5   D4   D3  D2  D1  D0
//SMS    B   A   右   左   下  上  
// FC    右  左  下   上   ST   S   B   A	
void sms_update_pad(void) 
{
	u8 key,key1;
//	key=255-fcpad.ctrlval;	//将FC手柄的值取反
    key=255-JOYPAD_Read();
	key1=(key>>4)|0xf0; 	//转换为SMS手柄的值
	key1&=((key<<4)|0xcf); 
    SetController(key1); 
}
//加载SMS游戏
//pname:sms游戏路径
//返回值:
//0,正常退出
//1,内存错误
//2,文件错误 
u8 sms_load(u8* pname)
{
	u8 bank_mun;//16K bank的数量
	u8 res=0;  
	FIL* f_sms; 
//	if(audiodev.status&(1<<7))		//当前在放歌??
//	{
//		audio_stop_req(&audiodev);	//停止音频播放
//		audio_task_delete();		//删除音乐播放任务.
//	}  
 	f_sms=(FIL *)mymalloc(SRAMEX,sizeof(FIL));		//开辟FIL字节的内存区域 
	if (f_sms==NULL) {
        return 1;//申请失败
    }
 	res=f_open(f_sms,(const TCHAR*)pname,FA_READ);	//打开文件
   	if (res==0) {
        res=sms_sram_malloc(f_sms->fsize);	//申请内存
        if (res==1) {
            myfree(SRAMEX,f_sms);
            return 1;
        }
    }
	if (res==0) {		 
		if((f_sms->fsize/512)&1) {					//照顾图像标题,如果存在
			SMS_romfile=sms_rom+512;
			bank_mun=((f_sms->fsize-512)/0x4000)-1;	//16K bank的数量
		} else {
			SMS_romfile=sms_rom;
            bank_mun=(f_sms->fsize/0x4000)-1;		//16K bank的数量
		}		   
		res=f_read(f_sms,sms_rom,f_sms->fsize,&br);	//读取整个SMS游戏文件 
		if (res) {
            res=2;								//文件错误
        }
		f_close(f_sms);   							//关闭文件
	}
	myfree(SRAMEX,f_sms);						 	//释放内存	
 	if (res==0) {
        sms_start(bank_mun);						//开始游戏
	} 	
	sms_sram_free(); 
	return res;
} 
 

//vu8 smstransferend=0;	//i2s传输完成标志
//vu8 smswitchbuf=0;		//i2sbufx指示标志
//nes,音频输出支持部分
extern vu16 nesbufpos;
extern vu8 nesplaybuf;		//即将播放的音频帧缓冲编号
extern vu8 nessavebuf;		//当前保存到的音频缓冲编号
//I2S音频播放回调函数
void sms_i2s_dma_tx_callback(void)
{ 
    u8 n;
	u8 nbytes;
	u8 *p; 
	if(nesplaybuf==nessavebuf)return;//还没有收到新的音频数据
	if(VS_DQ!=0)//可以发送数据给VS10XX
	{		 
		p=nesapusbuf[nesplaybuf]+nesbufpos; 
		nesbufpos+=32; 
		if(nesbufpos>APU_PCMBUF_SIZE)
		{
			nesplaybuf++;
			if(nesplaybuf>(NES_APU_BUF_NUM-1))nesplaybuf=0; 	
			nbytes=APU_PCMBUF_SIZE+32-nesbufpos;
			nesbufpos=0; 
		}else nbytes=32;
		for(n=0;n<nbytes;n++)
		{
			if(p[n]!=0)break;	//判断是不是剩余所有的数据都为0? 
		}
		if(n==nbytes)return;	//都是0,则直接不写入VS1053了,以免引起哒哒声. 
		VS_XDCS=0;  
		for(n=0;n<nbytes;n++)
		{
			SPI1_ReadWriteByte(p[n]);	 			
		}
		VS_XDCS=1;     				   
	} 
}
const u8 sms_wav_head[]=
{
    0X52,0X49,0X46,0X46,0XFF,0XFF,0XFF,0XFF,0X57,0X41,0X56,0X45,0X66,0X6D,0X74,0X20,
    0X10,0X00,0X00,0X00,0X01,0X00,0X01,0X00,0X11,0X2B,0X00,0X00,0X11,0X2B,0X00,0X00,
    0X01,0X00,0X08,0X00,0X64,0X61,0X74,0X61,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};
//SMS打开音频输出
int sms_sound_open(int sample_rate) 
{
	u8 *p;
	u8 i; 
	p=mymalloc(SRAMIN,100);	//申请100字节内存
	if(p==NULL)return 1;	//内存申请失败,直接退出
	printf("sound open:%d\r\n",sample_rate);
	for(i=0;i<sizeof(sms_wav_head);i++)//复制nes_wav_head内容
	{
		p[i]=sms_wav_head[i];
	}
	if(lcddev.width==480)	//是480*480屏幕
	{
		sample_rate=8000;	//设置8Khz,约原来速度的0.75倍
	}
	p[24]=sample_rate&0XFF;			//设置采样率
	p[25]=(sample_rate>>8)&0XFF;
	p[28]=sample_rate&0XFF;			//设置字节速率(8位模式,等于采样率)
	p[29]=(sample_rate>>8)&0XFF; 
	nesplaybuf=0;
	nessavebuf=0;	
	VS_HD_Reset();		   			//硬复位
	VS_Soft_Reset();  				//软复位 
	VS_Set_All();					//设置音量等参数 			 
	VS_Reset_DecodeTime();			//复位解码时间 	  	 
	while(VS_Send_MusicData(p));	//发送wav head
	while(VS_Send_MusicData(p+32));	//发送wav head
	TIM6_Int_Init(100-1,720-1);	//1ms中断一次
	myfree(SRAMIN,p);				//释放内存
	return 1;    
}
//SMS关闭音频输出
void sms_sound_close(void) 
{ 
    TIM6->CR1&=~(1<<0);				//关闭定时器6
	VS_SPK_Set(0);					//关闭喇叭输出 
	VS_Set_Vol(0);					//设置音量为0 
} 
//SMS音频输出到I2S缓存
void sms_apu_fill_buffer(int samples,u16* wavebuf)
{	
 	u16	i;	
	u8 tbuf;
	for(i=0;i<APU_PCMBUF_SIZE;i++)
	{
		nesapusbuf[nessavebuf][i]=wavebuf[i]; 
	}
	tbuf=nessavebuf;
	tbuf++;
	if(tbuf>(NES_APU_BUF_NUM-1))tbuf=0;
	while(tbuf==nesplaybuf)//输出数据赶上音频播放的位置了,等待.
	{ 
		delay_ms(5);
	}
	nessavebuf=tbuf; 
} 







