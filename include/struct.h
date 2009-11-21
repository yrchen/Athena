/*-------------------------------------------------------*/
/* struct.h     ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : all definitions about data structure         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#ifndef _STRUCT_H_
#define _STRUCT_H_


#define PATHLEN	128		/* hialan: Length of path */
#define STRLEN   80             /* Length of most string data */
#define BRC_STRLEN 15           /* Length of boardname */
#define BTLEN    48             /* Length of board title */
#define TTLEN    72             /* Length of title */
#define NAMELEN  40             /* Length of username/realname */
#define FNLEN    33             /* Length of filename  */
				/* Ptt ���o�̦�bug*/
#define IDLEN    12             /* Length of bid/uid */
#define PASSLEN  14             /* Length of encrypted passwd field */
#define REGLEN   38             /* Length of registration data */



typedef unsigned char uschar;   /* length = 1 */
typedef unsigned int usint;     /* length = 4 */

/* ----------------------------------------------------- */
/* .PASSWDS struct : 512 bytes                           */
/* ----------------------------------------------------- */
struct userec
{
  /*�򥻸�� 136 bytes*/
  char userid[IDLEN + 1];         /* �ϥΪ̦W��  13 bytes */
  char realname[20];              /* �u��m�W    20 bytes */
  char username[24];              /* �ʺ�        24 bytes */
  char passwd[PASSLEN];           /* �K�X        14 bytes */
  char email[50];                 /* E-MAIL      50 bytes */
  char countryid[11];		  /* �����Ҧr��  11 bytes */
  uschar month;                   /* �X�ͤ��     1 byte  */
  uschar day;                     /* �X�ͤ��     1 byte  */
  uschar year;                    /* �X�ͦ~��     1 byte  */
  uschar sex;                     /* �ʧO         1 byte  */

  /*�t���v�� 32 bytes*/
  uschar uflag;                   /* �ϥΪ̿ﶵ   1 byte  */
  usint userlevel;                /* �ϥΪ��v��   4 bytes */
  uschar invisible;               /* �����Ҧ�     1 bytes */
  uschar state;                   /* ���A??       1 byte  */
  usint habit;                    /* �ߦn�]�w     4 bytes */
  uschar pager;                   /* ������A     1 bytes */
  usint exmailbox;                /* �H�c�ʼ�     4 bytes */
  usint exmailboxk;               /* �H�cK��      4 bytes */
  time_t dtime;                   /* �s�ڮɶ�     4 bytes */
  int update_songtime; 		  /* �I�q���Ƨ�s 4 bytes */
  int scoretimes;		  /* ��������	  4 bytes */

  /*�t�θ�� 130 bytes*/
  ushort numlogins;               /* �W������     2 bytes */
  ushort numposts;                /* POST����     2 bytes */
  time_t lastlogin;               /* �e���W��     4 bytes */
  char lasthost[24];              /* �W���a�I    24 bytes */
  char vhost[24];                 /* �������}    24 bytes */
  unsigned long int totaltime;    /* �W�u�`�ɼ�   8 bytes */
  usint sendmsg;                  /* �o�T������   4 bytes */
  usint receivemsg;               /* ���T������   4 bytes */
  unsigned long int goldmoney;	  /* ���Ъ���     8 bytes */
  unsigned long int silvermoney;  /* �ȹ�         8 bytes */
  unsigned long int songtimes;    /* �벼����     8 bytes */
  usint toquery;                  /* �n�_��       4 bytes */
  usint bequery;                  /* �H���       4 bytes */
  char toqid[IDLEN + 1];	  /* �e���d��    13 bytes */
  char beqid[IDLEN + 1];	  /* �e���Q�֬d  13 bytes */

  /*���U��� 44 bytes*/
  time_t firstlogin;              /* ���U�ɶ�     4 bytes */
  char justify[REGLEN + 1];       /* ���U���    39 bytes */
  uschar rtimes;		  /* ����U�榸�� 1 bytes */

  /*�ߦn�]�w 61 bytes*/
  char feeling[5];		  /* �߱�����     5 bytes */
  char lightbar[5];		  /* ����	  5 bytes */
  char cursor[51];		  /* �ۭq���    51 bytes */
 
  int uid;			  /* �ϥΪ̽s��   4 bytes */
  char pad[107];                  /* �ŵ۶񺡦�512��      */
};

typedef struct userec userec;


/* these are flags in userec.uflag */
#define SIG_FLAG        0x3     /* signature number, 2 bits */
#define PAGER_FLAG      0x4     /* true if pager was OFF last session */
#define CLOAK_FLAG      0x8     /* true if cloak was ON last session */
#define FRIEND_FLAG     0x10    /* true if show friends only */
#define BRDSORT_FLAG    0x20    /* true if the boards sorted alphabetical */
#define MOVIE_FLAG      0x40    /* true if show movie */
#define COLOR_FLAG      0x80    /* true if the color mode open */

/* ----------------------------------------------------- */
/* LOG of games struct : 128 bytes                       */
/* ----------------------------------------------------- */

struct gamedata
{
  int five_win;
  int five_lost;
  int five_draw;
  char pad[116];		/* �d�ۥ[�j�� 128 bytes */
};
typedef struct gamedata gamedata;

/* ----------------------------------------------------- */
/* DIR of board struct : 128 bytes                       */
/* ----------------------------------------------------- */

struct fileheader
{
  char filename[FNLEN-1];         /* M.9876543210.A 	33 bytes*/
  char score;			/* ����                  1 bytes*/
  char savemode;                /* file save mode 	 1 bytes*/
  char owner[IDLEN + 2];        /* uid[.] 		14 bytes*/
  char date[6];                 /* [02/02] or space(5)   6 bytes*/
  char title[TTLEN + 1];	/* title		73 bytes*/
//  time_t chrono;                /* timestamp */
//  char dummy;
  uschar filemode;              /* must be last field @ boards.c 1 bytes*/
};
typedef struct fileheader fileheader;


struct PAL
{
  char userid[FNLEN];           /* list name/userid */
  char savemode;                
  char owner[IDLEN + 2];        /* /bbcall */
  char date[6];                 /* /birthday */
  char desc[TTLEN + 1];         /* list/user desc */
  uschar ftype;                 /* mode:  PAL, BAD */
};
typedef struct PAL PAL;

#define M_PAL		0x01
#define M_BAD		0x02
#define M_ALOHA		0x04

#define M_VISABLE	0x01
#define M_WATER		0x02
#define M_CANVOTE	0x04 


#define FILE_LOCAL      0x01    /* local saved */
#define FILE_READ       0x01    /* already read : mail only */
#define FILE_MARKED     0x02    /* opus: 0x8 */
#define FILE_DIGEST     0x04    /* digest */
#define FILE_TAGED      0x08    /* taged */
#define FILE_REPLYOK	0x10	/* reply ok : mail only*/
#define FILE_SCORED	0x10	/* scored   : article only */
#define FILE_REFUSE	0x20	/* refuse */
#define FILE_DIR	0x40	/* dir */
#define FILE_BOTTOM	0x80	/* bottom announce*/

/* ----------------------------------------------------- */
/* Structure used in UTMP file : ??? bytes               */
/* ----------------------------------------------------- */

/*
woju
Message queue
*/
typedef struct 
{
   pid_t last_pid;
   char last_userid[IDLEN + 1];
   char last_call_in[80];
//   char return_msg[80];
} msgque;

struct user_info
{
  int uid;                      /* Used to find user name in passwd file */
  pid_t pid;                    /* kill() to notify user of talk request */
  int sockaddr;                 /* ... */
  int destuid;                  /* talk uses this to identify who called */
  struct user_info* destuip;
  uschar active;                /* When allocated this field is true */
  uschar invisible;             /* Used by cloaking function in Xyz menu */
  uschar sockactive;            /* Used to coordinate talk requests */
  usint userlevel;
  uschar mode;                  /* UL/DL, Talk Mode, Chat Mode, ... */
  uschar pager;                 /* pager toggle, YEA, or NA */
  uschar in_chat;               /* for in_chat commands   */
  uschar sig;                   /* signal type */
  char userid[IDLEN + 1];
  char chatid[11];              /* chat id, if in chat mode */
  char realname[20];
  char username[24];
  char from[27];                /* machine name the user called in from */
  int from_alias;
  char birth;                   /* �O�_�O�ͤ� Ptt*/
  char tty[11];                 /* tty port */
  ushort friend[MAX_FRIEND];
  ushort reject[MAX_REJECT];
  uschar msgcount;
  msgque msgs[3];
  time_t uptime;
  time_t lastact;             /* �W���ϥΪ̰ʪ��ɶ� */
  usint brc_id;
  uschar lockmode;
  int turn;
  int dark_turn;
  char feeling[5];		/* �߱� */
  uschar sex;			/* �ʧO */
};
typedef struct user_info user_info;


/* ----------------------------------------------------- */
/* BOARDS struct : 512 bytes                             */
/* ----------------------------------------------------- */
#define BRD_NOZAP       00001         /* ���izap  */
#define BRD_NOCOUNT     00002         /* ���C�J�έp */
#define BRD_NOTRAN      00004         /* ����H */
#define BRD_GROUPBOARD  00010         /* �s�ժO */
#define BRD_HIDE        00020         /* ���êO (�ݪO�n�ͤ~�i��) */
#define BRD_POSTMASK    00040         /* ����o��ξ\Ū */
#define BRD_ANONYMOUS   00100         /* �ΦW�O? */
#define BRD_CLASS	00200         /* �����ݪO */
#define BRD_GOOD	00400         /* �u�}�ݪO */
#define BRD_PERSONAL	01000         /* �ӤH�ݪO */
#define BRD_GROUP	02000         /* ����ݪO */
#define BRD_NOFOWARD	04000	      /* �T����� */
#define BRD_POPULAR	10000	      /* �����ݪO */

struct boardheader
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
  /* wildcat note : check �o�� , expire.conf �����L�C�C�����a ...*/
  usint maxpost;		/* �峹�W��         4 bytes */
  usint maxtime;		/* �峹�O�d�ɶ�	    4 bytes */
  char desc[3][80];		/* ����y�z	  240 bytes */
  char bottom[5];		/* �m�����i��       5 bytes */
  char botm_color[5];		/* �m�����i�C��     5 bytes */
  char pad[77];			/* �񺡦� 512	   77 bytes */
};
typedef struct boardheader boardheader;

typedef struct
{
  int pos;
  uschar unread;
  uschar zap;
}      boardstat;

struct one_key
{                               /* Used to pass commands to the readmenu */
  int key;
//  int (*fptr) ();
  void *fptr;
  usint level;			
  char *desc;
  int mode;
};


/* ----------------------------------------------------- */
/* cache.c ���B�Ϊ���Ƶ��c                              */
/* ----------------------------------------------------- */


#define USHM_SIZE       (MAXACTIVE + 4)
struct UTMPFILE
{
  user_info uinfo[USHM_SIZE];
  time_t uptime;
  int number;
  int busystate;
};

typedef struct
{
  ushort uid[MAX_FRIEND];
  uschar type[MAX_FRIEND];			/* mode:  PAL, BAD */
  time_t lastloadtime;
}	HBFL;

struct BCACHE
{
  boardheader bcache[MAXBOARD];			/* �򥻸��			*/
  usint total[MAXBOARD];			/* ���������q			*/
  HBFL hbfl[MAXBOARD];				/* �ݪO�n�ͦW��			*/
  int nusers[MAXBOARD];				/* �ݪO���H�����               */
  time_t lastposttime[MAXBOARD];		/* �̫�o��ɶ�			*/
  char festival[MAXBOARD][15];                  /* �ݪO�`��                     */
  time_t feast_update[MAXBOARD];                /* �`���s�ɶ�                 */
  time_t uptime;
  time_t touchtime;
  int number;					/* �����ݪO���ƥ�		*/
  int busystate;
};

struct UCACHE
{
  char userid[MAXUSERS][IDLEN + 1];
  time_t uptime;
  time_t touchtime;
  int number;
  int busystate;
};

struct FROMCACHE
{
  char domain[MAX_FROM][50];
  char replace[MAX_FROM][50];
  int top;
  int max_user;
  time_t max_time;
  time_t uptime;
  time_t touchtime;
  int busystate;
};

struct BACACHE          
{                       
  char author[300][100];
  int top;              
  time_t uptime;        
  time_t touchtime;     
  int busystate;        
};                      

struct hosts
{
 char shortname[24];
 char address[40];
 char desc[24];
};

typedef struct hosts hosts;


typedef struct
{
  time_t chrono;
  int recno;
}      TagItem;


/* ----------------------------------------------------- */
/* screen.c ���B�Ϊ���Ƶ��c                             */
/* ----------------------------------------------------- */

#define ANSILINELEN (511)       /* Maximum Screen width in chars */

/* Line buffer modes */
#define MODIFIED (1)            /* if line has been modifed, screen output */
#define STANDOUT (2)            /* if this line has a standout region */

#define SL_MODIFIED	(1)	/* if line has been modifed, screen output */
#define SL_STANDOUT	(2)	/* if this line contains standout code */
#define SL_ANSICODE	(4)	/* if this line contains ANSI code */

struct screenline
{
  uschar oldlen;                /* previous line length */
  uschar len;                   /* current length of line */
  uschar width;			/* padding length of ANSI codes */
  uschar mode;                  /* status of line, as far as update */
  uschar smod;                  /* start of modified data */
  uschar emod;                  /* end of modified data */
  uschar sso;                   /* start stand out */
  uschar eso;                   /* end stand out */
  uschar data[ANSILINELEN];
};
typedef struct screenline screenline;

typedef struct LinkList
{
  struct LinkList *next;
  char data[0];
}        LinkList;

/* ----------------------------------------------------- */
/* name.c ���B�Ϊ���Ƶ��c                               */
/* ----------------------------------------------------- */

struct word
{
  char *word;
  struct word *next;
};


/* ----------------------------------------------------- */
/* edit.c ���B�Ϊ���Ƶ��c                               */
/* ----------------------------------------------------- */

#define WRAPMARGIN (500)

struct textline
{
  struct textline *prev;
  struct textline *next;
  int len;
  char data[WRAPMARGIN + 1];
};
typedef struct textline textline;

#endif                          /* _STRUCT_H_ */

/* ----------------------------------------------------- */
/* announce.c                                            */
/* ----------------------------------------------------- */

#define MAXITEMS        1000     /* �@�ӥؿ��̦h���X�� */

typedef struct
{
  fileheader *header;
  char mtitle[STRLEN];
  char *path;
  int num, page, now, level, mode;
} AMENU;

union x_item
{
  struct                        /* bbs_item */
  {
    char fdate[9];              /* [mm/dd/yy] */
    char editor[13];            /* user ID */
    char fname[31];
  }      B;

  struct                        /* gopher_item */
  {
    char path[81];
    char server[48];
    int port;
  }      G;
};

typedef struct
{
  char title[63];
  union x_item X;
}      ITEM;

typedef struct
{
  ITEM *item[MAXITEMS];
  char mtitle[STRLEN];
  char *path;
  int num, page, now, level;
}      GMENU;


/* ----------------------------------------------------- */
/* record.c                                              */
/* ----------------------------------------------------- */

/* new/old/lock file processing  PTT */
typedef struct
{
  char newfn[512];
  char oldfn[512];
  char lockfn[512];
}      nol;

/* �������� struct */

struct notedata {
  time_t date;
  char userid[IDLEN + 1];
  char username[19];
  char buf[3][80];
};
typedef struct notedata notedata;

/* bwboard �Ψ쪺 */

typedef struct
{
  int key;
  int (*func) ();
}      KeyFunc;


/* ----------------------------------------------------- */
/* read.c                                                */
/* ----------------------------------------------------- */


struct keeploc
{
  char *key;
  int top_ln;
  int crs_ln;
  struct keeploc *next;
};
typedef struct keeploc keeploc;

/* ----------------------------------------------------- */
/* talk.c                                                */
/* ----------------------------------------------------- */

typedef struct
{
  user_info *ui;
  time_t idle;
  usint friend;
}      pickup;

/* ----------------------------------------------------- */
/* mn.c ���B�Ϊ���Ƶ��c                                 */
/* ----------------------------------------------------- */
struct  money_note
{
  usint  year;      // �~         4 b
  uschar month;     // ��         1 b
  uschar day;       // ��         1 b
  uschar flag;      // ���J/��X  1 b
  usint  money;     // ���B       4 b
  uschar use_way;   // ���O(������|��?)    1b
  char   desc[50];  // ����       50 b
  char   pad[62];   // null pad   62 b
};
typedef struct money_note MN;

#define MODE_OUT        0x1     // ��X
#define MODE_IN         0x2     // ���J

#define WAY_EAT         0       // ��
#define WAY_WEAR        1       // ��
#define WAY_LIVE        2       // ��
#define WAY_MOVE        3       // ��
#define WAY_EDU         4       // �|
#define WAY_PLAY        5       // ��
#define WAY_OTHER       6       // ��L

