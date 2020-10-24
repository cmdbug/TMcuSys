#include "mp3player.h"
#include "vs10xx.h"	 
#include "delay.h"
#include "led.h"
#include "key.h"
#include "malloc.h"
#include "text.h"
#include "string.h"
#include "exfuns.h"
#include "fattester.h"	 
#include "ff.h"   
#include "flac.h"	
#include "includes.h"


u16 f_kbps=0;//歌曲文件位率	
u16 time=0;// 当前时间变量
u16 totalTime=0;//总长
//显示播放时间,比特率 信息 
//lenth:歌曲总长度
void mp3_msg_show(u32 lenth)
{	
	static u16 playtime=0;//播放时间标记	     
 	
	u16 temp=0;	  
	if(f_kbps==0xffff)//未更新过
	{
		playtime=0;
		f_kbps=VS_Get_HeadInfo();	   //获得比特率
	}	 	 
	time=VS_Get_DecodeTime(); //得到解码时间
	if (playtime==0) {
        playtime=time;
    } else if ((time!=playtime)&&(time!=0)) {//1s时间到,更新显示数据
		playtime=time;//更新时间 	 				    
		temp=VS_Get_HeadInfo(); //获得比特率	   				 
		if(temp!=f_kbps) {
			f_kbps=temp;//更新KBPS	  				     
		}			 
		if (f_kbps) {
            totalTime=(lenth/f_kbps)/125;//得到秒钟数   (文件长度(字节)/(1000/8)/比特率=持续秒钟数
        } else {
            totalTime=0;//非法位率
        }            
	}   		 
}			  		 


//播放一曲指定的歌曲				     	   									    	 
//返回值:0,正常播放完成
//		 1,下一曲
//       2,上一曲
//       0XFF,出现错误了
extern char musicPlaying;
extern char musicChange;
u8 mp3_play_song(u8 *pname)
{	 
 	FIL* fmp3;
    u16 br;
	u8 res,rval;	  
	u8 *databuf;	   		   
	u16 i=0; 
	u8 key; 
    OS_ERR err;
    CPU_SR_ALLOC();
			   
	rval=0;	    
	fmp3 = (FIL*)mymalloc(SRAMIN,sizeof(FIL));//申请内存
	databuf = (u8*)mymalloc(SRAMIN,4096);		//开辟4096字节的内存区域
	if (databuf==NULL||fmp3==NULL) {
        rval=0XFF ;//内存申请失败
    }
	if (rval==0) {	  
	  	VS_Restart_Play();  					//重启播放 
		VS_Set_All();        					//设置音量等信息 			 
		VS_Reset_DecodeTime();					//复位解码时间 	  
		res=f_typetell(pname);	 	 			//得到文件后缀	 			  	 						 
		if (res==0x4c) {//如果是flac,加载patch	
			VS_Load_Patch((u16*)vs1053b_patch,VS1053B_PATCHLEN);
		}  				 		   		 						  
		res=f_open(fmp3,(const TCHAR*)pname,FA_READ);//打开文件	 
 		if (res==0) {//打开成功
			VS_SPI_SpeedHigh();	//高速
			while (rval==0 && musicPlaying && (musicChange==0)) {
                OS_CRITICAL_ENTER();
				res=f_read(fmp3,databuf,4096,(UINT*)&br);//读出4096个字节
                OS_CRITICAL_EXIT();
				i=0;
				do {//主播放循环
					if (VS_Send_MusicData(databuf+i)==0) {//给VS10XX发送音频数据
						i+=32;
					} else {
                        OSTimeDlyHMSM(0, 0, 0, 20, OS_OPT_TIME_PERIODIC, &err);
						mp3_msg_show(fmp3->fsize);//显示信息	    
					}	    	    
				} while (i<4096 && musicPlaying && (musicChange==0));//循环发送4096个字节 
				if (br!=4096||res!=0) {
					rval=0;
					break;//读完了.		  
				} 							 
			}
			f_close(fmp3);
		} else {
            rval=0XFF;//出现错误
        }            
	}					
    if (musicPlaying==0 || musicChange==1) {
        rval = 3;
    }
    
	myfree(SRAMIN,databuf);	  	 		  	    
	myfree(SRAMIN,fmp3);
	return rval;	  	 		  	    
}




