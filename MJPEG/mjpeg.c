#include "mjpeg.h" 
#include "malloc.h"
#include "ff.h"  
#include "ili93xx.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//MJPEG视频处理 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/7/20
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   


struct jpeg_decompress_struct *cinfo;
struct my_error_mgr *jerr;
u8 *jpegbuf;			//jpeg数据缓存指针
u32 jbufsize;			//jpeg buf大小
u16 imgoffx,imgoffy;	//图像在x,y方向的偏移量


  
////////////////////////////////////////////////////////////////////////////////
//简单快速的内存分配,以提高速度
#define MJPEG_MAX_MALLOC_SIZE 		1024*35			//最大可以分配35K字节      38


u8 *jmembuf;			//mjpeg解码的 内存池
u32 jmempos;			//内存池指针

//mjpeg申请内存
void* mjpeg_malloc(u32 num)
{
	u32 curpos=jmempos; //此次分配的起始地址
 	jmempos+=num;		//下一次分配的起始地址 
	if(jmempos>38*1024) 
	{
		printf("mem error:%d,%d",curpos,num);
	}
	return (void *)&jmembuf[curpos];	//返回申请到的内存首地址
}  
////////////////////////////////////////////////////////////////////////////////
//错误退出
static void my_error_exit(j_common_ptr cinfo)
{ 
	my_error_ptr myerr=(my_error_ptr) cinfo->err; 
	(*cinfo->err->output_message) (cinfo);	 
	longjmp(myerr->setjmp_buffer, 1);	  
} 

METHODDEF(void) my_emit_message(j_common_ptr cinfo, int msg_level)
{
	my_error_ptr myerr=(my_error_ptr) cinfo->err;  
    if(msg_level<0)
	{
		printf("emit msg:%d\r\n",msg_level); 
		longjmp(myerr->setjmp_buffer, 1);		
	}
}

//初始化资源,不执行任何操作
static void init_source(j_decompress_ptr cinfo)
{
    //不需要做任何事情.
    return;
} 
//填充输入缓冲区,一次性读取整帧数据
static boolean fill_input_buffer(j_decompress_ptr cinfo)
{  
	if(jbufsize==0)//结束了
	{
		printf("jd read off\r\n");
        //填充结束符
        jpegbuf[0] = (u8) 0xFF;
        jpegbuf[1] = (u8) JPEG_EOI;
  		cinfo->src->next_input_byte =jpegbuf;
		cinfo->src->bytes_in_buffer = 2; 
	}else
	{
		cinfo->src->next_input_byte =jpegbuf;
		cinfo->src->bytes_in_buffer = jbufsize;
		jbufsize-=jbufsize;
	}
    return TRUE;
}
//在文件里面,跳过num_bytes个数据
static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{ 
    /* Just a dumb implementation for now.  Could use fseek() except
    * it doesn't work on pipes.  Not clear that being smart is worth
    * any trouble anyway --- large skips are infrequent.
    */
    if (num_bytes > 0)
    {
        while(num_bytes>(long) cinfo->src->bytes_in_buffer)
        {
            num_bytes-=(long)cinfo->src->bytes_in_buffer;
            (void)cinfo->src->fill_input_buffer(cinfo);
            /* note we assume that fill_input_buffer will never
            * return FALSE, so suspension need not be handled.
            */
        }
        cinfo->src->next_input_byte += (size_t) num_bytes;
        cinfo->src->bytes_in_buffer -= (size_t) num_bytes;
    }
} 
//在解码结束后,被jpeg_finish_decompress函数调用
static void term_source(j_decompress_ptr cinfo)
{
    //不做任何处理
    return;
}
//初始化jpeg解码数据源
static void jpeg_filerw_src_init(j_decompress_ptr cinfo)
{ 
    if (cinfo->src == NULL)     /* first time for this JPEG object? */
    {
        cinfo->src = (struct jpeg_source_mgr *)
                     (*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT,
                                              sizeof(struct jpeg_source_mgr)); 
    } 
    cinfo->src->init_source = init_source;
    cinfo->src->fill_input_buffer = fill_input_buffer;
    cinfo->src->skip_input_data = skip_input_data;
    cinfo->src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
    cinfo->src->term_source = term_source;
    cinfo->src->bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    cinfo->src->next_input_byte = NULL; /* until buffer loaded */
} 


//mjpeg 解码初始化
//offx,offy:x,y方向的偏移
//返回值:0,成功;
//       1,失败
u8 mjpegdec_init(u16 offx,u16 offy)
{
	cinfo=mymalloc(SRAMEX,sizeof(struct jpeg_decompress_struct));
	jerr=mymalloc(SRAMEX,sizeof(struct my_error_mgr));
	jmembuf=mymalloc(SRAMEX,MJPEG_MAX_MALLOC_SIZE);//MJPEG解码内存池申请
	if(cinfo==0||jerr==0||jmembuf==0)
	{
		mjpegdec_free();
		return 1;
	}
	//保存图像在x,y方向的偏移量
	imgoffx=offx;
	imgoffy=offy; 
	return 0;
}
//mjpeg结束,是否内存
void mjpegdec_free(void)
{    
	myfree(SRAMEX,cinfo);
	myfree(SRAMEX,jerr);
	myfree(SRAMEX,jmembuf);
}

//解码一副JPEG图片
//buf:jpeg数据流数组
//bsize:数组大小
//返回值:0,成功
//    其他,错误
u8 mjpegdec_decode(u8* buf,u32 bsize)
{
    JSAMPARRAY buffer;		
	if(bsize==0)return 1;
	jpegbuf=buf;
	jbufsize=bsize;	   
	jmempos=0;//MJEPG解码,重新从0开始分配
	
	cinfo->err=jpeg_std_error(&jerr->pub); 
	jerr->pub.error_exit = my_error_exit; 
	jerr->pub.emit_message = my_emit_message; 
	//if(bsize>20*1024)printf("s:%d\r\n",bsize); 
	if (setjmp(jerr->setjmp_buffer)) //错误处理
	{ 
 		jpeg_abort_decompress(cinfo);
		jpeg_destroy_decompress(cinfo); 
		return 2;
	} 
	jpeg_create_decompress(cinfo); 
	jpeg_filerw_src_init(cinfo);  
	jpeg_read_header(cinfo, TRUE); 
	cinfo->dct_method = JDCT_IFAST;
	cinfo->do_fancy_upsampling = 0;  
	jpeg_start_decompress(cinfo); 
	LCD_Set_Window(imgoffx,imgoffy,cinfo->output_width,cinfo->output_height);
	LCD_WriteRAM_Prepare();     		//开始写入GRAM	
	while (cinfo->output_scanline < cinfo->output_height) 
	{ 
		jpeg_read_scanlines(cinfo, buffer, 1);
	} 
	LCD_Set_Window(0,0,lcddev.width,lcddev.height);//恢复窗口
	jpeg_finish_decompress(cinfo); 
	jpeg_destroy_decompress(cinfo);  
	return 0;
}



