/*-------------------------------------------------------*/
/* config.h     ( WD-BBS Ver 2.3 )                       */
/*-------------------------------------------------------*/
/* target : site-configurable settings                   */
/* create : 95/03/29                                     */
/* update : 98/12/09                                     */
/*-------------------------------------------------------*/

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* ----------------------------------------------------- */
/* 定義 BBS 站名位址                                     */
/* ------------------------------------------------------*/

#define BOARDNAME       "雅典娜"                        /* 中文站名 */
#define BBSNAME         "Athena BBS"                    /* 英文站名 */
#define MYHOSTNAME      "Athena.ATCity.org"             /* 網址 */
#define MYIP            "210.201.176.99"                /* IP */
#define MYVERSION       "AT-BBS v3.0 (WD_hialan_yrchen)"/* 程式版本 */
#define MYBBSID         "AT-BBS"                        /* 轉信代碼 */
#define BBSHOME         "/home/bbs"                     /* BBS 的家 */
#define BBSUSER         "bbs"
#define BBSUID          9999
#define BBSGID          99
#define TAG_VALID       "[AT-BBS]To "
                        /* shakalaca.000814: 改用 MagicKey 認證
                                             其實就不用這個玩意了 :p */

/* ----------------------------------------------------- */
/* 組態規劃                                              */
/* ------------------------------------------------------*/

#define HAVE_CHKLOAD            /* check cpu load */
#ifdef HAVE_CHKLOAD
  #define MAX_CPULOAD     (10)            /* CPU 最高load */
  #define MAX_SWAPUSED    (10)            /* SWAP最高使用率 */
#endif

#define WITHOUT_CHROOT          /* 不需要 root set-uid */
#define HAVE_MOVIE              /* 顯示動態告示板 */
#define INTERNET_PRIVATE_EMAIL  /* 可以寄私人信件到 Internet */
#define BIT8                    /* 支援中文系統 */
#define DEFAULTBOARD    "SYSOP" /* 預設看板 */
#define LOGINATTEMPTS   (3)     /* 最大進站失誤次數 */
#define INTERNET_EMAIL          /* 支援 InterNet Email 功能(含 Forward) */
#undef  NEWUSER_LIMIT           /* 新手上路的三天限制 */
#undef  INVISIBLE               /* 隱藏 BBS user 來自何處 */
#define MULTI_NUMS      (1)     /* 最多重複上站人次 (guest除外) */
#define INITIAL_SETUP           /* 剛開站，還沒建立預設看板[SYSOP] */
#define HAVE_MMAP               /* 採用 mmap(): memory mapped I/O */
#define CAMERA                  /* 使用 camera 加速動態看板 */

#define HAVE_ANONYMOUS          /* 提供 Anonymous 板 */
#define HAVE_ORIGIN             /* 顯示 author 來自何處 */
#define HAVE_MAILCLEAN          /* 清理所有使用者個人信箱 */
#define WHERE                   /* 是否有故鄉功能 */
#define HAVE_NOTE_2             /* wildcat:小看板功能 */
#define HAVE_GEM_GOPHER         /* shakalaca: 連線精華區 */
#define gold_lower_bound (100)      /* 低於 100 金者不扣稅 */
#define exp_lower_bound (1000)      /* */
#define tax_rate_a      (1000)      /* 稅率 A  (千分之１? -*/
#define tax_rate_b      (100)       /* 稅率 B  (百分之１金) */
#define HAVE_ALLPOST            /* 提供所有文章查詢 */

#define LOGINASGUEST            /* 提供 guest 上站 */
#define LOGINASNEW              /* 採用上站申請帳號制度 */
#ifdef LOGINASNEW
  #define ATREGISTERMODE        /* 雅典娜註冊模式 (仿貓咪樂園) */
  #define FORM_REG              /* shakalaca: 填註冊單 */
  #define REG_MAGICKEY          /* 發出 MagicKey eMail 身份認證信函 */
  #define REG_FORM              /* shakalaca: 填註冊單 */
                                /* shakalaca.000813: 認證方式請擇一 */
#endif

/* ----------------------------------------------------- */
/* 隨 BBS 站規模成長而擴增                               */
/* ----------------------------------------------------- */

#define MAXUSERS        (65536)         /* 最高註冊人數 */
#define MAXBOARD        (1024)          /* 最大開版個數 */
#define MAXACTIVE       (512)           /* 最多同時上站人數 */
#define MAX_FRIEND      (128)           /* 載入 cache 之最多朋友數目 */
#define MAX_REJECT      (64)            /* 載入 cache 之最多壞人數目 */
#define MAX_MOVIE       (999)           /* 最多動態看版數 */
#define MAX_FROM        (1024)          /* 最多故鄉數 */
#define MAX_REVIEW      (12)            /* 最多水球回顧 */

/* ----------------------------------------------------- */
/* 其他系統上限參數                                      */
/* ----------------------------------------------------- */

#define MAX_HISTORY     12              /* 動態看板保持 12 筆歷史記錄 */
#define MAXKEEPMAIL     (100)           /* 最多保留幾封 MAIL？ */
#define MAXEXKEEPMAIL   (400)           /* 最多信箱加大多少封 */
#define MAX_NOTE        (32)            /* 最多保留幾篇留言？ */
#define MAXSIGLINES     (8)             /* 簽名檔引入最大行數 */
#define MAXQUERYLINES   (16)            /* 顯示 Query/Plan 訊息最大行數 */
#define MAXPAGES        (999)           /* more.c 中文章頁數上限 (lines/22) */
#define MOVIE_INT       (10)            /* 切換動畫的週期時間 12 秒 */
#define MAXTAGS         (256)           /* post/mail reader 標籤數目之上限 */

/* ----------------------------------------------------- */
/* 發呆過久自動簽退                                      */
/* ------------------------------------------------------*/

#define LOGIN_TIMEOUT   (1 * 60)                /* login 時多久沒成功簽入就斷線 */

#define  DOTIMEOUT

#ifdef  DOTIMEOUT
#define IDLE_TIMEOUT    (24 * 60 * 60)  /* 一般情況之 timeout (秒)*/
#define SHOW_IDLE_TIME                  /* 顯示閒置時間 */
#endif

/* ----------------------------------------------------- */
/* chat.c & xchatd.c 中採用的 port 及 protocol           */
/* ------------------------------------------------------*/

#define CHATPORT        3838

#define MAXROOM         16              /* 最多有幾間包廂？ */

#define EXIT_LOGOUT     0
#define EXIT_LOSTCONN   -1
#define EXIT_CLIERROR   -2
#define EXIT_TIMEDOUT   -3
#define EXIT_KICK       -4

#define CHAT_LOGIN_OK       "OK"
#define CHAT_LOGIN_EXISTS   "EX"
#define CHAT_LOGIN_INVALID  "IN"
#define CHAT_LOGIN_BOGUS    "BG"

#define BADCIDCHARS " *:/\,;.?"        /* Chat Room 中禁用於 nick 的字元 */


/* ----------------------------------------------------- */
/* 系統參數      cache                                   */
/* ----------------------------------------------------- */
#define MAGIC_KEY       2003   /* 身分認證信函編碼 */

#define SEM_ENTER      -1      /* enter semaphore */
#define SEM_LEAVE      1       /* leave semaphore */
#define SEM_RESET      0       /* reset semaphore */

#define BRDSHM_KEY      1515
#define UIDSHM_KEY      1517
#define UTMPSHM_KEY     1519
#define FILMSHM_KEY     1521    /* 動態看版 , 節日 */
#define FROMSHM_KEY     1523    /* whereis, 最多使用者 */

#define BRDSEM_KEY      2505
#define FILMSEM_KEY     2500    /* semaphore key */
#define FROMSEM_KEY     2503    /* semaphore key */

/* ----------------------------------------------------- */
/* 申請帳號時要求申請者真實資料                          */
/* ----------------------------------------------------- */

#define SHOWUID                 /* 看見使用者 UID */
#define SHOWTTY                 /* 看見使用者 TTY */
#define SHOWPID                 /* 看見使用者 PID */

#define REALINFO                /* 真實姓名 */

#ifdef  REALINFO
#undef  ACTS_REALNAMES          /* 主目錄的 (U)ser 顯示真實姓名 */
#undef  POST_REALNAMES          /* 貼文件時附上真實姓名 */
#undef  MAIL_REALNAMES          /* 寄站內信件時附上真實姓名 */
#undef  QUERY_REALNAMES         /* 被 Query 的 User 告知真實姓名 */
#endif

/* ----------------------------------------------------- */
/* http                                                  */
/* ----------------------------------------------------- */

#define USE_LYNX              /* 使用外部lynx dump ? */
#undef USE_PROXY
#ifdef  USE_PROXY
#define PROXYSERVER "140.112.28.165"
#define PROXYPORT   3128
#endif

#define LOCAL_PROXY           /* 是否開啟local 的proxy */
#ifdef  LOCAL_PROXY
#define HPROXYDAY   3         /* local的proxy refresh天數 */
#endif

/* ----------------------------------------------------- */
/* 提供外掛程式                                          */
/* ----------------------------------------------------- */

#define HAVE_EXTERNAL

  #ifdef HAVE_EXTERNAL
    #undef HAVE_GOPHER     /* 提供 gopher */
    #undef HAVE_WWW        /* 提供 www browser */
    #undef HAVE_ARCHIE     /* have arche */
    #define HAVE_GAME       /* 提供網路連線遊戲 */
    #define HAVE_NETTOOL   /* 提供網路服務 */
  #endif

#endif

#define LYNX_PATH       "/usr/local/bin/lynx --source"  /* lynx 的絕對路徑 */

/* ----------------------------------------------------- */
/* 其他                                                  */
/* ----------------------------------------------------- */

#define ANNOUNCE_BRD    "Announce"
#define VOTEBOARD       "VoteBoard"
#define PERSONAL_ALL_BRD        "Personal_All"
#define GROUP_ALL_BRD   "Group_All"
#define HIDE_ALL_BRD    "Hide_All"

#define DEF_MAXP        8000    /* 看板文章基本上限數量 */
#define DEF_MAXT        365     /* 看板文章基本保留天數 */

#define COLOR1          "\033[46m"
#define COLOR2          "\033[0;44m"
#define COLOR3          "\033[0;30;47m"         /* Title Color*/
#define DEFBARCOLOR     {4, 7, 1, 0, 0}         /* Default Lightbar color */

#define MAXMONEY(cuser) ((cuser.totaltime*10) + (cuser.numlogins*100) + (cuser.numposts*1000))
  /* 金錢上限 hialan:030131*/

#ifndef HBFLexpire
#define HBFLexpire      (432000)        /* 5 days 看板可見名單 */
#endif
