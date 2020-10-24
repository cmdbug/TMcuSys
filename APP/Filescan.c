#include "includes.h" //teng
#include "app.h" //teng
#include "EmWinHZFont.h"//teng
#include "ff.h"//teng
#include "malloc.h"//teng
#include "exfuns.h"//teng
//#include "treenleafimage.h"//teng


#define _DF1S	0x81
#define BROWSER_FILE_NAME_LEN 	100							//文件名长度，如果检测到文件名超过50 则丢弃这个文件

extern void setTreenFilesImage(u8 *fn,TREEVIEW_ITEM_Handle hItem,int index);
extern u8 filePathCheck(u8 *path);


/**
  * @brief  scan_files 递归扫描sd卡内的文件
  * @param  path:初始扫描路径 file_name：指向用来存储文件名的一段空间 hFile:用于记录文件路径的文件指针 hTree 目录树 hNode 目录结点
	*					hTree == NULL &&	hNode == NULL 的话，不创建目录树			
  * @retval result:文件系统的返回值
  */
static FRESULT scan_files(char* path,char* file_name,FIL *hFile,WM_HWIN hTree, TREEVIEW_ITEM_Handle hNode,int *fileNum, u8 ftype) 
{ 
    FRESULT res; 		          //部分在递归过程被修改的变量，不用全局变量	
    FILINFO fno; 
	unsigned int rw_num;			//已读或已写的字节数
//	char showname[60]={0};
    DIR dir; 
    int i; 
    char *fn; 	
	TREEVIEW_ITEM_Handle hItem=0;

#if _USE_LFN 
    static char lfn[_MAX_LFN * (_DF1S ? 2 : 1) + 1]; 	//长文件名支持
    fno.lfname = lfn; 
    fno.lfsize = sizeof(lfn); 
#endif 

    res = f_opendir(&dir, path);                            //打开目录
    if (res == FR_OK) 
	{ 
        i = strlen(path); 
        for (;;) 
		{ 
            res = f_readdir(&dir, &fno); 										//读取目录下的内容
            if (res != FR_OK || fno.fname[0] == 0) break; 	//为空时表示所有项目读取完毕，跳出
#if _USE_LFN 
            fn = *fno.lfname ? fno.lfname : fno.fname; 
#else 
            fn = fno.fname; 
#endif 
            if (*fn == '.') continue; 											//点表示当前目录，跳过			
            if (fno.fattrib & AM_DIR) 
			{ 																							//目录，递归读取
				if(hTree != NULL &&	hNode != NULL)
				{
//					gbk2utf8(fn,showname,60);
//					hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE,showname,0);
					hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_NODE,fn,0);						//目录，创建结点
					TREEVIEW_AttachItem(hTree,hItem,hNode,TREEVIEW_INSERT_FIRST_CHILD);		//把结点加入到目录树中
				}								
				sprintf(&path[i], "/%s", fn); 							//合成完整目录名
                res = scan_files(path,file_name,hFile,hTree,hItem,fileNum, ftype);		//递归遍历 
                if (res != FR_OK) 
					break; 																		//打开失败，跳出循环
                path[i] = 0; 
            } 
			else 																				//是文件
			{
				if( (f_typetell(fn)>>4) == (ftype>>4) )                                         //要查找文件类型的大类
				{
					/* 根据要求是否创建目录树 */
					if(hTree != NULL &&	hNode != NULL)												//创建目录树
					{
//						gbk2utf8(fn,showname,60);
//						hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_LEAF,showname,0);
						hItem = TREEVIEW_ITEM_Create(TREEVIEW_ITEM_TYPE_LEAF,fn,0);						//文件，创建树叶
						TREEVIEW_AttachItem(hTree,hItem,hNode,TREEVIEW_INSERT_FIRST_CHILD);		//把树叶添加到目录树
						
						setTreenFilesImage(fn,hItem,TREEVIEW_BI_LEAF);                          //设置树叶图像
					
						if (strlen(path)+strlen(fn)<BROWSER_FILE_NAME_LEN)
						{
							sprintf(file_name, "%s/%s", path,fn); 	
						}			
					}
				}
			}//else
        } //for
    } 
    return res; 
} 


/**
  * @brief  Fill_FileList处理非递归过程，然后调用递归函数scan_files扫描目录
	*					
  * @param  path:初始扫描路径
  * @retval none
  */
void Fill_FileList(char* path,WM_HWIN hTree, TREEVIEW_ITEM_Handle hNode,int *p, u8 ftype)
{
    CPU_SR_ALLOC();
	u8 result;
	char p_path[BROWSER_FILE_NAME_LEN]={0};				//目录名 指针
	char file_name[BROWSER_FILE_NAME_LEN]={0};				//用于存储的目录文件名
	strcpy(p_path,path);						//复制目录名到指针
	OS_CRITICAL_ENTER();
    result = scan_files(p_path,file_name,file,hTree,hNode,p, ftype);			//递归扫描文件		
    OS_CRITICAL_EXIT();
}



