#include "includes.h"
#include "DIALOG.h"
#include "EmWinHZFont.h"//teng
#include "rtc.h"
#include "gui.h"
#include "MessageBoxUser.h"

extern _calendar_obj calendar;
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_FRAMEWIN_0        (GUI_ID_USER + 0x00)
#define ID_TEXT_5        (GUI_ID_USER + 0x01)
#define ID_TEXT_6        (GUI_ID_USER + 0x02)

// USER START (Optionally insert additional defines)
WM_HTIMER hTimer2;


// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

void drawTimeClock2(WM_MESSAGE * pMsg, int fromTimer) {
    const u8 txt[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    int index;
    int rx=120;// X坐标 120
    int ry=90;// Y坐标 90
    int r=70;// 半径 70
    float PI = 3.1416f;
    // 表针
    float ta, txend, tyend;
    char ac[4];
    
//    if (fromTimer) {
//        WM_SelectWindow(pMsg->hWin);
//    }
    
    GUI_AA_SetFactor(3);
    // 加个画小点吧
    GUI_SetPenSize(2);
    GUI_SetColor(0x555555);
    for (index=0;index<60;index++) {
        float a = 360/60*index;
        float tx = (r-3)*cos(a*PI/180);
        float ty = (r-3)*sin(a*PI/180);
//        GUI_DrawPoint(rx+tx, ry+ty);
        GUI_AA_FillCircle(rx+tx, ry+ty, 1);
    }
    // 画刻度
    GUI_SetColor(0x000000);
    for (index=0;index<12;index++) {
        float a = 360/12*index;
        float tox = r*cos(a*PI/180);
        float toy = r*sin(a*PI/180);
        int tr = (index % 3 == 0) ? (r-5) : (r-4);
        float tix = tr*cos(a*PI/180);
        float tiy = tr*sin(a*PI/180);
        (index % 3 == 0) ? GUI_SetPenSize(4) : GUI_SetPenSize(3);
        GUI_AA_DrawLine(rx+tox, ry+toy, rx+tix, ry+tiy);
        if (index%(12/(sizeof(txt)/sizeof(txt[0])))==0) {
            sprintf(ac, "%d", txt[index/1]);//3
            GUI_SetTextMode(GUI_TM_TRANS);
            GUI_SetTextAlign(GUI_TA_VCENTER|GUI_TA_HCENTER);
            if (index==2 || index==5 || index==8 || index==11) {
                GUI_SetFont(&GUI_Font16B_ASCII);
            } else {
                GUI_SetFont(&GUI_Font16_ASCII);
            }
            tix = (r-13)*cos((a-90+30)*PI/180);
            tiy = (r-13)*sin((a-90+30)*PI/180);
            GUI_DispStringHCenterAt(ac, rx+tix, ry+tiy);
        }
    }
    // 画外圈
    GUI_SetColor(0x000000);
    GUI_SetPenSize(4);
    GUI_AA_DrawArc(rx, ry, r, r, 0, 360);
    // 画中心点
    GUI_SetColor(0x0000FF);
    GUI_AA_FillCircle(rx, ry, 4);
    GUI_SetPenSize(3);
    // 显示日期
    sprintf(ac, "%02d/%02d", calendar.w_month, calendar.w_date);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetTextAlign(GUI_TA_VCENTER|GUI_TA_HCENTER);
    GUI_SetFont(&GUI_Font13H_ASCII);
    GUI_SetColor(0x00AA00);
    GUI_DispStringHCenterAt(ac, rx, ry+r/2);
    // 画时针
    GUI_SetColor(0xFF00FF);
    GUI_SetPenSize(5);
    ta = 360/12*calendar.hour;
    ta += (360/12.0f*(calendar.min/60.0f));// 加上分钟走过的度数
    ta -= 90;
    txend = (r*0.42f)*cos(ta*PI/180);
    tyend = (r*0.42f)*sin(ta*PI/180);
    GUI_AA_DrawLine(rx, ry, rx+txend, ry+tyend);
    // 画分针
    GUI_SetColor(0x808000);
    GUI_SetPenSize(4);
    ta = 360/60*calendar.min;
    ta -= 90;
    txend = (r*0.6f)*cos(ta*PI/180);
    tyend = (r*0.6f)*sin(ta*PI/180);
    GUI_AA_DrawLine(rx, ry, rx+txend, ry+tyend);
    // 画秒针
    GUI_SetColor(0xFF0000);
    GUI_SetPenSize(3);
    ta = 360/60*calendar.sec;
    ta -= 90;
    txend = (r*0.75f)*cos(ta*PI/180);
    tyend = (r*0.75f)*sin(ta*PI/180);
    GUI_AA_DrawLine(rx, ry, rx+txend, ry+tyend);
    // 画秒针的尾
//    ta = 360/60*calendar.sec;
//    ta = ta - 90 + 180;
//    txend = (r*0.10f)*cos(ta*PI/180);
//    tyend = (r*0.10f)*sin(ta*PI/180);
//    GUI_AA_DrawLine(rx, ry, rx+txend, ry+tyend);
    
    GUI_AA_SetFactor(0);
//    printf("drawTimeClock\r\n");
}

// USER START (Optionally insert additional static data)
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "时间与日期", ID_FRAMEWIN_0, 0, 0, 240, 320, 0, 0x0, 0 },
  // USER START (Optionally insert additional widgets)
  { TEXT_CreateIndirect, "00:00:00", ID_TEXT_5, 60, 180, 120, 25, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "0000-00-00", ID_TEXT_6, 30, 215, 180, 25, 0, 0x0, 0 },
  // USER END
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

// USER START (Optionally insert additional static code)
// USER END

/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
  int NCode;
  int Id;
  // USER START (Optionally insert additional variables)
  WM_HWIN hItem;
  GUI_RECT Rect;
//  CALENDAR_DATE Date;
  // USER END
//  int i;
  char text_buffer[20]={0};
//  const u8 mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
//  int year, month, day, hour, min, sec;
  GUI_RECT clockRect = {50, 20, 190, 160};
  
  switch (pMsg->MsgId) {
      case WM_INIT_DIALOG:
        hItem = pMsg->hWin;
//        FRAMEWIN_SetTextColor(hItem,GUI_DARKGRAY);
        FRAMEWIN_SetTitleHeight(hItem, 20);
        FRAMEWIN_SetFont(hItem, &GUI_FontHZ16);
        FRAMEWIN_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
        FRAMEWIN_AddCloseButton(hItem,FRAMEWIN_BUTTON_RIGHT,0);
              
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_5);
        TEXT_SetFont(hItem, &GUI_Font32B_ASCII);
        TEXT_SetTextColor(hItem, 0x994444);
        TEXT_SetTextAlign(hItem, GUI_TA_VCENTER|GUI_TA_HCENTER);
        sprintf(text_buffer,"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);  //teng
        TEXT_SetText(hItem, text_buffer);
      
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_6);
        TEXT_SetFont(hItem, &GUI_Font24B_ASCII);
        TEXT_SetTextColor(hItem, 0x994444);
        TEXT_SetTextAlign(hItem, GUI_TA_VCENTER|GUI_TA_HCENTER);
        sprintf(text_buffer,"%02d-%02d-%02d",calendar.w_year,calendar.w_month,calendar.w_date);  //teng
        TEXT_SetText(hItem, text_buffer);
        
        WM_GetWindowRectEx(hItem, &Rect);
        WM_SetHasTrans(hItem);
        
      break;
      case WM_DELETE:
          WM_DeleteTimer(hTimer2);
          hTimer2=0;
      break;
      case WM_PAINT:
          GUI_DrawGradientV(0, 0, 240, 300, 0x00FFFF, 0x0099FF);
//          printf("wm_paint\r\n");
          drawTimeClock2(pMsg, 0);// 画表盘
          if (hTimer2==0) {
//              printf("createTimer()\r\n");
              hTimer2 = WM_CreateTimer(pMsg->hWin, 1, 900, 0);
          }
//          WM_RestartTimer(pMsg->Data.v, 1000);
      break;
      case WM_TIMER:
//          printf("wm_timer\r\n");
//          WM_InvalidateArea(&clockRect);// 5.26试了一天，目前只找到这个方法才能刷新
//          WM_InvalidateWindow(WM_GetActiveWindow());
          WM_InvalidateRect(pMsg->hWin, &clockRect);// 5.32
//          WM_SendMessageNoPara(WM_GetActiveWindow(), WM_PAINT);
//          drawTimeClock(pMsg, 1);// 画表盘
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_5);
          sprintf(text_buffer,"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);
          TEXT_SetText(hItem, text_buffer);
      
          WM_RestartTimer(pMsg->Data.v, 900);
      break;
      case WM_NOTIFY_PARENT:
        Id    = WM_GetId(pMsg->hWinSrc);
        NCode = pMsg->Data.v;
        switch(Id) {
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
WM_HWIN CreateDateTime(void);
WM_HWIN CreateDateTime(void) {
    WM_HWIN hWin;
    hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
    return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
