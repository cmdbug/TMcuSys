#include "includes.h"
#include "app.h"
#include "EmWinHZFont.h"//teng
/*
*********************************************************************************************************
*                                         宏定义 
*********************************************************************************************************
*/ 
#define COLOR_BORDER           0xBD814F
#define COLOR_KEYPAD0          0xA8D403
#define COLOR_KEYPAD1          0xBF5C00

#define ID_BUTTON              (GUI_ID_USER + 0)

#define BUTTON_WHITH   53
#define BUTTON_HIGHT   43
#define BUTTON_SpaceX  (3+BUTTON_WHITH) 
#define BUTTON_SpaceY  (3+BUTTON_HIGHT)
#define textlength   	20

typedef struct {
  int          xPos;
  int          yPos;
  int          xSize;
  int          ySize;
  const char * acLabel;   /* 按钮对应的字符 */
} BUTTON;

static const BUTTON ButtonData[] = 
{

	/* 第1排按钮 */
	{ 5,                   63, 				 	BUTTON_WHITH, 		BUTTON_HIGHT, "C"    },
	{ 5+BUTTON_SpaceX,     63,  				BUTTON_WHITH, 		BUTTON_HIGHT, "+/-"  },
	{ 5+BUTTON_SpaceX*2,   63,  				BUTTON_WHITH, 		BUTTON_HIGHT, "1/x"  },
	{ 5+BUTTON_SpaceX*3,   63,  				BUTTON_WHITH, 		BUTTON_HIGHT, "/"    },
	/* 第2排按钮 */
	{ 5,                   63+BUTTON_SpaceY,  	BUTTON_WHITH, 		BUTTON_HIGHT, "7" 	},
	{ 5+BUTTON_SpaceX,     63+BUTTON_SpaceY,  	BUTTON_WHITH, 		BUTTON_HIGHT, "8" 	},
	{ 5+BUTTON_SpaceX*2,   63+BUTTON_SpaceY,  	BUTTON_WHITH, 		BUTTON_HIGHT, "9" 	},
	{ 5+BUTTON_SpaceX*3,   63+BUTTON_SpaceY,  	BUTTON_WHITH, 		BUTTON_HIGHT, "*" 	},
	/* 第3排按钮 */
	{ 5,                   63+BUTTON_SpaceY*2,  BUTTON_WHITH, 		BUTTON_HIGHT, "4" 	},
	{ 5+BUTTON_SpaceX,     63+BUTTON_SpaceY*2,  BUTTON_WHITH, 		BUTTON_HIGHT, "5" 	},
	{ 5+BUTTON_SpaceX*2,   63+BUTTON_SpaceY*2,  BUTTON_WHITH, 		BUTTON_HIGHT, "6" 	},
	{ 5+BUTTON_SpaceX*3,   63+BUTTON_SpaceY*2,  BUTTON_WHITH, 		BUTTON_HIGHT, "-" 	},

	/* 第4排按钮 */
	{ 5,                   63+BUTTON_SpaceY*3,  BUTTON_WHITH,  		BUTTON_HIGHT, "1" 	},
	{ 5+BUTTON_SpaceX,     63+BUTTON_SpaceY*3,  BUTTON_WHITH, 		BUTTON_HIGHT, "2" 	},
	{ 5+BUTTON_SpaceX*2,   63+BUTTON_SpaceY*3,  BUTTON_WHITH, 		BUTTON_HIGHT, "3" 	},
	{ 5+BUTTON_SpaceX*3,   63+BUTTON_SpaceY*3,  BUTTON_WHITH, 		BUTTON_HIGHT, "+" 	},

	/* 第5排按钮 */
	{ 5,                   63+BUTTON_SpaceY*4,  BUTTON_WHITH*2+3, 	BUTTON_HIGHT, "0" 	},
	{ 5+BUTTON_SpaceX*2,   63+BUTTON_SpaceY*4,  BUTTON_WHITH, 		BUTTON_HIGHT, "." 	},
	{ 5+BUTTON_SpaceX*3,   63+BUTTON_SpaceY*4,  BUTTON_WHITH, 		BUTTON_HIGHT, "=" 	}
};
/*
*********************************************************************************************************
*                                      变量
*********************************************************************************************************
*/ 
//extern GUI_FONT    	XBF_Font;
static uint8_t 		s_Key;
static char 		text1[textlength]={0},	text2[textlength]={0};
static double 		data1=0.0,		data2=0.0;
static uint8_t    	offset=0,calculating=0,floaton=0,floatoffset=0;
static char cal=0;
//static int calinteger=0;
//static double d=0;
/***************************************************************************/
char* str(char *string)
{
	uint8_t i=0;
	while(string[i]!='\0')i++;
	do
	{
		string[i]='\0';
		i--;
	}
	while(string[i]==48 && i>0);
	if(string[i]=='.')string[i]='\0';
	return string;
}

void num(double data,char st[20])
{
	char text[40]={0},*string={0};
	uint8_t i=0;
	sprintf(text,"%.12f",data);
	//printf("text=%s\n",text);
	string=str(text);
	string[19]='\0';
	//printf("string=%s\n",string);
	for(i=0;i<20;i++)
	{
		st[i]=string[i];
	}
}

/*********************************************************************
*
*       _aDialogCreate
*/
static const GUI_WIDGET_CREATE_INFO _aDialogCreateCal[] = {
  { FRAMEWIN_CreateIndirect, "计算器",0, 0, 0, 240, 320, 0, 0x0, 0 },
  { TEXT_CreateIndirect, "calculator", GUI_ID_TEXT0, 5, 5, 220, 25, 0, 0x64, 0 },
  { TEXT_CreateIndirect, "input", GUI_ID_TEXT1, 5, 5, 220, 50, 0, 0x64, 0 },
  // USER START (Optionally insert additional widgets)
  // USER END
};
/*
*********************************************************************************************************
*	函 数 名: _cbKeyPad
*	功能说明: 回调函数
*	形    参：pMsg  指针参数
*	返 回 值: 无
*********************************************************************************************************
*/
static void _cbDialogCal(WM_MESSAGE * pMsg) 
{
	WM_HWIN hItem;
	WM_HWIN    hButton;
	int        Id;
	int        NCode;
	uint8_t    xSize=0;
	uint16_t   ySize=0;
	uint8_t    i=0;
	hItem = pMsg->hWin;
	switch (pMsg->MsgId) 
	{
		case WM_DELETE:
//		APP_DEBUG("Calculatorapp delete\n");
		s_Key=0;
		text1[0]='\0';
		text2[0]='\0';
		data1=0.0;
		data2=0.0;
		offset=0;
		calculating=0;
		floaton=0;
		floatoffset=0;
		app_prevent_refresh = 0;
		WM_InvalidateWindow(WM_HBKWIN);
		break;
		case WM_INIT_DIALOG:
			//
			// Initialization of 'Calculator'
			//
//			FRAMEWIN_SetFont(hItem, GUI_FONT_16B_ASCII);
			FRAMEWIN_SetFont(hItem, &GUI_FontHZ16);
			FRAMEWIN_SetTextAlign(hItem,GUI_TA_LEFT | GUI_TA_VCENTER);
			FRAMEWIN_SetTitleHeight(hItem, 20);
//			FRAMEWIN_SetTextColor(hItem,GUI_DARKGRAY);
			FRAMEWIN_AddCloseButton(hItem,FRAMEWIN_BUTTON_RIGHT,0);
			//
			// Initialization of 'calculator'
			//
			hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0);
			TEXT_SetTextColor(hItem, GUI_BLACK);
			TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_VCENTER);
			TEXT_SetFont(hItem, GUI_FONT_24_ASCII);
			TEXT_SetText(hItem, "");
			WM_BringToTop(hItem);
			TEXT_SetBkColor(hItem,GUI_WHITE);
			//
			// Initialization of 'input'
			//
			hItem = WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1);
			TEXT_SetFont(hItem, GUI_FONT_24B_ASCII);
			TEXT_SetTextColor(hItem, GUI_BLACK);
			TEXT_SetText(hItem, "0");
			TEXT_SetBkColor(hItem,GUI_WHITE);
			TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_BOTTOM);
			// USER START (Optionally insert additional code for further widget initialization)
			/* 创建所需的按钮 */
			for (i = 0; i < GUI_COUNTOF(ButtonData); i++) 
			{
				hButton = BUTTON_CreateEx(ButtonData[i].xPos, ButtonData[i].yPos, ButtonData[i].xSize, ButtonData[i].ySize, 
				WM_GetClientWindow(pMsg->hWin), WM_CF_SHOW, 0, ID_BUTTON + i);
				BUTTON_SetFont(hButton, &GUI_Font24B_ASCII);
				BUTTON_SetText(hButton, ButtonData[i].acLabel);
				BUTTON_SetTextAlign(hButton,GUI_TA_HCENTER|GUI_TA_VCENTER);
				//BUTTON_SetFocussable(hButton, 0);
			}
			WM_SetFocus(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1));
			// USER END
			break;
		/* 绘制背景 */	
		case WM_PAINT:	
			xSize = WM_GetWindowSizeX(hItem);
			ySize = WM_GetWindowSizeY(hItem);
			GUI_SetColor(COLOR_BORDER);
			GUI_DrawRect(0, 0, xSize - 1, ySize - 1);
			GUI_DrawGradientV(1, 1, xSize - 2, ySize - 2, COLOR_KEYPAD0, COLOR_KEYPAD1);
			break;
		
		/* 用于处理按钮的消息 */
		case WM_NOTIFY_PARENT:
			Id    = WM_GetId(pMsg->hWinSrc);
			NCode = pMsg->Data.v;
			if(Id==GUI_ID_TEXT0||Id==GUI_ID_TEXT1)break;
			switch (NCode) 
			{
				case WM_NOTIFICATION_CLICKED:
					// USER START (Optionally insert code for reacting on notification message)
//					LED1_ON;
					// USER END
				break;
				case WM_NOTIFICATION_RELEASED:
//					LED1_OFF;
					if (ButtonData[Id - ID_BUTTON].acLabel)
					{
						if(Id == (ID_BUTTON+0))//"C"
						{
							offset=0;
							floaton=0;
							floatoffset=0;
							data2=0;
							text2[0]='0';
							text2[1]='\0';
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1),text2);
							if(calculating==0)
							{
								data1=0;
								text1[0]='\0';
								TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0),text1);
							}
							/* 功能键处理完要退出 */
							break;
						}
						if(Id == (ID_BUTTON+1))//"+/-"
						{							
							data2=-data2;	
							num(data2,text2);						
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1),text2);
							/* 功能键处理完要退出 */
							break;
						}
						if(Id == (ID_BUTTON+2))//"1/x"
						{
							calculating=0;
							offset=0;
							data1=data2;
							data2=1/data2;
							num(data2,text2);
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1),text2);
							
							num(data1,text1);
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0),text1);
							/* 功能键处理完要退出 */
							break;
						}
						if(Id == (ID_BUTTON+3))//"/"
						{
							calculating=1;
							floaton=0;
							data1=data2;
							text2[0]='\0';
							cal='/';
							num(data1,text1);
							sprintf(text1,"%s%s",text1,"/");
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0),text1);
							/* 功能键处理完要退出 */
							break;
						}
						if(Id == (ID_BUTTON+7))//"*"
						{
							calculating=1;
							floaton=0;
							data1=data2;
							text2[0]='\0';
							cal='*';
							num(data1,text1);
							sprintf(text1,"%s%s",text1,"*");
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0),text1);
							/* 功能键处理完要退出 */
							break;
						}
						if(Id == (ID_BUTTON+11))//"-"
						{
							calculating=1;
							floaton=0;
							data1=data2;
							text2[0]='\0';
							cal='-';
							num(data1,text1);
							sprintf(text1,"%s%s",text1,"-");
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0),text1);
							/* 功能键处理完要退出 */
							break;
						}
						if(Id == (ID_BUTTON+15))//"+"
						{
							calculating=1;
							floaton=0;
							data1=data2;
							text2[0]='\0';
							cal='+';
							num(data1,text1);
							sprintf(text1,"%s%s",text1,"+");
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0),text1);
							/* 功能键处理完要退出 */
							break;
						}
						if(Id == (ID_BUTTON+18))//"="
						{
							calculating=0;
							floatoffset=0;
							offset=0;
							floaton=0;
							switch(cal)
							{
								case '+':
									data2=data1+data2;
									break;
								case '-':
									data2=data1-data2;
									break;
								case '*':
									data2=data1*data2;
									break;
								case '/':
									data2=data1/data2;
									break;
								default:break;
							}
							num(data2,text2);						
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1),text2);
							text1[0]='\0';
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT0),text1);
							/* 功能键处理完要退出 */
							break;
						}
						s_Key = ButtonData[Id - ID_BUTTON].acLabel[0];
						if(s_Key=='.')
						{
							if(floaton)break;
							if(offset==0)
							{
								offset=2;
								text2[0]='0';
								text2[1]='.';
								text2[2]='\0';
							}
							else
							{
								offset++;
								sprintf(text2,"%s%s",text2,".");
							}
							text2[19]='\0';//防止超出范围
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1),text2);
							floaton=1;
							break;
						}
						if((s_Key>='0')&&(s_Key<='9'))
						{
							if(s_Key=='0' && data2==0 && offset==0)break;		//最开始状态，按‘0’无效					
							if(offset==0)
								sprintf(text2,"%d",s_Key-48);
							else
								sprintf(text2,"%s%d",text2,s_Key-48);
							offset++;
							if(floaton)
							{
								floatoffset++;
							}
							data2=atof(text2);
							text2[19]='\0';//防止超出范围
							TEXT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_TEXT1),text2);
							break;
						}
					}
					break;
			}
			break;
	}
}
/*
*********************************************************************************************************
*	函 数 名: MainAPP
*	功能说明: GUI主函数 
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
WM_HWIN CreateCalculator(void);
WM_HWIN CreateCalculator(void)
{
//	APP_DEBUG("Calculatorapp create\n");
    WM_HWIN hWin;

	hWin = GUI_CreateDialogBox(_aDialogCreateCal, GUI_COUNTOF(_aDialogCreateCal), _cbDialogCal, WM_HBKWIN, 0, 0);
	return hWin;
}

