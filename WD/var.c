/*-------------------------------------------------------*/
/* global.h     ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : global definitions & variables               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

/* ----------------------------------------------------- */
/* ¦UºØÅv­­ªº¤¤¤å·N¸q                                    */
/* ----------------------------------------------------- */

char *permstrings[] = {
  "°ò¥»Åv¤O",                   /* PERM_BASIC */
  "¶i¤J²á¤Ñ«Ç",                 /* PERM_CHAT */
  "§ä¤H²á¤Ñ",                   /* PERM_PAGE */
  "µoªí¤å³¹",                   /* PERM_POST */
  "µù¥Uµ{§Ç»{ÃÒ",               /* PERM_LOGINOK */
  "«H¥óµL¤W­­",                 /* PERM_MAILLIMIT */
  "Áô¨­³N",                     /* PERM_CLOAK */
  "¬Ý¨£§ÔªÌ",                   /* PERM_SEECLOAK */
  "¥Ã¤[«O¯d±b¸¹",               /* PERM_XEMPT */
  "¯¸ªøÁô¨­³N",                 /* PERM_DENYPOST */
  "ªO¥D",                       /* PERM_BM */
  "±b¸¹Á`ºÞ",                   /* PERM_ACCOUNTS */
  "²á¤Ñ«ÇÁ`ºÞ",                 /* PERM_CHATCLOAK */
  "¬ÝªOÁ`ºÞ",                   /* PERM_BOARD */
  "¯¸ªø",                       /* PERM_SYSOP */
  "BBSADM",                     /* PERM_BBSADM & PERM_POSTMASK */
  "¤£¦C¤J±Æ¦æº]",               /* PERM_NOTOP */
  "ºÞ²z¯¸¤W¤å¥ó",               /* PERM_XFILE */
  "¬ãµo¤p²Õ",                   /* PERM_RESEARCH */
  "­×§ï¬G¶m",                   /* PERM_FROM */
  "¤åÃÀ®iµô§P",                 /* PERM_GOOD */
  "¨S·Q¨ì",                     /* PERM_ */
  "ºëµØ°ÏÁ`ªø",                 /* PERM_Announce */
  "¯S°È²Õ",                     /* PERM_MG */
  "¯S°È²Õªø",                   /* PERM_SMG */
  "¤å«Å²Õ",                     /* PERM_AD */
  "¤å«Å²Õªø",                   /* PERM_SAD */
  "¬ü¤u²Õ",                     /* PERM_PAINT */
  "¬ü¤u²Õªø",                   /* PERM_SPAINT */
  "¯µ®Ñ",                       /* PERM_SECRETARY */
  "¤p¯¸ªø",                     /* PERM_LSYSOP */
  "³¥¿ß¦Ñ¤j"                    /* PERM_CAVE */  
};

/* ----------------------------------------------------- */
/* GLOBAL VARIABLE                                       */
/* ----------------------------------------------------- */

char trans_buffer[256];         /* ¤@¯ë¶Ç»¼ÅÜ¼Æ add by Ptt */

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

