#include "remote.h"
#include "delay.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//红外遥控解码驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/12
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////


//红外遥控初始化
//设置IO以及定时器4的输入捕获
void Remote_Init(void)    			  
{									   
	RCC->APB1ENR|=1<<2;   	//TIM4 时钟使能 
	RCC->APB2ENR|=1<<3;    	//使能PORTB时钟 

	GPIOB->CRH&=0XFFFFFF0F;	//PB9 输入  
	GPIOB->CRH|=0X00000080;	//上拉输入     
	GPIOB->ODR|=1<<9;		//PB9 上拉
	  
 	TIM4->ARR=10000;  		//设定计数器自动重装值 最大10ms溢出  
	TIM4->PSC=71;  			//预分频器,1M的计数频率,1us加1.	 

	TIM4->CCMR2|=1<<8;		//CC4S=01 	选择输入端 IC4映射到TI4上
 	TIM4->CCMR2|=3<<12;  	//IC4F=0011 配置输入滤波器 8个定时器时钟周期滤波
 	TIM4->CCMR2|=0<<10;  	//IC4PS=00 	配置输入分频,不分频 

	TIM4->CCER|=0<<13; 		//CC4P=0	上升沿捕获
	TIM4->CCER|=1<<12; 		//CC4E=1 	允许捕获计数器的值到捕获寄存器中
	TIM4->DIER|=1<<4;   	//允许CC4IE捕获中断				
	TIM4->DIER|=1<<0;   	//允许更新中断				
	TIM4->CR1|=0x01;    	//使能定时器4
  	MY_NVIC_Init(1,3,TIM4_IRQn,2);//抢占1，子优先级3，组2									 
}

//遥控器接收状态
//[7]:收到了引导码标志
//[6]:得到了一个按键的所有信息
//[5]:保留	
//[4]:标记上升沿是否已经被捕获								   
//[3:0]:溢出计时器
u8 	RmtSta=0;	  	  
u16 Dval;		//下降沿时计数器的值
u32 RmtRec=0;	//红外接收到的数据	   		    
u8  RmtCnt=0;	//按键按下的次数	  
//定时器2中断服务程序	 
void TIM4_IRQHandler(void)
{ 		    	 
	u16 tsr;
	tsr=TIM4->SR;
	if(tsr&0X01)//溢出
	{
		if(RmtSta&0x80)//上次有数据被接收到了
		{	
			RmtSta&=~0X10;						//取消上升沿已经被捕获标记
			if((RmtSta&0X0F)==0X00)RmtSta|=1<<6;//标记已经完成一次按键的键值信息采集
			if((RmtSta&0X0F)<14)RmtSta++;
			else
			{
				RmtSta&=~(1<<7);//清空引导标识
				RmtSta&=0XF0;	//清空计数器	
			}						 	   	
		}							    
	}
	if(tsr&0x10)//CC4IE中断
	{	  
		if(RDATA)//上升沿捕获
		{
  			TIM4->CCER|=1<<13; 				//CC4P=1	设置为下降沿捕获
			TIM4->CNT=0;					//清空定时器值
			RmtSta|=0X10;					//标记上升沿已经被捕获
		}else //下降沿捕获
		{
			Dval=TIM4->CCR4;				//读取CCR1也可以清CC1IF标志位
  			TIM4->CCER&=~(1<<13);			//CC4P=0	设置为上升沿捕获
			if(RmtSta&0X10)					//完成一次高电平捕获 
			{
 				if(RmtSta&0X80)//接收到了引导码
				{
					
					if(Dval>300&&Dval<800)			//560为标准值,560us
					{
						RmtRec<<=1;	//左移一位.
						RmtRec|=0;	//接收到0	   
					}else if(Dval>1400&&Dval<1800)	//1680为标准值,1680us
					{
						RmtRec<<=1;	//左移一位.
						RmtRec|=1;	//接收到1
					}else if(Dval>2200&&Dval<2600)	//得到按键键值增加的信息 2500为标准值2.5ms
					{
						RmtCnt++; 		//按键次数增加1次
						RmtSta&=0XF0;	//清空计时器		
					}
 				}else if(Dval>4200&&Dval<4700)		//4500为标准值4.5ms
				{
					RmtSta|=1<<7;	//标记成功接收到了引导码
					RmtCnt=0;		//清除按键次数计数器
				}						 
			}
			RmtSta&=~(1<<4);
		}				 		     	    					   
	}
	TIM4->SR=0;//清除中断标志位 	    
}

//处理红外键盘
//返回值:
//	 0,没有任何按键按下
//其他,按下的按键键值.
u8 Remote_Scan(void)
{        
	u8 sta=0;       
    u8 t1,t2;  
	if(RmtSta&(1<<6))//得到一个按键的所有信息了
	{ 
	    t1=RmtRec>>24;			//得到地址码
	    t2=(RmtRec>>16)&0xff;	//得到地址反码 
 	    if((t1==(u8)~t2)&&t1==REMOTE_ID)//检验遥控识别码(ID)及地址 
	    { 
	        t1=RmtRec>>8;
	        t2=RmtRec; 	
	        if(t1==(u8)~t2)sta=t1;//键值正确	 
		}   
		if((sta==0)||((RmtSta&0X80)==0))//按键数据错误/遥控已经没有按下了
		{
		 	RmtSta&=~(1<<6);//清除接收到有效按键标识
			RmtCnt=0;		//清除按键次数计数器
		}
	}  
    return sta;
}



