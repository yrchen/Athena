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


/* ÀÉ¦W³]©w */

#define FN_PASSWD       ".PASSWDS"      /* User records */
#define FN_BOARD        ".BOARDS"       /* board list */
#define FN_GAME		".GAME"		/* RPG records */
#define FN_USIES        "usies"         /* BBS log */
#define FN_USSONG       "ussong"        /* ÂIºq */
#define FN_VOTE		".VCH"		/* §ë²¼ */
#define FN_LIST		".LIST"		/* ¦W³æ */
#define FN_POSTPREFIX	".POSTPREFIX"	/* ¼ÐÃDÃþ§O by hialan 3/21/2002*/
#define FN_POST_NOTE	".POSTNOTE"	/* ychia.021212:¬ÝªO¦Û­q¤å³¹µoªí­n»â */
#define FN_WINFORM	".WINFORM"	/* hialan.030705:µøµ¡¥~®Ø³]©w*/
#define FN_LOGINPLAN	".LOGINPLAN"    /* ¤W¯¸°õ¦æ¼@¥» */
#define FN_SOMAN	"soman"		/* DSO ºÞ²z­û */
#define FN_MAGICKEY	"MagicKey"	/* »{ÃÒ½XÀÉ®× */
#define FN_PAL		"pal"
#define FN_ALOHA	"aloha"
#define FN_BRDFEAST	".brdfeast"	/* ¬ÝªO¸`¤é */
#define FN_FORWARD	".forward"	/* «H½c¦Û°ÊÂà±H */

#define FN_GAMELOG 	"etc/game.log"	/* ¹C°O¬ö¿ý */
#define FN_TOPSONG      "log/topsong"
#define FN_GAMEMONEY    "game/money"
#define FN_MONEYLOG     "log/moneystat"
#define FN_OVERRIDES    "overrides"
#define FN_CANVOTE      "can_vote"
#define FN_WATER        "water"
#define FN_APPLICATION  "application"
#define FN_VISABLE      "visable"
#define FN_BOTTOM	".bottom"
#define	FN_LOGLOGIN	"login.log"	/* ­Ó¤H¤W¯¸¬ö¿ý */
#define FN_DISABLED	"etc/.disabled"	/* ¤£­ã¤W¯¸¤§ ID */

#define FN_TICKET_RECORD "game/ticket.data"
#define FN_TICKET_USER   "game/ticket.user"
#define FN_TICKET        "game/ticket.result"

#define FN_BMW          "bmw"           /* itoc.011104: for BMW */

#define DEFAULT_BOARD   str_sysop

/* Áä½L³]©w */

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
/* °T®§¦r¦ê¡G¿W¥ß¥X¨Ó¡A¥H§Q¤ä´©¦UºØ»y¨¥                  */
/* ----------------------------------------------------- */

#define STR_CURSOR      ">>"
#define STR_UNCUR       "  "

#define STR_AUTHOR1     "§@ªÌ:"
#define STR_AUTHOR2     "µo«H¤H:"
#define STR_POST1       "¬ÝªO:"
#define STR_POST2       "¯¸¤º:"

#define LEN_AUTHOR1     5
#define LEN_AUTHOR2     7

#define STR_SYSOP	"SYSOP"
#define STR_GUEST       "guest"
#define STR_ANONYMOUS	"ªQ¤l¤ß»y"

#define MSG_CHOOSE_YES		"yY)¬O"
#define MSG_CHOOSE_NO		"nN)§_"
#define MSG_CHOOSE_CANCEL	"qQ)¨ú®ø"

#define MSG_SEPERATOR   "\
¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w"

#define MSG_CLOAKED     "¼K¼K,¸ú°_¨ÓÅo!"
#define MSG_UNCLOAK     "­«²{¦¿´ò¤F...."

#define MSG_WORKING     "³B²z¤¤¡A½Ðµy­Ô..."

#define MSG_CANCEL      "¨ú®ø¡I"
#define MSG_USR_LEFT    "User ¤w¸gÂ÷¶}¤F"
#define MSG_NOBODY      "¥Ø«eµL¤H¤W½u"
#define MSG_MY_FAVORITE "§Úªº³Ì·R¬ÝªO"

#define MSG_DEL_OK      "§R°£§¹²¦"
#define MSG_DEL_CANCEL  "¨ú®ø§R°£"
#define MSG_DEL_ERROR   "§R°£¿ù»~"
#define MSG_DEL_NY      "½Ð½T©w§R°£  Y)½T©w  N)¨ú®ø¡H[N] "

#define MSG_FWD_OK      "¤å³¹Âà±H§¹¦¨¡I"
#define MSG_FWD_ERR1    "Âà±H¥¢»~¡G¨t²Îµo¥Í¿ù»~"
#define MSG_FWD_ERR2    "Âà±H¥¢»~¡G¦a§}¿ù»~¡A¬dµL¦¹¤H"

#define MSG_SURE_NY     "½Ð±z½T©w  Y)½T©w  N)¨ú®ø¡H[N] "
#define MSG_SURE_YN     "½Ð±z½T©w  Y)½T©w  N)¨ú®ø¡H[Y] "
#define MSG_SURE	"½Ð±z½T©w¡H"

#define MSG_BID         "½Ð¿é¤J¬ÝªO¦WºÙ¡G"
#define MSG_UID         "½Ð¿é¤J¨Ï¥ÎªÌ¥N¸¹¡G"
#define MSG_PASSWD      "½Ð¿é¤J±zªº±K½X¡G"

#define MSG_BIG_BOY     "«Ó­ô"
#define MSG_BIG_GIRL    "¬ü¤k"
#define MSG_LITTLE_BOY  "©³­}"
#define MSG_LITTLE_GIRL "¬ü¬Ü"
#define MSG_MAN         "¨û¨û"
#define MSG_WOMAN       "ªü«¼"
#define MSG_PLANT       "´Óª«"
#define MSG_MIME        "Äqª«"

#define ERR_BOARD_OPEN  ".BOARD ¶}±Ò¿ù»~"
#define ERR_BOARD_UPDATE        ".BOARD §ó·s¦³»~"
#define ERR_PASSWD_OPEN ".PASSWDS ¶}±Ò¿ù»~"

#define ERR_BID         "§A·d¿ù¤F°Õ¡I¨S¦³³o­ÓªO³á¡I"
#define ERR_UID         "³o¸Ì¨S¦³³o­Ó¤H°Õ¡I"
#define ERR_PASSWD      "±K½X¤£¹ï³á¡I§A¦³¨S¦³«_¥Î¤H®aªº¦W¦r°Ú¡H"
#define ERR_FILENAME    "ÀÉ¦W¤£¦Xªk¡I"

#define MSG_SELECT_BOARD        \
        COLOR1"[1m¡i [37m¿ï¾Ü¬ÝªO[33m ¡j[0m\n½Ð¿é¤J¬ÝªO¦WºÙ(«öªÅ¥ÕÁä¦Û°Ê·j´M)¡G"

#define P_BOARD "¹ï¤£°_¡A¦¹ªO¥u­ã¬ÝªO¦n¤Í¶i¤J¡A½Ð¦VªO¥D¥Ó½Ð¤J¹Ò³\\¥i¡I"

/* ----------------------------------------------------- */
/* GLOBAL VARIABLE                                       */
/* ----------------------------------------------------- */

extern char trans_buffer[256];         /* ¤@¯ë¶Ç»¼ÅÜ¼Æ add by Ptt */

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
