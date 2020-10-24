#include "MessageBoxUser.h"
#include "DIALOG.h"
#include "EmWinHZFont.h"//teng
#include "ili93xx.h"


#define ID_BUTTON_0        (GUI_ID_USER + 0x01)
static void _cbDialogFun(WM_MESSAGE *pMsg);
WM_CALLBACK *cb;
void(*funx)(void);

void MESSAGEBOX_Create_User(char *sMessage, const char *sCaption, int Flags, void(*fun)(void)) {
    WM_HWIN  hWin;
    WM_HWIN  hItem;
    GUI_RECT Rect;
    FRAMEWIN_SKINFLEX_PROPS props;
    int count = 0;
    char *pTxt;
    int width = 80;
    int lines = 1;
    int x, y, xs, ys;
    
    pTxt = sMessage;
    while (*pTxt != 0) {
        if (*pTxt > 127) {
            count += 2;
            pTxt += 2;
        } else {
            count++;
            pTxt++;
        }
    }

    FRAMEWIN_GetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
    props.Radius = 5;
    FRAMEWIN_SetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
    FRAMEWIN_GetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
    props.Radius = 5;
    FRAMEWIN_SetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
    
    hWin = MESSAGEBOX_Create(sMessage, sCaption, Flags);
    FRAMEWIN_AddCloseButton(hWin,FRAMEWIN_BUTTON_RIGHT,0);
    //
    // Change font of message box window
    //
    FRAMEWIN_SetFont(hWin, &GUI_FontHZ16);
    //
    // Adjust size
    //
    hItem = WM_GetDialogItem(hWin, GUI_ID_TEXT0);
    WM_HideWindow(hItem);// 不使用默认
    // 自己创建一个
    if (count>8&&count<23) {
        width = width + (count-8)*8;
    } else if (count>=23) {
        width = 8*23;
        lines = lines+count/23;
        if (lines>10) {
            lines=10;
        }
    }
    if (count<23) {
        hItem = TEXT_CreateEx(5,21+2,width,3+16*lines,hWin,WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_TEXT0, NULL);
        TEXT_SetWrapMode(hItem, GUI_WRAPMODE_WORD);
        TEXT_SetFont(hItem, &GUI_FontHZ16);
        TEXT_SetText(hItem, sMessage);
//    TEXT_SetBkColor(hItem, GUI_RED);// 测试看尺寸用的
    } else {
        hItem = MULTIEDIT_CreateEx(5,21,width,3+16*lines,hWin,WM_CF_SHOW,TEXT_CF_HCENTER|TEXT_CF_VCENTER, GUI_ID_MULTIEDIT0,50, NULL);
        MULTIEDIT_SetFont(hItem, &GUI_FontHZ16);
        MULTIEDIT_SetAutoScrollV(hItem, 1);
        MULTIEDIT_SetReadOnly(hItem, 1);
        MULTIEDIT_SetWrapWord(hItem);
        MULTIEDIT_SetInsertMode(hItem, 0);
        MULTIEDIT_SetFocussable(hItem, 0);
        MULTIEDIT_SetBkColor(hItem, MULTIEDIT_CI_READONLY, GUI_WHITE);
        MULTIEDIT_SetText(hItem, sMessage);
    }
    
    WM_GetWindowRectEx(hWin, &Rect);
    WM_SetWindowPos(hWin, (lcddev.width-(Rect.x1-Rect.x0+width-58+1))/2, 
                        Rect.y0-lines*8, 
                        Rect.x1 - Rect.x0 + 1 + width-58, 
                        Rect.y1 - Rect.y0 + 1 + lines*16-8);
    //
    // Change font of button widget
    //
    hItem = WM_GetDialogItem(hWin, GUI_ID_OK);
    BUTTON_SetFont(hItem, &GUI_FontHZ16);
    BUTTON_SetText(hItem, "关闭");
    
    if (fun != NULL) {
        funx = fun;
        WM_GetWindowRectEx(hItem, &Rect);
        x = Rect.x0 + (width-(Rect.x1-Rect.x0))/2;
        y = Rect.y0 + lines*16-11;
        xs = Rect.x1 - Rect.x0;
        ys = Rect.y1 - Rect.y0 + 1;
        WM_SetWindowPos(hItem, x-xs/2-4, y, xs, ys);
        
        hItem = BUTTON_CreateAsChild(0, 0, xs, ys, hWin, ID_BUTTON_0, WM_CF_SHOW);
        WM_SetWindowPos(hItem, x+xs/2, y, xs, ys);
        BUTTON_SetFont(hItem, &GUI_FontHZ16);
        BUTTON_SetText(hItem, "确认");
    } else {
        //
        // Adjust size of button widget
        //
        WM_GetWindowRectEx(hItem, &Rect);
        WM_SetWindowPos(hItem, Rect.x0 + (width-(Rect.x1-Rect.x0))/2-8, 
                             Rect.y0 + lines*16-11, 
                             Rect.x1 - Rect.x0 + 10, 
                             Rect.y1 - Rect.y0 + 1);
    }
    
    cb = WM_SetCallback(hWin, _cbDialogFun);
    // 模糊
//    GUI_MEMDEV_BlurWinBk(hWin, 3000, 10);
//    GUI_MEMDEV_FadeInWindow(hWin, 3000);
//    GUI_MEMDEV_BlendWinBk(hWin, 3000, GUI_BLACK, 125);
//    GUI_MEMDEV_BlurAndBlendWinBk(hWin, 3000, 5, GUI_BLACK, 125);
//    WM_MakeModal(hWin);

    GUI_Delay(1);//要加这个不然下面的不起作用
    FRAMEWIN_GetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
    props.Radius = 0;
    FRAMEWIN_SetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_ACTIVE);
    FRAMEWIN_GetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
    props.Radius = 0;
    FRAMEWIN_SetSkinFlexProps(&props, FRAMEWIN_SKINFLEX_PI_INACTIVE);
}

static void _cbDialogFun(WM_MESSAGE *pMsg) {
    int NCode;
    int Id;
//    cb(pMsg);
    switch (pMsg->MsgId) {
        case WM_NOTIFY_PARENT:
            Id    = WM_GetId(pMsg->hWinSrc);
            NCode = pMsg->Data.v;
            switch(Id) {
                case ID_BUTTON_0:
                    switch(NCode) {
                        case WM_NOTIFICATION_CLICKED:
                        break;
                        case WM_NOTIFICATION_RELEASED:
                        funx();
                        GUI_EndDialog(pMsg->hWin, 0);
                        break;
                    }
                    break;
                default:cb(pMsg);;break;
            }
            break;
        default:cb(pMsg);;break;
    }
}

void MESSAGEBOX_Create_User_Modal(char * sMessage, const char * sCaption) {
    MESSAGEBOX_Create_User(sMessage, sCaption, GUI_MESSAGEBOX_CF_MODAL, NULL);
}

void MESSAGEBOX_Create_User_Modal_With_SureBTN(char *sMessage, const char *sCaption, void(*fun)(void)) {
    MESSAGEBOX_Create_User(sMessage, sCaption, GUI_MESSAGEBOX_CF_MODAL, fun);
}

