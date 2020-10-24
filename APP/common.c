#include "common.h"
#include "ILI93xx.h"
#include "ff.h"

const u8 APP_WZTENG_ICO[168]=
{
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X01,0XC0,0X00,0X03,0X80,0X00,0X00,0X07,0XE1,0X81,
    0X87,0XE0,0X00,0X00,0X1F,0XC1,0XC3,0X83,0XF8,0X00,0X00,0X7F,0X80,0XFF,0X01,0XFE,
    0X00,0X01,0XFF,0X80,0XBD,0X01,0XFF,0X80,0X03,0XFF,0X81,0X99,0X81,0XFF,0XC0,0X07,
    0XFF,0XE3,0XFF,0XC7,0XFF,0XE0,0X0F,0XFF,0XFF,0XFF,0XFF,0XFF,0XF0,0X1F,0XFF,0XFF,
    0XFF,0XFF,0XFF,0XF8,0X3F,0XFF,0XFF,0XFF,0XFF,0XFF,0XFC,0X7F,0XFF,0XFF,0XFF,0XFF,
    0XFF,0XFE,0X7F,0XFF,0XFF,0XFF,0XFF,0XFF,0XFE,0XF0,0XFF,0XFF,0XFF,0XFF,0XFF,0X0F,
    0XC0,0X1F,0XFF,0XFF,0XFF,0XF8,0X03,0X00,0X07,0XFF,0XFF,0XFF,0XE0,0X00,0X00,0X03,
    0XFF,0XFF,0XFF,0XC0,0X00,0X00,0X03,0X83,0XFF,0XC1,0XC0,0X00,0X00,0X01,0X00,0XFF,
    0X00,0X80,0X00,0X00,0X00,0X00,0X3C,0X00,0X00,0X00,0X00,0X00,0X00,0X18,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
};
////////////////////////////////伪随机数产生办法////////////////////////////////
u32 random_seed=1;
void app_srand(u32 seed)
{
	random_seed=seed;
}
//获取伪随机数
//可以产生0~RANDOM_MAX-1的随机数
//seed:种子
//max:最大值	  		  
//返回值:0~(max-1)中的一个值 		
u32 app_get_rand(u32 max)
{			    	    
	random_seed=random_seed*22695477+1;
	return (random_seed)%max; 
}  

//m^n函数
long long gui_pow(u8 m,u8 n)
{
	long long result=1;	 
	while(n--)result*=m;    
	return result;
}

//得到数字的位数
//num:数字
//dir:0,从高位到低位.1,从低位到高位.
//返回值:数字的位数.(最大为10位)
//注:0,的位数为0位.
u8 app_get_numlen(long long num,u8 dir)
{
#define MAX_NUM_LENTH		10		//最大的数字长度
	u8 i=0,j;
	u8 temp=0;  
	if(dir==0)//从高到底
	{
		i=MAX_NUM_LENTH-1;
		while(1)
		{
			temp=(num/gui_pow(10,i))%10;
			if(temp||i==0)break;
			i--;
		}
	}else	//从低到高
	{
		j=0;
 		while(1)
		{
			if(num%10)
			{
				i=app_get_numlen(num,0);    
				return i;
			}
			if(j==(MAX_NUM_LENTH-1))break;	 
			num/=10;
			j++;
		}
	}
	if(i)return i+1;
	else if(temp)return 1;
	else return 0;	
}

//显示单色图标
//x,y,width,height:坐标及尺寸.
//icosbase:点阵位置
//color:画点的颜色.
//bkcolor:背景色
void app_show_mono_icos(u16 x,u16 y,u8 width,u8 height,u8 *icosbase,u16 color,u16 bkcolor)
{
	u16 rsize;
	u8 i,j;
	u8 temp;
	u8 t=0;
	u16 x0=x;//保留x的位置
	rsize=width/8+((width%8)?1:0);//每行的字节数
 	for(i=0;i<rsize*height;i++)
	{
		temp=icosbase[i];
		for(j=0;j<8;j++)
		{
	        if(temp&0x80)LCD_Fast_DrawPoint(x,y,color);  
			else LCD_Fast_DrawPoint(x,y,bkcolor);  	 
			temp<<=1;
			x++;
			t++;			//宽度计数器
			if(t==width)
			{
				t=0;
				x=x0;
				y++;  
				break;
			}
		}
	}				
}				   


//得到STM32的序列号
//sn0,sn1,sn2:3个固有序列号
void app_getstm32_sn(u32 *sn0,u32 *sn1,u32 *sn2)
{
	*sn0=*(vu32*)(0x1FFFF7E8);
	*sn1=*(vu32*)(0x1FFFF7EC);
	*sn2=*(vu32*)(0x1FFFF7F0);    
}
//打印SN
void app_usmart_getsn(void)
{
	u32 sn0,sn1,sn2;
	app_getstm32_sn(&sn0,&sn1,&sn2);
	printf("\r\nSerial Number:%X%X%X\r\n",sn0,sn1,sn2);
}
//得到版本信息
//buf:版本缓存区(最少需要6个字节,如保存:V1.00)
//ver:版本宏定义
//len:版本位数.1,代表只有1个位的版本,V1;2,代表2个位版本,V1.0;3,代表3个位版本,V1.00;
void app_get_version(u8*buf,u32 ver,u8 len)
{
	u8 i=0;
	buf[i++]='V';
	if(len==1)
	{
		buf[i++]=ver%10+'0';
	}else if(len==2)
	{ 	
		buf[i++]=(ver/10)%10+'0';
		buf[i++]='.';
		buf[i++]=ver%10+'0';
	}else
	{
		buf[i++]=(ver/100)%10+'0';
		buf[i++]='.';
		buf[i++]=(ver/10)%10+'0';
		buf[i++]=ver%10+'0';	 
	}
	buf[i]=0;//加入结束符	 			   
}

