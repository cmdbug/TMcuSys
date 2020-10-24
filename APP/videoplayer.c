#include "videoplayer.h" 
#include "string.h"  
#include "key.h" 
#include "usart.h"   
#include "delay.h"
#include "timer.h"
#include "ili93xx.h"
#include "led.h"
#include "key.h"
#include "malloc.h"
//#include "i2s.h" 
//#include "wm8978.h" 
#include "mjpeg.h" 
#include "avi.h"
#include "exfuns.h"
#include "text.h"
#include "exti.h"
#include "textfont.h"
   
extern u16 frame;
extern vu8 frameup;//视频播放时隙控制变量,当等于1的时候,可以更新下一帧视频
extern vu8 framecnt;		//统一的帧计数器
extern vu8 framecntout;	//统一的帧计数器输出变量

u8 videoruning=0;

volatile u8 i2splaybuf;	//即将播放的音频帧缓冲编号
u8* i2sbuf[4]; 			//音频缓冲帧,共4帧,4*5K=20K
  

//显示当前播放时间
//favi:当前播放的视频文件
//aviinfo;avi控制结构体
void video_time_show(FIL *favi,AVI_INFO *aviinfo)
{	 
	static u32 oldsec;	//上一次的播放时间
	u8* buf;
	u32 totsec=0;		//video文件总时间 
	u32 cursec; 		//当前播放时间 
	totsec=(aviinfo->SecPerFrame/1000)*aviinfo->TotalFrame;	//总长度(单位:ms) 
	totsec/=1000;		//秒钟数. 
  	cursec=((double)favi->fptr/favi->fsize)*totsec;//当前播放到多少秒了?  
	if(oldsec!=cursec)	//需要更新显示时间
	{
		buf=mymalloc(SRAMEX,100);//申请100字节内存
		oldsec=cursec; 
		sprintf((char*)buf,"%02d:%02d:%02d/%02d:%02d:%02d",cursec/3600,(cursec%3600)/60,cursec%60,totsec/3600,(totsec%3600)/60,totsec%60);
 		Show_Str(136,291,lcddev.width-5,12,buf,12,0,LGRAY);
		myfree(SRAMEX,buf);		
	}
    LCD_Fill(0, 320, lcddev.width*((float)cursec/totsec), 320, LGRAY);
}
//显示当前视频文件的相关信息 
//aviinfo;avi控制结构体
//0:成功 1:失败
u8 video_info_show(AVI_INFO *aviinfo)
{	  
	u8 *buf;
	buf=mymalloc(SRAMEX,100);//申请100字节内存 
    sprintf((char*)buf,"FPS:%02d",0); 
    Show_Str(79,291,lcddev.width-10,12,buf,12,0,LGRAY);
    sprintf((char*)buf,"%02d:%02d:%02d/%02d:%02d:%02d",0,0,0,0,0,0);
 	Show_Str(136,291,lcddev.width-5,12,buf,12,0,LGRAY);
	sprintf((char*)buf,"帧率:%02d帧",1000/(aviinfo->SecPerFrame/1000)); 
 	Show_Str(3,304,lcddev.width-10,12,buf,12,0,LGRAY);
    sprintf((char*)buf,"%d x %d",aviinfo->Width, aviinfo->Height); 
 	Show_Str(3,291,lcddev.width-10,12,buf,12,0,LGRAY);
    sprintf((char*)buf,"音频通道:%d  音频采样率:%d",aviinfo->Channels,aviinfo->SampleRate*10); 
 	Show_Str(64,304,lcddev.width-10,12,buf,12,0,LGRAY);
	myfree(SRAMEX,buf);
    return ((aviinfo->Width > lcddev.width) || (aviinfo->Height > lcddev.height));
}

void video_fps(void) {
    u8 *buf;
	buf=mymalloc(SRAMEX, 20);//申请100字节内存
    sprintf((char*)buf,"%02d",framecntout); 
    Show_Str(79+6*4,291,lcddev.width-10,12,buf,12,0,LGRAY);
    myfree(SRAMEX,buf);
}


//播放一个mjpeg文件
//pname:文件名
//返回值:
//KEY0_PRES:下一曲
//KEY1_PRES:上一曲
//其他:错误
u8 video_play_mjpeg(u8 *pname)
{
    OS_ERR err;
	u8* framebuf;	//视频解码buf	 
	u8* pbuf;		//buf指针  
	FIL *favi;
	u8  res=0;
	u16 offset=0; 
	u32	nr; 
	u8 key;   
    u8 i2ssavebuf;
    
    videoruning = 1;
    
	i2sbuf[0]=mymalloc(SRAMEX,AVI_AUDIO_BUF_SIZE);	//申请音频内存
//	i2sbuf[1]=mymalloc(SRAMEX,AVI_AUDIO_BUF_SIZE);	//申请音频内存
//	i2sbuf[2]=mymalloc(SRAMEX,AVI_AUDIO_BUF_SIZE);	//申请音频内存
//	i2sbuf[3]=mymalloc(SRAMEX,AVI_AUDIO_BUF_SIZE);	//申请音频内存 
	framebuf=mymalloc(SRAMEX,AVI_VIDEO_BUF_SIZE);	//申请视频buf
	favi=(FIL*)mymalloc(SRAMEX,sizeof(FIL));		//申请favi内存 
	memset(i2sbuf[0],0,AVI_AUDIO_BUF_SIZE);
//	memset(i2sbuf[1],0,AVI_AUDIO_BUF_SIZE); 
//	memset(i2sbuf[2],0,AVI_AUDIO_BUF_SIZE);
//	memset(i2sbuf[3],0,AVI_AUDIO_BUF_SIZE); 
	if(i2sbuf[0]==NULL||framebuf==NULL||favi==NULL) {
//		printf("memory error!\r\n");
		res=0XFF;
	}   
	while (res==0 && videoruning) { 
		res=f_open(favi,(char *)pname,FA_READ);
		if (res==0) {
			pbuf=framebuf;			
			res=f_read(favi,pbuf,AVI_VIDEO_BUF_SIZE,&nr);//开始读取	
			if (res) {
//				printf("fread error:%d\r\n",res);
                f_close(favi);
				break;
			} 	 
			//开始avi解析
			res=avi_init(pbuf,AVI_VIDEO_BUF_SIZE);	//avi解析
			if (res) {
//				printf("avi err:%d\r\n",res);
                f_close(favi);
				break;
			}
            framecnt = 0;
			res=video_info_show(&avix);
            if (res==1) {
                videoruning = 0;
                res=0x03;
                f_close(favi);
                break;
            }
			TIM6_Int_Init(avix.SecPerFrame/100-1,7200-1);//10Khz计数频率,加1是100us 
            TIM7_Int_Init(10000-1,7200-1);//teng
            EXTIX_Init();//teng使用中断来退出
			offset=avi_srarch_id(pbuf,AVI_VIDEO_BUF_SIZE,"movi");//寻找movi ID	 
			avi_get_streaminfo(pbuf+offset+4);			//获取流信息 
			f_lseek(favi,offset+12);					//跳过标志ID,读地址偏移到流数据开始处	 
			res=mjpegdec_init((lcddev.width-avix.Width)/2, (lcddev.height-avix.Height)/2);//JPG解码初始化 
//			if(avix.SampleRate)							//有音频信息,才初始化
//			{
//				WM8978_I2S_Cfg(2,0);	//飞利浦标准,16位数据长度
//				I2S2_Init(I2S_Standard_Phillips,I2S_Mode_MasterTx,I2S_CPOL_Low,I2S_DataFormat_16bextended);		//飞利浦标准,主机发送,时钟低电平有效,16位帧长度
//				I2S2_SampleRate_Set(avix.SampleRate);	//设置采样率
//				I2S2_TX_DMA_Init(i2sbuf[1],i2sbuf[2],avix.AudioBufSize/2); //配置DMA
//				i2s_tx_callback=audio_i2s_dma_callback;	//回调函数指向I2S_DMA_Callback
//				i2splaybuf=0;
//				i2ssavebuf=0; 
//				I2S_Play_Start(); //开启I2S播放 
//			}
 			while (videoruning) {//播放循环
				if(avix.StreamID==AVI_VIDS_FLAG) {	//视频流
					pbuf=framebuf;
					f_read(favi,pbuf,avix.StreamSize+8,&nr);		//读入整帧+下一数据流ID信息  
					res=mjpegdec_decode(pbuf,avix.StreamSize);
					if (res) {
//						printf("decode error!\r\n");
					} 
					while(frameup==0);	//等待时间到达(在TIM6的中断里面设置为1)
					frameup=0;			//标志清零
					frame++;
                    framecnt++;
				} else {	//音频流
//                    video_info_show(&avix); 
                    video_fps();
					video_time_show(favi,&avix); 	//显示当前播放时间
//					i2ssavebuf++;
//					if(i2ssavebuf>3)i2ssavebuf=0;
//					do
//					{
//						nr=i2splaybuf;
//						if(nr)nr--;
//						else nr=3; 
//					}while(i2ssavebuf==nr);//碰撞等待. 
                    i2ssavebuf=0;
					f_read(favi,i2sbuf[i2ssavebuf],avix.StreamSize+8,&nr);//填充i2sbuf	 
					pbuf=i2sbuf[i2ssavebuf];  
				} 
//				key=KEY_Scan(0);
//				if(key==KEY0_PRES||key==KEY2_PRES)//KEY0/KEY2按下,播放下一个/上一个视频
//				{
//					res=key;
//					break; 
//				}else if(key==KEY1_PRES||key==WKUP_PRES)
//				{
//					I2S_Play_Stop();//关闭音频
//					video_seek(favi,&avix,framebuf);
//					pbuf=framebuf;
//					I2S_Play_Start();//开启DMA播放 
//				}
				if(avi_get_streaminfo(pbuf+avix.StreamSize))//读取下一帧 流标志
				{
//					printf("frame error \r\n"); 
					res=KEY0_PRES;
					break; 
				}
                OSTimeDlyHMSM(0, 0, 0, 2, OS_OPT_TIME_PERIODIC, &err);                
			}
//			I2S_Play_Stop();	//关闭音频
			TIM6->CR1&=~(1<<0); //关闭定时器6
            TIM_Cmd(TIM7,DISABLE); //关闭定时器7 teng
            EXTI->IMR &= ~(EXTI_Line2);// 关闭中断吧
			LCD_Set_Window(0,0,lcddev.width,lcddev.height);//恢复窗口
			mjpegdec_free();	//释放内存
			f_close(favi); 
		}
	}
	myfree(SRAMEX,i2sbuf[0]);
//	myfree(SRAMEX,i2sbuf[1]);
//	myfree(SRAMEX,i2sbuf[2]);
//	myfree(SRAMEX,i2sbuf[3]);
	myfree(SRAMEX,framebuf);
	myfree(SRAMEX,favi);
    videoruning = 0;
	return res;
}
//avi文件查找
u8 video_seek(FIL *favi,AVI_INFO *aviinfo,u8 *mbuf)
{
	u32 fpos=favi->fptr;
	u8 *pbuf;
	u16 offset;
	u32 br;
	u32 delta;
	u32 totsec;
	u8 key; 
	totsec=(aviinfo->SecPerFrame/1000)*aviinfo->TotalFrame;	
	totsec/=1000;//秒钟数.
	delta=(favi->fsize/totsec)*5;		//每次前进5秒钟的数据量 
	while(1) {
		key=KEY_Scan(1); 
		if(key==WKUP_PRES)//快进
		{
			if(fpos<favi->fsize)fpos+=delta; 
			if(fpos>(favi->fsize-AVI_VIDEO_BUF_SIZE))
			{
				fpos=favi->fsize-AVI_VIDEO_BUF_SIZE;
			}
		} else if(key==KEY1_PRES)//快退
		{
			if(fpos>delta)fpos-=delta;
			else fpos=0; 
		}else break;
		f_lseek(favi,fpos);
		f_read(favi,mbuf,AVI_VIDEO_BUF_SIZE,&br);	//读入整帧+下一数据流ID信息 
		pbuf=mbuf; 
		if(fpos==0) //从0开始,得先寻找movi ID
		{
			offset=avi_srarch_id(pbuf,AVI_VIDEO_BUF_SIZE,"movi"); 
		} else offset=0;
		offset+=avi_srarch_id(pbuf+offset,AVI_VIDEO_BUF_SIZE,avix.VideoFLAG);	//寻找视频帧	
		avi_get_streaminfo(pbuf+offset);			//获取流信息 
		f_lseek(favi,fpos+offset+8);				//跳过标志ID,读地址偏移到流数据开始处	 
		if(avix.StreamID==AVI_VIDS_FLAG) {
			f_read(favi,mbuf,avix.StreamSize+8,&br);	//读入整帧 
			mjpegdec_decode(mbuf,avix.StreamSize); 		//显示视频帧
		} else {
			printf("error flag");
		}
		video_time_show(favi,&avix);  
	}
	return 0;
}



