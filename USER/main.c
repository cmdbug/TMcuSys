#include "led.h"
#include "beep.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "ILI93xx.h"
#include "usart.h"	 
#include "24cxx.h"
#include "touch.h"
#include "sram.h"
#include "timer.h"
#include "rtc.h"//teng
#include "sdio_sdcard.h"
#include "mmc_sd.h"//teng
#include "spi.h"
#include "malloc.h"
#include "GUI.h"
#include "ff.h"
#include "exfuns.h"
#include "w25qxx.h"
#include "fontupd.h"
#include "textfont.h"//teng
#include "EmWinHZFont.h"
#include "pngdisplay.h"
#include "gifdisplay.h"
#include "bmpdisplay.h"
#include "jpegdisplay.h"
#include "includes.h"
#include "WM.h"
#include "DIALOG.h"
#include "GUIDemo.h"//teng
#include "app.h"//teng
#include "ExAppRun.h"
#include "common.h"
#include "tpad.h"
#include "adxl345.h"
#include "rda5820.h"
#include "vs10xx.h"
#include "audiosel.h"
#include "dht11.h"
#include "remote.h"
#include "24l01.h"
#include "MusicDLG.h"
#include "mp3player.h"
/************************************************

************************************************/

//START任务
//设置任务的优先级
#define START_TASK_PRIO				03
//任务堆栈大小 
#define START_STK_SIZE			    128
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void StartTask(void *p_arg);


//设置任务优先级
#define GUI_TASK_PRIO			15
//任务堆栈大小
#define GUI_STK_SIZE			1280
//任务控制块
OS_TCB GUITaskTCB;
//任务堆栈
CPU_STK GUI_TASK_STK[GUI_STK_SIZE];
//task任务
void GUITask(void *p_arg);


//LED任务
//设置任务优先级
#define LED_TASK_PRIO 				25
//任务堆栈大小
#define LED_STK_SIZE				64
//任务控制块
OS_TCB LedTaskTCB;
//任务堆栈
CPU_STK LED_TASK_STK[LED_STK_SIZE];
//led任务
void LedTask(void *p_arg);


//设置任务优先级
#define GUIKEYPAD_TASK_PRIO			26
//任务堆栈大小
#define GUIKEYPAD_STK_SIZE			256
//任务控制块
OS_TCB GUIKEYPADTaskTCB;
//任务堆栈
CPU_STK GUIKEYPAD_TASK_STK[GUIKEYPAD_STK_SIZE];
//task任务
void GUIKEYPADTask(void *p_arg);


//设置任务优先级
#define PRTSC_TASK_PRIO			27
//任务堆栈大小
#define PRTSC_STK_SIZE			192
//任务控制块
OS_TCB PRTSCTaskTCB;
//任务堆栈
CPU_STK PRTSC_TASK_STK[PRTSC_STK_SIZE];
//task任务
void PRTSCTask(void *p_arg);


//设置任务优先级
#define MUSIC_TASK_PRIO			13
//任务堆栈大小
#define MUSIC_STK_SIZE			128
//任务控制块
OS_TCB MUSICTaskTCB;
//任务堆栈
CPU_STK MUSIC_TASK_STK[MUSIC_STK_SIZE];
//task任务
void MUSICTask(void *p_arg);



//外部内存测试(最大支持1M字节内存测试)
//返回值:0,成功;1,失败.
u8 system_exsram_test(u16 x,u16 y)
{  
	u32 i=0;  	  
	u16 temp=0;	   
	u16 sval=0;	//在地址0读到的数据	  				   
  	LCD_ShowString(x,y,239,y+12,12,(void *)"Ex Memory:   0KB", 1, POINT_COLOR); 
	//每隔1K字节,写入一个数据,总共写入1024个数据,刚好是1M字节
	for(i=0;i<1024*128;i+=8192)
	{
		FSMC_SRAM_WriteBuffer((u8*)&temp,i,2);
		temp++;
	}
	//依次读出之前写入的数据,进行校验
 	for(i=0;i<1024*128;i+=8192) 
	{
  		FSMC_SRAM_ReadBuffer((u8*)&temp,i,2);
		if(i==0)sval=temp;
 		else if(temp<=sval)break;//后面读出的数据一定要比第一次读到的数据大.        
		LCD_ShowxNum(x+10*6,y,(u16)(temp-sval+1),4,12,0, GBLUE);//显示内存容量  
 	}
	if(i>=1024*128)
	{
		LCD_ShowxNum(x+10*6,y,i/128,4,12, 0, GBLUE);//显示内存值  		
		return 0;//内存正常,成功
	}
	return 1;//失败
}

//显示错误信息
//x,y:坐标.err:错误信息
void system_error_show(u16 x,u16 y,u8*err)
{
 	while(1) {
		LCD_ShowString(x,y,240,320,12,err, 0, RED);
		delay_ms(400);
		LCD_Fill(x,y,240,y+18,BACK_COLOR);
		delay_ms(100);
		LED0=!LED0;
	} 
}	

u8 DHT11_CHECK_OK = 0;
int main(void)
{	
	OS_ERR err;
	CPU_SR_ALLOC();
    
    u16 ypos=0;
	u8 *version=0; 
	u8 verbuf[12];
    u8 j=0;
    const u8 okoffset=216-12+3;//186
    u16 temp=0;
    u32 dtsize=0,dfsize=0;
    u8 res=0;
    u8 *stastr=0;
    
    Stm32_Clock_Init(9);		//设置时钟,72Mhz 
	delay_init(72);	    	//延时函数初始化
    
    exeplay_app_check();	//==========检测是否需要运行APP代码

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
 	LED_Init();			    //LED端口初始化
	TFTLCD_Init();			//LCD初始化
    LCD_PWM_Init(15);
	TP_Init();				//触摸屏初始化
	KEY_Init();	 			//按键初始化
	BEEP_Init();			//初始化蜂鸣器
    DHT11_CHECK_OK = !DHT11_Init();
    Remote_Init();          //有使用了TIM4
    Audiosel_Init();        //音频输出选择
    Audiosel_Set(AUDIO_NONE);
	TIM3_Int_Init(999, 71);	//1KHZ 定时器1ms      teng
	FSMC_SRAM_Init();		//初始化SRAM
	my_mem_init(SRAMIN); 	//初始化内部内存池
	my_mem_init(SRAMEX);  	//初始化外部内存池
    
    // 显示开机信息
//    LCD_Clear(BLUE);
    ypos=2;
	app_show_mono_icos(11,ypos,56,24,(u8*)APP_WZTENG_ICO,YELLOW,BLACK);
    POINT_COLOR=WHITE;
	BACK_COLOR=BLACK;
    LCD_ShowString(72,ypos+12*j++,240,320,12, (void *)">> WZTENG << STM32F103ZET6", 1, YELLOW);
	LCD_ShowString(72,ypos+12*j++,240,320,12, (void *)"TMcuSystem welcome for you", 1, YELLOW);
    app_get_version(verbuf,HARDWARE_VERSION,2);
    version=mymalloc(SRAMIN,31);//申请31个字节内存
	strcpy((char*)version,"HARDWARE:");
	strcat((char*)version,(const char*)verbuf);
	strcat((char*)version,"  SOFTWARE:");
	app_get_version(verbuf,SOFTWARE_VERSION,3);
	strcat((char*)version,(const char*)verbuf);
	LCD_ShowString(5,ypos+12*j++,240,320,12,version, 1, POINT_COLOR);
    myfree(SRAMIN, version);
    LCD_ShowString(5,ypos+12*j++,240,320,12, (void *)"MCU:STM32F103ZET6 72MHz", 1, POINT_COLOR);
	LCD_ShowString(5,ypos+12*j++,240,320,12, (void *)"FLASH:512KB  SRAM:64KB", 1, POINT_COLOR);
    sprintf((char*)verbuf,"uCOS %d.%02d.%02d",OS_VERSION/10000, (OS_VERSION/100)%100, OS_VERSION%100);
    LCD_ShowString(5+6*24+15,ypos+12*(j-2),240,320,12, verbuf, 1, POINT_COLOR);
    sprintf((char*)verbuf,"emWin %d.%02d.%02d",GUI_VERSION/10000, (GUI_VERSION/100)%100, GUI_VERSION%100);
    LCD_ShowString(5+6*24+9,ypos+12*(j-1),240,320,12, verbuf, 1, POINT_COLOR);
    if (lcddev.id==0x9341) {   //9341    37697
		sprintf((char*)verbuf,"LCD ID:ILI%04X",lcddev.id);
	} else {
		sprintf((char*)verbuf,"LCD ID:%04X",lcddev.id);		//LCD ID打印到verbuf里面
	}
	LCD_ShowString(5,ypos+12*j,240,320,12, verbuf, 1, POINT_COLOR);	//显示LCD ID
    // 启动方式
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET) {//这是上电复位
        LCD_ShowString(200,ypos+12*j++,240,320,12, (u8 *)"PORRST", 1, POINT_COLOR);
    } else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET) {//这是外部RST管脚复位
        LCD_ShowString(200,ypos+12*j++,240,320,12, (u8 *)"PINRST", 1, POINT_COLOR);
    } else if (RCC_GetFlagStatus(RCC_FLAG_SFTRST)!= RESET) {//这是软件复位
        LCD_ShowString(200,ypos+12*j++,240,320,12, (u8 *)"SFTRST", 1, RED);
    } else if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST)!= RESET) {//这是低电量复位
        LCD_ShowString(194,ypos+12*j++,240,320,12, (u8 *)"LPWRRST", 1, YELLOW);
    }        
    RCC_ClearFlag();//清除RCC中复位标志
    
    // 空一行
    j++;
    if(system_exsram_test(5,ypos+12*j)) {
        system_error_show(5,ypos+12*j++,(void *)"EX Memory Error!");
    }
	LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12,(void *)"PASS", 1, POINT_COLOR);			   
	my_mem_init(SRAMEX);	//外部内存池的初始化必须放到内存检测之后！！
    
    W25QXX_Init();	//W25Qxx初始化
    res = W25QXX_ReadID();
	switch(res) {	
		case 0x13: temp=1*1024; break;
		case 0x14: temp=2*1024; break;
		case 0x15: temp=4*1024; break;
		case 0x16: temp=8*1024; break;
		default: system_error_show(5,ypos+12*j++,(void *)"Ex Flash Error!");
			break;
	}
	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"Ex Flash:    KB", 1, POINT_COLOR);			   
	LCD_ShowxNum(5+9*6,ypos+12*j,temp,4,12,0, GBLUE);//显示flash大小  
	LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);
    //SD Card
    LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"SD Card Check...", 1, POINT_COLOR);
#if SD_MODE == 0
    res = SD_Init();	//初始化SD卡SDIO
#else
    res = SD_Initialize();  //初始化SD卡SPI
#endif
    if (res == 0) {
        LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);
    } else {
        system_error_show(5,ypos+12*(j+1),(void *)"SD Card Init Error!");
    }
    
    // FATFS
    LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"FATFS Check...", 1, POINT_COLOR);//FATFS检测			   
	if(exfuns_init()) {
        system_error_show(5,ypos+12*(j+1),(void *)"FATFS Memory Error!");//内存分配
    }
	f_mount(fs[0],"0:",1);	//挂载SD卡
	f_mount(fs[1],"1:",1);	//挂载FLASH
	LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);
    //SD卡大小检测
    LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"SD Card:     MB", 1, POINT_COLOR);
    if (res == 0) {
        temp=0;	
        do {
            temp++;
            res=exf_getfree((void *)"0:",&dtsize,&dfsize);//得到SD卡剩余容量和总容量
            delay_ms(200);		   
        } while (res&&temp<2);//连续检测2次
        if (res==0) { //得到容量正常
            temp=dtsize>>10;//单位转换为MB
            stastr=(void *)"PASS";
        } else {
            temp=0;//出错了,单位为0
            stastr=(void *)"ERROR";
            system_error_show(5,ypos+12*(j+1),(void *)"SD Card Status Error!");
        }
        LCD_ShowxNum(5+8*6,ypos+12*j,temp,5,12,0, GBLUE);				//显示SD卡容量大小
        LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12,stastr, 1, POINT_COLOR);	//SD卡状态
    } else {
        temp = 0;
        stastr=(void *)"ERROR";
        system_error_show(5,ypos+12*(j+1),(void *)"SD Card Init Error!");
//        LCD_ShowxNum(5+8*6,ypos+12*j,temp,5,12,0, POINT_COLOR);				//显示SD卡容量大小
//        LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12,stastr, 1, POINT_COLOR);	//SD卡状态
    }
    //W25Q64检测,如果不存在文件系统,则先创建.
	temp=0;	
 	do {
		temp++;
 		res=exf_getfree((void *)"1:",&dtsize,&dfsize);//得到FLASH剩余容量和总容量
		delay_ms(200);		   
	} while (res&&temp<2);//改为2次,连续检测20次		  
	if (res==0X0D) { //文件系统不存在
		LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"Flash Disk Formatting...", 1, POINT_COLOR);	//格式化FLASH
		res=f_mkfs((void *)"1:",1,4096);//格式化FLASH,1,盘符;1,不需要引导区,8个扇区为1个簇
		if(res==0) {
			LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);//标志格式化成功
 			res=exf_getfree((void *)"1:",&dtsize,&dfsize);//重新获取容量
		}
	}   
	if (res==0) { //得到FLASH卡剩余容量和总容量
		LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"Flash Disk:    KB", 1, POINT_COLOR);//FATFS检测			   
		temp=dtsize; 	   
 	} else {
        system_error_show(5,ypos+12*(j+1),(void *)"Flash Fat Error!");	//flash 文件系统错误
    }        
 	LCD_ShowxNum(5+11*6,ypos+12*j,temp,4,12, 0, GBLUE);						//显示容量大小
	LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12,(void *)"PASS", 1, POINT_COLOR);			//状态
    //TPAD检测		 
 	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"TPAD Check...", 1, POINT_COLOR);			   
 	if (TPAD_Init(36)) {
        system_error_show(5,ypos+12*(j+1),(void *)"TPAD Error!");//触摸按键检测
    } else {
        LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);
    }
    //RTC检测
  	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"RTC Check...", 1, POINT_COLOR);			   
 	if (RTC_Init()) {
        system_error_show(5,ypos+12*(j+1),(void *)"RTC Error!");//RTC检测
    } else {
        LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);
    }
    //ADXL345检测
	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"ADXL345 Check...", 1, POINT_COLOR);			   
 	if (ADXL345_Init()) {
        system_error_show(5,ypos+12*(j+1),(void *)"ADXL345 Error!");//ADXL345检测
    } else {
        LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);
    }
    //AT24C02检测
   	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"AT24C02 Check...", 1, POINT_COLOR);			   
 	if (AT24CXX_Check()) {
        system_error_show(5,ypos+12*(j+1),(void *)"AT24C02 Error!");//AT24C02检测
    } else {
        LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);
    }
    //RDA5820检测			   
 	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"RDA5820 Check...", 1, POINT_COLOR);			   
 	if (RDA5820_Init()) {
        system_error_show(5,ypos+12*(j+1), (void *)"RDA5820 Error!");//RDA5820检测
    } else {
        LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);
    }
    //VS1053检测
 	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"VS1053 Check...", 1, POINT_COLOR);			   
	VS_Init();		//初始化VS1053接口
	VS_Sine_Test();	//正弦测试
	BEEP=0;		   
	if (VS_Ram_Test()!=0X83FF) {
        system_error_show(5,ypos+12*(j+1),(void *)"VS1053 Error!");//RAM测试
    } else {
        LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);
    }
    //字库检测									    
   	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"Font Check...", 1, POINT_COLOR);
	res=KEY_Scan(1);//检测按键			   
  	while(font_init()||res==KEY_DOWN) {	//检测字体,如果字体不存在/按下KEY1,则更新字库
		res=0;//按键无效
 		if(update_font(5,ypos+12*j,12,0)!=0) { //从SD卡更新
 			if(update_font(5,ypos+12*j,12,1)!=0) {//从FLASH更新
                system_error_show(5,ypos+12*(j+1), (void *)"Font Error!");	//字体错误
            }
		}	 
		LCD_Fill(5,ypos+12*j,240,ypos+12*(j+1),BLACK);//填充底色
    	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"Font Check...", 1, POINT_COLOR);			   
 	} 
	LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);//字库检测OK
    //触摸屏检测 
	LCD_ShowString(5,ypos+12*j,240,320,12, (void *)"Touch Check...", 1, POINT_COLOR);			   
	res=KEY_Scan(1);//检测按键			   
	if(TP_Init()||res==KEY_RIGHT) { //有更新/按下了KEY0,执行校准	 	
		if(res==1) {
            TP_Adjust();
        }
		res=0;//按键无效
//		goto REINIT;				//重新开始初始化
	}
	LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, (void *)"PASS", 1, POINT_COLOR);//触摸屏检测OK
//    //默认开发板系统文件检测
//    LCD_ShowString(5,ypos+12*j,240,320,12, "SYSTEM Files Check...", 1, POINT_COLOR);			   
//    while(app_system_file_check()) { //系统文件检测
//        LCD_Fill(5,ypos+12*j,240,ypos+12*(j+1),BLACK);		//填充底色
//        LCD_ShowString(5,ypos+12*j,6*8,12,12, "Updating", 1, POINT_COLOR); 	//显示updating	
//        app_boot_cpdmsg_set(5,ypos+12*j);//设置到坐标
//        if(app_system_update(app_boot_cpdmsg)) { //更新出错
//            system_error_show(5,ypos+12*(j+1), "SYSTEM File Error!");
//        }
//        LCD_Fill(5,ypos+12*j,240,ypos+12*(j+1),BLACK);//填充底色
//        LCD_ShowString(5,ypos+12*j,240,320,12, "SYSTEM Files Check...", 1, POINT_COLOR); 
//        if(app_system_file_check()) { //更新了一次，再检测，如果还有不全，说明SD卡文件就不全！
//            system_error_show(5,ypos+12*(j+1),"SYSTEM File Lost!");
//        } else {
//            break;
//        }            
//    }
//    LCD_ShowString(5+okoffset,ypos+12*j++,240,320,12, "OK", 1, POINT_COLOR);
    // 开机次数
    dtsize = AT24CXX_ReadLenByte(0, 2);
    AT24CXX_WriteLenByte(0, ++dtsize, 2);
    LCD_ShowxNum(206, 308, dtsize, 5, 12, 0x00, LBBLUE);
    
    LCD_ShowString(5,308,240,320,12, (void *)">> Starting", 1, LBBLUE);
    for (j=0;j<22;j++) {
        LCD_ShowString(5+6*11+j*6,308,240-12*11,320,12, (void *)".", 1, LBBLUE);
        delay_ms(50);
    }
    
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"StartTask", 		//任务名字
                 (OS_TASK_PTR )StartTask, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while(1);
}


//开始任务函数
void StartTask(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
	OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif

#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC,ENABLE);//开启CRC时钟
	WM_SetCreateFlags(WM_CF_MEMDEV);
    WM_MOTION_Enable(1);
    WM_MULTIBUF_Enable(1);
	GUI_Init();  			//STemWin初始化

	OS_CRITICAL_ENTER();	//进入临界区
	//UI任务	
	OSTaskCreate((OS_TCB*     )&GUITaskTCB,		
				 (CPU_CHAR*   )"GUITask", 		
                 (OS_TASK_PTR )GUITask, 			
                 (void*       )0,					
                 (OS_PRIO	  )GUI_TASK_PRIO,     
                 (CPU_STK*    )&GUI_TASK_STK[0],	
                 (CPU_STK_SIZE)GUI_STK_SIZE/10,	
                 (CPU_STK_SIZE)GUI_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);
		 
	//LED任务
	OSTaskCreate((OS_TCB*     )&LedTaskTCB,		
				 (CPU_CHAR*   )"LedTask", 		
                 (OS_TASK_PTR )LedTask, 			
                 (void*       )0,					
                 (OS_PRIO	  )LED_TASK_PRIO,     
                 (CPU_STK*    )&LED_TASK_STK[0],	
                 (CPU_STK_SIZE)LED_STK_SIZE/10,	
                 (CPU_STK_SIZE)LED_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);
                 
	//keypad任务
	OSTaskCreate((OS_TCB*     )&GUIKEYPADTaskTCB,		
				 (CPU_CHAR*   )"GUIKEYPADTask", 		
                 (OS_TASK_PTR )GUIKEYPADTask, 			
                 (void*       )0,					
                 (OS_PRIO	  )GUIKEYPAD_TASK_PRIO,     
                 (CPU_STK*    )&GUIKEYPAD_TASK_STK[0],	
                 (CPU_STK_SIZE)GUIKEYPAD_STK_SIZE/10,	
                 (CPU_STK_SIZE)GUIKEYPAD_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);
				 
	//PRTSC任务
	OSTaskCreate((OS_TCB*     )&PRTSCTaskTCB,		
				 (CPU_CHAR*   )"PRTSCTask", 		
                 (OS_TASK_PTR )PRTSCTask, 			
                 (void*       )0,					
                 (OS_PRIO	  )PRTSC_TASK_PRIO,     
                 (CPU_STK*    )&PRTSC_TASK_STK[0],	
                 (CPU_STK_SIZE)PRTSC_STK_SIZE/10,	
                 (CPU_STK_SIZE)PRTSC_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);

    //MUSIC任务
	OSTaskCreate((OS_TCB*     )&MUSICTaskTCB,		
				 (CPU_CHAR*   )"MUSICTask", 		
                 (OS_TASK_PTR )MUSICTask, 			
                 (void*       )0,					
                 (OS_PRIO	  )MUSIC_TASK_PRIO,     
                 (CPU_STK*    )&MUSIC_TASK_STK[0],	
                 (CPU_STK_SIZE)MUSIC_STK_SIZE/10,	
                 (CPU_STK_SIZE)MUSIC_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,  					
                 (void*       )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR*     )&err);                 
				 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//退出临界区
}

//GUI任务
void GUITask(void *p_arg)
{
//	GUI_CURSOR_Show();
//	while(1) {
//		GUIDEMO_Main();   //emWin例程
//	}

	while (1) {
		MainAPP();
	}
}

// 优先级低统计不太准确
///////////////////////CPU使用率监视/////////////////
#define MON_DATA_SIZE 226 // 要跟 PerfMonDLG 保持一样这里就另外定义一个了
volatile I16 cpuusage[MON_DATA_SIZE];//CPU使用率
/////////////////////内部内存使用监视///////////
volatile I16 inmemusage[MON_DATA_SIZE];//内部内存
volatile I16 exmemusage[MON_DATA_SIZE];//扩展内存
volatile U8 runhour=0,runmin=0,runsec=0;         //运行时间

//LED任务
void LedTask(void *p_arg)
{
	OS_ERR err;
    int i = 0;
    int count = 0;
    runhour=0;
    runmin=0;
    runsec=0;
    
	while(1) {        
        count++;
//        if ((count%29==0) || (count%30==0)) {
//            LED_RED = !LED_RED;
//        }
        if ((count % 2) == 0) {
            cpuusage[MON_DATA_SIZE-1] = (I16)OSStatTaskCPUUsage/100;//CPU
            for(i=1;i<MON_DATA_SIZE;i++) {
                cpuusage[i-1] = cpuusage[i];
            }
            inmemusage[MON_DATA_SIZE-1] = my_mem_perused(0);//内部内存
            for(i=1;i<MON_DATA_SIZE;i++) {
                inmemusage[i-1] = inmemusage[i];
            }
            exmemusage[MON_DATA_SIZE-1] = my_mem_perused(1);//扩展内存
            for(i=1;i<MON_DATA_SIZE;i++) {
                exmemusage[i-1] = exmemusage[i];
            }
            // 运行时间
            runsec++;
            if(runsec==60) {
                runsec = 0;
                runmin++;
                if(runmin==60) {
                    runmin = 0;
                    runhour++;
                }
            }
        }
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_PERIODIC, &err);//延时500ms
	}
}

extern keyflag;  //teng
static void GUIKEYPADTask(void *p_arg)
{
	OS_ERR   err;
	uint16_t *msg;
	OS_MSG_SIZE	msg_size;
	CPU_TS			ts;
	(void)p_arg;		/* 避免编译器告警 */	
	while (1) {
		msg = OSTaskQPend(0,
						OS_OPT_PEND_BLOCKING,
						&msg_size,
						&ts,
						&err);
		if(err == OS_ERR_NONE) {
			OSSchedLock(&err);    //teng
			//printf("pend size=%d,hedit[0]=%d,hedit[1]=%d\n",msg_size,msg[0],msg[1]);
			keypad(msg);
			keyflag = 1;          //teng
			OSSchedUnlock(&err);  //teng
		}
        OSTimeDlyHMSM(0, 0, 0, 200, OS_OPT_TIME_PERIODIC, &err);//延时500ms
    }	
}

/*
*********************************************************************************************************
*	函 数 名: _WriteByte2File()
*	功能说明: 
*	形    参：  	
*	返 回 值: 
*********************************************************************************************************
*/
FIL Prtscfile;
void _WriteByte2File(uint8_t Data, void * p) 
{
	FRESULT result;
	result = f_write(p, &Data, 1, &bw);
}

/*
*********************************************************************************************************
*	函 数 名: PRTSCTask
*	功能说明: 
*	形    参：p_arg 是在创建该任务时传递的形参
*	返 回 值: 无
	优 先 级：3
*********************************************************************************************************
*/
static void PRTSCTask(void *p_arg)
{
	OS_ERR err;
	CPU_TS Ts;
	char buf[35];
	FRESULT result;
	(void)p_arg;
//	f_mkdir("0:/PRTSC");
	while(1) {	
		if(KEY_Scan(0) == KEY1_PRES) {
            LED_GREEN = 0;
			sprintf(buf,"0:/PRTSC/PRTSC%02d%02d%02d%02d%02d%02d.bmp",
						calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
			OSSchedLock(&err);
			/* 创建截图 */
			result = f_open(&Prtscfile, buf, FA_WRITE|FA_CREATE_ALWAYS);
			/* 向SD卡绘制BMP图片 */
			GUI_BMP_Serialize(_WriteByte2File, &Prtscfile);
			
			/* 创建完成后关闭file */
			result = f_close(&Prtscfile);
			OSSchedUnlock(&err);
            LED_GREEN = 1;
			BEEP=1;
			OSTimeDlyHMSM(0, 0, 0, 120, OS_OPT_TIME_HMSM_STRICT, &err);
			BEEP=0;
		}
		OSTimeDlyHMSM(0, 0, 0, 30, OS_OPT_TIME_HMSM_STRICT, &err);
	}   
}

//任务
extern MUSIC_LIST *music;
extern char musicPlaying;
extern char musicChange;
extern int musicIndex;
void MUSICTask(void *p_arg)
{
	OS_ERR err;
    u8 res=0;
    uint16_t *msg;
    OS_MSG_SIZE	msg_size;
	CPU_TS ts;

//    OS_TaskSuspend((OS_TCB*)&MUSICTaskTCB,&err);
    VS_HD_Reset();
    VS_Soft_Reset();
    vsset.mvol=190;	
    VS_Set_Vol(vsset.mvol);
    Audiosel_Set(AUDIO_MP3);
//    mp3_play_song((u8 *)"0:/MUSIC/千年等一回.mp3");
	while(1) {
        if (musicPlaying && music && music->music_path) {
            res = mp3_play_song((u8 *)music->music_path);
            if (musicChange) {
                musicChange = 0;
                musicPlaying = 1;
            }
            if (res == 0 && music && music->nextMusic) {
                music = music->nextMusic;
                musicIndex++;
            } else if (res == 0 && music && (music->nextMusic == 0)) {
                musicPlaying = 0;
            }
        } else {
//            msg = OSTaskQPend(0,
//						OS_OPT_PEND_BLOCKING,
//						&msg_size,
//						&ts,
//						&err);
//            if (err == OS_ERR_NONE) {
//                OSSchedLock(&err);    //teng
//                //printf("pend size=%d,hedit[0]=%d,hedit[1]=%d\n",msg_size,msg[0],msg[1]);
//                OSSchedUnlock(&err);  //teng
//            }
        }
        OSTimeDlyHMSM(0, 0, 0, 500, OS_OPT_TIME_PERIODIC, &err);
	}
}

//硬件错误提示
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
    u32 temp;
    int x = 0;
    u8 sbuff[50]={0};
    
//    temp=SCB->CFSR;					//fault状态寄存器(@0XE000ED28)包括:MMSR,BFSR,UFSR
//    printf("CFSR:%8X\r\n",temp);	//显示错误值
//    temp=SCB->HFSR;					//硬件fault状态寄存器
//    printf("HFSR:%8X\r\n",temp);	//显示错误值
//    temp=SCB->DFSR;					//调试fault状态寄存器
//    printf("DFSR:%8X\r\n",temp);	//显示错误值
//    temp=SCB->AFSR;					//辅助fault状态寄存器
//    printf("AFSR:%8X\r\n",temp);	//显示错误值
    
    LCD_Clear(BLACK);
    LCD_Fill(0, 0, 240, 16, 0x6000);
    LCD_ShowString(20,16*x++,240,16,16,(void *)"=== HardFault_Handler ===", 1, WHITE);

//    sprintf((char *)sbuff, "SCB->CPUID:%d", SCB->CPUID);
//    LCD_ShowString(2,16*x++,240,16,16,sbuff, 1, WHITE);
    sprintf((char *)sbuff, "SCB->CFSR:%d", SCB->CFSR);
    LCD_ShowString(2,16*x++,240,16,16,sbuff, 1, WHITE);
    sprintf((char *)sbuff, "SCB->HFSR:%d", SCB->HFSR);
    LCD_ShowString(2,16*x++,240,16,16,sbuff, 1, WHITE);
    sprintf((char *)sbuff, "SCB->DFSR:%d", SCB->DFSR);
    LCD_ShowString(2,16*x++,240,16,16,sbuff, 1, WHITE);
    sprintf((char *)sbuff, "SCB->AFSR:%d", SCB->AFSR);
    LCD_ShowString(2,16*x++,240,16,16,sbuff, 1, WHITE);
//    sprintf((char *)sbuff, "SCB->ICSR:%d", SCB->ICSR);
//    LCD_ShowString(2,16*x++,240,16,16,sbuff, 1, WHITE);
//    sprintf((char *)sbuff, "SCB->VTOR:%d", SCB->VTOR);
//    LCD_ShowString(2,16*x++,240,16,16,sbuff, 1, WHITE);
    
    LCD_ShowString(2,320-16,240,16,16,(void *)">> Restart Soon...", 1, YELLOW);
    BACK_COLOR = BLACK;
    
    x = 3;//错误显示时间
	while (x>=0) {
		u32 i;
		LED_GREEN=!LED_GREEN;
        LCD_ShowNum(2+8*18, 320-16, x, 1, 16, 0, YELLOW);
        
        LED_RED=!LED_RED;
        for(i=0;i<0x550000;i++);//大约0.5s
        LED_RED=!LED_RED;
        for(i=0;i<0x550000;i++);//大约0.5s
        x--;
	}
    NVIC_SystemReset();
}




