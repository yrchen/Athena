/*-------------------------------------------------------*/
/* global.h     ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : global definitions & variables               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

/* ----------------------------------------------------- */
/* 各種權限的中文意義                                    */
/* ----------------------------------------------------- */

char *permstrings[] = {
  "基本權力",                   /* PERM_BASIC */
  "進入聊天室",                 /* PERM_CHAT */
  "找人聊天",                   /* PERM_PAGE */
  "發表文章",                   /* PERM_POST */
  "註冊程序認證",               /* PERM_LOGINOK */
  "信件無上限",                 /* PERM_MAILLIMIT */
  "隱身術",                     /* PERM_CLOAK */
  "看見忍者",                   /* PERM_SEECLOAK */
  "永久保留帳號",               /* PERM_XEMPT */
  "站長隱身術",                 /* PERM_DENYPOST */
  "板主",                       /* PERM_BM */
  "帳號總管",                   /* PERM_ACCOUNTS */
  "聊天室總管",                 /* PERM_CHATCLOAK */
  "看板總管",                   /* PERM_BOARD */
  "站長",                       /* PERM_SYSOP */
  "BBSADM",                     /* PERM_BBSADM & PERM_POSTMASK */
  "不列入排行榜",               /* PERM_NOTOP */
  "管理站上文件",               /* PERM_XFILE */
  "研發小組",                   /* PERM_RESEARCH */
  "修改故鄉",                   /* PERM_FROM */
  "文藝展裁判",                 /* PERM_GOOD */
  "沒想到",                     /* PERM_ */
  "精華區總長",                 /* PERM_Announce */
  "特務組",                     /* PERM_MG */
  "特務組長",                   /* PERM_SMG */
  "文宣組",                     /* PERM_AD */
  "文宣組長",                   /* PERM_SAD */
  "美工組",                     /* PERM_PAINT */
  "美工組長",                   /* PERM_SPAINT */
  "秘書",                       /* PERM_SECRETARY */
  "小站長",                     /* PERM_LSYSOP */
  "野貓老大"                    /* PERM_CAVE */  
};

/* ----------------------------------------------------- */
/* GLOBAL VARIABLE                                       */
/* ----------------------------------------------------- */

char trans_buffer[256];         /* 一般傳遞變數 add by Ptt */

int usernum;
pid_t currpid;                  /* current process ID */
usint currstat;
int currmode = 0;
int curredit = 0;
int showansi = 1;
time_t login_start_time;
time_t update_time;
time_t schedule_time;
char schedule_string[100];
userec cuser;                   /* current user structure */
userec xuser;                   /* lookup user structure */
char quote_file[80] = "\0";
char quote_user[80] = "\0";
time_t paste_time;
char paste_title[STRLEN];
char paste_path[256];
int  paste_level;
char currtitle[TTLEN + 1] = "\0";
char vetitle[44] = "\0";
char currowner[STRLEN] = "\0";
char currauthor[IDLEN + 2] = "\0";
char currfile[FNLEN];           /* current file name @ bbs.c mail.c */
uschar currfmode;               /* current file mode */
char currmsg[100];
char currboard[IDLEN + 2];
usint currbrdattr;
char currBM[IDLEN * 3 + 10];
char reset_color[4] = "[m";
int inmore = 0;
char *msg_choose_cancel = MSG_CHOOSE_CANCEL;
char *msg_choose[3]={MSG_CHOOSE_YES, MSG_CHOOSE_NO, MSG_CHOOSE_CANCEL};
/* global string variables */


/* filename */
char *fn_passwd         = FN_PASSWD;
char *fn_board          = FN_BOARD;
char *fn_note_ans       = "note.ans";
char *fn_register       = "register.new";
char *fn_plans          = "plans";
char *fn_writelog       = "writelog";
char *fn_talklog        = "talklog";
char *fn_magickey	= FN_MAGICKEY;
char *fn_overrides      = FN_OVERRIDES;
char *fn_canvote        = FN_CANVOTE;
char *fn_notes          = "notes";
char *fn_water          = FN_WATER;
char *fn_visable        = FN_VISABLE;
char *fn_mandex         = "/.Names";

/* message */
char *msg_seperator     = MSG_SEPERATOR;

char *msg_cancel        = MSG_CANCEL;
char *msg_usr_left      = MSG_USR_LEFT;
char *msg_nobody        = MSG_NOBODY;

char *msg_sure_ny       = MSG_SURE_NY;
char *msg_sure_yn       = MSG_SURE_YN;
char *msg_sure		= MSG_SURE;

char *msg_bid           = MSG_BID;
char *msg_uid           = MSG_UID;

char *msg_del_ok        = MSG_DEL_OK;
char *msg_del_ny        = MSG_DEL_NY;

char *msg_fwd_ok        = MSG_FWD_OK;
char *msg_fwd_err1      = MSG_FWD_ERR1;
char *msg_fwd_err2      = MSG_FWD_ERR2;

char *err_board_update  = ERR_BOARD_UPDATE;
char *err_bid           = ERR_BID;
char *err_uid           = ERR_UID;
char *err_filename      = ERR_FILENAME;

char *str_mail_address  = "." BBSUSER "@" MYHOSTNAME;
char *str_new           = "new";
char *str_reply         = "Re: ";
char *str_space         = " \t\n\r";
char *str_sysop         = "SYSOP";
char *str_author1       = STR_AUTHOR1;
char *str_author2       = STR_AUTHOR2;
char *str_post1         = STR_POST1;
char *str_post2         = STR_POST2;
char *BoardName         = BOARDNAME;
char *str_dotdir        = ".DIR";
char tmpbuf[512];

/* visio.c */
int KEY_ESC_arg;

int t_lines = 24;
int b_lines = 23;
int p_lines = 20;
int t_columns = 80;

/* msg.c */
char watermode = 0;

/* read.c */
char currdirect[64];

