#include "ExAppRun.h"
#include "DIALOG.h"
#include "EmWinHZFont.h"//teng
#include "ff.h"
#include "malloc.h"
#include "sram.h"

//假函数,让PC指针跑到新的main函数去
dummyfun jump2app;		
//代码存放地址
extern u8 mem2base[MEM2_MAX_SIZE];	

  
//写入标志值
//val:标志值
void exeplay_write_appmask(u16 val)
{
  	RCC->APB1ENR|=1<<28;  	//使能电源时钟	    
	RCC->APB1ENR|=1<<27; 	//使能备份时钟	    
	PWR->CR|=1<<8;      	//取消备份区写保护 
	BKP->DR2=val;			//标记要执行外部程序
}
//在主函数最开始的时候被调用.
//检测是否有app程序需要执行.如果是,则直接执行.
void exeplay_app_check(void)
{
	if(BKP->DR2==0X5050) { //检查DR2,如果为0X5050,则说明需要执行app代码
		exeplay_write_appmask(0X0000);	//写入0,防止复位后再次执行app代码.
		FSMC_SRAM_Init();				//初始化SRAM,因为需要从外部sram拷贝数据到内部sram
		mymemcpy((u8*)EXEPLAY_APP_BASE,(u8*)EXEPLAY_SRC_BASE,EXEPLAY_APP_SIZE);//拷贝EXEPLAY_APP_SIZE字节 
		jump2app=(dummyfun)*(vu32*)(EXEPLAY_APP_BASE+4);	//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(vu32*)EXEPLAY_APP_BASE);			 		//初始化APP堆栈指针(用户代码区的第一个字用于存放堆栈地址)
		jump2app();	  										//执行app代码.
	}
}  
//运行器   
u8 exe_play(u8 *path)
{
  	FIL* f_exe;		 
	DIR exeplaydir;		//exeplaydir专用	  
	FILINFO exeplayinfo;	 	   
	u8 res;
	u8 rval=0;			//返回值	  
	u8 *pname=0;
	u8 *fn;

	//为长文件名申请缓存区
 	exeplayinfo.lfsize = _MAX_LFN * 2 + 1;
    exeplayinfo.lfname = mymalloc(SRAMIN, exeplayinfo.lfsize);
	if(exeplayinfo.lfname==NULL) {
        rval=1;//申请内存失败
    } else {
        mymemset((u8*)exeplayinfo.lfname,0,exeplayinfo.lfsize);
    }
	f_exe=(FIL *)mymalloc(SRAMIN, sizeof(FIL));	//开辟FIL字节的内存区域 
	if(f_exe==NULL) {
        rval=1;//申请失败
    }
	
   	while(rval==0) {
        fn=(u8*)(*exeplayinfo.lfname?exeplayinfo.lfname:exeplayinfo.fname);
        pname=mymalloc(SRAMIN, strlen((const char*)fn)+strlen((const char*)path)+2);//申请内存
        if(pname==NULL) {
            rval=1;//申请失败
        } else {//by wzteng
            pname = path;
            rval=f_open(f_exe,(const TCHAR*)pname,FA_READ);	//只读方式打开文件
            if(rval) {
//                    MESSAGEBOX_Create_User_Modal("2", "提示");
                break;	//打开失败
            }                    
            if(f_exe->fsize<MEM2_MAX_SIZE) {//可以放的下,并且用户确定执行
                UINT br;
                mymemset(mem2base,0,f_exe->fsize);	//清空这片内存
                rval=f_read(f_exe,mem2base,f_exe->fsize,(UINT*)&br);//读出BIN的所有内容       
                if(rval) {
//                        MESSAGEBOX_Create_User_Modal("3", "提示");
                    break;//打开失败,直接退出 
                }                        
                if(((*(vu32*)(mem2base+4))&0xFF000000)==0x20000000) { //判断是否为0X20XXXXXX.检测APP的合法性。
                    exeplay_write_appmask(0X5050);	//写入标志字,标志有app需要运行
                    Sys_Soft_Reset();				//产生一次软复位
                } else { //非法APP文件 
//                    MESSAGEBOX_Create_User_Modal("验证失败", "提示");
                    rval = 255;
                    break;
                }
            } 
        } 
//        myfree(SRAMIN, pname);	//释放内存		  
	}	
 	myfree(SRAMIN, pname);			//释放内存		  
 	myfree(SRAMIN, exeplayinfo.lfname);
	myfree(SRAMIN, f_exe);					   
	return rval;   
}
