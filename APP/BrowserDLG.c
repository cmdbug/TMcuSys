#include "DIALOG.h"
#include "includes.h" //teng
#include "app.h" //teng
#include "EmWinHZFont.h"//teng
#include "ff.h"//teng
#include "malloc.h"//teng
#include "exfuns.h"//teng
#include "treenleafimage.h"//teng
#include "MessageBoxUser.h"


extern u8*const FILE_TYPE_TBL[7][13];


static 	uint16_t messagesnote[2];//teng
uint8_t keyflag=0;//键盘是否在使用：1：使用中，0:无效中//teng
extern OS_TCB GUIKEYPADTaskTCB;
OS_ERR      err;
TREEVIEW_Handle treeitemx;
TREEVIEW_ITEM_Handle   hTreeItemSelx;

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
#define ID_TEXT_1     (GUI_ID_USER + 0x0B)
#define ID_TEXT_2     (GUI_ID_USER + 0x0C)


// USER START (Optionally insert additional defines)

#define ID_MENU              (GUI_ID_USER + 0x10)
#define ID_MENU_FILE_OPEN    (GUI_ID_USER + 0x11)
#define ID_MENU_FILE_ATTR    (GUI_ID_USER + 0x12)
#define ID_MENU_FILE_EXIT    (GUI_ID_USER + 0x13)
#define ID_MENU_EDIT_RENAME  (GUI_ID_USER + 0x14)
#define ID_MENU_EDIT_MOVE    (GUI_ID_USER + 0x15)
#define ID_MENU_EDIT_COPY    (GUI_ID_USER + 0x16)
#define ID_MENU_EDIT_DELETE  (GUI_ID_USER + 0x17)
#define ID_MENU_SET_LINE     (GUI_ID_USER + 0x18)
#define ID_MENU_SET_SEL      (GUI_ID_USER + 0x19)
#define ID_MENU_HELP_ABOUT   (GUI_ID_USER + 0x1A)

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
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
	{ FRAMEWIN_CreateIndirect, "资源管理器", ID_FRAMEWIN_0, 0, 0, 240, 320, 0, 0x00, 0 },
//	{ MENU_CreateIndirect, "Menu", ID_MENU_0, 0, 1, 230, 16, 0, 0x00, 0 },
	{ EDIT_CreateIndirect, "Edit", ID_EDIT_0, 35, 2, 195, 18, 0, 0x64, 0 },
	{ TEXT_CreateIndirect, "Text", ID_TEXT_0, 2, 4, 35, 14, 0, 0x64, 0 },
	{ TREEVIEW_CreateIndirect, "Treeview", ID_TREEVIEW_0, 0, 21, 230, 236, 0, 0x00, 0 },
	{ TEXT_CreateIndirect, "Text", ID_TEXT_1, 2, 258, 100, 14, 0, 0x64, 0 },
	{ TEXT_CreateIndirect, "Text", ID_TEXT_2, 110, 258, 100, 14, 0, 0x64, 0 },
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
#define _DF1S	0x81
#define BROWSER_FILE_NAME_LEN 	100							//文件名长度，如果检测到文件名超过50 则丢弃这个文件 
//#define BROWSER_PATH_LEN		50
//#define FILE_LIST_PATH 			"0:/filelist.txt"	//文件记录列表文件的目录

//设置树形图文件图标
void setTreenFilesImage(u8 *fn,TREEVIEW_ITEM_Handle hItem,int index)
{
//	u8 fntype;
//	fntype = f_typetell(fn);
//	if ((fntype == T_TEXT) || (fntype == T_C) || (fntype == T_H))     //文本
//		TREEVIEW_ITEM_SetImage(hItem, index, &_bmtxt);
//	else if ((fntype == T_BIN))                                       //Bin
//		TREEVIEW_ITEM_SetImage(hItem, index, &_bmbin);
//	else if ((fntype == T_LRC))                                       //歌词
//		TREEVIEW_ITEM_SetImage(hItem, index, &_bmlrc);
//	else if ((fntype == T_NES))                                       //NES
//		TREEVIEW_ITEM_SetImage(hItem, index, &_bmgame);
//	else if ((fntype >= 0x40) && (fntype <= 0x4C))                    //音乐类
//		TREEVIEW_ITEM_SetImage(hItem, index, &_bmmusic);
//	else if ((fntype == T_BMP) || (fntype == T_JPG) || (fntype == T_JPEG) || (fntype == T_PNG)) //图像
//		TREEVIEW_ITEM_SetImage(hItem, index, &_bmpictures);
//	else if ((fntype == T_GIF))                                       //gif
//		TREEVIEW_ITEM_SetImage(hItem, index, &_bmgif);
//	else if ((fntype == T_AVI))                                       //AVI
//		TREEVIEW_ITEM_SetImage(hItem, index, &_bmmovie);
//	else                                                              //其它
//		TREEVIEW_ITEM_SetImage(hItem, index, &_bmother);
}


/**
  * @brief  scan_files 递归扫描sd卡内的文件
  * @param  path:初始扫描路径 file_name：指向用来存储文件名的一段空间 hFile:用于记录文件路径的文件指针 hTree 目录树 hNode 目录结点
	*					hTree == NULL &&	hNode == NULL 的话，不创建目录树			
  * @retval result:文件系统的返回值
  */
static FRESULT xscan_files (char* path,char* file_name,FIL *hFile,WM_HWIN hTree, TREEVIEW_ITEM_Handle hNode,int *fileNum) 
{ 
    FRESULT res; 		          //部分在递归过程被修改的变量，不用全局变量	
    FILINFO fno; 
//	unsigned int rw_num;			//已读或已写的字节数
//	char showname[60]={0};
    DIR dir; 
    int i; 
    char *fn; 	
	TREEVIEW_ITEM_Handle hItem=0;

#if _USE_LFN 
    static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1]; 	//长文件名支持
    fno.lfname = lfn; 
    fno.lfsize = sizeof(lfn); 
#endif 

    res = f_opendir(&dir, path);                            //打开目录
    if (res == FR_OK) 
	{ 
        i = strlen(path); 
        for (;;) 
		{ 
            res = f_readdir(&dir, &fno); 										//读取目录下的内容
            if (res != FR_OK || fno.fname[0] == 0) break; 	//为空时表示所有项目读取完毕，跳出
#if _USE_LFN 
            fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
            fn = fno.fname; 
#endif 
            if (*fn == '.') continue; 											//点表示当前目录，跳过			
            if (fno.fattrib & AM_DIR) 
			{ 																							//目录，递归读取
				if(hTree != NULL &&	hNode != NULL)
				{
//					gbk2utf8(fn,showname,60);
//					hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE,showname,0);
					hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE,fn,0);						//目录，创建结点
					TREEVIEW_AttachItem(hTree,hItem,hNode,TREEVIEW_INSERT_FIRST_CHILD);		//把结点加入到目录树中
				}								
				sprintf(&path[i], "/%s", fn); 							//合成完整目录名
                res = xscan_files(path,file_name,hFile,hTree,hItem,fileNum);		//递归遍历 
                if (res != FR_OK) 
					break; 																		//打开失败，跳出循环
                path[i] = 0; 
            } else { 				
				/* 根据要求是否创建目录树 */
				if(hTree != NULL &&	hNode != NULL)																			//创建目录树
				{
//					gbk2utf8(fn,showname,60);
//					hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_LEAF,showname,0);
					hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_LEAF,fn,0);						//文件，创建树叶
					TREEVIEW_AttachItem(hTree,hItem,hNode,TREEVIEW_INSERT_FIRST_CHILD);		//把树叶添加到目录树
			        
//					setTreenFilesImage((u8 *)fn,hItem,TREEVIEW_BI_LEAF);// 容易崩溃?                //设置树叶图像
				
					if (strlen(path)+strlen(fn)<BROWSER_FILE_NAME_LEN)
					{
						sprintf(file_name, "%s/%s", path,fn); 	
						
						//存储文件名到filelist(含路径)										
//					//	res = f_lseek (hFile, hItem*BROWSER_FILE_NAME_LEN);  
//					//	res = f_write (hFile, file_name, BROWSER_FILE_NAME_LEN, &rw_num);	
						//printf("\nfileItem=%ld:%s",hItem,file_name);
					}			
				}
			}//else
        } //for
    } 
    return res; 
} 


/**
  * @brief  Fill_FileList处理非递归过程，然后调用递归函数scan_files扫描目录
	*					
  * @param  path:初始扫描路径
  * @retval none
  */
void xFill_FileList(char* path,WM_HWIN hTree, TREEVIEW_ITEM_Handle hNode,int *p)
{
    CPU_SR_ALLOC();
	u8 result;
	char p_path[BROWSER_FILE_NAME_LEN]={0};				//目录名 指针
	char file_name[BROWSER_FILE_NAME_LEN]={0};				//用于存储的目录文件名，
//	result = f_unlink(record_file);//删除旧的filelist		// TBD 增加自建目录
//	result = f_open (file, record_file, FA_READ|FA_WRITE|FA_CREATE_ALWAYS ); //打开创建索引文件
//     if(result != FR_OK)
//      return ;
	strcpy(p_path,path);						//复制目录名到指针
    OS_CRITICAL_ENTER();
	result = xscan_files(p_path,file_name,file,hTree,hNode,p);			//递归扫描文件
    OS_CRITICAL_EXIT();	
//	f_close (file);					//关闭索引文件	
}


u8 filePathCheck(u8 *path)
{
	u8 i;	
	u8 ExFlash[] = {"ExFlash"};
	u8 SDCard[]  = {"SDCard"};
	if(strlen((char *)path)>0) {
		if(strncmp((char *)ExFlash,(char *)path,7) == 0) {
			i=1;
		}
		if(strncmp((char *)SDCard,(char *)path,6) == 0) {
			i=0;
		}
	} else
		i=0xFF;
	return i;
}


/*********************************************************************
*
*       _AddMenuItem
*/
static void _AddMenuItem(MENU_Handle hMenu, MENU_Handle hSubmenu, const char* pText, U16 Id, U16 Flags) {
	MENU_ITEM_DATA Item;
	MENU_SetFont(hMenu,&GUI_FontHZ16);//teng
	Item.pText    = pText;
	Item.hSubmenu = hSubmenu;
	Item.Flags    = Flags;
	Item.Id       = Id;
	MENU_AddItem(hMenu, &Item);
}
/*********************************************************************
*
*       _CreateMenu
*
* Purpose:
*   Creates the menu widget
*/
static WM_HWIN _CreateMenu(WM_HWIN hParent) {
	MENU_Handle hMenu, hMenuFile, hMenuEdit, hMenuSet, hMenuHelp;
	// Create main menu
	hMenu       = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_HORIZONTAL, ID_MENU);
	// Create sub menus
	hMenuFile   = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	hMenuEdit   = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	hMenuSet    = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	hMenuHelp   = MENU_CreateEx(0, 0, 0, 0, WM_UNATTACHED, 0, MENU_CF_VERTICAL, 0);
	// Add menu items to menu 'File'
	_AddMenuItem(hMenuFile, 0,             "打开",   ID_MENU_FILE_OPEN,    0);
	_AddMenuItem(hMenuFile, 0,             "属性",   ID_MENU_FILE_ATTR,    0);
	_AddMenuItem(hMenuFile, 0,             0,   	   0,            MENU_IF_SEPARATOR);
	_AddMenuItem(hMenuFile, 0,             "退出",   ID_MENU_FILE_EXIT,    0);
	// Add menu items to menu 'Edit'
	_AddMenuItem(hMenuEdit,   0,           "重命名",    ID_MENU_EDIT_RENAME,    0);
	_AddMenuItem(hMenuEdit,   0,           "移动",    ID_MENU_EDIT_MOVE,   0);
	_AddMenuItem(hMenuEdit,   0,           "复制",    ID_MENU_EDIT_COPY, 0);
	_AddMenuItem(hMenuEdit,   0,           "删除",    ID_MENU_EDIT_DELETE,   0);
	// Add menu items to menu 'Set'
	_AddMenuItem(hMenuSet,    0,           "树形线",     ID_MENU_SET_LINE,   0);
	_AddMenuItem(hMenuSet,    0,           "选中方式",     ID_MENU_SET_SEL,   0);
	// Add menu items to menu 'Help'
	_AddMenuItem(hMenuHelp,   0,           "关于",    ID_MENU_HELP_ABOUT,  0);
	// Add menu items to main menu
	_AddMenuItem(hMenu,       hMenuFile,   "文件",     0,                   0);
	_AddMenuItem(hMenu,       hMenuEdit,   "编辑",     0,                   0);
	_AddMenuItem(hMenu,       hMenuSet,    "设置",     0,                   0);
	_AddMenuItem(hMenu,       hMenuHelp,   "帮助",     0,                   0);
	// Attach menu to parent window
	FRAMEWIN_AddMenu(hParent, hMenu);
	return hMenu;
}

//树形文件初始化
WM_HWIN TREEViewInit(WM_MESSAGE * pMsg,U16 ID)
{
	WM_HWIN hItem;
	TREEVIEW_ITEM_Handle hTreeItemCur;  
	TREEVIEW_ITEM_Handle hTreeItemNew;
	hItem = WM_GetDialogItem(pMsg->hWin, ID);
treeitemx = hItem;
	TREEVIEW_SetAutoScrollH(hItem,1);
	TREEVIEW_SetAutoScrollV(hItem,1);
	TREEVIEW_SetFont(hItem,&GUI_FontHZ16);
	TREEVIEW_SetSelMode(hItem, TREEVIEW_SELMODE_ROW);                                //整行选中模式

	hTreeItemCur = TREEVIEW_GetItem(hItem, 0, TREEVIEW_GET_LAST);
	hTreeItemNew = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE, "ExFlash", 0);      //外部ExFlash
//	xFill_FileList("1:",hItem,hTreeItemNew,NULL);
	TREEVIEW_AttachItem(hItem, hTreeItemNew, hTreeItemCur, TREEVIEW_INSERT_BELOW);   //增加到树形图

	hTreeItemCur = TREEVIEW_GetItem(hItem, 0, TREEVIEW_GET_LAST);
	hTreeItemNew = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE, "SDCard", 0);       //SDCard
	xFill_FileList("0:",hItem,hTreeItemNew,NULL);                                    //扫描SDCard文件
	TREEVIEW_AttachItem(hItem, hTreeItemNew, hTreeItemCur, TREEVIEW_INSERT_BELOW);   //增加到树形图

	hTreeItemCur = TREEVIEW_GetItem(hItem, 0, TREEVIEW_GET_FIRST);
	xFill_FileList("1:",hItem,hTreeItemCur,NULL);                                    //扫描ExFlash文件
	TREEVIEW_AttachItem(hItem, hTreeItemNew, hTreeItemCur, TREEVIEW_INSERT_BELOW);   //增加到树形图
	return hItem;
}


//树形文件路径显示，并返回盘符名称句柄
TREEVIEW_ITEM_Handle TREEVIEWShowPath(WM_MESSAGE *pMsg, U16 TREEVIEW_IDx, U16 EDIT_IDx)
{
	u8 pbuff[150]={0};
	u8 pbufft[150]={0};
	u8 pathbuff[150]={0};
	u8 pathbufftemp[150]={0};
	TREEVIEW_ITEM_INFO ItemInfo;
	u8 treelevel,i;
//	WM_HWIN _hMenu;
	WM_HWIN hItem;

	TREEVIEW_ITEM_Handle   hTreeItemCur;  
	TREEVIEW_ITEM_Handle   hTreeItemNew;

	hItem = WM_GetDialogItem(pMsg->hWin, TREEVIEW_IDx);                             //获取句柄
	hTreeItemCur = TREEVIEW_GetSel(hItem);                                          //获取选中条目
hTreeItemSelx = hTreeItemCur;
	TREEVIEW_ITEM_GetInfo(hTreeItemCur,&ItemInfo);                                  //获取选中条目信息
	treelevel = ItemInfo.Level;                                                     //复制缩进深度
	TREEVIEW_ITEM_GetText(hTreeItemCur,pbuff,50);                                   //获取选中条目名称
	for(i=0;i<treelevel;i++)
	{
		hTreeItemNew = TREEVIEW_GetItem(hItem,hTreeItemCur,TREEVIEW_GET_PARENT);    //获取上级对象
		TREEVIEW_ITEM_GetText(hTreeItemNew,pbufft,50);                              //获取上级对象名称
		sprintf((char *)pathbuff,"%s/%s",pbufft,pbuff);                                     //格式化文本
		sprintf((char *)pbuff,"%s",pathbuff);                                               //复制
		hTreeItemCur = hTreeItemNew;                                                //对换、更新句柄
	}
	//以下为判断盘符并替路径名称
	i = filePathCheck(pathbuff);                                                    //判断盘符
	if(i != 0xFF)
	{
		if(i == 0)                                                                  //修改路径为x:/xxxx/xxx/xx.xxx格式
		{                                                                           //SDCard
			strcpy((char *)pathbufftemp,(char *)pathbuff);
			pathbufftemp[0] = '0';
			pathbufftemp[1] = ':';
			for(i=0;i<strlen((char *)pathbuff);i++) {
				pathbufftemp[i+2] = pathbuff[i+6];
			}
		} else if(i == 1) {                                                         //ExFlash
			strcpy((char *)pathbufftemp,(char *)pathbuff);
			pathbufftemp[0] = '1';
			pathbufftemp[1] = ':';
			for(i=0;i<strlen((char *)pathbuff);i++) {
				pathbufftemp[i+2] = pathbuff[i+7];
			}
		}
	}
	strcpy((char *)pathbuff,(char *)pathbufftemp);
    hItem = WM_GetDialogItem(pMsg->hWin, EDIT_IDx);                                //获取路径EDIT句柄
	EDIT_SetText(hItem,(char *)pathbuff);                                                  //显示路径
	TREEVIEW_SetSel(treeitemx, hTreeItemSelx);    //设置回点击item
	return hTreeItemCur;
}

u8 HasLines=1, HasRowSelect=1;
//菜单选项处理函数
void fun_MENU_ON_ITEMSELECT(WM_MESSAGE *pMsg, MENU_MSG_DATA *pData)
{
	WM_HWIN hItem;
	TREEVIEW_ITEM_INFO ItemInfo;
	
	hItem = WM_GetDialogItem(pMsg->hWin, ID_TREEVIEW_0);
	TREEVIEW_ITEM_GetInfo(TREEVIEW_GetSel(hItem), &ItemInfo);                      //获取选中条目信息
	
	switch(pData->ItemId)
	{
		case ID_MENU_FILE_OPEN:
			MESSAGEBOX_Create_User_Modal("未完成open", "提示");
		break;
		case ID_MENU_FILE_ATTR:
			MESSAGEBOX_Create_User_Modal("未完成attr", "提示");
		break;
		case ID_MENU_FILE_EXIT:
			GUI_EndDialog(pMsg->hWin, 0);
		//	MESSAGEBOX_Create("Sorry exit", "About",GUI_MESSAGEBOX_CF_MODAL);
		break;
		case ID_MENU_EDIT_RENAME:
			MESSAGEBOX_Create_User_Modal("未完成rename", "提示");
		break;
		case ID_MENU_EDIT_MOVE:
			MESSAGEBOX_Create_User_Modal("未完成move", "提示");
		break;
		case ID_MENU_EDIT_COPY:
			MESSAGEBOX_Create_User_Modal("未完成copy", "提示");
		break;
		case ID_MENU_EDIT_DELETE:
			MESSAGEBOX_Create_User_Modal("未完成delete", "提示");
		break;                                                                     
		case ID_MENU_SET_LINE:                                                     //设置是否显示连接线条
			if(HasLines == 1) {
				TREEVIEW_SetHasLines(hItem, 0);           //对线条方式进行反选
				HasLines = 0;
			} else {
				TREEVIEW_SetHasLines(hItem, 1);           //对线条方式进行反选
				HasLines = 1;
			}
		//	TREEVIEW_SetHasLines(hItem, !(ItemInfo.HasLines));           //对线条方式进行反选
		//	TREEVIEW_SetHasLines(hItem, (ItemInfo.HasLines == 1)? 0: 1);           //对线条方式进行反选
		break;
		case ID_MENU_SET_SEL:
			if(HasRowSelect == 1)
			{
				TREEVIEW_SetSelMode(hItem, 0);	 //对选中方式进行反选
				HasRowSelect = 0;
			} else {
				TREEVIEW_SetSelMode(hItem, 1);	 //对选中方式进行反选
				HasRowSelect = 1;
			}
		//	TREEVIEW_SetSelMode(hItem, !(ItemInfo.HasRowSelect));	 //对选中方式进行反选
		//	TREEVIEW_SetSelMode(hItem, (ItemInfo.HasRowSelect == 1)? TREEVIEW_SELMODE_TEXT: TREEVIEW_SELMODE_ROW);	 //对选中方式进行反选
		break;
		case ID_MENU_HELP_ABOUT:
			messagesnote[0]=WM_GetDialogItem(pMsg->hWin, ID_EDIT_0);
			messagesnote[1]=ID_EDIT_0;
			if(!keyflag)
				OSTaskQPost(&GUIKEYPADTaskTCB,
					(void *)&messagesnote,
					2,
					OS_OPT_POST_FIFO,
					&err);
			WM_DefaultProc(pMsg);
        // USER END
		//	MESSAGEBOX_Create("by WZTENG", "About",GUI_MESSAGEBOX_CF_MODAL);   //可以修改窗体属性 framewin
		break;
		default:
			WM_DefaultProc(pMsg); 
	}
}

u32 sdcardtotal,sdcardfree,exflashtotal,exflashfree;
/*********************************************************************
*
*       _cbDialog
*/
static void _cbDialog(WM_MESSAGE * pMsg) {
	
MENU_MSG_DATA *pData;
//MENU_ITEM_DATA Data;
	
	u8 pbuff[50]={0};
	u8 pbufft[50]={0};
//	u8 pathbuff[100]={0};
//	u8 pathbufftemp[100]={0};
//	TREEVIEW_ITEM_INFO ItemInfo;
//	u8 treelevel,i;
	WM_HWIN _hMenu;

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
        case WM_DELETE:
            treeitemx = 0;
            hTreeItemSelx = 0;
            break;
		case WM_INIT_DIALOG:
			//
			// Initialization of 'Browser'
			//
			hItem = pMsg->hWin;
//			FRAMEWIN_SetTextColor(hItem,GUI_DARKGRAY);
			FRAMEWIN_SetTitleHeight(hItem, 20);
			FRAMEWIN_SetFont(hItem, &GUI_FontHZ16);
			FRAMEWIN_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
			FRAMEWIN_AddCloseButton(hItem,FRAMEWIN_BUTTON_RIGHT,0);
			//
			// Initialization of 'Menu'
			//
			_hMenu = _CreateMenu(hItem);
			//
			// Initialization of 'Edit'
			//
			hItem = WM_GetDialogItem(pMsg->hWin, ID_EDIT_0);
			EDIT_SetText(hItem, "");
			EDIT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
			EDIT_SetFont(hItem, &GUI_FontHZ12);
			WM_DisableWindow(hItem);           //禁用状态
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
			hItem = TREEViewInit(pMsg,ID_TREEVIEW_0); //树形文件初始化
			//
			// Initialization of 'Text'
			//
			hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
			TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
			TEXT_SetText(hItem, "容量:");
			TEXT_SetFont(hItem, &GUI_FontHZ12);
			//
			// Initialization of 'Text'
			//
			hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
			TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_VCENTER);
			TEXT_SetText(hItem, "剩余:");
			TEXT_SetFont(hItem, &GUI_FontHZ12);
			// USER START (Optionally insert additional code for further widget initialization)
			
			exf_getfree((u8 *)"0:",&sdcardtotal,&sdcardfree);            //获取SDCard卡总量与空间容量
			exf_getfree((u8 *)"1:",&exflashtotal,&exflashfree);          //获取ExFlash卡总量与空间容量
			hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
			sprintf((char *)pbuff,"容量:%dKB",exflashtotal);
			TEXT_SetText(hItem, (char *)pbuff);
			hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
			sprintf((char *)pbuff,"剩余:%dKB",exflashfree);
			TEXT_SetText(hItem, (char *)pbuff);
			
			// USER END
		break;
		
		case WM_MENU:
			pData = (MENU_MSG_DATA*)pMsg->Data.p;
			switch (pData->MsgType) {
				case MENU_ON_ITEMPRESSED:
					//
					// This message is send regardless of the item state is disabled or not
					//
				break;
				case MENU_ON_ITEMACTIVATE:
					//
					// This message is send on highlighting a menu item
					//
				break;
				case MENU_ON_ITEMSELECT:
					//
					// This message is send only if an enabled item has been selected
					//
					fun_MENU_ON_ITEMSELECT(pMsg, pData);      //菜单选项处理函数
				break;
				case MENU_ON_INITMENU:
					
				break;
				default:
					WM_DefaultProc(pMsg);                     //必须加这句**************************************
			}
		break;
		
		
		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);
			NCode = pMsg->Data.v;
			switch(Id) {
				case ID_MENU: // Notifications sent by 'Menu'
					switch(NCode) {
						case WM_NOTIFICATION_CLICKED:
						// USER START (Optionally insert code for reacting on notification message)
						// USER END
						break;
						case WM_NOTIFICATION_RELEASED:
						// USER START (Optionally insert code for reacting on notification message)
//						printf("ID_MENU:released\r\n");
						// USER END
						break;
						case WM_NOTIFICATION_VALUE_CHANGED:
						// USER START (Optionally insert code for reacting on notification message)
						// USER END
						break;
						// USER START (Optionally insert additional code for further notification handling)
						// USER END
                        default:
                            WM_DefaultProc(pMsg);
					}
				break;
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
                        default:
                            WM_DefaultProc(pMsg);
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

							hTreeItemLast = TREEVIEWShowPath(pMsg,ID_TREEVIEW_0,ID_EDIT_0);					//根据选中条目显示路径

							hItem = WM_GetDialogItem(pMsg->hWin, ID_TREEVIEW_0);                            //获取树形句柄
							TREEVIEW_ITEM_GetText(hTreeItemLast,pbufft,50);                                 //获取选中文本
							if(strcmp((char *)pbufft,"SDCard")==0)                                                  //SDCard
							{                                                                               //显示信息
								hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
								sprintf((char *)pbuff,"容量:%dKB",sdcardtotal);
								TEXT_SetText(hItem, (char *)pbuff);
								hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
								sprintf((char *)pbuff,"剩余:%dKB",sdcardfree);
								TEXT_SetText(hItem, (char *)pbuff);
							} else if(strcmp((char *)pbufft,"ExFlash")==0)                                           //ExFlash
							{                                                                               //显示信息
								hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_1);
								sprintf((char *)pbuff,"容量:%dKB",exflashtotal);
								TEXT_SetText(hItem, (char *)pbuff);
								hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_2);
								sprintf((char *)pbuff,"剩余:%dKB",exflashfree);
								TEXT_SetText(hItem, (char *)pbuff);
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
                        default:
                            WM_DefaultProc(pMsg);
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
WM_HWIN CreateBrowser(void);
WM_HWIN CreateBrowser(void) {
	WM_HWIN hWin;

	hWin = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
    return hWin;
}

// USER START (Optionally insert additional public code)
// USER END

/*************************** End of file ****************************/
