// USER START (Optionally insert additional includes)
#include "MusicDLG.h"
#include "EmWinHZFont.h"//teng
#include "includes.h" //teng
#include "ff.h"//teng
#include "malloc.h"//teng
#include "exfuns.h"//teng
#include "vs10xx.h"	
#include "audiosel.h"
#include "mp3player.h"
// USER END

#include "DIALOG.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_FRAMEWIN_0        (GUI_ID_USER + 0x00)
#define ID_BUTTON_0        (GUI_ID_USER + 0x01)
#define ID_BUTTON_1        (GUI_ID_USER + 0x02)
#define ID_BUTTON_2        (GUI_ID_USER + 0x03)
#define ID_BUTTON_3        (GUI_ID_USER + 0x04)
#define ID_BUTTON_4        (GUI_ID_USER + 0x05)
#define ID_TEXT_0        (GUI_ID_USER + 0x06)
#define ID_PROGBAR_0        (GUI_ID_USER + 0x07)
#define ID_SLIDER_0        (GUI_ID_USER + 0x08)
#define ID_IMAGE_0        (GUI_ID_USER + 0x09)
#define ID_TEXT_1       (GUI_ID_USER + 0x0A)
#define ID_TEXT_2       (GUI_ID_USER + 0x0B)
#define ID_TEXT_3       (GUI_ID_USER + 0x0C)
#define ID_TEXT_4       (GUI_ID_USER + 0x0D)
#define ID_TEXT_5       (GUI_ID_USER + 0x0E)
#define ID_TEXT_6       (GUI_ID_USER + 0x0F)
#define ID_TEXT_7       (GUI_ID_USER + 0x10)
#define ID_TEXT_8       (GUI_ID_USER + 0x11)

#define ID_IMAGE_0_IMAGE_0       0x00

// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
/*********************************************************************
*
*       _acImage_0, "BMP", ID_IMAGE_0_IMAGE_0
*/
static const U8 _acImage_0[463] = {
  0x42, 0x4D, 0xCE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x12, 0x0B, 0x00, 0x00,
  0x12, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21,
  0x40, 0x00, 0x90, 0x84, 0x21, 0x08, 0x42, 0x10, 0xC0, 0x00, 0x88, 0x42, 0x10, 0x84, 0x21, 0x08, 0x40, 0x00, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21, 0x40, 0x00,
  0x90, 0x84, 0x21, 0x08, 0x42, 0x10, 0xC0, 0x00, 0x88, 0x42, 0x10, 0x84, 0x21, 0x08, 0x40, 0x00, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21, 0x40, 0x00, 0x90, 0x84,
  0x21, 0x08, 0x42, 0x10, 0xC0, 0x00, 0x88, 0x42, 0x10, 0x84, 0x21, 0x08, 0x40, 0x00, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21, 0x40, 0x00, 0x90, 0x84, 0x21, 0x08,
  0x42, 0x10, 0xC0, 0x00, 0x88, 0x42, 0x10, 0x84, 0x21, 0x08, 0x40, 0x00, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21, 0x40, 0x00, 0x90, 0x84, 0x21, 0x08, 0x42, 0x10,
  0xC0, 0x00, 0x88, 0x42, 0x10, 0x84, 0x21, 0x08, 0x40, 0x00, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21, 0x40, 0x00, 0x90, 0x84, 0x21, 0x08, 0x42, 0x10, 0xC0, 0x00,
  0x88, 0x42, 0x10, 0x84, 0x21, 0x08, 0x40, 0x00, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21, 0x40, 0x00, 0x90, 0x84, 0x21, 0x08, 0x42, 0x10, 0xC0, 0x00, 0x88, 0x42,
  0x10, 0x84, 0x21, 0x08, 0x40, 0x00, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21, 0x40, 0x00, 0x90, 0x84, 0x21, 0x08, 0x42, 0x10, 0xC0, 0x00, 0x88, 0x42, 0x10, 0x84,
  0x21, 0x08, 0x40, 0x00, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21, 0x40, 0x00, 0x90, 0x84, 0x21, 0x08, 0x42, 0x10, 0xC0, 0x00, 0x88, 0x42, 0x10, 0x84, 0x21, 0x08,
  0x40, 0x00, 0x84, 0x21, 0x08, 0x42, 0x10, 0x84, 0x40, 0x00, 0xC2, 0x10, 0x84, 0x21, 0x08, 0x42, 0x40, 0x00, 0xA1, 0x08, 0x42, 0x10, 0x84, 0x21, 0x40, 0x00, 0x90, 0x84, 0x21, 0x08, 0x42, 0x10, 0xC0, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0x00,
  0x00,
};

// USER START (Optionally insert additional static data)
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "音乐播放器", ID_FRAMEWIN_0, 0, 0, 240, 320, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "列表", ID_BUTTON_0, 0, 247, 46, 46, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "上一曲", ID_BUTTON_1, 46, 247, 46, 46, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "播放", ID_BUTTON_2, 92, 247, 46, 46, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "下一曲", ID_BUTTON_3, 138, 247, 46, 46, 0, 0x0, 0 },
  { BUTTON_CreateIndirect, "后台", ID_BUTTON_4, 184, 247, 46, 46, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "暂无选择音乐", ID_TEXT_0, 2, 2, 230-4, 20, 0, 0x0, 0 },
  { PROGBAR_CreateIndirect, "Progbar", ID_PROGBAR_0, 5, 80, 220, 10, 0, 0x0, 0 },
  { SLIDER_CreateIndirect, "Slider", ID_SLIDER_0, 123, 110, 100, 20, 0, 0x0, 0 },
  { IMAGE_CreateIndirect, "Image", ID_IMAGE_0, 5, 110, 65, 65, 0, 0, 0 },
  { TEXT_CreateIndirect, "上一句歌词", ID_TEXT_1, 5, 180, 220, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "当前唱的歌词", ID_TEXT_2, 5, 200, 220, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "下一句歌词", ID_TEXT_3, 5, 220, 220, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "60%", ID_TEXT_4, 123, 130, 80, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "000/000", ID_TEXT_5, 123, 150, 80, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "00:00/00:00", ID_TEXT_6, 5, 90, 80, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "000kbps", ID_TEXT_7, 160, 90, 65, 20, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "Vol", ID_TEXT_8, 105, 115, 20, 20, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

MUSIC_LIST *music = 0;
int musicCounts = 0;
int musicIndex = 1;
char musicPlaying = 0;
char musicChange = 0;

char musicPlayToTask = 0;

extern u16 f_kbps;//歌曲文件位率	
extern u16 time;// 时间变量
extern u16 totalTime;//总长
/*********************************************************************
*
*       _GetImageById
*/
static const void * _GetImageById(U32 Id, U32 * pSize) {
  switch (Id) {
  case ID_IMAGE_0_IMAGE_0:
    *pSize = sizeof(_acImage_0);
    return (const void *)_acImage_0;
  }
  return NULL;
}

// USER START (Optionally insert additional static code)

#define _DF1S	0x81
#define BROWSER_FILE_NAME_LEN 	100	
static FRESULT scan_files(char *path, u8 ftype) 
{ 
    FRESULT res; 		          //部分在递归过程被修改的变量，不用全局变量	
    FILINFO fno; 
	unsigned int rw_num;			//已读或已写的字节数
    DIR dir; 
    int i; 
    char *fn;
    
    char *pfile_name;
    char *pfnt;
    MUSIC_LIST *musictemp;
    MUSIC_LIST *musicindex;

#if _USE_LFN 
    static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1]; 	//长文件名支持
    fno.lfname = lfn; 
    fno.lfsize = sizeof(lfn); 
#endif 
    res = f_opendir(&dir, path);                            //打开目录
    if (res == FR_OK) { 
        i = strlen(path); 
        for (;;) { 
            res = f_readdir(&dir, &fno); 										//读取目录下的内容
            if (res != FR_OK || fno.fname[0] == 0) break; 	//为空时表示所有项目读取完毕，跳出
#if _USE_LFN 
            fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
            fn = fno.fname; 
#endif 
            if (*fn == '.') continue; 											//点表示当前目录，跳过			
            if (fno.fattrib & AM_DIR) { 										//目录，递归读取
				sprintf(&path[i], "/%s", fn); 							//合成完整目录名
                res = scan_files(path, ftype);		//递归遍历 
                if (res != FR_OK) 
					break; 																		//打开失败，跳出循环
                path[i] = 0; 
            } else {																				//是文件
				if( (f_typetell((u8 *)fn)>>4) == (ftype>>4) ) {                                        //要查找文件类型的大类
                    if (strlen(path)+strlen(fn)<BROWSER_FILE_NAME_LEN) {
                        pfile_name = mymalloc(SRAMEX, BROWSER_FILE_NAME_LEN);
                        sprintf(pfile_name, "%s/%s", path, fn); 	
                    }
                    //保存到列表
                    pfnt = mymalloc(SRAMEX, BROWSER_FILE_NAME_LEN/2);
                    strcpy(pfnt, fn);
                    if (music == 0) {
                        musicCounts = 0;
                        musicIndex = 1;
                        music = mymalloc(SRAMEX, sizeof(MUSIC_LIST));
                        music->music_name = (char *)pfnt;
                        music->music_path = (char *)pfile_name;
                        music->nextMusic = 0;
                        music->preMusic = 0;
//                        musicindex = mymalloc(SRAMEX, sizeof(MUSIC_LIST));
//                        musicindex = music;
                        musicCounts++;
                    } else {
                        musictemp = mymalloc(SRAMEX, sizeof(MUSIC_LIST));
                        musictemp->music_name = (char *)pfnt;
                        musictemp->music_path = (char *)pfile_name;
                        musictemp->nextMusic = music;//反一下
                        musictemp->preMusic = 0;
                        music->preMusic = musictemp;
                        music = musictemp;
                        musicCounts++;
                    }
				}
			}
        }
    }    
//    if (musicindex) {
//        myfree(SRAMEX, musicindex);
//        musicindex = 0;
//    }
    return res; 
}

void freeMusicRam() {
    MUSIC_LIST *temp;
    if (music==0) {
        return;
    }
    myfree(SRAMEX, music->music_name);
    myfree(SRAMEX, music->music_path);
    music->music_name = 0;
    music->music_path = 0;
    temp = music->preMusic;
    while (temp) {
        myfree(SRAMEX, temp->music_name);
        myfree(SRAMEX, temp->music_path);
        temp->music_name = 0;
        temp->music_path = 0;
        temp = temp->preMusic;
        myfree(SRAMEX, temp->nextMusic);
    }
    myfree(SRAMEX, music);
    music = 0;
}

u8 hasNextMusic(WM_HWIN button) {
//    if (music && music->nextMusic) {
//        BUTTON_SetState(button, BUTTON_CI_DISABLED);
//    } else {
//        BUTTON_SetState(button, BUTTON_CI_UNPRESSED);
//    }
}

u8 hasPreMusic(WM_HWIN button) {
//    if (music && music->preMusic) {
//        BUTTON_SetState(button, BUTTON_CI_DISABLED);
//    } else {
//        BUTTON_SetState(button, BUTTON_CI_UNPRESSED);
//    }
}

u8 changePreOrNextButton(WM_HWIN pre, WM_HWIN next) {
    musicChange = 1;
    musicPlaying = 1;
    hasPreMusic(pre);
    hasNextMusic(next);
}

void playOrPauseMusic(WM_HWIN button) {
    if (musicPlaying) {
        musicPlaying = 0;
        BUTTON_SetText(button, "播放");
    } else {
        musicPlaying = 1;
        BUTTON_SetText(button, "停止");
    }
}

void closeMusicWindows() {
    if (musicPlayToTask) {
        musicPlaying = 1;
    } else {
        musicPlaying = 0;
        freeMusicRam();
    }
}
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
  const void * pData;
  WM_HWIN      hItem;
  U32          FileSize;
  int          NCode;
  int          Id;
  // USER START (Optionally insert additional variables)
  char p_path[BROWSER_FILE_NAME_LEN]={0};				//目录名 指针
  char buff[50]={0};
  // USER END

  switch (pMsg->MsgId) {
  case WM_DELETE:
      closeMusicWindows();
  break;
  case WM_TIMER:
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_6);
      sprintf(buff, "%02d:%02d/%02d:%02d", time/60, time%60, totalTime/60, totalTime%60);
      TEXT_SetText(hItem, buff);
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_7);
      sprintf(buff, "%03dkbps", f_kbps);
      TEXT_SetText(hItem, buff);
      hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_0);
      PROGBAR_SetMinMax(hItem, 0, totalTime);
      PROGBAR_SetValue(hItem, time);
  
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
      TEXT_SetText(hItem, (char *)music->music_name);
  
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_5);
      sprintf(buff, "%03d/%03d", musicIndex, musicCounts);
      TEXT_SetText(hItem, buff);
  
      hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_2);
      if (musicPlaying) {
          BUTTON_SetText(hItem, "停止");
      } else {
          BUTTON_SetText(hItem, "播放");
      }
  
      WM_RestartTimer(pMsg->Data.v, 1000);
  break;
  case WM_PAINT:
//      GUI_DrawGradientV(0, 0, 240, 300, 0x008888, 0x007788);
      GUI_SetColor(GUI_ORANGE);
      GUI_FillRect(0, 0, 240, 320);
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_5);
      if (musicCounts >= 1) {
        sprintf(buff, "%03d/%03d", musicIndex, musicCounts);
      } else {
        sprintf(buff, "%03d/%03d", 0, 0);
      }
      TEXT_SetText(hItem, buff);
  break;
  case WM_INIT_DIALOG:
      if (musicPlayToTask==0) {
          strcpy(p_path,"0:");						//复制目录名到指针
          scan_files(p_path, T_MP1);
      }
      musicPlayToTask = 0;
    //
    // Initialization of 'Image'
    //
    hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_0);
    pData = _GetImageById(ID_IMAGE_0_IMAGE_0, &FileSize);
    IMAGE_SetBMP(hItem, pData, FileSize);
    // USER START (Optionally insert additional code for further widget initialization)
    hItem = pMsg->hWin;
    FRAMEWIN_SetTitleHeight(hItem, 20);
    FRAMEWIN_SetFont(hItem, &GUI_FontHZ16);
    FRAMEWIN_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
    FRAMEWIN_AddCloseButton(hItem,FRAMEWIN_BUTTON_RIGHT,0);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_0);
    BUTTON_SetFont(hItem, &GUI_FontHZ12);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1);//上一曲
    BUTTON_SetFont(hItem, &GUI_FontHZ12);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_2);
    BUTTON_SetFont(hItem, &GUI_FontHZ12);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_3);//下一曲
    BUTTON_SetFont(hItem, &GUI_FontHZ12);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_4);
    BUTTON_SetFont(hItem, &GUI_FontHZ12);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
    TEXT_SetFont(hItem, &GUI_FontHZ12);
    TEXT_SetTextAlign(hItem, GUI_TA_HCENTER);
    if (music) {
        TEXT_SetText(hItem, (char *)music->music_name);
    }
    
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
    TEXT_SetFont(hItem, &GUI_FontHZ12);
    TEXT_SetTextAlign(hItem, GUI_TA_VCENTER|GUI_TA_HCENTER);
    TEXT_SetTextColor(hItem, GUI_BLUE);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
    TEXT_SetFont(hItem, &GUI_FontHZ16);
    TEXT_SetTextAlign(hItem, GUI_TA_VCENTER|GUI_TA_HCENTER);
    TEXT_SetTextColor(hItem, GUI_DARKGREEN);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_3);
    TEXT_SetFont(hItem, &GUI_FontHZ12);
    TEXT_SetTextAlign(hItem, GUI_TA_VCENTER|GUI_TA_HCENTER);
    TEXT_SetTextColor(hItem, GUI_BLUE);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_4);
    TEXT_SetTextAlign(hItem, GUI_TA_VCENTER);
    sprintf(buff, "%02d%%", (int)((170-100.0f)/150.0f*100.0f));
    TEXT_SetText(hItem, buff);
    
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_5);
    TEXT_SetTextAlign(hItem, GUI_TA_VCENTER);
    hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_7);
    TEXT_SetTextAlign(hItem, GUI_TA_RIGHT);
  
    hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_0);
    PROGBAR_SetText(hItem, "");
    
    hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
    SLIDER_SetRange(hItem, 100, 250);
    SLIDER_SetValue(hItem, 190);
    
  
    // USER END
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);
    NCode = pMsg->Data.v;
    switch(Id) {
    case ID_BUTTON_0: // Notifications sent by 'Menu'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_BUTTON_1: // Notifications sent by 'Pre'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
        if (music && music->preMusic) {
            music = music->preMusic;
            TEXT_SetText(hItem, (char *)music->music_name);
            changePreOrNextButton(
                    WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1), 
                    WM_GetDialogItem(pMsg->hWin, ID_BUTTON_3));
            musicIndex--;
            sprintf(buff, "%03d/%03d", musicIndex, musicCounts);
            TEXT_SetText(WM_GetDialogItem(pMsg->hWin, ID_TEXT_5), buff);
        }
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_BUTTON_2: // Notifications sent by 'Pause'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        hItem = WM_GetDialogItem(pMsg->hWin, ID_BUTTON_2);
        playOrPauseMusic(hItem);
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_BUTTON_3: // Notifications sent by 'Next'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
        if (music && music->nextMusic) {
            music = music->nextMusic;
            TEXT_SetText(hItem, (char *)music->music_name);
            changePreOrNextButton(
                    WM_GetDialogItem(pMsg->hWin, ID_BUTTON_1), 
                    WM_GetDialogItem(pMsg->hWin, ID_BUTTON_3));
            musicIndex++;
            sprintf(buff, "%03d/%03d", musicIndex, musicCounts);
            TEXT_SetText(WM_GetDialogItem(pMsg->hWin, ID_TEXT_5), buff);
        }
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_BUTTON_4: // Notifications sent by 'Back'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        musicPlayToTask = 1;
        GUI_EndDialog(pMsg->hWin, 0);
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    case ID_SLIDER_0: // Notifications sent by 'Slider'
      switch(NCode) {
      case WM_NOTIFICATION_CLICKED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_RELEASED:
        // USER START (Optionally insert code for reacting on notification message)
        // USER END
        break;
      case WM_NOTIFICATION_VALUE_CHANGED:
        // USER START (Optionally insert code for reacting on notification message)
        hItem = WM_GetDialogItem(pMsg->hWin, ID_SLIDER_0);
        vsset.mvol=SLIDER_GetValue(hItem);	
        VS_Set_Vol(vsset.mvol);
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_4);
        sprintf(buff, "%02d%%", (int)((vsset.mvol-100.0f)/150.0f*100.0f));
        TEXT_SetText(hItem, buff);
        // USER END
        break;
      // USER START (Optionally insert additional code for further notification handling)
      // USER END
      }
      break;
    // USER START (Optionally insert additional code for further Ids)
    // USER END
    }
    break;
  // USER START (Optionally insert additional message handling)
  // USER END
  default:
    WM_DefaultProc(pMsg);
    break;
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CreateFramewin
*/
WM_HWIN CreateMusic(void);
WM_HWIN CreateMusic(void) {
  WM_HWIN hWin;
  WM_HTIMER hTimer;
    
  hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  hTimer = WM_CreateTimer(WM_GetClientWindow(hWin), 0, 1000, 0);
    
  return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
