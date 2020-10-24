#include "includes.h"		 
#include "audioplay.h"
#include "settings.h"
#include "ucos_ii.h"
#include "mp3player.h" 
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

//audio播放控制器
//_m_audiodev  *audiodev=NULL;
_lyric_obj *lrcdev=NULL;				//歌词控制器 
//音乐播放控制器
__audiodev audiodev;	 

//AUDIO PLAY任务
//设置任务优先级
#define AUDIO_PLAY_TASK_PRIO       		2 
//设置任务堆栈大小
#define AUDIO_PLAY_STK_SIZE  		    800
//任务堆栈，采用内存管理的方式控制申请	
OS_STK * AUDIO_PLAY_TASK_STK;	
//任务函数
void audio_play_task(void *pdata); 
//播放音乐任务邮箱
OS_EVENT * audiombox;//事件控制块	
 
////////////////////////////////////////////////////////////////////////////////////

u8*const AUDIO_BTN_PIC_TBL[2][5]=  	//5个图片按钮的路径
{
{
	"1:/SYSTEM/APP/AUDIO/ListR.bmp",
	"1:/SYSTEM/APP/AUDIO/PrevR.bmp", 
	"1:/SYSTEM/APP/AUDIO/PauseR.bmp", 
	"1:/SYSTEM/APP/AUDIO/NextR.bmp",
	"1:/SYSTEM/APP/AUDIO/ExitR.bmp", 
},
{
	"1:/SYSTEM/APP/AUDIO/ListP.bmp",  
	"1:/SYSTEM/APP/AUDIO/PrevP.bmp",  
	"1:/SYSTEM/APP/AUDIO/PlayP.bmp",  
	"1:/SYSTEM/APP/AUDIO/NextP.bmp", 
	"1:/SYSTEM/APP/AUDIO/ExitP.bmp",  
},
};
u8*const AUDIO_PLAYR_PIC="1:/SYSTEM/APP/AUDIO/PlayR.bmp";		//播放 松开
u8*const AUDIO_PLAYP_PIC="1:/SYSTEM/APP/AUDIO/PlayP.bmp";		//播放 按下
u8*const AUDIO_PAUSER_PIC="1:/SYSTEM/APP/AUDIO/PauseR.bmp";		//暂停 松开
u8*const AUDIO_PAUSEP_PIC="1:/SYSTEM/APP/AUDIO/PauseP.bmp";		//暂停 按下

u8*const AUDIO_BACK_PIC[3]=										//背景图片	
{
"1:/SYSTEM/APP/AUDIO/a_240164.jpg",
"1:/SYSTEM/APP/AUDIO/a_320296.jpg",
"1:/SYSTEM/APP/AUDIO/a_480550.jpg",	
};
////////////////////////////////////////////////////////////////////////////////////

 
//开始音频播放
void audio_start(void)
{
	audiodev.status|=1<<1;		//开启播放
	audiodev.status|=1<<0;		//非暂停状态
} 
//停止音频播放
void audio_stop(void)
{
	audiodev.status&=~(1<<0);	//暂停位清零
	audiodev.status&=~(1<<1);	//结束播放
}    
void audio_task_delete(void);
//音乐播放任务,通过其他程序创建
void audio_play_task(void *pdata)
{
	DIR audiodir;			//audiodir专用	  
 	FILINFO audioinfo;	 	   
	u8 rval;	
	u8 *pname=0; 
	u8 res;  		
	VS_HD_Reset();
	VS_Soft_Reset();			 	
	while(audiodev.status&0x80)
	{
		audiodev.curindex=(u32)OSMboxPend(audiombox,0,&rval)-1;	//请求邮箱,要减去1,因为发送的时候增加了1
 		audioinfo.lfsize=_MAX_LFN*2+1;							//长文件名最大长度
		audioinfo.lfname=gui_memin_malloc(audioinfo.lfsize);	//为长文件缓存区分配内存 
		if(audioinfo.lfname)rval=f_opendir(&audiodir,(const TCHAR*)audiodev.path);	//打开选中的目录 
		while(rval==0)
		{	  	 			   
			ff_enter(audiodir.fs);//进入fatfs,防止被打断.
			dir_sdi(&audiodir,audiodev.mfindextbl[audiodev.curindex]);
			ff_leave(audiodir.fs);//退出fatfs,继续运行os等
			rval=f_readdir(&audiodir,&audioinfo);//读取文件信息
			if(rval)break;//打开失败   
			audiodev.name=(u8*)(*audioinfo.lfname?audioinfo.lfname:audioinfo.fname);
			pname=gui_memin_malloc(strlen((const char*)audiodev.name)+strlen((const char*)audiodev.path)+2);//申请内存
			if(pname==NULL)break;//申请失败	    
			pname=gui_path_name(pname,audiodev.path,audiodev.name);	//文件名加入路径  
			audiodev.status|=1<<5;//标记切歌了
			audiodev.status|=1<<4;//标记正在播放音乐 
			printf("play:%s\r\n",pname); 
			if((f_typetell(pname)&0X40)==0X40)//是音频文件
			{ 
				res=mp3_play_song(pname);	//播放音乐文件 
			}
			gui_memin_free(pname);//释放内存
			if(res&0X80)printf("audio error:%d\r\n",res); 
			printf("audiodev.status:%d\r\n",audiodev.status);
			if((audiodev.status&(1<<6))==0)//不终止播放
			{
				if(systemset.audiomode==0)//顺序播放
				{
					if(audiodev.curindex<(audiodev.mfilenum-1))audiodev.curindex++;
					else audiodev.curindex=0;
				}else if(systemset.audiomode==1)//随机播放
				{						    
					audiodev.curindex=app_get_rand(audiodev.mfilenum);//得到下一首歌曲的索引	  
				}else audiodev.curindex=audiodev.curindex;//单曲循环				
			}else break;   
		}
		printf("audio play over:%d\r\n",audiodev.status);
 		gui_memin_free(audioinfo.lfname);   
		audiodev.status&=~(1<<6);	//标记已经成功终止播放
		audiodev.status&=~(1<<4);	//标记无音乐播放
	}
	audio_task_delete();			//删除任务 
} 
///////////////////////////////////////////////////////////////////////////////////////////
//请求停止audio播放
//audiodevx:audio结构体
void audio_stop_req(__audiodev *audiodevx)
{
	while(audiodevx->status&(1<<4))	//等待终止播放成功  
	{
		audiodevx->status&=~(1<<1);	//请求结束播放,以退出正在播放的进程 
		audiodevx->status|=1<<6;	//请求终止播放,停止自动循环/随机播放
		delay_ms(10);
	};
	audiodev.status&=~(1<<6);		//撤销请求 
}   
//音乐列表
u8*const MUSIC_LIST[GUI_LANGUAGE_NUM]=
{
	"音乐列表","音樂列表","MUSIC LIST",
};
 
//audio文件浏览,带文件存储功能
//audiodevx:audio结构体
//返回值:0,正常返回/按了退出按钮.
//		 1,内存分配失败		 
u8 audio_filelist(__audiodev *audiodevx)
{
	FILINFO audioinfo;	 	   
	u8 res;
	u8 rval=0;			//返回值	  
  	u16 i;	    						   
 	_btn_obj* rbtn;		//返回按钮控件
 	_btn_obj* qbtn;		//退出按钮控件

   	_filelistbox_obj * flistbox;
	_filelistbox_list * filelistx; 	//文件
 	app_filebrower((u8*)MUSIC_LIST[gui_phy.language],0X07);	//选择目标文件,并得到目标数量
 
  	flistbox=filelistbox_creat(0,gui_phy.tbheight,lcddev.width,lcddev.height-gui_phy.tbheight*2,1,gui_phy.listfsize);//创建一个filelistbox
 	if(flistbox==NULL)rval=1;							//申请内存失败.
	else if(audiodevx->path==NULL)  
	{
		flistbox->fliter=FLBOX_FLT_MUSIC;	//查找音乐文件
 		filelistbox_add_disk(flistbox);		//添加磁盘路径
		filelistbox_draw_listbox(flistbox);
	}else
	{
		flistbox->fliter=FLBOX_FLT_MUSIC;		//查找音乐文件	 
		flistbox->path=(u8*)gui_memin_malloc(strlen((const char*)audiodevx->path)+1);//为路径申请内存
		strcpy((char *)flistbox->path,(char *)audiodevx->path);//复制路径	    
		filelistbox_scan_filelist(flistbox);	//重新扫描列表 
		flistbox->selindex=flistbox->foldercnt+audiodevx->curindex;//选中条目为当前正在播放的条目
		if(flistbox->scbv->totalitems>flistbox->scbv->itemsperpage)flistbox->scbv->topitem=flistbox->selindex;  
		filelistbox_draw_listbox(flistbox);		//重画 		 
	} 	 		 
	//为长文件名申请缓存区
 	audioinfo.lfsize = _MAX_LFN * 2 + 1;
	audioinfo.lfname = gui_memin_malloc(audioinfo.lfsize);
	if(audioinfo.lfname==NULL)rval=1;//申请内存失败 
   	else gui_memset((u8 *)audioinfo.lfname,0,audioinfo.lfsize); 
	rbtn=btn_creat(lcddev.width-2*gui_phy.tbfsize-8-1,lcddev.height-gui_phy.tbheight,2*gui_phy.tbfsize+8,gui_phy.tbheight-1,0,0x03);//创建文字按钮
  	qbtn=btn_creat(0,lcddev.height-gui_phy.tbheight,2*gui_phy.tbfsize+8,gui_phy.tbheight,0,0x03);//创建退出文字按钮
	if(rbtn==NULL||qbtn==NULL)rval=1;	//没有足够内存够分配
	else
	{
	 	rbtn->caption=(u8*)GUI_BACK_CAPTION_TBL[gui_phy.language];	//返回
	 	rbtn->font=gui_phy.tbfsize;//设置新的字体大小	 	 
		rbtn->bcfdcolor=WHITE;	//按下时的颜色
		rbtn->bcfucolor=WHITE;	//松开时的颜色
		btn_draw(rbtn);//画按钮 
		
	 	qbtn->caption=(u8*)GUI_QUIT_CAPTION_TBL[gui_phy.language];	//名字
	 	qbtn->font=gui_phy.tbfsize;//设置新的字体大小	 
		qbtn->bcfdcolor=WHITE;	//按下时的颜色
		qbtn->bcfucolor=WHITE;	//松开时的颜色
		btn_draw(qbtn);//画按钮
	}	   
   	while(rval==0)
	{
		tp_dev.scan(0);    
		in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);	//得到按键键值   
		delay_ms(1000/OS_TICKS_PER_SEC);		//延时一个时钟节拍
		if(system_task_return)break;			//TPAD返回
		filelistbox_check(flistbox,&in_obj);	//扫描文件
		res=btn_check(rbtn,&in_obj);
		if(res)
		{
			if(((rbtn->sta&0X80)==0))//按钮状态改变了
			{
				if(flistbox->dbclick!=0X81)
				{
 					filelistx=filelist_search(flistbox->list,flistbox->selindex);//得到此时选中的list的信息
					if(filelistx->type==FICO_DISK)//已经不能再往上了
					{				 
						break;
					}else filelistbox_back(flistbox);//退回上一层目录	 
				} 
 			}	 
		}
		res=btn_check(qbtn,&in_obj);
		if(res)
		{
			if(((qbtn->sta&0X80)==0))//按钮状态改变了
			{ 
				break;//退出
 			}	 
		}   
		if(flistbox->dbclick==0X81)//双击文件了
		{											 
			audio_stop_req(audiodevx);
			gui_memin_free(audiodevx->path);		//释放内存
			gui_memex_free(audiodevx->mfindextbl);	//释放内存
			audiodevx->path=(u8*)gui_memin_malloc(strlen((const char*)flistbox->path)+1);//为新的路径申请内存
			if(audiodevx->path==NULL){rval=1;break;}
			audiodevx->path[0]='\0';//在最开始加入结束符.
 			strcpy((char *)audiodevx->path,(char *)flistbox->path);
			audiodevx->mfindextbl=(u16*)gui_memex_malloc(flistbox->filecnt*2);//为新的tbl申请内存
			if(audiodevx->mfindextbl==NULL){rval=1;break;}
		    for(i=0;i<flistbox->filecnt;i++)audiodevx->mfindextbl[i]=flistbox->findextbl[i];//复制
			audiodevx->mfilenum=flistbox->filecnt;		//记录文件个数	 
			OSMboxPost(audiombox,(void*)(flistbox->selindex-flistbox->foldercnt+1));//发送邮箱,因为邮箱不能为空,所以在这必须加1
 			flistbox->dbclick=0;
			break;	 							   			   
		}
	}	
	filelistbox_delete(flistbox);	//删除filelist
	btn_delete(qbtn);				//删除按钮	  	 
	btn_delete(rbtn);				//删除按钮	   
  	gui_memin_free(audioinfo.lfname);	
	if(rval)
	{
		gui_memin_free(audiodevx->path);		//释放内存
		gui_memex_free(audiodevx->mfindextbl); 	//释放内存
		gui_memin_free(audiodevx);
 	}	 
 	return rval; 
} 
//audio加载主界面
void audio_load_ui(void)
{	
	u8 aui_tpbar_height=0;	//顶部标题栏高度
	u8 aui_msgbar_height=0;	//信息栏高度
	u8 aui_prgbar_height=0;	//进度条区域高度
	u8 aui_btnbar_height=0;	//底部按钮条高度 
	u8 pixpitch=0;
	u8 toffy=0; 
	u8 idx=0;
 	if(lcddev.width==240)
	{
		aui_tpbar_height=20;
		aui_msgbar_height=46;
		aui_prgbar_height=30;
		aui_btnbar_height=60;
		pixpitch=6; 
		idx=0;
	}else if(lcddev.width==320)
	{
		aui_tpbar_height=24;
		aui_msgbar_height=50;
		aui_prgbar_height=30;
		aui_btnbar_height=80;
		pixpitch=20; 
		idx=1;
	}else if(lcddev.width==480)
	{
		aui_tpbar_height=30;
		aui_msgbar_height=60;
		aui_prgbar_height=40;
		aui_btnbar_height=120;
		pixpitch=30; 
		idx=2;
	} 
	gui_fill_rectangle(0,0,lcddev.width,aui_tpbar_height,AUDIO_TITLE_BKCOLOR);	//填充标题栏底色 
	gui_show_strmid(0,0,lcddev.width,aui_tpbar_height,AUDIO_TITLE_COLOR,16,(u8*)APP_MFUNS_CAPTION_TBL[2][gui_phy.language]);	//显示标题
	gui_fill_rectangle(0,aui_tpbar_height,lcddev.width,aui_msgbar_height,AUDIO_MAIN_BKCOLOR);									//填充信息栏背景色   
	toffy=(aui_msgbar_height-44)/2;
	minibmp_decode((u8*)APP_VOL_PIC,pixpitch+2,aui_tpbar_height+20+toffy-2,16,16,0,0);				//解码音量图标
 	gui_show_string("00%",pixpitch+20,aui_tpbar_height+20+toffy+12,66,12,12,AUDIO_INFO_COLOR); 		//显示音量
  	gui_fill_rectangle(0,lcddev.height-aui_btnbar_height-aui_prgbar_height,lcddev.width,aui_prgbar_height,AUDIO_MAIN_BKCOLOR);	//填充进度条栏背景色
	gui_fill_rectangle(0,lcddev.height-aui_btnbar_height,lcddev.width,aui_btnbar_height,AUDIO_BTN_BKCOLOR);						//填充按钮栏背景色   
	gui_fill_rectangle(0,aui_tpbar_height+aui_msgbar_height,lcddev.width,lcddev.height-aui_tpbar_height-aui_msgbar_height-aui_prgbar_height-aui_btnbar_height,AUDIO_MAIN_BKCOLOR);//填充底色
 	ai_load_picfile(AUDIO_BACK_PIC[idx],0,aui_tpbar_height+aui_msgbar_height,lcddev.width,lcddev.height-aui_tpbar_height-aui_msgbar_height-aui_prgbar_height-aui_btnbar_height,0);//加载背景图片
	if(lrcdev!=NULL)audio_lrc_bkcolor_process(lrcdev,0);//读取LRC背景色 
}
//显示音量百分比  
//pctx:百分比值
void audio_show_vol(u8 pctx)
{
	u16 sx,sy;
	if(lcddev.width==240)
	{
		sx=6+20;
		sy=20+20+1+12; 
	}else if(lcddev.width==320)
	{
		sx=20+20;
		sy=24+20+3+12;  
	}else if(lcddev.width==480)
	{
		sx=30+20;
		sy=30+20+8+12;  
	}
	gui_phy.back_color=AUDIO_MAIN_BKCOLOR;//设置背景色为底色
 	gui_fill_rectangle(sx,sy,24,12,AUDIO_MAIN_BKCOLOR);//填充背景色 
	if(pctx==100)
	{
		gui_show_num(sx,sy,3,AUDIO_INFO_COLOR,12,pctx,0x80);//显示音量百分比   
		gui_show_ptchar(sx+18,sy,sx+18+6,sy+12,0,AUDIO_INFO_COLOR,12,'%',0);	//显示百分号
	}else 
	{
		gui_show_num(sx,sy,2,AUDIO_INFO_COLOR,12,pctx,0x80);//显示音量百分比   
 		gui_show_ptchar(sx+12,sy,sx+12+6,sy+12,0,AUDIO_INFO_COLOR,12,'%',0);	//显示百分号
  	}	 
}  
//显示audio播放时间
//sx,sy:起始坐标
//sec:时间
void audio_time_show(u16 sx,u16 sy,u16 sec)
{
	u16 min;
	min=sec/60;//得到分钟数
	if(min>99)min=99;
	sec=sec%60;//得到秒钟数 
	gui_phy.back_color=AUDIO_MAIN_BKCOLOR;//设置背景色为底色
	gui_show_num(sx,sy,2,AUDIO_INFO_COLOR,12,min,0x80);//显示时间
	gui_show_ptchar(sx+12,sy,lcddev.width,lcddev.height,0,AUDIO_INFO_COLOR,12,':',0);//显示冒号
	gui_show_num(sx+18,sy,2,AUDIO_INFO_COLOR,12,sec,0x80);//显示时间	  
}
//audiodevx:audio控制器
//audioprgbx:进度条
//lrcx:歌词控制器
void audio_info_upd(__audiodev *audiodevx,_progressbar_obj* audioprgbx,_progressbar_obj* volprgbx,_lyric_obj* lrcx)
{
	static u16 temp;
	u8 aui_tpbar_height;	//顶部标题栏高度
	u8 aui_msgbar_height;	//信息栏高度 
	u8 pixpitch;
	u8 toffy;
	u16 tempx;
	u8 *buf;
	float ftemp;
	if(lcddev.width==240)
	{
		aui_tpbar_height=20;
		aui_msgbar_height=46;
		pixpitch=6;
	}else if(lcddev.width==320)
	{
		aui_tpbar_height=24;
		aui_msgbar_height=50;
		pixpitch=20;
	}else if(lcddev.width==480)
	{
		aui_tpbar_height=30;
		aui_msgbar_height=60;
		pixpitch=30;
	}
	toffy=(aui_msgbar_height-44)/2;
	if((audiodevx->status&(1<<5))&&(audiodevx->status&(1<<1))&&audiodev.samplerate!=0)//执行了一次歌曲切换,且新音乐已经在播放了且正确获取了采样率,更新歌曲名字和当前播放曲目索引,audioprgb长度等信息
	{
		audiodevx->status&=~(1<<5);//清空标志位  
		buf=gui_memin_malloc(100);	//申请100字节内存
		if(buf==NULL)return;		//game over   
		gui_fill_rectangle(0,aui_tpbar_height+3,lcddev.width,14,AUDIO_MAIN_BKCOLOR);//上下各多清空一点,清空之前的显示 
		gui_show_ptstrwhiterim(4,aui_tpbar_height+4,lcddev.width-4,lcddev.height,0,0X0000,0XFFFF,12,audiodevx->name);	//显示新的名字		
		audiodevx->namelen=strlen((const char*)audiodevx->name);//得到所占字符的个数
		audiodevx->namelen*=6;//得到点数
		audiodevx->curnamepos=0;//得到点数
		gui_phy.back_color=AUDIO_MAIN_BKCOLOR;//设置背景色为底色 
		//显示音量百分比
		audio_show_vol((volprgbx->curpos*100)/volprgbx->totallen);//显示音量百分比 			
		//显示曲目编号
		sprintf((char*)buf,"%03d/%03d",audiodevx->curindex+1,audiodevx->mfilenum);
		gui_fill_rectangle(pixpitch+20+volprgbx->width-42,aui_tpbar_height+20+toffy+12,42,12,AUDIO_MAIN_BKCOLOR);	//清空之前的显示  
		gui_show_string(buf,pixpitch+20+volprgbx->width-42,aui_tpbar_height+20+toffy+12,42,12,12,AUDIO_INFO_COLOR);	
		//显示xxxKhz
		tempx=pixpitch*2+20+volprgbx->width+6;//起始x坐标
		gui_fill_rectangle(tempx,aui_tpbar_height+20+toffy,54,12,AUDIO_MAIN_BKCOLOR);		//清空之前的显示  
		ftemp=(float)audiodevx->samplerate/1000;//xxx.xKhz 
		sprintf((char*)buf,"%3.1fKhz",ftemp);
		gui_show_string(buf,tempx,aui_tpbar_height+20+toffy,54,12,12,AUDIO_INFO_COLOR);	 
		//显示位数	 	
		tempx=pixpitch*2+20+volprgbx->width+60;//起始x坐标
		gui_fill_rectangle(tempx,aui_tpbar_height+20+toffy,30,12,AUDIO_MAIN_BKCOLOR);		//清空之前的显示
		sprintf((char*)buf,"%02dbit",audiodevx->bps); 
		gui_show_string(buf,tempx,aui_tpbar_height+20+toffy,30,12,12,AUDIO_INFO_COLOR);	       
 		//其他处理
 	 	temp=0;
 		audioprgbx->totallen=audiodevx->file->fsize;	//更新总长度	
		audioprgbx->curpos=0;	 
		if(lrcx)
		{
			lrc_read(lrcx,audiodevx->path,audiodevx->name);
			audio_lrc_bkcolor_process(lrcx,1);//恢复背景色
			lrcdev->curindex=0;	//重新设置歌词位置为0.
			lrcdev->curtime=0;	//重设时间
		}
		gui_memin_free(buf);//释放内存		
	}
	if(audiodevx->namelen>lcddev.width-8)//大于屏幕长度
	{
		gui_fill_rectangle(0,aui_tpbar_height+3,lcddev.width,14,AUDIO_MAIN_BKCOLOR);//上下各多清空一点,清空之前的显示 
		gui_show_ptstrwhiterim(4,aui_tpbar_height+4,lcddev.width-4,lcddev.height,audiodevx->curnamepos,0X0000,0XFFFF,12,audiodevx->name);	//显示新的名字
		audiodevx->curnamepos++;
		if(audiodevx->curnamepos+lcddev.width-8>(audiodevx->namelen+lcddev.width/2-10))audiodevx->curnamepos=0;//循环显示		
	}
	if(audiodevx->status&(1<<7))//audio正在播放
	{ 
	 	audioprgbx->curpos=f_tell(audiodevx->file);//得到当前的播放位置
		progressbar_draw_progressbar(audioprgbx);//更新进度条位置 
		if(temp!=audiodevx->cursec)
		{
 			temp=audiodevx->cursec;
			buf=gui_memin_malloc(100);	//申请100字节内存
			if(buf==NULL)return;		//game over   
			//显示码率(Kbps )		 
			tempx=pixpitch*2+20+volprgbx->width+6;//起始x坐标 
			gui_fill_rectangle(tempx,aui_tpbar_height+20+toffy+12,48,12,AUDIO_MAIN_BKCOLOR);	//清空之前的显示   
			sprintf((char*)buf,"%04dKbps",audiodevx->bitrate/1000);  
			gui_show_string(buf,tempx,aui_tpbar_height+20+toffy+12,48,12,12,AUDIO_INFO_COLOR);	//显示分辨率
			gui_memin_free(buf);		//释放内存		
 			//显示时间
			if(lcddev.width==240)
			{
				audio_time_show(5,lcddev.height-48-12-12-9,audiodevx->cursec);			//显示播放时间   
				audio_time_show(40+160+5,lcddev.height-48-12-12-9,audiodevx->totsec);	//显示总时间   
			}else if(lcddev.width==320)
			{
				audio_time_show(10,lcddev.height-60-20-12-9,audiodevx->cursec);			//显示播放时间   
				audio_time_show(45+230+5,lcddev.height-60-20-12-9,audiodevx->totsec);	//显示总时间   
			}else if(lcddev.width==480)
			{
				audio_time_show(30,lcddev.height-80-40-12-14,audiodevx->cursec);		//显示播放时间   
				audio_time_show(70+340+10,lcddev.height-80-40-12-14,audiodevx->totsec);	//显示总时间   
			}
		}	    
	}
}
//lrc背景色处理
//lrcx:歌词控制结构体
//mode:0,读取背景色
//     1,恢复背景色
void audio_lrc_bkcolor_process(_lyric_obj* lrcx,u8 mode)
{ 
	u8 lrcdheight;
	u16 sy; 
	u16 i; 
	if(lcddev.width==240)
	{
		lrcdheight=4;
		sy=20+46+(164-8*lrcdheight-112)/2; 
	}else if(lcddev.width==320)
	{
		lrcdheight=6;
		sy=24+50+(296-8*lrcdheight-112)/2; 
	}else if(lcddev.width==480)
	{
		lrcdheight=10;
		sy=30+60+(550-8*lrcdheight-112)/2; 
	}	 
	for(i=0;i<7;i++)
	{
		if(mode==0)//读取背景色
		{
			 app_read_bkcolor(20,sy,lcddev.width-40,16,lrcx->lrcbkcolor[i]);
		}else
		{ 
			app_recover_bkcolor(20,sy,lcddev.width-40,16,lrcx->lrcbkcolor[i]);			
		}
		if(i==2||i==3)sy+=16+lrcdheight*2;
		else sy+=16+lrcdheight;		
	} 
}
//显示歌词
//audiodevx:audio控制器
//lrcx:歌词控制器
void audio_lrc_show(__audiodev *audiodevx,_lyric_obj* lrcx)
{
	u8 t;
	u16 temp,temp1;	  
	u16 lrcwidth=lcddev.width-40;
	u8 lrcdheight;
	u16 sy;
	u16 syadd;
	if(lcddev.width==240)
	{
		lrcdheight=4;
		sy=20+46+(164-8*lrcdheight-112)/2; 
	}else if(lcddev.width==320)
	{
		lrcdheight=6;
		sy=24+50+(296-8*lrcdheight-112)/2; 
	}else if(lcddev.width==480)
	{
		lrcdheight=10;
		sy=30+60+(550-8*lrcdheight-112)/2; 
	}
	if(lrcx->oldostime!=GUI_TIMER_10MS)//每10ms更新一下
	{
		t=gui_disabs(GUI_TIMER_10MS,lrcx->oldostime);//防止很久没有进入主程序导致的漏加
		lrcx->oldostime=GUI_TIMER_10MS;
		if(t>10)t=1;
		lrcx->curtime+=t;//增加10ms	 
		if(lrcx->indexsize)//有歌词存在 
		{
			lrcx->detatime+=t;//标志时间增加了10ms
			if(lrcx->curindex<lrcx->indexsize)//还没显示完
			{
	 			if((lrcx->curtime%100)>80)//1秒钟后过了800ms,需要查询解码时间寄存器以同步歌词
				{							 	 
					lrcx->curtime=audiodevx->cursec*100;//更新秒钟
				}
				if(lrcx->curtime>=lrcx->time_tbl[lrcx->curindex])//当前时间超过了,需要更新歌词
				{   
					syadd=sy;
					temp1=lrcx->curindex;//备份当前lrcx->curindex.
					if(lrcx->curindex>=3)
					{
						lrcx->curindex-=3;
						temp=0;
					}else 
					{
						temp=3-lrcx->curindex;
						lrcx->curindex=0;
					}
					for(t=0;t<7;t++)	//显示7条歌词
					{
						if(t!=3)lrcx->color=AUDIO_LRC_SCOLOR;
						else lrcx->color=AUDIO_LRC_MCOLOR;
						
						app_recover_bkcolor(20,syadd,lcddev.width-40,16,lrcx->lrcbkcolor[t]);//恢复背景色  
						if(lrcx->curindex<=(lrcx->indexsize-1)&&lrcx->curindex>=temp)
						{
							lrc_show_linelrc(lrcx,20,syadd,lrcwidth,16);//显示歌词
							lrcx->curindex++;
						}
						if(temp)temp--;
						if(t==2||t==3)syadd+=16+lrcdheight*2;
						else syadd+=16+lrcdheight;	 
					}
					lrcx->curindex=temp1;//恢复原来的值
					lrc_show_linelrc(lrcx,0,0,0,0);//读取当前歌词,但是不显示.
					lrcx->curindex++;	//偏移到下一条歌词
					if(lrcx->namelen>(lcddev.width-40))//需要滚动歌词
					{
						if(lrcx->curindex<lrcx->indexsize)//本句的下一句歌词还是存在的.
						{
							temp=lrcx->time_tbl[lrcx->curindex-1];//当前歌词的时间
							temp1=lrcx->time_tbl[lrcx->curindex]; //下一句歌词的时间 	 	   
							lrcx->updatetime=(temp1-temp)/(lrcx->namelen-150);//计算得到滚动间隔时间,这里多了50个单位,因为前面的程序执行时间影响.
							if(lrcx->updatetime>20)lrcx->updatetime=20;//最大不超过200ms;
 						}else lrcx->updatetime=5;//默认滚动时间.50ms	  
					}													   
				}
			}
			if(lrcx->detatime>=lrcx->updatetime)//每lrcx->updatetime*10ms滚动显示当前歌词(如果需要滚动的话)
			{			  
				if(lrcx->namelen>(lcddev.width-40))//超过了显示区域,需要滚动显示本句歌词
				{
					syadd=sy+3*16+lrcdheight*4; 
					app_recover_bkcolor(20,syadd,lcddev.width-40,16,lrcx->lrcbkcolor[3]);  
					gui_show_ptstr(20,syadd,lcddev.width-21,lcddev.height,lrcx->curnamepos,AUDIO_LRC_MCOLOR,lrcx->font,lrcx->buf,1);//滚动显示本句歌词
					lrcx->curnamepos++;
					if(lrcx->curnamepos+200>lrcx->namelen+50)lrcx->curnamepos=0;//循环显示		
				}
				lrcx->detatime=0;		  
			}
		}   
	}
}	 
//创建audio task
//返回值:0,成功
//    其他,错误代码
u8 audio_task_creat(void)
{ 
    OS_CPU_SR cpu_sr=0;
	u8 res;     	 
	u8 i;
	u32 size;
	lrcdev=lrc_creat();									//创建歌词管理结构体
	if(lrcdev)
	{
		lrcdev->font=16; 
		size=(lcddev.width-40)*16*2;//一条歌词背景的内存大小
		for(i=0;i<7;i++)//申请7条歌词的背景色表
		{
			lrcdev->lrcbkcolor[i]=gui_memex_malloc(size);
			if(lrcdev->lrcbkcolor[i]==NULL)break;
		}
		if(i!=7)//内存申请失败
		{
			audio_task_delete();
			return 2;
		}
	}else return 1;										//创建失败
 	AUDIO_PLAY_TASK_STK=gui_memin_malloc(AUDIO_PLAY_STK_SIZE*sizeof(OS_STK));
	if(AUDIO_PLAY_TASK_STK==0)return 1;					//内存申请失败
	audiodev.status=1<<7;								//允许音频播放任务运行
	OS_ENTER_CRITICAL();//进入临界区(无法被中断打断)    
	res=OSTaskCreate(audio_play_task,(void *)0,(OS_STK*)&AUDIO_PLAY_TASK_STK[AUDIO_PLAY_STK_SIZE-1],AUDIO_PLAY_TASK_PRIO);						   
	OS_EXIT_CRITICAL();	//退出临界区(可以被中断打断) 
	return res;
}
//删除audio_task
void audio_task_delete(void)
{
	u8 i;
 	if(audiodev.status==0)return;		//播放已经停止了
	audiodev.status=0;					//播放停止 
	gui_memin_free(audiodev.path);		//释放内存
	gui_memex_free(audiodev.mfindextbl);//释放内存  
	for(i=0;i<7;i++)//释放内存
	{
		gui_memex_free(lrcdev->lrcbkcolor[i]); 
	}	
	lrc_delete(lrcdev);					//释放歌词显示申请的内存
	lrcdev=NULL;						//指向空地址
	VS_SPK_Set(0);						//关闭喇叭输出 
	VS_Set_Vol(0);						//设置音量为0 
	gui_memin_free(AUDIO_PLAY_TASK_STK);//释放内存
	OSTaskDel(AUDIO_PLAY_TASK_PRIO);	//删除音乐播放任务
}

//audio播放
u8 audio_play(void)
{
	u8 i;
	u8 res;
	u8 tcnt=0;
	u8 rval=0;		//1,内存错误;2,返回,audio继续播放;3,返回,停止audio播放.
 	u16 lastvolpos; 
	_progressbar_obj* audioprgb,*volprgb;
	_btn_obj* tbtn[5];		    
	
	if((audiodev.status&(1<<7))==0)	//音频播放任务已经删除?/第一次进入?
	{
		memset(&audiodev,0,sizeof(__audiodev));//audiodev所有数据清零.
		res=audio_task_creat();		//创建任务 
		if(res==0)res=audio_filelist(&audiodev);//选择音频文件进行播放
		if(res||audiodev.status==0X80)			//创建任务失败/内存分配失败/没有选择音频播放
		{
			audio_task_delete();
			return 1;
		}
		system_task_return=0;
	}else
	{
		audiodev.status|=1<<5;		//模拟一次切歌,以更新主界面内容
	}
 	if(lcddev.width==240)
	{
		audioprgb=progressbar_creat(40,lcddev.height-48-12-10-11,160,12,0X20);		//audio播放进度条
		if(audioprgb==NULL)rval=1;
		volprgb=progressbar_creat(6+20,20+20+1+1,110,10,0X20);	//声音大小进度条
		if(volprgb==NULL)rval=1;	   
		volprgb->totallen=150;	//更新总长度,音频从100~250.偏移为100. 	
	}else if(lcddev.width==320)
	{
		audioprgb=progressbar_creat(45,lcddev.height-60-20-10-11,230,12,0X20);		//audio播放进度条
		if(audioprgb==NULL)rval=1;
		volprgb=progressbar_creat(20+20,24+20+3+1,150,10,0X20);	//声音大小进度条
		if(volprgb==NULL)rval=1;	   
		volprgb->totallen=150;	//更新总长度,音频从100~250.偏移为100.
	}else if(lcddev.width==480)
	{
		audioprgb=progressbar_creat(70,lcddev.height-80-40-10-16,340,12,0X20);		//audio播放进度条
		if(audioprgb==NULL)rval=1;
		volprgb=progressbar_creat(30+20,30+20+8+1,280,10,0X20);	//声音大小进度条
		if(volprgb==NULL)rval=1;	   
		volprgb->totallen=150;	//更新总长度,音频从100~250.偏移为100.
	} 
	if(vsset.mvol>=100&&vsset.mvol<=250)volprgb->curpos=vsset.mvol-100;
	else//错误的数据 
	{
		vsset.mvol=0;
		volprgb->curpos=0;
	}	  
	lastvolpos=volprgb->curpos;//设定最近的位置
	for(i=0;i<5;i++)//循环创建5个按钮
	{
		if(lcddev.width==240)tbtn[i]=btn_creat(0+i*48,lcddev.height-48-6,48,48,0,1);			//创建图片按钮
		else if(lcddev.width==320)tbtn[i]=btn_creat(2+i*(60+4),lcddev.height-60-10,60,60,0,1);	//创建图片按钮
		else if(lcddev.width==480)tbtn[i]=btn_creat(8+i*(80+16),lcddev.height-80-20,80,80,0,1);	//创建图片按钮
		if(tbtn[i]==NULL){rval=1;break;}		//创建失败.
		tbtn[i]->bcfdcolor=0X2CFF;				//按下时的背景色
		tbtn[i]->bcfucolor=AUDIO_BTN_BKCOLOR;	//松开时背景色 
		tbtn[i]->picbtnpathu=(u8*)AUDIO_BTN_PIC_TBL[0][i];
		tbtn[i]->picbtnpathd=(u8*)AUDIO_BTN_PIC_TBL[1][i];
 		tbtn[i]->sta=0;	 
	}
	if(rval==0)//没有错误
	{
	   	audioprgb->inbkcolora=0x738E;			//默认色
	 	audioprgb->inbkcolorb=AUDIO_INFO_COLOR;	//默认色 
	 	audioprgb->infcolora=0X75D;				//默认色
	 	audioprgb->infcolorb=0X596;				//默认色  
	   	volprgb->inbkcolora=AUDIO_INFO_COLOR;	//默认色
	 	volprgb->inbkcolorb=AUDIO_INFO_COLOR;	//默认色 
	 	volprgb->infcolora=0X75D;				//默认色
	 	volprgb->infcolorb=0X596;				//默认色   
		audio_load_ui();//加载主界面
		for(i=0;i<5;i++)btn_draw(tbtn[i]);		//画按钮
		tbtn[2]->picbtnpathu=(u8*)AUDIO_PLAYR_PIC;//按下一次之后变为播放松开状态
  		progressbar_draw_progressbar(audioprgb);//画进度条	 
		progressbar_draw_progressbar(volprgb);	//画进度条
		system_task_return=0;
		tcnt=0;  
		while(rval==0)
		{
			tcnt++;//计时增加.
			tp_dev.scan(0);    
			in_obj.get_key(&tp_dev,IN_TYPE_TOUCH);	//得到按键键值   
			delay_ms(1000/OS_TICKS_PER_SEC);		//延时一个时钟节拍  
			for(i=0;i<5;i++)
			{
				res=btn_check(tbtn[i],&in_obj);	 
				if((res&&((tbtn[i]->sta&(1<<7))==0)&&(tbtn[i]->sta&(1<<6)))||system_task_return)//有按键按下且松开,并且TP松开了或者TPAD返回
				{ 
					if(system_task_return)i=4;//TPAD返回
					switch(i)
					{
						case 0://file list
							audio_filelist(&audiodev); 
							//////////////////////////////////////////////////////////////////
							audio_load_ui();//重新加载主界面
							audiodev.status|=1<<5;//模拟一次切歌,以更新主界面内容
							if(audiodev.status&(1<<0))	//继续播放?
							{
								tbtn[2]->picbtnpathu=(u8*)AUDIO_PAUSER_PIC;
							}else tbtn[2]->picbtnpathu=(u8*)AUDIO_PLAYR_PIC;
							for(i=0;i<5;i++)btn_draw(tbtn[i]);		//画按钮
							if(audiodev.status&(1<<0))	
							{
								tbtn[2]->picbtnpathu=(u8*)AUDIO_PLAYR_PIC;
							}else tbtn[2]->picbtnpathu=(u8*)AUDIO_PAUSER_PIC; 
							progressbar_draw_progressbar(audioprgb);	//画进度条	 
							progressbar_draw_progressbar(volprgb);	//画进弱
							if(system_task_return)//刚刚退出文件浏览
							{
								delay_ms(100);
								system_task_return=0;
							}		 
							break;
						case 1://上一曲或者下一曲
						case 3:
							audio_stop_req(&audiodev);
							if(systemset.audiomode==1)//随机播放
							{						    
								audiodev.curindex=app_get_rand(audiodev.mfilenum);//得到下一首歌曲的索引	  
							}else
							{
								if(i==1)//上一曲
								{
									if(audiodev.curindex)audiodev.curindex--;
									else audiodev.curindex=audiodev.mfilenum-1;
								}else
								{
									if(audiodev.curindex<(audiodev.mfilenum-1))audiodev.curindex++;
									else audiodev.curindex=0;
								}
							}
							OSMboxPost(audiombox,(void*)(audiodev.curindex+1));//发送邮箱,因为邮箱不能为空,所以在这必须加1
							break;
						case 2://播放/暂停
							if(audiodev.status&(1<<0))//是暂停
							{ 
								audiodev.status&=~(1<<0);//标记暂停 
								tbtn[2]->picbtnpathd=(u8*)AUDIO_PLAYP_PIC; 
								tbtn[2]->picbtnpathu=(u8*)AUDIO_PAUSER_PIC;
							}else//暂停状态
							{
								audiodev.status|=1<<0;//取消暂停 
								tbtn[2]->picbtnpathd=(u8*)AUDIO_PAUSEP_PIC; 
								tbtn[2]->picbtnpathu=(u8*)AUDIO_PLAYR_PIC;
							}
							break;
						case 4://停止播放/返回
							if((audiodev.status&(1<<0))==0)//暂停状态下按了返回
							{		   
								rval=3;//退出播放,audio停止播放
								audio_stop_req(&audiodev);//请求停止播放
							}else//暂停状态下按返回,那就关闭audio播放功能.
							{
								rval=2;//退出播放界面,audio继续播放
							}	 
							break; 
					}
				}  
			}				
			res=progressbar_check(volprgb,&in_obj);//检查音量进度条
			if(res&&lastvolpos!=volprgb->curpos)//被按下了,且位置变化了.执行音量调整
			{		  
				lastvolpos=volprgb->curpos;
				if(volprgb->curpos)vsset.mvol=100+volprgb->curpos;
				else vsset.mvol=0;		
				VS_Set_Vol(vsset.mvol);//设置音量	 
				audio_show_vol((volprgb->curpos*100)/volprgb->totallen);	//显示音量百分比   
			}	  
			res=progressbar_check(audioprgb,&in_obj);
			if(res&&((audioprgb->sta&&PRGB_BTN_DOWN)==0))//被按下了,并且松开了,执行快进快退
			{
				//printf("audioprgb->curpos:%d\r\n",audioprgb->curpos);
				lrcdev->curindex=0;	//重新设置歌词位置为0.
				lrcdev->curtime=0;	//重设时间
				audioprgb->curpos=audiodev.file_seek(audioprgb->curpos);//快进快退 
 			}	  
			if((tcnt%20)==0)audio_info_upd(&audiodev,audioprgb,volprgb,lrcdev);//更新显示信息,每100ms执行一次
			if(lrcdev!=NULL&&((audiodev.status&(1<<5))==0)&&(audiodev.status&(1<<7)))//正在播放,不是暂停,且歌词结构体正常
			{
				audio_lrc_show(&audiodev,lrcdev);	//可以显示歌词	  
			}
		}
	} 
 	for(i=0;i<5;i++)btn_delete(tbtn[i]);//删除按钮	
	progressbar_delete(audioprgb);
	progressbar_delete(volprgb);
	if(rval==3)//退出audio播放.且不后台播放
	{
		audio_task_delete();//删除音频播放任务 
	} 		  
	return rval;
}
 











