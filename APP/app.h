#ifndef	_APP_H_
#define	_APP_H_

#include "GUI.h"
#include "DIALOG.h"

#define  APP_DEBUG(format,...)	//printf("File:"__FILE__",Line:%03d:"format"\n",__LINE__,##__VA_ARGS__) 
/* 用户定义的消息 */
#define APPBKCOLOR  	GUI_YELLOW
#define APPTEXTCOLOR    GUI_MAGENTA


extern GUI_BITMAP bmphone;
extern GUI_BITMAP bmmessages;
extern GUI_BITMAP bmbrowser;
extern GUI_BITMAP bmapp;

extern GUI_BITMAP bmebooks;
extern GUI_BITMAP bmphotos;
extern GUI_BITMAP bmmusic;
extern GUI_BITMAP bmvideos;

extern GUI_BITMAP bmclock;
extern GUI_BITMAP bmexeplay;
extern GUI_BITMAP bmgames;
extern GUI_BITMAP bmnotes;

extern GUI_BITMAP bmsettings;
extern GUI_BITMAP bmrecoder;
extern GUI_BITMAP bmcamera;
extern GUI_BITMAP bmcalculator;

extern GUI_BITMAP bmradio;
extern GUI_BITMAP bmusb;
extern GUI_BITMAP bminternet;
extern GUI_BITMAP bmwireless;

extern GUI_BITMAP bmwifi;
extern GUI_BITMAP bmcompass;
extern GUI_BITMAP bmbluetooth;
extern GUI_BITMAP bmgps;

extern GUI_BITMAP bmlogo;   //GUI_CONST_STORAGE
extern const unsigned char *icos_path_tbl[3][12];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/
void MainAPP(void);
void Soft_Reset(void);
void Touch_MainTask(void);
extern __IO uint8_t app_prevent_refresh;


#endif	//_APP_H_
