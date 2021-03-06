/*-------------------------------------------------------*/
/* global.h     ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : global definitions & variables               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/


#ifndef _GLOBAL_H_
#define _GLOBAL_H_

/* ----------------------------------------------------- */
/* GLOBAL DEFINITION                                     */
/* ----------------------------------------------------- */


/* 檔名設定 */

#define FN_PASSWD       ".PASSWDS"      /* User records */
#define FN_BOARD        ".BOARDS"       /* board list */
#define FN_GAME		".GAME"		/* RPG records */
#define FN_USIES        "usies"         /* BBS log */
#define FN_USSONG       "ussong"        /* 點歌 */
#define FN_VOTE		".VCH"		/* 投票 */
#define FN_LIST		".LIST"		/* 名單 */
#define FN_POSTPREFIX	".POSTPREFIX"	/* 標題類別 by hialan 3/21/2002*/
#define FN_POST_NOTE	".POSTNOTE"	/* ychia.021212:看板自訂文章發表要領 */
#define FN_WINFORM	".WINFORM"	/* hialan.030705:視窗外框設定*/
#define FN_LOGINPLAN	".LOGINPLAN"    /* 上站執行劇本 */
#define FN_SOMAN	"soman"		/* DSO 管理員 */
#define FN_MAGICKEY	"MagicKey"	/* 認證碼檔案 */
#define FN_PAL		"pal"
#define FN_ALOHA	"aloha"
#define FN_BRDFEAST	".brdfeast"	/* 看板節日 */
#define FN_FORWARD	".forward"	/* 信箱自動轉寄 */

#define FN_GAMELOG 	"etc/game.log"	/* 遊記紀錄 */
#define FN_TOPSONG      "log/topsong"
#define FN_GAMEMONEY    "game/money"
#define FN_MONEYLOG     "log/moneystat"
#define FN_OVERRIDES    "overrides"
#define FN_CANVOTE      "can_vote"
#define FN_WATER        "water"
#define FN_APPLICATION  "application"
#define FN_VISABLE      "visable"
#define FN_BOTTOM	".bottom"
#define	FN_LOGLOGIN	"login.log"	/* 個人上站紀錄 */
#define FN_DISABLED	"etc/.disabled"	/* 不准上站之 ID */

#define FN_TICKET_RECORD "game/ticket.data"
#define FN_TICKET_USER   "game/ticket.user"
#define FN_TICKET        "game/ticket.result"

#define FN_BMW          "bmw"           /* itoc.011104: for BMW */

#define DEFAULT_BOARD   str_sysop

/* 鍵盤設定 */

#ifndef EXTEND_KEY
#define EXTEND_KEY
#define KEY_TAB         9
#define KEY_ESC         27
#define KEY_UP          0x0101
#define KEY_DOWN        0x0102
#define KEY_RIGHT       0x0103
#define KEY_LEFT        0x0104
#define KEY_STAB        0x0105	/* shift-tab */
#define KEY_HOME        0x0201
#define KEY_INS         0x0202
#define KEY_DEL         0x0203
#define KEY_END         0x0204
#define KEY_PGUP        0x0205
#define KEY_PGDN        0x0206
#define KEY_BKSP	0x007F
#define KEY_F1		0x0301
#define KEY_F2		0x0302
#define KEY_F3		0x0303
#define KEY_F4		0x0304
#define KEY_F5		0x0305
#define KEY_F6		0x0306
#define KEY_F7		0x0307
#define KEY_F8		0x0308
#define KEY_F9		0x0309
#define KEY_F10		0x030A
#define KEY_F11		0x030B
#define KEY_F12		0x030C
#endif

#define Ctrl(c)         ( c & 037 )

#ifdef SYSV
#undef CTRL                     /* SVR4 CTRL macro is hokey */
#define CTRL(c) ('c'&037)       /* This gives ESIX a warning...ignore it! */
#endif

/* ----------------------------------------------------- */
/* External function declarations                        */
/* ----------------------------------------------------- */

#define TRACE   log_usies

/* ----------------------------------------------------- */
/* 訊息字串：獨立出來，以利支援各種語言                  */
/* ----------------------------------------------------- */

#define STR_CURSOR      ">>"
#define STR_UNCUR       "  "

#define STR_AUTHOR1     "作者:"
#define STR_AUTHOR2     "發信人:"
#define STR_POST1       "看板:"
#define STR_POST2       "站內:"

#define LEN_AUTHOR1     5
#define LEN_AUTHOR2     7

#define STR_SYSOP	"SYSOP"
#define STR_GUEST       "guest"
#define STR_ANONYMOUS	"松子心語"

#define MSG_CHOOSE_YES		"yY)是"
#define MSG_CHOOSE_NO		"nN)否"
#define MSG_CHOOSE_CANCEL	"qQ)取消"

#define MSG_SEPERATOR   "\
───────────────────────────────────────"

#define MSG_CLOAKED     "嘿嘿,躲起來囉!"
#define MSG_UNCLOAK     "重現江湖了...."

#define MSG_WORKING     "處理中，請稍候..."

#define MSG_CANCEL      "取消！"
#define MSG_USR_LEFT    "User 已經離開了"
#define MSG_NOBODY      "目前無人上線"
#define MSG_MY_FAVORITE "我的最愛看板"

#define MSG_DEL_OK      "刪除完畢"
#define MSG_DEL_CANCEL  "取消刪除"
#define MSG_DEL_ERROR   "刪除錯誤"
#define MSG_DEL_NY      "請確定刪除  Y)確定  N)取消？[N] "

#define MSG_FWD_OK      "文章轉寄完成！"
#define MSG_FWD_ERR1    "轉寄失誤：系統發生錯誤"
#define MSG_FWD_ERR2    "轉寄失誤：地址錯誤，查無此人"

#define MSG_SURE_NY     "請您確定  Y)確定  N)取消？[N] "
#define MSG_SURE_YN     "請您確定  Y)確定  N)取消？[Y] "
#define MSG_SURE	"請您確定？"

#define MSG_BID         "請輸入看板名稱："
#define MSG_UID         "請輸入使用者代號："
#define MSG_PASSWD      "請輸入您的密碼："

#define MSG_BIG_BOY     "帥哥"
#define MSG_BIG_GIRL    "美女"
#define MSG_LITTLE_BOY  "底迪"
#define MSG_LITTLE_GIRL "美眉"
#define MSG_MAN         "叔叔"
#define MSG_WOMAN       "阿姨"
#define MSG_PLANT       "植物"
#define MSG_MIME        "礦物"

#define ERR_BOARD_OPEN  ".BOARD 開啟錯誤"
#define ERR_BOARD_UPDATE        ".BOARD 更新有誤"
#define ERR_PASSWD_OPEN ".PASSWDS 開啟錯誤"

#define ERR_BID         "你搞錯了啦！沒有這個板喔！"
#define ERR_UID         "這裡沒有這個人啦！"
#define ERR_PASSWD      "密碼不對喔！你有沒有冒用人家的名字啊？"
#define ERR_FILENAME    "檔名不合法！"

#define MSG_SELECT_BOARD        \
        COLOR1"[1m【 [37m選擇看板[33m 】[0m\n請輸入看板名稱(按空白鍵自動搜尋)："

#define P_BOARD "對不起，此板只准看板好友進入，請向板主申請入境許\可！"

/* ----------------------------------------------------- */
/* GLOBAL VARIABLE                                       */
/* ----------------------------------------------------- */

extern char trans_buffer[256];         /* 一般傳遞變數 add by Ptt */

extern int usernum;
extern pid_t currpid;
extern usint currstat;
extern int currmode;
extern int curredit;
extern int showansi;
extern int talkrequest;
extern time_t login_start_time;
extern time_t update_time;
extern time_t schedule_time;
extern char schedule_string[100];
extern userec cuser;            /* current user structure */
extern userec xuser;            /* lookup user structure */

extern char quote_file[80];
extern char quote_user[80];
extern time_t paste_time;
extern char paste_title[STRLEN];
extern char paste_path[256];
extern int  paste_level;
extern char currowner[STRLEN];
extern char currauthor[IDLEN + 2];
extern uschar currfmode;               /* current file mode */
extern char currtitle[TTLEN + 1];
extern char vetitle[44];
extern char currfile[FNLEN];
extern char currmsg[100];

extern char currboard[];        /* name of currently selected board */
extern usint currbrdattr;
extern char currBM[];           /* BM of currently selected board */
extern char reset_color[];
extern int inmore;
extern char *msg_choose_cancel;
extern char *msg_choose[3];
/* global string variable */

/* filename */

extern char *fn_passwd;
extern char *fn_board;
extern char *fn_note_ans;
extern char *fn_register;
extern char *fn_plans;
extern char *fn_writelog;
extern char *fn_talklog;
extern char *fn_magickey;
extern char *fn_overrides;
extern char *fn_reject;
extern char *fn_canvote;
extern char *fn_notes;
extern char *fn_water;
extern char *fn_visable;
extern char *fn_mandex;

/* message */
extern char *msg_seperator;
extern char *msg_mailer;

extern char *msg_cancel;
extern char *msg_usr_left;
extern char *msg_nobody;

extern char *msg_sure_ny;
extern char *msg_sure_yn;
extern char *msg_sure;

extern char *msg_bid;
extern char *msg_uid;

extern char *msg_del_ok;
extern char *msg_del_ny;

extern char *msg_fwd_ok;
extern char *msg_fwd_err1;
extern char *msg_fwd_err2;

extern char *err_board_update;
extern char *err_bid;
extern char *err_uid;
extern char *err_filename;

extern char *str_mail_address;
extern char *str_new;
extern char *str_reply;
extern char *str_space;
extern char *str_sysop;
extern char *str_author1;
extern char *str_author2;
extern char *str_post1;
extern char *str_post2;
extern char *BoardName;
extern char *str_dotdir;
extern char tmpbuf[512];

extern int errno;

extern user_info *currutmp;

extern char fromhost[];
extern char save_title[];       /* used by editor when inserting */

extern int KEY_ESC_arg;

/* visio.c */
extern int t_lines, t_columns;  /* Screen size / width */
extern int b_lines;             /* Screen bottom line number: t_lines-1 */
extern int p_lines;             /* a Page of Screen line numbers: tlines-4 */

/* msg.c */
extern char watermode;

/* read.c */
extern char currdirect[64];

/* cache.c (not put in var.c for including easily) */
extern struct UCACHE *uidshm;
extern struct BCACHE *brdshm;
extern struct UTMPFILE *utmpshm;

void doent();   /* read,mail,announce list */

#endif                          /* _GLOBAL_H_ */
