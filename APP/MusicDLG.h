#ifndef	_MUSIC_DLG_H_
#define	_MUSIC_DLG_H_

//#include "sys.h"

typedef struct musiclist MUSIC_LIST;
struct musiclist {
    char *music_name;//显示的名称
    char *music_path;//歌曲路径
    char blrc;//是否有歌词文件
    char *lrc_path;//歌词路径
    MUSIC_LIST *nextMusic;//下一曲
    MUSIC_LIST *preMusic;//上一曲
};


#endif
