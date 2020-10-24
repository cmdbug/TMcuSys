/*
*********************************************************************************************************
*	                                  
*	系统所需的图片数据
*
*********************************************************************************************************
*/

#include <stdlib.h>

#include "GUI.h"



//icos的路径表
const unsigned char *icos_path_tbl[3][12]=
{
	{//底部
	 	"0:/WZTSYSTEM/SPB/ICOS/phone.bin",	    
	 	"0:/WZTSYSTEM/SPB/ICOS/messages.bin",	    
		"0:/WZTSYSTEM/SPB/ICOS/browser.bin",
	 	"0:/WZTSYSTEM/SPB/ICOS/app.bin",	    
	},
	{//第一页
		"0:/WZTSYSTEM/SPB/ICOS/ebooks.bin",
	 	"0:/WZTSYSTEM/SPB/ICOS/photos.bin",	    
	  	"0:/WZTSYSTEM/SPB/ICOS/music.bin",	    
	 	"0:/WZTSYSTEM/SPB/ICOS/videos.bin",
	    
	 	"0:/WZTSYSTEM/SPB/ICOS/clock.bin",	    
	 	"0:/WZTSYSTEM/SPB/ICOS/exeplay.bin",	    
	  	"0:/WZTSYSTEM/SPB/ICOS/games.bin",	    
	 	"0:/WZTSYSTEM/SPB/ICOS/notes.bin",
	    
	 	"0:/WZTSYSTEM/SPB/ICOS/settings.bin",
		"0:/WZTSYSTEM/SPB/ICOS/recoder.bin",	    
	 	"0:/WZTSYSTEM/SPB/ICOS/radio.bin",	    
	 	"0:/WZTSYSTEM/SPB/ICOS/calculator.bin",
	},	    
 	{//第二页
	 	"0:/WZTSYSTEM/SPB/ICOS/camera.bin",	    
	 	"0:/WZTSYSTEM/SPB/ICOS/usb.bin",	    
		"0:/WZTSYSTEM/SPB/ICOS/internet.bin",
	 	"0:/WZTSYSTEM/SPB/ICOS/wireless.bin",
	    
	  	"0:/WZTSYSTEM/SPB/ICOS/wifi.bin",	    
	  	"0:/WZTSYSTEM/SPB/ICOS/compass.bin",	    
	 	"0:/WZTSYSTEM/SPB/ICOS/bluetooth.bin",
	  	"0:/WZTSYSTEM/SPB/ICOS/gps.bin",
	    
//	 	"0:/WZTSYSTEM/SPB/ICOS/calendar.bin",
//		"0:/WZTSYSTEM/SPB/ICOS/xxx.bin",	    
//	 	"0:/WZTSYSTEM/SPB/ICOS/xxx.bin",	    
//	 	"0:/WZTSYSTEM/SPB/ICOS/xxx.bin",
	},	
};

//#ifndef GUI_CONST_STORAGE
//  #define GUI_CONST_STORAGE const
//#endif


GUI_BITMAP bmphone = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acphone,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmmessages = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acmessages,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmbrowser = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acbrowser,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmapp = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acapp,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmebooks = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acebooks,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmphotos = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acphotos,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmmusic = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acmusic,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmvideos = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acvideos,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmclock = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acclock,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmexeplay = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acexeplay,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmgames = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acgames,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmnotes = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acnotes,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmsettings = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acsettings,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmrecoder = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acrecoder,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmcamera = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_accamera,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmcalculator = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_accalculator,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmradio = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acradio,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmusb = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acusb,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bminternet = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acinternet,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmwireless = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acwireless,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmwifi = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acwifi,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmcompass = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_accompass,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmbluetooth = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acbluetooth,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmgps = {
  48, // xSize
  48, // ySize
  192, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_acgps,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};


GUI_BITMAP bmlogo = {
  39, // xSize
  18, // ySize
  156, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)NULL,//_aclogo,  // Pointer to picture data
  NULL,  // Pointer to palette
  GUI_DRAW_BMP8888
};





/*************************** End of file ****************************/
