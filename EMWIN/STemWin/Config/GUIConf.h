#ifndef GUICONF_H
#define GUICONF_H

#define WIDGET_USE_FLEX_SKIN      1   // 使用默认皮肤设置 teng

#define GUI_NUM_LAYERS            2   // 显示的最大层数

#define GUI_OS                    (1)  // 使用操作系统
#define GUI_MAXTASK				  (1)  // 最大可调用EMWIN任务数 3
#define GUI_SUPPORT_TOUCH         (1)  // 支持触摸

#define GUI_DEFAULT_FONT          &GUI_Font8x16	//默认字体
#define GUI_SUPPORT_MOUSE             (0)    	//支持鼠标
#define GUI_WINSUPPORT                (1)    	//窗口管理
#define GUI_SUPPORT_MEMDEV            (1)    	//存储设备
#define GUI_SUPPORT_DEVICES           (0)    	//使用设备指针
#endif  
