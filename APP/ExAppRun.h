#ifndef	_EX_APP_RUN_H_
#define	_EX_APP_RUN_H_

#include "GUI.h"
#include "DIALOG.h"
#include "sys.h"

#define EXEPLAY_APP_SIZE	60*1024					//app代码的最大尺寸.这里为60K字节（还包括了SRAM，实际上不可能运行60K的代码）.
#define EXEPLAY_APP_BASE	0x20001000				//app执行代码的目的地址,也就是将要运行的代码存放的地址
#define	EXEPLAY_SRC_BASE	0x68000000				//app执行代码的源地址,也就是软复位之前,app代码存放的地址


typedef  void (*dummyfun)(void);	//定义一个函数类型.
extern dummyfun jump2app;			//假函数,让PC指针跑到新的main函数去
void exeplay_write_appmask(u16 val);
void exeplay_app_check(void);
u8 exe_play(u8 *path);	


#endif
