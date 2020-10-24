/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              礐/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUICharP.C
Purpose     : Implementation of Proportional fonts
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"
#include "ff.h"
#include "spi.h"
#include "fontupd.h"
#include "w25qxx.h"
#include "usart.h"
#include "sys.h"
#include "EmWin_ASCII_Font.h"
#include "ili93xx.h"
#include "string.h"
#include <stddef.h>
#include "EmWin_ASCII_Font.h"
#include "EmWinHZFont.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103战舰开发板
//STemwin 汉字显示(FLASH外置字库)   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/4/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	


//字模数据暂存数组,单个字模的最大字节数
//单个字模最大为1024
#define BYTES_PER_FONT	1024	//32*32=1024，最大显示32*32点阵字库	
U8 GUI_FontDataBuf[BYTES_PER_FONT];

//获取字模数据
static void GUI_GetDataFromMemory(const GUI_FONT_PROP GUI_UNI_PTR *pProp, U16P c)
{
	unsigned char qh,ql;
	unsigned char i;					  
    unsigned long foffset; 
	unsigned char t;
	unsigned char *mat;
    u8 size,csize;//字体大小
    U16 BytesPerFont;
	GUI_FONT EMWINFONT;
	EMWINFONT = *GUI_pContext->pAFont;
    BytesPerFont = GUI_pContext->pAFont->YSize * pProp->paCharInfo->BytesPerLine; //每个字模的数据字节数
    if (BytesPerFont > BYTES_PER_FONT) BytesPerFont = BYTES_PER_FONT;
	
	//判断字体的大小
	if(memcmp(&EMWINFONT,&GUI_FontHZ12,sizeof(GUI_FONT)) == 0) size=12;			//12字体
	else if(memcmp(&EMWINFONT,&GUI_FontHZ16,sizeof(GUI_FONT)) == 0) size=16;	//16字体
	else if(memcmp(&EMWINFONT,&GUI_FontHZ24,sizeof(GUI_FONT)) == 0)	size=24;	//24字体
		
	csize = (size/8+((size%8)?1:0))*(size);	//得到字体一个字符对应点阵集所占的字节数	
    if (c < 0x80)	//英文字符地址偏移算法
    {
		switch(size)
		{
			case 12:
				for(t=0;t<12;t++) GUI_FontDataBuf[t]=emwin_asc2_1206[c-0x20][t];
				break;
			case 16:
				for(t=0;t<16;t++) GUI_FontDataBuf[t]=emwin_asc2_1608[c-0x20][t];
				break;
//			case 24:
//				for(t=0;t<48;t++) GUI_FontDataBuf[t]=emwin_asc2_2412[c-0x20][t];
//				break;	
		} 
	}
    else                                                                          
    {
		ql=c/256;
		qh=c%256;
		if(qh<0x81||ql<0x40||ql==0xff||qh==0xff)//非常用汉字
		{   		    
			for(i=0;i<(size*2);i++) *mat++=0x00;//填充满格
			return; //结束访问
		}          
		if(ql<0x7f)ql-=0x40; 
		else ql-=0x41;
		qh-=0x81;   
		foffset=((unsigned long)190*qh+ql)*csize;//得到字库中的字节偏移量		  
		switch(size)
		{
			case 12:
				W25QXX_Read(GUI_FontDataBuf,foffset+ftinfo.f12addr,csize);
				break;
			case 16:
				W25QXX_Read(GUI_FontDataBuf,foffset+ftinfo.f16addr,csize);
				break;
			case 24:
				W25QXX_Read(GUI_FontDataBuf,foffset+ftinfo.f24addr,csize);
				break;	
		} 
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
*       GUIPROP_DispChar
*
* Purpose:
*   This is the routine that displays a character. It is used by all
*   other routines which display characters as a subroutine.
*/
void GUIPROP_X_DispChar(U16P c) 
{	
	int BytesPerLine;
    GUI_DRAWMODE DrawMode = GUI_pContext->TextMode;
    const GUI_FONT_PROP GUI_UNI_PTR *pProp = GUI_pContext->pAFont->p.pProp;
    //搜索定位字库数据信息 
    for (; pProp; pProp = pProp->pNext)                                         
    {
        if ((c >= pProp->First) && (c <= pProp->Last)) break;
    }
    if (pProp)
    {
        GUI_DRAWMODE OldDrawMode;
        const GUI_CHARINFO GUI_UNI_PTR * pCharInfo = pProp->paCharInfo;
        GUI_GetDataFromMemory(pProp, c);//取出字模数据
        BytesPerLine = pCharInfo->BytesPerLine;                
        OldDrawMode  = LCD_SetDrawMode(DrawMode);
        LCD_DrawBitmap(GUI_pContext->DispPosX, GUI_pContext->DispPosY,
                       pCharInfo->XSize, GUI_pContext->pAFont->YSize,
                       GUI_pContext->pAFont->XMag, GUI_pContext->pAFont->YMag,
                       1,     /* Bits per Pixel */
                       BytesPerLine,
                       &GUI_FontDataBuf[0],
                       &LCD_BKCOLORINDEX
                       );
        /* Fill empty pixel lines */
        if (GUI_pContext->pAFont->YDist > GUI_pContext->pAFont->YSize) 
        {
            int YMag = GUI_pContext->pAFont->YMag;
            int YDist = GUI_pContext->pAFont->YDist * YMag;
            int YSize = GUI_pContext->pAFont->YSize * YMag;
            if (DrawMode != LCD_DRAWMODE_TRANS) 
            {
                LCD_COLOR OldColor = GUI_GetColor();
                GUI_SetColor(GUI_GetBkColor());
                LCD_FillRect(GUI_pContext->DispPosX, GUI_pContext->DispPosY + YSize, 
                             GUI_pContext->DispPosX + pCharInfo->XSize, 
                             GUI_pContext->DispPosY + YDist);
                GUI_SetColor(OldColor);
            }
        }
        LCD_SetDrawMode(OldDrawMode); /* Restore draw mode */
		GUI_pContext->DispPosX += pCharInfo->XDist * GUI_pContext->pAFont->XMag;
    }
}

/*********************************************************************
*
*       GUIPROP_GetCharDistX
*/
int GUIPROP_X_GetCharDistX(U16P c) 
{
    const GUI_FONT_PROP GUI_UNI_PTR * pProp = GUI_pContext->pAFont->p.pProp;  
    for (; pProp; pProp = pProp->pNext)                                         
    {
        if ((c >= pProp->First) && (c <= pProp->Last))break;
    }
    return (pProp) ? (pProp->paCharInfo)->XSize * GUI_pContext->pAFont->XMag : 0;
}
