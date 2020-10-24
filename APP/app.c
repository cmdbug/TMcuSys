#include "includes.h"
#include "app.h"
#include "rtc.h"   //teng
#include "EmWinHZFont.h"//teng
#include "ff.h"//teng
#include "malloc.h"//teng
#include "appskin.h"//teng
#include "ILI93xx.h"//测试能不能用原始lcd方法显示teng
#include "MessageBoxUser.h"
#include "tpad.h"
#include "led.h"
#include <stdlib.h>  /* for rand */
/*
*********************************************************************************************************
*                                         宏定义 
*********************************************************************************************************
*/
#define ICONVIEW_Width     48   /* 控件ICONVIEW的宽 */  
#define ICONVIEW_Height    48   /* 控件ICONVIEW的高，包括图标的Y方向间距 */  
#define ICONVIEW_XSpace    10   /* 控件ICONVIEW的X方向间距 */ 
#define ICONVIEW_YSpace    15   /* 控件ICONVIEW的Y方向间距 */ 
#define ICONVIEW_XPos	   4    /* 控件ICONVIEW的X方向调整距离*/
#define VIRTUAL_WIN_NUM	   1    /* icon的页数 */ // 2
#define ICONVIEW_ImagNum   20   /* 控件ICONVIEW中的图标个数 */ 
#define BOTWIN_YSpace	   60   /* 屏幕下方部分窗口Y方向间距*/
#define MIDWIN_yPos		   20   /* 中间窗口Y方向起始位置即顶部文字可用高度*/
#define MIDWIN_xPos		   0 	/* 中间窗口Y方向起始位置*/

/*
*********************************************************************************************************
*                                      变量
*********************************************************************************************************
*/ 
/*时间结构体*/
//extern struct rtc_time systmtime;
extern _calendar_obj calendar;//时钟结构体
//GUI_XBF_DATA XBF_Data;
//GUI_FONT     XBF_Font;

// 主屏背景颜色
#define TOP_BG_COLOR        0x00fd71a9
#define MID_BG_TOP_COLOR    0x00f7aac9
#define MID_BG_BOTTOM_COLOR 0x00f95d9b
#define BOTTOM_BG_COLOR     0x00ed9cbc
#define ICON_BG_ALPHA       0x80000000

/* 用于桌面ICONVIEW图标的创建 */
typedef struct {
  GUI_BITMAP * pBitmap;
  const char       * pText;
} BITMAP_ITEM;

typedef struct WIN_PARA{			//窗口使用到的用户定义参数，方便在回调函数中使用
	int xSizeLCD, ySizeLCD;			//LCD屏幕尺寸
	int xPosWin,  yPosWin;			//窗口的起始位置
	int xSizeWin, ySizeWin;			//窗口尺寸	
	int xSizeBM,  ySizeBM;
	int ySizeBotWin;
	
	WM_HWIN hWinBot;				//控制窗口的句柄（底部）
	WM_HWIN hWinMid;				//主显示窗的句柄（中部）
	
}WIN_PARA;
WIN_PARA WinPara;			//用户使用的窗口额外数据
__IO uint8_t app_prevent_refresh = 0;  /* 这个参数比较重要，用于防止ICON控件点击下时就触发回调函数的WM_PAINT消息*/
/* 用于桌面ICONVIEW0图标的创建 */
static BITMAP_ITEM _aBitmapItem0[] = {
	{&bmphone,     "拨号" 	},
	{&bmmessages,  "短信"	},
	{&bmbrowser,   "浏览器" 	},
	{&bmapp,  	   "应用"	},
};
/* 用于桌面ICONVIEW1图标的创建 */
static BITMAP_ITEM _aBitmapItem1[] = {
	{&bmebooks,		"电子书"	},
	{&bmphotos,  	"相册"	},  
	{&bmmusic,      "音乐"	},
	{&bmvideos,		"视频"	},

	{&bmclock,		"时钟" 	},
	{&bmexeplay, 	"运行器"},
	{&bmgames,   	"游戏" 	},
	{&bmnotes,  	"记事本"},

	{&bmsettings, 	"设置"	},  
	{&bmrecoder,  	"录音机" },
	{&bmradio,      "收音机" },
	{&bmcalculator,	"计算器"	}
};

/* 用于桌面ICONVIEW2图标的创建 */
static BITMAP_ITEM _aBitmapItem2[] = {
	{&bmcamera,  	"照相机"	},
	{&bmusb,		"USB连接" 	},
	{&bminternet,   "互联网" 	},
	{&bmwireless,   "无线"		},
	
	{&bmwifi,		"WIFI"		},  
	{&bmcompass,   	"罗盘" 		},	
	{&bmbluetooth,  "蓝牙" 		},
	{&bmgps,    	"定位"		},
	
//	{&bmcalendar,	"日历"		},  
//	{&bmxxx,   		"xxx" 		},	
//	{&bmxxx,  		"xxx" 		},
//	{&bmxxx,    	"xxx"		}
};


u8 *ICONBuff[3][12];//图标缓存要保持有效,一个图标大约9-12KB
//从外部加载图标.bin文件
//ix:当前图标
//哪个区域图标0底部1第一页2第二页
u8 Load_ICON(u8 ix, GUI_BITMAP *pBitmapx, u8 itemx)
{
	FIL ICONFile;	  		//文件1
	UINT br,bw;				//读写变量
	u8 res;
//	u8 *databuf = NULL;

	res = f_open(&ICONFile, (u8*)icos_path_tbl[itemx][ix], FA_READ);     //打开图标文件
	if(res == FR_OK)
	{
		ICONBuff[itemx][ix] = (u8*)mymalloc(SRAMEX, ICONFile.fsize);     //申请图标大小的外部内存
		if(ICONBuff[itemx][ix] == NULL)
		{
			f_close(&ICONFile);                                          //申请失败退出
			return 0x01;
		}
		res = f_read(&ICONFile, ICONBuff[itemx][ix], ICONFile.fsize, &br);    //读取图标文件放入缓存区
		if(res == FR_OK)
		{
			pBitmapx->pData = ICONBuff[itemx][ix];                            //传递图标缓存指针
		}
	}
	else
	{
		return res;
	}
	f_close(&ICONFile);                                                       //关闭图标文件
//	myfree(SRAMEX, databuf);                                                  //保持图标有效不能释放
	return res;
}


/*
*********************************************************************************************************
*	函 数 名: _cbBkWindow
*	功能说明: 桌面窗口的回调函数 
*	形    参：WM_MESSAGE * pMsg
*	返 回 值: 无
*********************************************************************************************************
*/
void _cbBkWindow(WM_MESSAGE * pMsg) 
{
	char text_buffer[20]={0};
	WM_HWIN hText;
    GUI_PID_STATE pState;
	switch (pMsg->MsgId) 
	{
		case WM_TIMER://处理时间显示的信息，每秒钟修改一次时间
			/* 获取text句柄 */ 
            hText = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1);
			/* 转换rtc值至北京时间 */
//			RTC_TimeCovr(&systmtime);
//            RTC_Get();
			/* 转换成字符串 */
			sprintf(text_buffer,"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);  //teng
			/* 输出时间 */
			TEXT_SetText(hText,text_buffer);
		
			/* 获取text句柄 */      
			hText = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT2);
			sprintf(text_buffer, "CPU:%3d.%02d%%", OSStatTaskCPUUsage/100, OSStatTaskCPUUsage%100);
			/* 输出CPU */
			TEXT_SetText(hText,text_buffer);
		
			WM_RestartTimer(pMsg->Data.v, 1000);
		break;
		/* 重绘消息*/
		case WM_PAINT:		
            GUI_SetBkColor(TOP_BG_COLOR);      //GUI_ORANGE
            GUI_Clear();
//			GUI_DrawBitmap(&bmlogo,1,1);
            if (GUI_TOUCH_GetState(&pState)==0) {
                ICONVIEW_SetSel(WM_GetDialogItem(WinPara.hWinMid, GUI_ID_ICONVIEW2),-1);
                ICONVIEW_SetSel(WM_GetDialogItem(WinPara.hWinMid, GUI_ID_ICONVIEW1),-1);
                ICONVIEW_SetSel(WM_GetDialogItem(WinPara.hWinBot, GUI_ID_ICONVIEW0),-1);
            }
		break;
			
	 default:
		WM_DefaultProc(pMsg);
		break;
	}
}
/*
*********************************************************************************************************
*	函 数 名: _cbMidWin
*	功能说明: 
*	形    参：WM_MESSAGE * pMsg
*	返 回 值: 无
*********************************************************************************************************
*/
void _cbMidWin(WM_MESSAGE * pMsg) 
{
	int NCode, Id;
	WM_MOTION_INFO * pInfo;
	switch (pMsg->MsgId) 
	{
		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);      /* Id of widget */
			NCode = pMsg->Data.v;                 /* Notification code */
			switch (Id) 
			{
				case GUI_ID_ICONVIEW1:
					switch (NCode) 
					{
						/* ICON控件点击消息 */
						case WM_NOTIFICATION_CLICKED:
							app_prevent_refresh = 1;
							break;
						
						/* ICON控件释放消息 */
						case WM_NOTIFICATION_RELEASED: 
							
							/* 打开相应选项 */
							switch(ICONVIEW_GetSel(pMsg->hWinSrc))
							{
								case 0:	
                                    MESSAGEBOX_Create_User_Modal("电子书", "提示");                                
									break;	
								case 1:
                                    MESSAGEBOX_Create_User_Modal("图片:Flash不足,自行在代码中取消注释", "提示");
//                                    CreatePicture();//需要修改malloc.h申请空间小点
									break;
								case 2:
                                    CreateMusic();
									break;
								case 3:
                                    MESSAGEBOX_Create_User_Modal("视频:Flash不足,自行在代码中取消注释", "提示"); 
//                                    CreateVideo();
									break;
								case 4:
                                    CreateDateTime();
									break;
								case 5:
                                    CreateExAppRun();
									break;
								case 6:
                                    MESSAGEBOX_Create_User_Modal("游戏:Flash不足,自行在代码中取消注释", "提示");                                    
//                                    CreateNesGame();// 游戏
									break;
								case 7:
                                    MESSAGEBOX_Create_User_Modal("记事本", "提示");
									break;
								case 8:
                                    CreateSetting();
									break;
								case 9:
                                    MESSAGEBOX_Create_User_Modal("录音机", "提示");
									break;
								case 10:
                                    MESSAGEBOX_Create_User_Modal("收音机", "提示");
									break;
								case 11:
                                    CreateCalculator();
									break;
								default:break;
								}
							 break;
						}
					break;
				case GUI_ID_ICONVIEW2:
					switch (NCode) 
					{
						/* ICON控件点击消息 */
						case WM_NOTIFICATION_CLICKED:
							app_prevent_refresh = 1;
							break;
						
						/* ICON控件释放消息 */
						case WM_NOTIFICATION_RELEASED: 
							
							/* 打开相应选项 */
							switch(ICONVIEW_GetSel(pMsg->hWinSrc))
							{			
								case 0:
                                    MESSAGEBOX_Create_User_Modal("照相机", "提示");
									break;
								case 1:
                                    MESSAGEBOX_Create_User_Modal("USB连接", "提示");
									break;
								case 2:
                                    MESSAGEBOX_Create_User_Modal("互联网", "提示");
									break;
								case 3:
                                    MESSAGEBOX_Create_User_Modal("无线", "提示");
									break;
								case 4:
                                    MESSAGEBOX_Create_User_Modal("WIFI", "提示");
									break;
								case 5:
                                    MESSAGEBOX_Create_User_Modal("罗盘", "提示");
									break;
								case 6:
                                    MESSAGEBOX_Create_User_Modal("蓝牙", "提示");
									break;
								case 7:
                                    MESSAGEBOX_Create_User_Modal("GPS", "提示");
									break;
								default:break;
							}	
						 break;
					}
				break;
			}
			break;
		
		case WM_MOTION:
			 pInfo = (WM_MOTION_INFO *)pMsg->Data.p;
			 if (pInfo) 
			 {
				//printf("mooton_message=%d\n",pInfo->Cmd);
				 switch (pInfo->Cmd) 
				 {				    
					case WM_MOTION_INIT:
						pInfo->SnapX = WinPara.xSizeLCD;		//像素对齐
						pInfo->SnapY = 0;
						//pInfo->Flags = WM_CF_MOTION_X | WM_MOTION_MANAGE_BY_WINDOW;
						//pInfo->Period = 500;
					break;
					
					case WM_MOTION_MOVE:
						WM_DisableWindow(WM_GetDialogItem(pMsg->hWin,GUI_ID_ICONVIEW1));
						WM_DisableWindow(WM_GetDialogItem(pMsg->hWin,GUI_ID_ICONVIEW2));
					break;							
					case WM_MOTION_GETPOS:
//						printf("\n dx = %d",pInfo->dx);
//						printf(" xpos = %d\n",pInfo->xPos);
//						printf(" da = %d\n",pInfo->da);
//						printf(" SnapX = %d\n",pInfo->SnapX);
					break;								
					default:
						break;						
				  }
				/* 移动结束 */
				if (pInfo->FinalMove || pInfo->dx==0) {
					WM_EnableWindow(WM_GetDialogItem(pMsg->hWin,GUI_ID_ICONVIEW1));
					WM_EnableWindow(WM_GetDialogItem(pMsg->hWin,GUI_ID_ICONVIEW2));
				}
			 }	
			
			break;
			
		/* 重绘消息*/
		case WM_PAINT:
			{
				GUI_DrawGradientV(0, 0, WinPara.xSizeWin, WinPara.ySizeWin, MID_BG_TOP_COLOR, MID_BG_BOTTOM_COLOR);
			//	GUI_DrawGradientV(0,0,WinPara.xSizeWin,WinPara.ySizeWin,GUI_ORANGE+0x303000,GUI_LIGHTYELLOW-0x303000);
			//	GUI_SetBkColor(GUI_RED);
			//	GUI_Clear();
			}			
		break;			
	 default:
		WM_DefaultProc(pMsg);
		break;
	}
}


/**
  * @brief  _cbBotWin,控制栏回调函数
  * @param  none
  * @retval none
  */
static void _cbBotWin(WM_MESSAGE * pMsg)
{
	int NCode, Id;
	switch (pMsg->MsgId) 
	{			 		 
		case WM_PAINT:	                             //重绘背景	
			GUI_SetBkColor(BOTTOM_BG_COLOR);         //GUI_LIGHTYELLOW
			GUI_Clear();
		break;
		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);      /* Id of widget */
			NCode = pMsg->Data.v;                 /* Notification code */
            switch (Id) 
            {
                case GUI_ID_ICONVIEW0:
                    switch (NCode) 
                    {
                        /* ICON控件点击消息 */
                        case WM_NOTIFICATION_CLICKED:
                            app_prevent_refresh = 1;
                            break;
                        
                        /* ICON控件释放消息 */
                        case WM_NOTIFICATION_RELEASED: 
                            
                            /* 打开相应选项 */
                            switch(ICONVIEW_GetSel(pMsg->hWinSrc))
                            {
                                case 0:	
                                    CreatePerfMon();
                                    break;	
                                case 1:
                                    MESSAGEBOX_Create_User_Modal("提示点啥", "提示");
                                    break;
                                case 2:
                                    CreateBrowser();
                                    break;
                                case 3:
                                    MESSAGEBOX_Create_User_Modal("写点啥", "提示");
                                    break;
                            }	
                        break;
                    }
                break;
            }
        break;		
        default:		
            WM_DefaultProc(pMsg);
        break;		 
	}
}

/**
  * @brief  CreateTopWin，创建顶部的窗口
  * @param  none
  * @retval none
  */
static void CreateTopWin(void)
{
	WM_HWIN hText;
	/* 顶部的文本 */
	hText = TEXT_CreateEx(3, 2, 60 , 16, WM_HBKWIN, WM_CF_SHOW, GUI_TA_LEFT|TEXT_CF_VCENTER, GUI_ID_TEXT0, "WZTENG");
	TEXT_SetFont(hText,GUI_FONT_10_ASCII);
	TEXT_SetTextColor(hText,GUI_DARKGRAY);
	
//	GUI_DrawBitmap(&bmlogo,1,1);
	
	/* 状态栏的时间显示文本 */
	hText = TEXT_CreateEx(WinPara.xSizeLCD/2-25,2,50,16,WM_HBKWIN,WM_CF_SHOW,GUI_TA_HCENTER|TEXT_CF_VCENTER,GUI_ID_TEXT1,"11:01:29");
	//TEXT_SetBkColor(hText,GUI_INVALID_COLOR);
	TEXT_SetTextColor(hText,GUI_DARKGRAY);
	TEXT_SetFont(hText,GUI_FONT_10_ASCII);
	
	/* 状态栏的CPU显示文本 */
	hText = TEXT_CreateEx(170,2,70,16,WM_HBKWIN,WM_CF_SHOW,TEXT_CF_VCENTER,GUI_ID_TEXT2,"CPU:000.00%");
	//TEXT_SetBkColor(hText,GUI_INVALID_COLOR);
	TEXT_SetTextColor(hText,GUI_DARKGRAY);
	TEXT_SetFont(hText,GUI_FONT_10_ASCII);
}
/**
  * @brief  CreateMidWin，创建中间的窗口
  * @param  none
  * @retval none
  */
static void CreateMidWin(void)
{
	uint8_t i=0;
	WM_HWIN MIDWIN;
	WinPara.hWinMid= WM_CreateWindowAsChild(
											WinPara.xPosWin,											
											WinPara.yPosWin,
											WinPara.xSizeWin,
											WinPara.ySizeWin,	
											WM_HBKWIN, 
											WM_CF_SHOW | WM_CF_MOTION_X, 
											_cbMidWin, 
											sizeof(WIN_PARA *)
											);
/*-------------------------------------------------------------------------------------*/
	/*在指定位置创建指定尺寸的ICONVIEW1 小工具*/
	MIDWIN=ICONVIEW_CreateEx(
							 ICONVIEW_XPos, 					/* 小工具的最左像素（在父坐标中）*/
							 0, 								/* 小工具的最上像素（在父坐标中）*/
							 WinPara.xSizeLCD-ICONVIEW_XPos,	/* 小工具的水平尺寸（单位：像素）*/
							 3*(WinPara.ySizeBM+30), 			/* 小工具的垂直尺寸（单位：像素）*/
							 WinPara.hWinMid, 				    /* 父窗口的句柄。如果为0 ，则新小工具将成为桌面（顶级窗口）的子窗口 */
							 WM_CF_SHOW | WM_CF_HASTRANS,       /* 窗口创建标记。为使小工具立即可见，通常使用 WM_CF_SHOW */ 
							 0,//ICONVIEW_CF_AUTOSCROLLBAR_V, 	/* 默认是0，如果不够现实可设置增减垂直滚动条 */
							 GUI_ID_ICONVIEW1, 			        /* 小工具的窗口ID */
							 ICONVIEW_Width, 				    /* 图标的水平尺寸 */
							 ICONVIEW_Height+13);				/* 图标的垂直尺寸 */
											 
	/* 向ICONVIEW 小工具添加新图标 */
	for (i = 0; i < GUI_COUNTOF(_aBitmapItem1); i++) 
	{
		Load_ICON(i, _aBitmapItem1[i].pBitmap, 1);
		ICONVIEW_AddBitmapItem(MIDWIN, _aBitmapItem1[i].pBitmap,_aBitmapItem1[i].pText);
	}
    ICONVIEW_SetTextColor(MIDWIN,ICONVIEW_CI_UNSEL,GUI_DARKGRAY);
    ICONVIEW_SetTextColor(MIDWIN,ICONVIEW_CI_SEL,GUI_BLUE);
	/* 设置小工具的背景色 32 位颜色值的前8 位可用于alpha混合处理效果*/
	ICONVIEW_SetBkColor(MIDWIN, ICONVIEW_CI_SEL, GUI_WHITE | ICON_BG_ALPHA);
	/* 设置字体 */
//	ICONVIEW_SetFont(MIDWIN, &GUI_Font10_ASCII);
	ICONVIEW_SetFont(MIDWIN, &GUI_FontHZ12);
	/* 设置初始选择的图标为 -1 (表示尚未选择)*/
	ICONVIEW_SetSel(MIDWIN,-1);
	/* 设置图标在x 或y 方向上的间距。*/
	ICONVIEW_SetSpace(MIDWIN, GUI_COORD_X, ICONVIEW_XSpace);
	ICONVIEW_SetSpace(MIDWIN, GUI_COORD_Y, ICONVIEW_YSpace);
//	ICONVIEW_SetSpace(hWin, GUI_COORD_Y, ICONVIEW_YSpace);
	/* 设置对齐方式 在5.22版本中最新加入的 */
	ICONVIEW_SetIconAlign(MIDWIN, ICONVIEW_IA_HCENTER|ICONVIEW_IA_TOP);
	
    //teng
    if (VIRTUAL_WIN_NUM <= 1) {
        // 卡暂时只弄一页好了
        return;
    }
/*-------------------------------------------------------------------------------------*/
	/*在指定位置创建指定尺寸的ICONVIEW2小工具*/
	MIDWIN=ICONVIEW_CreateEx(
							 WinPara.xSizeLCD+ICONVIEW_XPos,	/* 小工具的最左像素（在父坐标中）*/
							 0, 								/* 小工具的最上像素（在父坐标中）*/
							 WinPara.xSizeLCD-ICONVIEW_XPos,	/* 小工具的水平尺寸（单位：像素）*/
							 2*(WinPara.ySizeBM+30),			/* 小工具的垂直尺寸（单位：像素）*/
							 WinPara.hWinMid, 				    /* 父窗口的句柄。如果为0 ，则新小工具将成为桌面（顶级窗口）的子窗口 */
							 WM_CF_SHOW | WM_CF_HASTRANS,       /* 窗口创建标记。为使小工具立即可见，通常使用 WM_CF_SHOW */ 
							 0,//ICONVIEW_CF_AUTOSCROLLBAR_V, 	/* 默认是0，如果不够现实可设置增减垂直滚动条 */
							 GUI_ID_ICONVIEW2, 			        /* 小工具的窗口ID */
							 ICONVIEW_Width, 				    /* 图标的水平尺寸 */
							 ICONVIEW_Height+13);				/* 图标的垂直尺寸 */
	/*-------------------------------------------------------------------------------------*/						 
//	WM_EnableMemdev(WinPara.hWinMid); //teng
	/* 向ICONVIEW 小工具添加新图标 */
	for (i = 0; i < GUI_COUNTOF(_aBitmapItem2); i++) 
	{
		Load_ICON(i, _aBitmapItem2[i].pBitmap, 2);
		ICONVIEW_AddBitmapItem(MIDWIN, _aBitmapItem2[i].pBitmap,_aBitmapItem2[i].pText);
	}
    ICONVIEW_SetTextColor(MIDWIN,ICONVIEW_CI_UNSEL,GUI_DARKGRAY);
    ICONVIEW_SetTextColor(MIDWIN,ICONVIEW_CI_SEL,GUI_BLUE);
	/* 设置小工具的背景色 32 位颜色值的前8 位可用于alpha混合处理效果*/
	ICONVIEW_SetBkColor(MIDWIN, ICONVIEW_CI_SEL, GUI_WHITE | ICON_BG_ALPHA);
	/* 设置字体 */
//	ICONVIEW_SetFont(MIDWIN, &GUI_Font10_ASCII);
	ICONVIEW_SetFont(MIDWIN, &GUI_FontHZ12);
	/* 设置初始选择的图标为 -1 (表示尚未选择)*/
	ICONVIEW_SetSel(MIDWIN,-1);
	/* 设置图标在x 或y 方向上的间距。*/
	ICONVIEW_SetSpace(MIDWIN, GUI_COORD_X, ICONVIEW_XSpace);
	ICONVIEW_SetSpace(MIDWIN, GUI_COORD_Y, ICONVIEW_YSpace);
//	ICONVIEW_SetSpace(hWin, GUI_COORD_Y, ICONVIEW_YSpace);
	/* 设置对齐方式 在5.22版本中最新加入的 */
	ICONVIEW_SetIconAlign(MIDWIN, ICONVIEW_IA_HCENTER|ICONVIEW_IA_TOP);
}
/**
  * @brief  CreateBotWin，创建底部的窗口
  * @param  none
  * @retval none
  */
static void CreateBotWin(void)
{
	uint8_t i;
	WM_HWIN BOTWIN;
	WinPara.hWinBot= WM_CreateWindowAsChild(
											0,											
											WinPara.ySizeLCD-WinPara.ySizeBotWin,	//底部位置
											WinPara.xSizeLCD,
											WinPara.ySizeBotWin,	//底部剩余宽度
											WM_HBKWIN, 
											WM_CF_SHOW,
											_cbBotWin, 
											sizeof(WIN_PARA *)
											);
	/*在指定位置创建指定尺寸的ICONVIEW 小工具*/
	BOTWIN=ICONVIEW_CreateEx(
							 ICONVIEW_XPos, 					/* 小工具的最左像素（在父坐标中）*/
							 0, 								/* 小工具的最上像素（在父坐标中）*/
							 WinPara.xSizeLCD-ICONVIEW_XPos,	/* 小工具的水平尺寸（单位：像素）*/
							 WinPara.ySizeBotWin,				/* 小工具的垂直尺寸（单位：像素）*/
							 WinPara.hWinBot, 				    /* 父窗口的句柄。如果为0 ，则新小工具将成为桌面（顶级窗口）的子窗口 */
							 WM_CF_SHOW | WM_CF_HASTRANS,       /* 窗口创建标记。为使小工具立即可见，通常使用 WM_CF_SHOW */ 
							 0,//ICONVIEW_CF_AUTOSCROLLBAR_V, 	/* 默认是0，如果不够现实可设置增减垂直滚动条 */
							 GUI_ID_ICONVIEW0, 			        /* 小工具的窗口ID */
							 ICONVIEW_Width, 				    /* 图标的水平尺寸 */
							 ICONVIEW_Height);					/* 图标的垂直尺寸 */
	WM_BringToTop(WinPara.hWinBot);
	/* 向ICONVIEW 小工具添加新图标 */
	for (i = 0; i < GUI_COUNTOF(_aBitmapItem0); i++) 
	{
		Load_ICON(i, _aBitmapItem0[i].pBitmap, 0);
		ICONVIEW_AddBitmapItem(BOTWIN, _aBitmapItem0[i].pBitmap,"");
	}	
	/* 设置小工具的背景色 32 位颜色值的前8 位可用于alpha混合处理效果*/
	ICONVIEW_SetBkColor(BOTWIN, ICONVIEW_CI_SEL, GUI_WHITE | ICON_BG_ALPHA);
	/* 设置字体 */
//	ICONVIEW_SetFont(hWinicon0, &GUI_Font10_ASCII);
	/* 设置初始选择的图标为 -1 (表示尚未选择)*/
	ICONVIEW_SetSel(BOTWIN,-1);
	/* 设置图标在x 或y 方向上的间距。*/
	ICONVIEW_SetSpace(BOTWIN, GUI_COORD_X, ICONVIEW_XSpace);
//	ICONVIEW_SetSpace(hWin, GUI_COORD_Y, ICONVIEW_YSpace);
	/* 设置对齐方式 在5.22版本中最新加入的 */
	ICONVIEW_SetIconAlign(BOTWIN, ICONVIEW_IA_HCENTER|ICONVIEW_IA_TOP);
}

extern u8 nesruning;//nes_main.c定义
extern u8 videoruning;//videoplayer.c
#define SCREEN_TIME 10000
static void _cbScreenWin(WM_MESSAGE *pMsg);
/*
*********************************************************************************************************
*	函 数 名: MainAPP
*	功能说明: GUI主函数 
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void MainAPP(void)
{
//    GUI_HWIN activeWin;
    int show_screen = 0;
    int no_touch_count = 0;
    GUI_PID_STATE pState;
    WM_HWIN screenWin;
    WM_HTIMER screenTimer;
    
    app_prevent_refresh = 0;
	//准备建立3个窗口，以下是使用到的用户定义参数，方便在回调函数中使用
	WinPara.xSizeLCD = LCD_GetXSize();				//LCD屏幕尺寸
	WinPara.ySizeLCD = LCD_GetYSize();				//LCD屏幕尺寸
	WinPara.xSizeBM  = ICONVIEW_Width;				//图标宽度
	WinPara.ySizeBM  = ICONVIEW_Height;				//图标高度
	WinPara.ySizeBotWin = BOTWIN_YSpace;			//界面下方窗口高度
	WinPara.xSizeWin = WinPara.xSizeLCD * VIRTUAL_WIN_NUM;						//窗口尺寸
	WinPara.ySizeWin = WinPara.ySizeLCD - WinPara.ySizeBotWin - WinPara.yPosWin;	//窗口尺寸
	WinPara.xPosWin	 = MIDWIN_xPos;							//窗口的起始位置
	WinPara.yPosWin  = MIDWIN_yPos;							//窗口的起始位置
    
	WM_SetSize(WM_HBKWIN, WinPara.xSizeLCD, WinPara.ySizeLCD);//设置背景窗口大小，保证中间窗口移动效果
	/***************************设置默认皮肤色*****************************/
//	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);
//	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
//	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
//	CHECKBOX_SetDefaultSkin(CHECKBOX_SKIN_FLEX);
//	DROPDOWN_SetDefaultSkin(DROPDOWN_SKIN_FLEX);
//	SCROLLBAR_SetDefaultSkin(SCROLLBAR_SKIN_FLEX);
//	SLIDER_SetDefaultSkin(SLIDER_SKIN_FLEX);
//	HEADER_SetDefaultSkin(HEADER_SKIN_FLEX);
//	RADIO_SetDefaultSkin(RADIO_SKIN_FLEX);
//	MULTIPAGE_SetDefaultSkin(MULTIPAGE_SKIN_FLEX);
//	MENU_SetDefaultSkin(MENU_SKIN_FLEX);
//	SPINBOX_SetDefaultSkin(SPINBOX_SKIN_FLEX);
	/***************************设置自定义皮肤**********************************/
    setAppCustomSkin(0);
    /***************************设置默认字体**********************************/
	GUI_SetDefaultFont(&GUI_FontHZ16);
	/*************************************************************************/
	WM_Activate();//5.32必须调用才会显示界面，试了一天了。。。
	WM_MOTION_Enable(1);
	WM_MOTION_SetDefaultPeriod(250);
	WM_SetCallback(WM_HBKWIN, _cbBkWindow);
	WM_CreateTimer(WM_HBKWIN, 0, 1000, 0);
	/* 创建三个窗口 状态栏、控制栏、主窗口*/
	CreateMidWin();
	CreateTopWin();
	CreateBotWin();
    
	while(1) {
		GUI_Delay(30);
        
//        if (TPAD_Scan(0)) {
//            if (WinPara.hWinBot != activeWin && WinPara.hWinMid != activeWin) {
//                GUI_EndDialog(activeWin, 0);
//            }
//        }
        
        // 类似锁屏
        if (GUI_TOUCH_GetState(&pState)==0) {// 没点屏幕
            no_touch_count++;
        } else {
            no_touch_count = 0;
        }
        if (no_touch_count>(1800*3) && show_screen==0 && nesruning==0 && videoruning==0) {// 长时间没点屏幕并且还没显示
            show_screen = 1;
            screenWin = WM_CreateWindow(0,0,lcddev.width,lcddev.height,WM_CF_SHOW|WM_CF_STAYONTOP,_cbScreenWin,0);
            WM_SelectWindow(screenWin);
            screenTimer = WM_CreateTimer(screenWin, 0, 1000, 0);
            TIM_SetCompare2(TIM8, 3);
        } else if (show_screen==1 && GUI_TOUCH_GetState(&pState)==1) {// 正在显示锁屏并点了下屏幕
            show_screen = 0;
            WM_DeleteTimer(screenTimer);
            WM_SetCallback(screenWin, NULL);
            WM_DeleteWindow(screenWin);
            screenWin = 0;
            TIM_SetCompare2(TIM8, 15);
            GUI_Delay(300);
        }
	}
}

static void _cbScreenWin(WM_MESSAGE *pMsg)
{
	int NCode, Id;
    char text_buffer[20]={0};
    u8 r, g, b;
    const u8 count=30;
    int xa[count], ya[count], ra[count], i;
	switch (pMsg->MsgId) 
	{
        case WM_INIT_DIALOG:
            GUI_SetFont(&GUI_FontHZ12);
            break;
        case WM_TIMER:
//            r = rand()%256;
//            g = rand()%256;
//            b = rand()%256;
//            GUI_SetBkColor((r<<16)|(g<<8)|b);
            GUI_SetBkColor(GUI_BLACK);
			GUI_Clear();
        
            for (i=0;i<count;i++) {
                do {
                    xa[i] = rand()%lcddev.width;
                } while(xa[i]<=0 && xa[i]>=lcddev.width);
            }
            for (i=0;i<count;i++) {
                do {
                    ya[i] = rand()%lcddev.height;
                } while(ya[i]<=0 && ya[i]>=lcddev.height);
            }
            for (i=0;i<count;i++) {
                do {
                    ra[i] = rand()%15;
                } while(ra[i]<=0 && ra[i]>=15);
            }
            for (i=0;i<count;i++) {
                r = rand()%256;
                g = rand()%256;
                b = rand()%256;
                GUI_SetColor((r<<16)|(g<<8)|b);
                GUI_FillCircle(xa[i], ya[i], ra[i]);
            }
        
            GUI_SetFont(&GUI_FontHZ12);
            GUI_SetTextMode(GUI_TM_XOR);
            GUI_DispStringHCenterAt("点击屏幕退出", lcddev.width/2, lcddev.height-17);
            GUI_SetFont(&GUI_Font16_ASCII);
            sprintf(text_buffer,"%d/%02d/%02d %02d:%02d",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min);
            GUI_DispStringHCenterAt(text_buffer, lcddev.width/2, lcddev.height-40);
            GUI_SetTextMode(GUI_TM_NORMAL);
            WM_RestartTimer(pMsg->Data.v, SCREEN_TIME);
            break;
		case WM_PAINT:	                             //重绘背景	
			GUI_SetBkColor(GUI_BLACK);
			GUI_Clear();
            GUI_SetColor(GUI_WHITE);
            GUI_SetFont(&GUI_FontHZ12);
            GUI_SetTextMode(GUI_TM_XOR);
            GUI_DispStringHCenterAt("点击屏幕退出", lcddev.width/2, lcddev.height-17);
            GUI_SetFont(&GUI_Font16_ASCII);
            sprintf(text_buffer,"%d/%02d/%02d %02d:%02d",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min);
            GUI_DispStringHCenterAt(text_buffer, lcddev.width/2, lcddev.height-40);
            GUI_SetTextMode(GUI_TM_NORMAL);
            break;
        default:		
            WM_DefaultProc(pMsg);
            break;		 
	}
}
