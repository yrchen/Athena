#include "bbs.h"

struct new
{
  char brdname[IDLEN + 1];      /* 看板英文名稱    13 bytes */
  char title[BTLEN + 1];        /* 看板中文名稱    49 bytes */
  char BM[IDLEN * 3 + 3];       /* 板主ID和"/"     39 bytes */
  usint brdattr;                /* 看板的屬性       4 bytes */
  time_t bupdate;               /* note update time 4 bytes */
  uschar bvote;                 /* Vote flags       1 bytes */
  time_t vtime;                 /* Vote close time  4 bytes */
  usint level;                  /* 可以看此板的權限 4 bytes */
  unsigned long int totalvisit; /* 總拜訪人數       8 bytes */
  unsigned long int totaltime;  /* 總停留時間       8 bytes */
  char lastvisit[IDLEN + 1];    /* 最後看該板的人  13 bytes */
  time_t opentime;              /* 開板時間         4 bytes */
  time_t lastime;               /* 最後拜訪時間     4 bytes */
  char passwd[PASSLEN];         /* 密碼            14 bytes */
  unsigned long int postotal;   /* 總水量 :p        8 bytes */
  usint maxpost;
  usint maxtime;
  char desc[3][80];
  char pad[87];
};
typedef struct new new;


struct old
{
  char brdname[IDLEN + 1];      /* 看板英文名稱    13 bytes */
  char title[BTLEN + 1];        /* 看板中文名稱    49 bytes */
  char BM[IDLEN * 3 + 3];       /* 板主ID和"/"     39 bytes */
  usint brdattr;                /* 看板的屬性       4 bytes */
  time_t bupdate;               /* note update time 4 bytes */
  uschar bvote;                 /* Vote flags       1 bytes */
  time_t vtime;                 /* Vote close time  4 bytes */
  usint level;                  /* 可以看此板的權限 4 bytes */
  unsigned long int totalvisit; /* 總拜訪人數       8 bytes */
  unsigned long int totaltime;  /* 總停留時間       8 bytes */
  char lastvisit[IDLEN + 1];    /* 最後看該板的人  13 bytes */
  time_t opentime;              /* 開板時間         4 bytes */
  time_t lastime;               /* 最後拜訪時間     4 bytes */
  char holiday[17];             /* 看板節日        17 bytes */
  int bid;                      /* 看板自己的id     4 bytes */
  int gid;                      /* 看板所屬的group  4 bytes */
  ushort maxno;                 /* 看板文章上限     2 bytes */
  ushort minno;                 /* 看板文章下限     2 bytes */
  ushort chat;                  /* 看板聊天人數     2 bytes */ /* 31 */
  char pad[70];
};
typedef struct old old;


int
invalid_brdname (brd)		/* 定義錯誤看板名稱 */
     char *brd;
{
  register char ch;

  ch = *brd++;
  if (not_alnum (ch))
    return 1;
  while (ch = *brd++)
    {
      if (not_alnum (ch) && ch != '_' && ch != '-' && ch != '.')
	return 1;
    }
  return 0;
}

main()
{
  int fdr,fdw, i = 0;
  new new;
  old tboard;
  
  fdr=open(BBSHOME"/.BOARDS",O_RDONLY);
  fdw=open(BBSHOME"/BOARDS.NEW",O_WRONLY | O_CREAT | O_TRUNC, 0644);

  printf("size of new struct is %d\n",sizeof(new));
  printf("size of old struct is %d\n",sizeof(old));  
  while(read(fdr,&tboard,sizeof(old))==sizeof(old))
  {     
  	i++;
	if(!tboard.brdname[0]) continue;
	if(invalid_brdname(tboard.brdname)) continue;
	printf("\n \
=====================================================\n \
brd num   : %d\n \
boardname : %s\n \
title     : %s\n \
totalvisit: %d\n \
=====================================================\n"
,i,tboard.brdname,tboard.title,tboard.totalvisit);
        memcpy(new.brdname,tboard.brdname,IDLEN+1);
        memcpy(new.title,tboard.title,BTLEN + 1);
        memcpy(new.BM,tboard.BM,24);
        new.brdattr = tboard.brdattr;  
        new.bupdate=tboard.bupdate;
        new.bvote = tboard.bvote;
        new.vtime=tboard.vtime;
	new.level=tboard.level;  
        new.totalvisit=tboard.totalvisit;
        new.totaltime=tboard.totaltime;
        memcpy(new.lastvisit,tboard.lastvisit,IDLEN+1);
        new.opentime=tboard.opentime;
        new.lastime=tboard.lastime;
        memcpy(new.passwd,"",PASSLEN); 
        memcpy(new.desc[0],"尚未編輯",80);
        memcpy(new.desc[1],"尚未編輯",80);
        memcpy(new.desc[2],"尚未編輯",80);
  	new.postotal=0;
//  	if(tboard.maxtime=365)
    	  new.maxtime=1000;
//      else 
//        new.maxtime=tboard.maxtime;
  	new.maxpost=5000;
        write(fdw,&new,sizeof(new));
   }
   close(fdr);
   close(fdw);
}     
