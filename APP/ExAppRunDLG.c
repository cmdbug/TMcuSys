#include "DIALOG.h"
#include "includes.h" //teng
#include "app.h" //teng
#include "EmWinHZFont.h"//teng
#include "ff.h"//teng
#include "malloc.h"//teng
#include "exfuns.h"//teng
#include "ExAppRun.h"
#include "MessageBoxUser.h"
//#include "treenleafimage.h"//teng

extern u8*const FILE_TYPE_TBL[7][13];
/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ID_FRAMEWIN_0 (GUI_ID_USER + 0x06)
//#define ID_MENU_0     (GUI_ID_USER + 0x07)
#define ID_EDIT_0     (GUI_ID_USER + 0x08)
#define ID_TEXT_0     (GUI_ID_USER + 0x09)
#define ID_TREEVIEW_0 (GUI_ID_USER + 0x0A)
//#define ID_TEXT_1     (GUI_ID_USER + 0x0B)
//#define ID_TEXT_2     (GUI_ID_USER + 0x0C)


TREEVIEW_ITEM_Handle   ehTreeItemSel;
TREEVIEW_ITEM_Handle   ehTreeItemInc;
TREEVIEW_ITEM_Handle   ehTreeItemDec;
WM_HWIN ehWinx;
WM_HWIN ehItemTree;
u8 epathbuffx[150]={0};
TREEVIEW_Handle etreeitem;


// USER START (Optionally insert additional defines)
// USER END

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

// USER START (Optionally insert additional static data)
// USER END

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreatex[] = {
	{ FRAMEWIN_CreateIndirect, "运行浏览器", ID_FRAMEWIN_0, 0, 0, 240, 320, 0, 0x00, 0 },
	{ EDIT_CreateIndirect, "Edit", ID_EDIT_0, 35, 2, 195, 18, 0, 0x64, 0 },
	{ TEXT_CreateIndirect, "Text", ID_TEXT_0, 1, 4, 35, 14, 0, 0x64, 0 },
	{ TREEVIEW_CreateIndirect, "Treeview", ID_TREEVIEW_0, 0, 21, 230, 271, 0, 0x00, 0 },
//	{ TEXT_CreateIndirect, "Text", ID_TEXT_1, 1, 262, 100, 14, 0, 0x64, 0 },
//	{ TEXT_CreateIndirect, "Text", ID_TEXT_2, 110, 262, 100, 14, 0, 0x64, 0 },
	// USER START (Optionally insert additional widgets)
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

//树形文件初始化
WM_HWIN eTREEViewInitx(WM_MESSAGE * pMsg,U16 ID)
{
	WM_HWIN hItem;
	TREEVIEW_ITEM_Handle hTreeItemCur;  
	TREEVIEW_ITEM_Handle hTreeItemNew;
	hItem = WM_GetDialogItem(pMsg->hWin, ID);
etreeitem = hItem;
	TREEVIEW_SetAutoScrollH(hItem,1);
	TREEVIEW_SetAutoScrollV(hItem,1);
	TREEVIEW_SetFont(hItem,&GUI_FontHZ16);
	TREEVIEW_SetSelMode(hItem, TREEVIEW_SELMODE_ROW);                                //整行选中模式

	hTreeItemCur = TREEVIEW_GetItem(hItem, 0, TREEVIEW_GET_LAST);
	hTreeItemNew = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE, "ExFlash", 0);      //外部ExFlash
//	Fill_FileList("1:",hItem,hTreeItemNew,NULL);
	TREEVIEW_AttachItem(hItem, hTreeItemNew, hTreeItemCur, TREEVIEW_INSERT_BELOW);   //增加到树形图

	hTreeItemCur = TREEVIEW_GetItem(hItem, 0, TREEVIEW_GET_LAST);
	hTreeItemNew = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE, "SDCard", 0);       //SDCard
	Fill_FileList("0:",hItem,hTreeItemNew,NULL,0X00);                                    //扫描SDCard文件
	TREEVIEW_AttachItem(hItem, hTreeItemNew, hTreeItemCur, TREEVIEW_INSERT_BELOW);   //增加到树形图

	hTreeItemCur = TREEVIEW_GetItem(hItem, 0, TREEVIEW_GET_FIRST);
	Fill_FileList("1:",hItem,hTreeItemCur,NULL,0X00);                                    //扫描ExFlash文件
	TREEVIEW_AttachItem(hItem, hTreeItemNew, hTreeItemCur, TREEVIEW_INSERT_BELOW);   //增加到树形图
	return hItem;
}

//u8 pathbuffx[150]={0};
//树形文件路径显示，并返回盘符名称句柄
TREEVIEW_ITEM_Handle eTREEVIEWShowPathx(WM_MESSAGE *pMsg, U16 TREEVIEW_IDx, U16 EDIT_IDx)
{
	u8 pbuff[150]={0};
	u8 pbufft[150]={0};
	u8 pathbuff[150]={0};
	u8 pathbufftemp[150]={0};
	TREEVIEW_ITEM_INFO ItemInfo;
	u8 treelevel,i;
	WM_HWIN _hMenu;
	WM_HWIN hItem;

	TREEVIEW_ITEM_Handle   hTreeItemCur;  
	TREEVIEW_ITEM_Handle   hTreeItemNew;
//	TREEVIEW_ITEM_Handle   hTreeItemSel;

	hItem = WM_GetDialogItem(pMsg->hWin, TREEVIEW_IDx);                             //获取句柄
	hTreeItemCur = TREEVIEW_GetSel(hItem);                                          //获取选中条目
	ehTreeItemSel = hTreeItemCur;
	TREEVIEW_ITEM_GetInfo(hTreeItemCur,&ItemInfo);                                  //获取选中条目信息
	treelevel = ItemInfo.Level;                                                     //复制缩进深度
	TREEVIEW_ITEM_GetText(hTreeItemCur,pbuff,50);                                   //获取选中条目名称
	for(i=0;i<treelevel;i++)
	{
		hTreeItemNew = TREEVIEW_GetItem(hItem,hTreeItemCur,TREEVIEW_GET_PARENT);    //获取上级对象
		TREEVIEW_ITEM_GetText(hTreeItemNew,pbufft,50);                              //获取上级对象名称
		sprintf(pathbuff,"%s/%s",pbufft,pbuff);                                     //格式化文本
		sprintf(pbuff,"%s",pathbuff);                                               //复制
		hTreeItemCur = hTreeItemNew;                                                //对换、更新句柄
	}
	//以下为判断盘符并替路径名称
	i = filePathCheck(pathbuff);                                                    //判断盘符
	if(i != 0xFF)
	{
		if(i == 0)                                                                  //修改路径为x:/xxxx/xxx/xx.xxx格式
		{                                                                           //SDCard
			strcpy(pathbufftemp,pathbuff);
			pathbufftemp[0] = '0';
			pathbufftemp[1] = ':';
			for(i=0;i<strlen(pathbuff);i++)
			{
				pathbufftemp[i+2] = pathbuff[i+6];
			}
		}
		else if(i == 1)
		{                                                                           //ExFlash
			strcpy(pathbufftemp,pathbuff);
			pathbufftemp[0] = '1';
			pathbufftemp[1] = ':';
			for(i=0;i<strlen(pathbuff);i++)
			{
				pathbufftemp[i+2] = pathbuff[i+7];
			}
		}
	}
	strcpy(pathbuff,pathbufftemp);
    hItem = WM_GetDialogItem(pMsg->hWin, EDIT_IDx);                                //获取路径EDIT句柄
	EDIT_SetText(hItem,pathbuff);                                                  //显示路径
	strcpy(epathbuffx,pathbuff);
TREEVIEW_SetSel(etreeitem, ehTreeItemSel);    //设置回点击item
//TREEVIEW_IncSel(treeitem);                //测试用
//TREEVIEWShowPathx(pMsg,ID_TREEVIEW_0,ID_EDIT_0);   //测试用
	return ehTreeItemSel;
}

void runExAppRun(u8 *path) {
    u8 pbuff[20]={0};
    u8 r = exe_play(path);
    if (r == 255) {
        MESSAGEBOX_Create_User_Modal("验证失败", "提示");
    } else if (r != 0) {
        sprintf(pbuff,"错误:%d",r); 
        MESSAGEBOX_Create_User_Modal(pbuff, "提示");
    }
}

void runExAppFromDialog(void) {
    runExAppRun(epathbuffx);
}


u8 efilesameflag=0;
u8 epbuffx[50]={0};
//TREEVIEW_ITEM_Handle   hTreeItemSel;
/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialogx(WM_MESSAGE * pMsg) {
	
	u8 pbuff[50]={0};
	u8 pbufft[50]={0};
//	u8 pathbuff[100]={0};
//	u8 pathbufftemp[100]={0};
//	TREEVIEW_ITEM_INFO ItemInfo;
//	u8 treelevel,i;

	TREEVIEW_ITEM_Handle   hTreeItemLast;
//	TREEVIEW_ITEM_Handle   hTreeItemCur;  
//	TREEVIEW_ITEM_Handle   hTreeItemNew;
		
//	MENU_ITEM_DATA         ItemData;
	WM_HWIN                hItem;
	int                    NCode;
	int                    Id;
	// USER START (Optionally insert additional variables)
	// USER END
	switch (pMsg->MsgId) {
		case WM_INIT_DIALOG:
			//
			// Initialization of 'Browser'
			//
			hItem = pMsg->hWin;
			FRAMEWIN_SetTextColor(hItem,GUI_DARKGRAY);
			FRAMEWIN_SetTitleHeight(hItem, 20);
			FRAMEWIN_SetFont(hItem, &GUI_FontHZ16);
			FRAMEWIN_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
			FRAMEWIN_AddCloseButton(hItem,FRAMEWIN_BUTTON_RIGHT,0);
			//
			// Initialization of 'Edit'
			//
			hItem = WM_GetDialogItem(pMsg->hWin, ID_EDIT_0);
			EDIT_SetText(hItem, "");
			EDIT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
			EDIT_SetFont(hItem, &GUI_FontHZ12);
		//	WM_DisableWindow(hItem);           //禁用状态
			//
			// Initialization of 'Text'
			//
			hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_0);
			TEXT_SetText(hItem, "路径:");
			TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
			TEXT_SetFont(hItem, &GUI_FontHZ12);
			//
			// Initialization of 'Treeview'
			//
			hItem = eTREEViewInitx(pMsg,ID_TREEVIEW_0); //树形文件初始化
			// USER END
		break;
		
		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);
			NCode = pMsg->Data.v;
			switch(Id) {
				case ID_EDIT_0: // Notifications sent by 'Edit'
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
						// USER END
						break;
						// USER START (Optionally insert additional code for further notification handling)
						// USER END
					}
				break;
				case ID_TREEVIEW_0: // Notifications sent by 'Treeview'
					switch(NCode) {
						case WM_NOTIFICATION_CLICKED:
						// USER START (Optionally insert code for reacting on notification message)
						// USER END
						break;
						case WM_NOTIFICATION_RELEASED:
						// USER START (Optionally insert code for reacting on notification message)

							ehTreeItemSel = eTREEVIEWShowPathx(pMsg,ID_TREEVIEW_0,ID_EDIT_0);					//根据选中条目显示路径

							hItem = WM_GetDialogItem(pMsg->hWin, ID_TREEVIEW_0);                            //获取树形句柄
							TREEVIEW_ITEM_GetText(ehTreeItemSel,pbufft,50);                                 //获取选中文本
							if( (f_typetell(pbufft)>>4) == (0x00>>4) ) {
								if(strcmp(epbuffx,pbufft)==0) {
									efilesameflag = 1;
								} else {
									strcpy(epbuffx,pbufft);
									efilesameflag = 0;
								}
								if (efilesameflag==1) {  //第二次单击则打开文件
//                                    runExAppRun(epathbuffx);// 运行
                                    MESSAGEBOX_Create_User_Modal_With_SureBTN("执行BIN文件后,系统将无法继续运行,只能复位重启,您确认执行吗", "说明", runExAppFromDialog);
								}
							}
							
						// USER END
						break;
						case WM_NOTIFICATION_MOVED_OUT:
						// USER START (Optionally insert code for reacting on notification message)
						// USER END
						break;
						case WM_NOTIFICATION_SEL_CHANGED:
						// USER START (Optionally insert code for reacting on notification message)
						// USER END
						break;
						// USER START (Optionally insert additional code for further notification handling)
						// USER END
					}
				break;
				// USER START (Optionally insert additional code for further Ids)
				// USER END
				default:
					WM_DefaultProc(pMsg);
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
*       CreateBrowser
*/
WM_HWIN CreateExAppRun(void);
WM_HWIN CreateExAppRun(void) {
	WM_HWIN hWin;

	hWin = GUI_CreateDialogBox(_aDialogCreatex, GUI_COUNTOF(_aDialogCreatex), _cbDialogx, WM_HBKWIN, 0, 0);
	ehWinx = hWin;
    return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
