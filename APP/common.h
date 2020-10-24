#ifndef __COMMON_H
#define __COMMON_H 	
#include "sys.h"
#include "touch.h"	 
#include "includes.h"


//硬件平台软硬件版本定义	   	
#define HARDWARE_VERSION	   		20		//硬件版本,放大10倍,如1.0表示为10
#define SOFTWARE_VERSION	    	629		//软件版本,放大100倍,如1.00,表示为100
//硬件V2.0
//1，增加TVS电源保护。
//2，输入电源采用DCDC电源方案代替线性稳压方案。
//硬件V2.2
//1，丝印位置稍微变化.
//2，新增二维码.
//V2.25  20121027
//增加3.5寸大分辨率LCD支持
//V2.26  20131124
//1,增加对NT35310驱动器的支持
//2,采用最新的SYSTEM文件夹,支持MDK3~MDK4.
//3,全面兼容V3.5库头文件.
//4,USMART采用最新的V3.1版本,支持函数执行时间查看.


//系统数据保存基址			  
#define SYSTEM_PARA_SAVE_BASE 		100		//系统信息保存首地址.从100开始.		    

extern const u8 APP_WZTENG_ICO[168];	//启动界面图标,存放在flash

/////////////////////////////////////////////////////////////////////////																		    
void app_srand(u32 seed);
//获取伪随机数
//可以产生0~RANDOM_MAX-1的随机数
//seed:种子
//max:最大值	  		  
//返回值:0~(max-1)中的一个值 		
u32 app_get_rand(u32 max);
//m^n函数
long long gui_pow(u8 m,u8 n);
//得到数字的位数
//num:数字
//dir:0,从高位到低位.1,从低位到高位.
//返回值:数字的位数.(最大为10位)
//注:0,的位数为0位.
u8 app_get_numlen(long long num,u8 dir);
//显示单色图标
//x,y,width,height:坐标及尺寸.
//icosbase:点阵位置
//color:画点的颜色.
//bkcolor:背景色
void app_show_mono_icos(u16 x,u16 y,u8 width,u8 height,u8 *icosbase,u16 color,u16 bkcolor);			   
//得到STM32的序列号
//sn0,sn1,sn2:3个固有序列号
void app_getstm32_sn(u32 *sn0,u32 *sn1,u32 *sn2);
//打印SN
void app_usmart_getsn(void);
//得到版本信息
//buf:版本缓存区(最少需要6个字节,如保存:V1.00)
//ver:版本宏定义
//len:版本位数.1,代表只有1个位的版本,V1;2,代表2个位版本,V1.0;3,代表3个位版本,V1.00;
void app_get_version(u8*buf,u32 ver,u8 len);


#endif



