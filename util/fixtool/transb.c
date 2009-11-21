#include "bbs.h"

struct new
{
  char brdname[IDLEN + 1];      /* �ݪO�^��W��    13 bytes */
  char title[BTLEN + 1];        /* �ݪO����W��    49 bytes */
  char BM[IDLEN * 3 + 3];       /* �O�DID�M"/"     39 bytes */
  usint brdattr;                /* �ݪO���ݩ�       4 bytes */
  time_t bupdate;               /* note update time 4 bytes */
  uschar bvote;                 /* Vote flags       1 bytes */
  time_t vtime;                 /* Vote close time  4 bytes */
  usint level;                  /* �i�H�ݦ��O���v�� 4 bytes */
  unsigned long int totalvisit; /* �`���X�H��       8 bytes */
  unsigned long int totaltime;  /* �`���d�ɶ�       8 bytes */
  char lastvisit[IDLEN + 1];    /* �̫�ݸӪO���H  13 bytes */
  time_t opentime;              /* �}�O�ɶ�         4 bytes */
  time_t lastime;               /* �̫���X�ɶ�     4 bytes */
  char passwd[PASSLEN];         /* �K�X            14 bytes */
  unsigned long int postotal;   /* �`���q :p        8 bytes */
  usint maxpost;
  usint maxtime;
  char desc[3][80];
  char pad[87];
};
typedef struct new new;


struct old
{
  char brdname[IDLEN + 1];      /* �ݪO�^��W��    13 bytes */
  char title[BTLEN + 1];        /* �ݪO����W��    49 bytes */
  char BM[IDLEN * 3 + 3];       /* �O�DID�M"/"     39 bytes */
  usint brdattr;                /* �ݪO���ݩ�       4 bytes */
  time_t bupdate;               /* note update time 4 bytes */
  uschar bvote;                 /* Vote flags       1 bytes */
  time_t vtime;                 /* Vote close time  4 bytes */
  usint level;                  /* �i�H�ݦ��O���v�� 4 bytes */
  unsigned long int totalvisit; /* �`���X�H��       8 bytes */
  unsigned long int totaltime;  /* �`���d�ɶ�       8 bytes */
  char lastvisit[IDLEN + 1];    /* �̫�ݸӪO���H  13 bytes */
  time_t opentime;              /* �}�O�ɶ�         4 bytes */
  time_t lastime;               /* �̫���X�ɶ�     4 bytes */
  char holiday[17];             /* �ݪO�`��        17 bytes */
  int bid;                      /* �ݪO�ۤv��id     4 bytes */
  int gid;                      /* �ݪO���ݪ�group  4 bytes */
  ushort maxno;                 /* �ݪO�峹�W��     2 bytes */
  ushort minno;                 /* �ݪO�峹�U��     2 bytes */
  ushort chat;                  /* �ݪO��ѤH��     2 bytes */ /* 31 */
  char pad[70];
};
typedef struct old old;


int
invalid_brdname (brd)		/* �w�q���~�ݪO�W�� */
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
        memcpy(new.desc[0],"�|���s��",80);
        memcpy(new.desc[1],"�|���s��",80);
        memcpy(new.desc[2],"�|���s��",80);
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
