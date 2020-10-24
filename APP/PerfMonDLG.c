
//#include "PerfMonDLG.h"//teng
#include <stdlib.h>
#include <string.h>

#include "DIALOG.h"
#include "GRAPH.h"

#include "EmWinHZFont.h"//teng

#include "includes.h"
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define MAX_VALUE 100
#define REFRESH_SETP 1000

char perfmonruning=0;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static GRAPH_DATA_Handle  _ahData[3]; /* Array of handles for the GRAPH_DATA objects */
static GRAPH_SCALE_Handle _hScaleV;   /* Handle of vertical scale */
static GRAPH_SCALE_Handle _hScaleH;   /* Handle of horizontal scale */

static I16 _aValue[3];
static int _Stop;

static GUI_COLOR _aColor[] = {GUI_RED, GUI_GREEN, GUI_BLUE}; /* Array of colors for the GRAPH_DATA objects */

/* Dialog ressource */
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "资源监视器",  0                ,   0,   50, 240, 215, 0},
};

static const GUI_WIDGET_CREATE_INFO _aDialogCreate2[] = {
  { WINDOW_CreateIndirect, "资源监视器",  0                ,   0,   60, 229, 195, 0},
  { GRAPH_CreateIndirect,     0,                   GUI_ID_GRAPH0    ,   0,   0, 229, 108 },
  { TEXT_CreateIndirect,      "SX",        0                ,  2, 110,  12,  16 },
  { TEXT_CreateIndirect,      "SY",        0                ,  2, 130,  12,  16 },
  { SLIDER_CreateIndirect,    0,                   GUI_ID_SLIDER0   ,  20, 110,  60,  16 },
  { SLIDER_CreateIndirect,    0,                   GUI_ID_SLIDER1   ,  20, 130,  60,  16 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK0    , 85, 110,  50,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK1    , 85, 130,  50,   0 },
  { TEXT_CreateIndirect,      "边框",              GUI_ID_TEXT0     , 2,   151,  37,  15 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK2    , 30,  150,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK3    , 62,  150,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK4    , 96,  150,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK5    , 128,  150,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK6    , 130, 110,  50,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK7    , 130, 130,  50,   0 },
  { BUTTON_CreateIndirect,    "放大",      		   GUI_ID_BUTTON0   , 185, 110,  43,  18 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK8    , 185, 130,  70,   0 },
  { TEXT_CreateIndirect,      "00:00:00",            GUI_ID_TEXT1   , 176, 149,  53,  15 },
};

static const GUI_WIDGET_CREATE_INFO _aDialogCreate3[] = {
  { WINDOW_CreateIndirect,    "Dialog 3", 0,                   0,   0, 229, 195, FRAMEWIN_CF_MOVEABLE },
  { LISTVIEW_CreateIndirect,     "任务详情",         GUI_ID_LISTVIEW0,      0,   0,   229,   168, 0, 3},
};
/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _AddValues
*
* Purpose:
*   This routine calculates new random values in dependence of the previous added values
*   and adds them to the GRAPH_DATA objects
*/
//extern OSStatTaskCPUUsage;
static void _AddValues(WM_HWIN hGraph) {
//	int i;
//	for (i = 0; i < GUI_COUNTOF(_aColor); i++) {
//		int Add = rand() % (2 + i * i);
//		int Vz  = ((rand() % 2) << 1) - 1;
//		_aValue[i] += Add * Vz;
//		if (_aValue[i] > MAX_VALUE) {
//			_aValue[i] = MAX_VALUE;
//		} else if (_aValue[i] < 0) {
//			_aValue[i] = 0;
//		}
//		GRAPH_DATA_YT_AddValue(_ahData[i], _aValue[i]);
//	}
	_aValue[0] = (I16)OSStatTaskCPUUsage/100;//CPU
	GRAPH_DATA_YT_AddValue(_ahData[0], _aValue[0]);
	_aValue[1] = (I16)my_mem_perused(0);//内部
	GRAPH_DATA_YT_AddValue(_ahData[1], _aValue[1]);
	_aValue[2] = (I16)my_mem_perused(1);//外部
	GRAPH_DATA_YT_AddValue(_ahData[2], _aValue[2]);
}

/*********************************************************************
*
*       _UserDraw
*
* Purpose:
*   This routine is called by the GRAPH object before anything is drawn
*   and after the last drawing operation.
*/
static void _UserDraw(WM_HWIN hWin, int Stage) {
  if (Stage == GRAPH_DRAW_LAST) {
	const char acCpu[] = "-CPU";
    const char acInram[] = "-RAM";
    const char acExram[] = "-SRAM";
    GUI_RECT Rect, RectInvalid;
//    int FontSizeY;
    GUI_SetFont(&GUI_Font13_ASCII);
//	GUI_SetFont(&GUI_FontHZ12);
//    FontSizeY = GUI_GetFontSizeY();
    WM_GetInsideRect(&Rect);
//    WM_GetInvalidRect(hWin, &RectInvalid);
//    Rect.x1 = Rect.x0 + FontSizeY;
//    GUI_SetColor(GUI_YELLOW);
//    GUI_DispStringInRectEx(acText, &Rect, GUI_TA_HCENTER, strlen(acText), GUI_ROTATE_CCW);
      
//    GUI_EnableAlpha(1);
    GUI_SetColor(GUI_RED|0x85000000);
    GUI_DispStringAt(acCpu, Rect.x1-6*4-1, Rect.y0+1);
    GUI_SetColor(GUI_GREEN|0x90000000);
    GUI_DispStringAt(acInram, Rect.x1-6*9-1, Rect.y0+1);
    GUI_SetColor(GUI_BLUE|0x80000000);
    GUI_DispStringAt(acExram, Rect.x1-6*15-1, Rect.y0+1);
//    GUI_EnableAlpha(0);
  }
}

/*********************************************************************
*
*       _ForEach
*
* Purpose:
*   This routine hides/shows all windows except the button, graph and scroll bar widgets
*/
static void _ForEach(WM_HWIN hWin, void * pData) {
  int Id, FullScreenMode;
  FullScreenMode = *(int *)pData;
  Id = WM_GetId(hWin);
  if ((Id == GUI_ID_GRAPH0) || (Id == GUI_ID_BUTTON0) || (Id == GUI_ID_VSCROLL) || (Id == GUI_ID_HSCROLL)) {
    return;
  }
  if (FullScreenMode) {
    WM_HideWindow(hWin);
  } else {
    WM_ShowWindow(hWin);
  }
}

/*********************************************************************
*
*       _ToggleFullScreenMode
*
* Purpose:
*   This routine switches between full screen mode and normal mode by hiding or showing the
*   widgets of the dialog, enlarging/shrinking the graph widget and modifying some other
*   attributes of the dialog widgets.
*/
static void _ToggleFullScreenMode(WM_HWIN hDlg) {
  static int FullScreenMode;
  static GUI_RECT Rect;
  static unsigned ScalePos;
  WM_HWIN hGraph, hButton;
  hGraph  = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
  hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
  BUTTON_SetFont(hButton,&GUI_FontHZ12);
  FullScreenMode ^= 1;
  if (FullScreenMode) {
    /* Enter the full screen mode */
    WM_HWIN hClient;
    GUI_RECT RectInside;
    hClient = WM_GetClientWindow(hDlg);
    BUTTON_SetText(hButton, "缩小");
    WM_MoveWindow(hButton, 0, 40);
//    FRAMEWIN_SetTitleVis(hDlg, 0);
    WM_GetInsideRectEx(hClient, &RectInside);
    WM_GetWindowRectEx(hGraph, &Rect);
    WM_ForEachDesc(hClient, _ForEach, &FullScreenMode); /* Hide all descendants */
    WM_SetWindowPos(hGraph, WM_GetWindowOrgX(hClient), WM_GetWindowOrgX(hClient)+60, RectInside.x1, RectInside.y1);
    ScalePos = GRAPH_SCALE_SetPos(_hScaleH, RectInside.y1 - 20);
  } else {
    /* Return to normal mode */
    BUTTON_SetText(hButton, "放大");
    WM_MoveWindow(hButton, 0, -40);
    WM_ForEachDesc(WM_GetClientWindow(hDlg), _ForEach, &FullScreenMode); /* Show all descendants */
    WM_SetWindowPos(hGraph, Rect.x0, Rect.y0, Rect.x1 - Rect.x0 + 1, Rect.y1 - Rect.y0 + 1);
//    FRAMEWIN_SetTitleVis(hDlg, 1);
    GRAPH_SCALE_SetPos(_hScaleH, ScalePos);
  }
}

/*********************************************************************
*
*       _cbCallback
*
* Purpose:
*   Callback function of the dialog
*/
#define MON_DATA_SIZE 226 // main 任务里也有一个就不处理了
extern I16 cpuusage[MON_DATA_SIZE];//CPU使用率
extern I16 inmemusage[MON_DATA_SIZE];//内部内存
extern I16 exmemusage[MON_DATA_SIZE];//扩展内存
extern U8 runhour,runmin,runsec;     //运行时间
static void _cbCallback2(WM_MESSAGE * pMsg) {
  WM_HWIN hGraph = 0;
  int i, NCode, Id, Value;
  WM_HWIN hDlg, hItem;
  char pbuff[10]={0};
  hDlg = pMsg->hWin;
  switch (pMsg->MsgId) {
      // 定时刷新数据
      case WM_TIMER:
        if (!_Stop) {
			if (!hGraph) {
				hGraph = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
			}
			_AddValues(hGraph);
		}
        // 运行时间
        hItem = WM_GetDialogItem(hDlg, GUI_ID_TEXT1);
        sprintf(pbuff,"%02d:%02d:%02d", runhour, runmin, runsec);
        TEXT_SetText(hItem, pbuff);
        
        WM_RestartTimer(pMsg->Data.v, REFRESH_SETP);
        break;

  case WM_INIT_DIALOG:
	GUI_SetDefaultFont(&GUI_FontHZ12);
	hItem = pMsg->hWin;
//	FRAMEWIN_SetMoveable(hItem, 1);
//	FRAMEWIN_SetTitleHeight(hItem, 20);
//	FRAMEWIN_SetFont(hItem,&GUI_FontHZ16);
//	FRAMEWIN_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
//	FRAMEWIN_AddCloseButton(hItem,FRAMEWIN_BUTTON_RIGHT,0);
//    FRAMEWIN_AddMinButton(hItem,FRAMEWIN_BUTTON_RIGHT,1);
  
    hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
    /* Add graphs */
    for (i = 0; i < GUI_COUNTOF(_aColor); i++) {
//      _aValue[i] = rand() % 180;
	  _aValue[i] = 0;
//      _ahData[i] = GRAPH_DATA_YT_Create(_aColor[i], 226, 0, 0);
//      GRAPH_AttachData(hItem, _ahData[i]);
        // 从缓存中取
        if (i == 0) {
          _ahData[i] = GRAPH_DATA_YT_Create(_aColor[i], 233, cpuusage, MON_DATA_SIZE);
        } else if (i == 1) {
            _ahData[i] = GRAPH_DATA_YT_Create(_aColor[i], 233, inmemusage, MON_DATA_SIZE);
        } else if (i == 2) {
            _ahData[i] = GRAPH_DATA_YT_Create(_aColor[i], 233, exmemusage, MON_DATA_SIZE);
        }
        GRAPH_AttachData(hItem, _ahData[i]);
    }
    /* Set graph attributes */
    GRAPH_SetGridDistY(hItem, 25); //网格间距
    GRAPH_SetGridVis(hItem, 0);  // 是否显示网络
    GRAPH_SetGridFixedX(hItem, 1);
	GRAPH_SetVSizeY(hItem,50);
//	GRAPH_SetLineStyle(hItem, GUI_LS_DOT);
	
    GRAPH_SetUserDraw(hItem, _UserDraw);
    /* Create and add vertical scale */
    _hScaleV = GRAPH_SCALE_Create(1, GUI_TA_LEFT, GRAPH_SCALE_CF_VERTICAL, 25);
    GRAPH_SCALE_SetTextColor(_hScaleV, GUI_LIGHTMAGENTA);
    GRAPH_AttachScale(hItem, _hScaleV);
    /* Create and add horizontal scale */
    _hScaleH = GRAPH_SCALE_Create(100, GUI_TA_HCENTER, GRAPH_SCALE_CF_HORIZONTAL, 30);
    GRAPH_SCALE_SetTextColor(_hScaleH, GUI_LIGHTMAGENTA);
    GRAPH_AttachScale(hItem, _hScaleH);
    
    hItem = WM_GetDialogItem(hDlg, GUI_ID_TEXT0);
    TEXT_SetFont(hItem, &GUI_FontHZ12);
    // 运行时间
    hItem = WM_GetDialogItem(hDlg, GUI_ID_TEXT1);
    TEXT_SetFont(hItem, &GUI_Font16_ASCII);
    sprintf(pbuff,"%02d:%02d:%02d", runhour, runmin, runsec);
    TEXT_SetText(hItem, pbuff);
    
    hItem = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
    BUTTON_SetFont(hItem, &GUI_FontHZ12);
    /* Init check boxes */
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK2);
    CHECKBOX_SetFont(hItem,&GUI_FontHZ12);
    CHECKBOX_SetText(hItem, "左");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK3);
    CHECKBOX_SetFont(hItem,&GUI_FontHZ12);
    CHECKBOX_SetText(hItem, "上");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK4);
    CHECKBOX_SetFont(hItem,&GUI_FontHZ12);
    CHECKBOX_SetText(hItem, "右");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK5);
    CHECKBOX_SetFont(hItem,&GUI_FontHZ12);
    CHECKBOX_SetText(hItem, "下");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK0);
    CHECKBOX_SetFont(hItem,&GUI_FontHZ12);
    CHECKBOX_SetText(hItem, "暂停");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK1);
    CHECKBOX_SetFont(hItem,&GUI_FontHZ12);
    CHECKBOX_SetText(hItem, "网格");
    CHECKBOX_SetState(hItem, 0);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK6);
    CHECKBOX_SetFont(hItem,&GUI_FontHZ12);
    CHECKBOX_SetText(hItem, "H滚动");
    CHECKBOX_SetNumStates(hItem, 3);
    CHECKBOX_SetState(hItem, 2);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK7);
    CHECKBOX_SetFont(hItem,&GUI_FontHZ12);
    CHECKBOX_SetText(hItem, "V滚动");
    CHECKBOX_SetNumStates(hItem, 3);
    CHECKBOX_SetState(hItem, 2);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK8);
    CHECKBOX_SetFont(hItem,&GUI_FontHZ12);
    CHECKBOX_SetText(hItem, "镜像");
    /* Init slider widgets */
    hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER0);
    SLIDER_SetRange(hItem, 0, 10);
    SLIDER_SetValue(hItem, 3);
    SLIDER_SetNumTicks(hItem, 6);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER1);
    SLIDER_SetRange(hItem, 0, 20);
    SLIDER_SetValue(hItem, 5);
    SLIDER_SetNumTicks(hItem, 6);
    /* Init radio widget */
//    hItem = WM_GetDialogItem(hDlg, GUI_ID_RADIO0);
//    RADIO_SetText(hItem, "3D", 0);
//    RADIO_SetText(hItem, "flat", 1);
//    RADIO_SetText(hItem, "-", 2);
    /* Init button widget */
    hItem = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
    WM_SetStayOnTop(hItem, 1);
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      /* Id of widget */
    NCode = pMsg->Data.v;                 /* Notification code */
    switch (NCode) {
    case WM_NOTIFICATION_CLICKED:
      switch (Id) {
      case GUI_ID_BUTTON0:
        _ToggleFullScreenMode(hDlg);
        break;
      }
      break;
    case WM_NOTIFICATION_VALUE_CHANGED:
      switch (Id) {
      case GUI_ID_CHECK0:
        /* Toggle stop mode */
        _Stop ^= 1;
        break;
      case GUI_ID_CHECK1:
        /* Toggle grid */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        GRAPH_SetGridVis(hItem, CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK1)));
        break;
      case GUI_ID_CHECK2:
      case GUI_ID_CHECK3:
      case GUI_ID_CHECK4:
      case GUI_ID_CHECK5:
        /* Toggle border */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        GRAPH_SetBorder(hItem, 
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK2)) * 17, 
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK3)) * 3, 
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK4)) * 3, 
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK5)) * 3);
        break;
      case GUI_ID_SLIDER0:
        /* Set horizontal grid spacing */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        Value = SLIDER_GetValue(pMsg->hWinSrc) * 10;
        GRAPH_SetGridDistX(hItem, Value);
        GRAPH_SCALE_SetTickDist(_hScaleH, Value);
        break;
      case GUI_ID_SLIDER1:
        /* Set vertical grid spacing */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        Value = SLIDER_GetValue(pMsg->hWinSrc) * 5;
        GRAPH_SetGridDistY(hItem, Value);
        GRAPH_SCALE_SetTickDist(_hScaleV, Value);
        break;
//      case GUI_ID_RADIO0:
//        /* Set the widget effect */
//        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
//        switch (RADIO_GetValue(pMsg->hWinSrc)) {
//        case 0:
//          WIDGET_SetEffect(hItem, &WIDGET_Effect_3D);
//          break;
//        case 1:
//          WIDGET_SetEffect(hItem, &WIDGET_Effect_Simple);
//          break;
//        case 2:
//          WIDGET_SetEffect(hItem, &WIDGET_Effect_None);
//          break;
//        }
//        break;
      case GUI_ID_CHECK6:
        /* Toggle horizontal scroll bar */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK6))) {
          GRAPH_SetVSizeX(hItem, 231);
        } else {
          GRAPH_SetVSizeX(hItem, 0);
        }
        CHECKBOX_SetState(WM_GetDialogItem(hDlg, GUI_ID_CHECK6), 2);
        break;
      case GUI_ID_CHECK7:
        /* Toggle vertical scroll bar */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK7))) {
          GRAPH_SetVSizeY(hItem, 100);
        } else {
          GRAPH_SetVSizeY(hItem, 0);
        }
        CHECKBOX_SetState(WM_GetDialogItem(hDlg, GUI_ID_CHECK7), 2);
        break;
      case GUI_ID_CHECK8:
        /* Toggle alignment */
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        for (i = 0; i < GUI_COUNTOF(_aColor); i++) {
          if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK8))) {
            GRAPH_DATA_YT_SetAlign(_ahData[i], GRAPH_ALIGN_LEFT);
            GRAPH_DATA_YT_MirrorX (_ahData[i], 1);
          } else {
            GRAPH_DATA_YT_SetAlign(_ahData[i], GRAPH_ALIGN_RIGHT);
            GRAPH_DATA_YT_MirrorX (_ahData[i], 0);
          }
        }
        break;
      }
      break;
    }
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}


static void _cbCallback(WM_MESSAGE * pMsg) {
  WM_HWIN hGraph = 0;
  int i, NCode, Id, Value;
  WM_HWIN hDlg, hItem;
  char pbuff[10]={0};
  hDlg = pMsg->hWin;
  switch (pMsg->MsgId) {
      case WM_DELETE:
          perfmonruning = 0;
          break;
      case WM_INIT_DIALOG:
        GUI_SetDefaultFont(&GUI_FontHZ12);
        hItem = pMsg->hWin;
        FRAMEWIN_SetMoveable(hItem, 1);
        FRAMEWIN_SetTitleHeight(hItem, 20);
        FRAMEWIN_SetFont(hItem,&GUI_FontHZ16);
        FRAMEWIN_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
        FRAMEWIN_AddCloseButton(hItem,FRAMEWIN_BUTTON_RIGHT,0);
        FRAMEWIN_AddMinButton(hItem, FRAMEWIN_BUTTON_RIGHT,1);
        break;
      default:
        WM_DefaultProc(pMsg);
  }
}

#define TASK_COUNT (2 + 6)
extern OS_TCB StartTaskTCB;
extern OS_TCB GUITaskTCB;
extern OS_TCB LedTaskTCB;
extern OS_TCB GUIKEYPADTaskTCB;
extern OS_TCB PRTSCTaskTCB;
extern OS_TCB MUSICTaskTCB;

void changeStatus(WM_HWIN hItem) {
    int i, j;
    u8 cpuI, cpuF;
    char pbuff[10]={0};
    OS_TCB tempTcb;
//    OS_TCB *p_tcb = OSTaskDbgListPtr;// 为什么不行
    
    for (i=0;i<TASK_COUNT;i++) {
        if (i==0) {
            tempTcb = StartTaskTCB;
            tempTcb.NamePtr="启动任务";
        } else if (i==1) {
            tempTcb = GUITaskTCB;
            tempTcb.NamePtr="UI任务";
        } else if (i==2) {
            tempTcb = LedTaskTCB;
            tempTcb.NamePtr="简单任务";
        } else if (i==3) {
            tempTcb = GUIKEYPADTaskTCB;
            tempTcb.NamePtr="键盘任务";
        } else if (i==4) {
            tempTcb = MUSICTaskTCB;
            tempTcb.NamePtr="音乐任务";
        } else if (i==5) {
            tempTcb = PRTSCTaskTCB;
            tempTcb.NamePtr="截屏任务";
        } else if (i==6) {
            tempTcb = OSIdleTaskTCB;
            tempTcb.NamePtr="空闲任务";
        } else if (i==7) {
            tempTcb = OSStatTaskTCB;
            tempTcb.NamePtr="统计任务";
        }
//        else if (i==7)tempTcb = OSTmrTaskTCB;
        
        for (j=0;j<6;j++) {
            if (j==0) {
                sprintf(pbuff, "%s", tempTcb.NamePtr);
            } else if (j==1) {
                sprintf(pbuff, "%d", tempTcb.Prio);
            } else if (j==2) {
                cpuI = tempTcb.CPUUsage/100;
                cpuF = tempTcb.CPUUsage%100;
                sprintf(pbuff, "%d.%02d", cpuI, cpuF);
                if (i != (TASK_COUNT-2)) {// 空闲任务不显示
                    if (cpuI < 20) {
                        LISTVIEW_SetItemBkColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_WHITE);
                    } else if (cpuI < 50) {
//                        LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_BLACK);
                        LISTVIEW_SetItemBkColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_DARKGREEN);
                    } else if (cpuI < 80) {
//                        LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_ORANGE);
                        LISTVIEW_SetItemBkColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_ORANGE);
                    } else {
//                        LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_RED);
                        LISTVIEW_SetItemBkColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_RED);
                    }
                }
            } else if (j==3) {
                if (tempTcb.TaskState==OS_TASK_STATE_DEL) {
                    sprintf(pbuff, "%s", "DEL");
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_DARKRED);
                } else if (tempTcb.TaskState==OS_TASK_STATE_RDY) {
                    sprintf(pbuff, "%s", "RDY");
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_DARKGREEN);
                } else if (tempTcb.TaskState==OS_TASK_STATE_DLY) {
                    sprintf(pbuff, "%s", "DLY");
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_DARKBLUE);
                } else if (tempTcb.TaskState==OS_TASK_STATE_PEND) {
                    sprintf(pbuff, "%s", "PEND");
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_ORANGE);
                } else if (tempTcb.TaskState==OS_TASK_STATE_PEND_TIMEOUT) {
                    sprintf(pbuff, "%s", "PEND_TIMEOUT");
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_ORANGE);
                } else if (tempTcb.TaskState==OS_TASK_STATE_SUSPENDED) {
                    sprintf(pbuff, "%s", "SUSPENDED");
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_DARKYELLOW);
                } else if (tempTcb.TaskState==OS_TASK_STATE_DLY_SUSPENDED) {
                    sprintf(pbuff, "%s", "DLY_SUSPENDED");
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_DARKYELLOW);
                } else if (tempTcb.TaskState==OS_TASK_STATE_PEND_SUSPENDED) {
                    sprintf(pbuff, "%s", "PEND_SUSPENDED");
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_DARKYELLOW);
                } else if (tempTcb.TaskState==OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED) {
                    sprintf(pbuff, "%s", "PEND_TIMEOUT_SUSPENDED");
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_DARKYELLOW);
                } else {
                    sprintf(pbuff, "%d", tempTcb.TaskState);
                    LISTVIEW_SetItemTextColor(hItem, j, i, LISTVIEW_CI_UNSEL, GUI_BLACK);
                }
            } else if (j==4) {
                sprintf(pbuff, "%d/%d", tempTcb.StkUsed, tempTcb.StkSize);
            } else if (j==5) {
                sprintf(pbuff, "%d", tempTcb.CtxSwCtr);
            }
            LISTVIEW_SetItemText(hItem, j, i, pbuff);
        }
    }
}

static void _cbCallback3(WM_MESSAGE * pMsg) {
  int i, j, NCode, Id, Value;
  WM_HWIN hDlg, hItem;
//  char pbuff[10]={0};
  const char *aTable[6]={"--","--","--","--","--","--"};
  hDlg = pMsg->hWin;
  switch (pMsg->MsgId) {
      case WM_INIT_DIALOG:
        hItem = WM_GetDialogItem(hDlg, GUI_ID_LISTVIEW0);
        LISTVIEW_SetFont(hItem, &GUI_FontHZ16);
        HEADER_SetFont(LISTVIEW_GetHeader(hItem), &GUI_FontHZ12);
        LISTVIEW_SetHeaderHeight(hItem, 16);
        LISTVIEW_SetRowHeight(hItem, 20);
        LISTVIEW_SetGridVis(hItem, 1);
        LISTVIEW_SetAutoScrollH(hItem, 1);
        LISTVIEW_SetAutoScrollV(hItem, 1);
        LISTVIEW_AddColumn(hItem, 75, "任务名称", GUI_TA_CENTER);
        LISTVIEW_AddColumn(hItem, 45, "优先级", GUI_TA_CENTER);
        LISTVIEW_AddColumn(hItem, 50, "CPU(%)", GUI_TA_CENTER);
        LISTVIEW_AddColumn(hItem, 70, "状态", GUI_TA_CENTER);
        LISTVIEW_AddColumn(hItem, 75, "堆栈情况", GUI_TA_CENTER);
        LISTVIEW_AddColumn(hItem, 70, "频繁程度", GUI_TA_CENTER);

        for (i=0;i<sizeof(aTable);i++) {
            LISTVIEW_SetTextAlign(hItem, i, GUI_TA_HCENTER | GUI_TA_VCENTER);
        }
        for (i=0;i<TASK_COUNT;i++) {
            LISTVIEW_AddRow(hItem, aTable);
        }
        changeStatus(hItem);
      
      break;
      
      case WM_TIMER:
          hItem = WM_GetDialogItem(hDlg, GUI_ID_LISTVIEW0);
          changeStatus(hItem);
          WM_RestartTimer(pMsg->Data.v, REFRESH_SETP);
      break;
        
      default:
        WM_DefaultProc(pMsg);
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
*       MainTask
*/
WM_HWIN CreatePerfMon(void);
WM_HWIN CreatePerfMon(void) {
    WM_HWIN hDlg, hGraph = 0;
    WM_HTIMER hTimer, hTimer3;
    WM_HWIN hMultiPage;
    
    if (perfmonruning==1) {
        return hDlg;
    }
    perfmonruning = 1;
#if GUI_SUPPORT_MEMDEV
	WM_SetCreateFlags(WM_CF_MEMDEV);
#endif
	hDlg = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), &_cbCallback, 0, 0, 0);
    hMultiPage = MULTIPAGE_CreateEx(0, 0, 234, 200, WM_GetClientWindow(hDlg), WM_CF_SHOW, 0, 0);
    MULTIPAGE_SetFont(hMultiPage, &GUI_FontHZ12);
    hGraph = GUI_CreateDialogBox(_aDialogCreate2, GUI_COUNTOF(_aDialogCreate2), &_cbCallback2, WM_UNATTACHED, 0, 0);
    MULTIPAGE_AddPage(hMultiPage, hGraph, "总览");
    
    hTimer = WM_CreateTimer(WM_GetClientWindow(hGraph), 0, REFRESH_SETP, 0);
    
    hGraph = GUI_CreateDialogBox(_aDialogCreate3, GUI_COUNTOF(_aDialogCreate3), _cbCallback3, WM_UNATTACHED, 0, 0);
    MULTIPAGE_AddPage(hMultiPage, hGraph, "任务详情");
    hTimer3 = WM_CreateTimer(WM_GetClientWindow(hGraph), 0, REFRESH_SETP, 0);
    
    MULTIPAGE_SelectPage(hMultiPage, 0);
//    hTimer = WM_CreateTimer(WM_GetClientWindow(hGraph), 0, REFRESH_SETP, 0);
    return hDlg;
}

/*************************** End of file ****************************/

