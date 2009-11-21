/*-------------------------------------------------------*/
/* cache.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : cache up data by shared memory               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"
#include <sys/ipc.h>
#include <sys/shm.h>

#ifdef  HAVE_MMAP
#include <sys/mman.h>
#else
int usernumber;
#endif
struct UCACHE *uidshm = NULL;
struct BCACHE *brdshm = NULL;

/*-------------------------------------------------------*/
/* .PASSWDS cache                                        */
/*-------------------------------------------------------*/

void *shm_new(int shmkey, int shmsize);
#ifndef HAVE_MMAP
static int
fillucache(uentp)
  userec *uentp;
{
  if (usernumber < MAXUSERS)
  {
    strncpy(uidshm->userid[usernumber], uentp->userid, IDLEN + 1);
    uidshm->userid[usernumber++][IDLEN] = '\0';
  }
  return 0;
}
#endif


/* -------------------------------------- */
/* (1) 系統啟動後，第一個 BBS user 剛進來 */
/* (2) .PASSWDS 更新時                    */
/* -------------------------------------- */
reload_ucache()
{
   if (uidshm->busystate)
   {
     /* ------------------------------------------ */
     /* 其他 user 正在 flushing ucache ==> CSMA/CD */
     /* ------------------------------------------ */

     if (uidshm->touchtime - uidshm->uptime > 30)
     {
       uidshm->busystate = 0;  /* leave busy state */
       uidshm->uptime = uidshm->touchtime - 1;
#if !defined(_BBS_UTIL_C_)
       log_usies("CACHE", "refork token");
#endif
     }
     else
       sleep(1);
   }
   else
   {
     uidshm->busystate = 1;    /* enter busy state */
#ifdef  HAVE_MMAP
     {
       register int fd, usernumber;

       usernumber = 0;

       if ((fd = open(".PASSWDS", O_RDONLY)) > 0)
       {
         caddr_t fimage, mimage;
         struct stat stbuf;

         fstat(fd, &stbuf);
         fimage = mmap(NULL, stbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
         if (fimage == (char *) -1)
           exit(-1);
         close(fd);
         fd = stbuf.st_size / sizeof(userec);
         if (fd > MAXUSERS)
           fd = MAXUSERS;
         for (mimage = fimage; usernumber < fd; mimage += sizeof(userec))
         {
           memcpy(uidshm->userid[usernumber++], mimage, IDLEN);
         }
         munmap(fimage, stbuf.st_size);
       }
       uidshm->number = usernumber;
     }
#else
     usernumber = 0;
     rec_apply(".PASSWDS", fillucache, sizeof(userec));
     uidshm->number = usernumber;
#endif

     /* 等 user 資料更新後再設定 uptime */
     uidshm->uptime = uidshm->touchtime;

#if !defined(_BBS_UTIL_C_)
     log_usies("CACHE", "reload ucache");
#endif
     uidshm->busystate = 0;    /* leave busy state */
   }
}

void
resolve_ucache()
{
  if (uidshm == NULL)
  {
    uidshm = shm_new(UIDSHM_KEY, sizeof(*uidshm));
    if (uidshm->touchtime == 0)
      uidshm->touchtime = 1;
  }
  while (uidshm->uptime < uidshm->touchtime)
     reload_ucache();
}

int
searchuser(char *userid)
{
  register char *ptr;
  register int i, j;
  resolve_ucache();
  i = 0;
  j = uidshm->number;
  while (i < j)
  {
    ptr = uidshm->userid[i++];
    if (!strcasecmp(ptr, userid))
    {
      strcpy(userid, ptr);
      return i;
    }
  }
  return 0;
}

#if !defined(_BBS_UTIL_C_)
int
getuser(char *userid)
{
  int uid;

  if (uid = searchuser(userid))
    rec_get(fn_passwd, &xuser, sizeof(xuser), uid);
  
  xuser.uid = uid;	/* 無論如何都要指定, 防止寫入錯誤 */
  
  return uid;
}

int
do_getuser(char *userid, userec *tuser)
{
  int uid;

  if (uid = searchuser(userid))
    rec_get(fn_passwd, tuser, sizeof(userec), uid);

  tuser->uid = uid;	/* 無論如何都要指定, 防止寫入錯誤 */

  return uid;
}

char *
getuserid(int num)
{
  if (--num >= 0 && num < MAXUSERS)
  {
    return ((char *) uidshm->userid[num]);
  }
  return NULL;
}


void
setuserid(int num, char *userid)
{
  if (num > 0 && num <= MAXUSERS)
  {
    if (num > uidshm->number)
      uidshm->number = num;
    strlcpy(uidshm->userid[num - 1], userid, IDLEN + 1);
  }
}


int
searchnewuser(mode)
  int mode;

/* 0 ==> 找過期帳號 */
/* 1 ==> 建立新帳號 */
{
  register int i, num;

  resolve_ucache();
  num = uidshm->number;
  i = 0;
  while (i < num)
  {
    if (!uidshm->userid[i++][0])
      return (i);
  }
  if (mode && (num < MAXUSERS))
    return (num + 1);
  return 0;
}

/*-------------------------------------------------------*/
/* .UTMP cache                                           */
/*-------------------------------------------------------*/

struct UTMPFILE *utmpshm;
user_info *currutmp = NULL;

void
resolve_utmp()
{
  if (utmpshm == NULL)
  {
    utmpshm = shm_new(UTMPSHM_KEY, sizeof(*utmpshm));
    if (utmpshm->uptime == 0)
      utmpshm->uptime = utmpshm->number = 1;
  }
}

void
setutmpmode(int mode)
{
  if (currstat != mode)
    currutmp->mode = currstat = mode;
}


/*
woju
*/
resetutmpent()
{
  extern int errno;
  register time_t now;
  register int i;
  register pid_t pid;
  register user_info *uentp;

  resolve_utmp();
  now = time(NULL) - 79;
  if (now > utmpshm->uptime)
    utmpshm->busystate = 0;

  while (utmpshm->busystate)
  {
    sleep(1);
  }
  utmpshm->busystate = 1;
  /* ------------------------------ */
  /* for 幽靈傳說: 每 79 秒更新一次 */
  /* ------------------------------ */

  for (i = now = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (pid = uentp->pid)
    {
       if ((kill(pid, 0) == -1) && (errno == ESRCH))
           memset(uentp, 0, sizeof(user_info));
        else
           now++;
    }
  }
  utmpshm->number = now;
  time(&utmpshm->uptime);
  utmpshm->busystate = 0;
}


void
getnewutmpent(up)
  user_info *up;
{
  extern int errno;
  register time_t now;
  register int i;
  register pid_t pid;
  register user_info *uentp;

  resolve_utmp();
  now = time(NULL) - 79;
  if (now > utmpshm->uptime)
    utmpshm->busystate = 0;

  while (utmpshm->busystate)
  {
    sleep(1);
  }
  utmpshm->busystate = 1;
  /* ------------------------------ */
  /* for 幽靈傳說: 每 79 秒更新一次 */
  /* ------------------------------ */

  if (now > utmpshm->uptime)
  {
    for (i = now = 0; i < USHM_SIZE; i++)
    {
      uentp = &(utmpshm->uinfo[i]);
      if (pid = uentp->pid)
      {
        if ((kill(pid, 0) == -1) && (errno == ESRCH))
          memset(uentp, 0, sizeof(user_info));
        else
          now++;
      }
    }
    utmpshm->number = now;
    time(&utmpshm->uptime);
  }

  for (i = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (!(uentp->pid))
    {
      memcpy(uentp, up, sizeof(user_info));
      currutmp = uentp;
      utmpshm->number++;
      utmpshm->busystate = 0;
      return;
    }
  }
  utmpshm->busystate = 0;
  sleep(1);
  exit(1);
}


int
apply_ulist(fptr)
  int (*fptr) ();
{
  register user_info *uentp;
  register int i, state;

  resolve_utmp();
  for (i = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (uentp->pid && (PERM_HIDE(currutmp) || !PERM_HIDE(uentp)))
      if (state = (*fptr) (uentp))
        return state;
  }
  return 0;
}


user_info *
search_ulist(fptr, farg)
  int (*fptr) ();
int farg;
{
  register int i;
  register user_info *uentp;

  resolve_utmp();
  for (i = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if ((*fptr) (farg, uentp))
      return uentp;
  }
  return 0;
}


int
count_multi()
{
  register int i, j;
  register user_info *uentp;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (uentp->uid == usernum)
      j++;
  }
  return j;
}


/* -------------------- */
/* for multi-login talk */
/* -------------------- */

user_info *
search_ulistn(fptr, farg, unum)
  int (*fptr) ();
int farg;
int unum;
{
  register int i, j;
  register user_info *uentp;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if ((*fptr) (farg, uentp))
    {
      if (++j == unum)
        return uentp;
    }
  }
  return 0;
}


int
count_logins(fptr, farg, show)
  int (*fptr) ();
int farg;
int show;
{
  register user_info *uentp;
  register int i, j;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if ((*fptr) (farg, uentp))
    {
      j++;
      if (show)
      {
        prints("(%d) 目前狀態為: %-17.16s(來自 %s)\n", j,
          modestring(uentp, 0), uentp->from);
      }
    }
  }
  return j;
}


void
purge_utmp(uentp)
  user_info *uentp;
{
  memset(uentp, 0, sizeof(user_info));
  if (utmpshm->number)
    utmpshm->number--;
}


/*copy count_ulist過來, 用 guest 的權限算站上人數 hialan.020722*/

int
guest_count_ulist()
{
   int ans = utmpshm->number;
   register user_info *uentp;
   int ch = 0;

   while (ch < USHM_SIZE) 
   {
      uentp = &(utmpshm->uinfo[ch++]);
      if (uentp->pid && 
          (uentp->invisible || (uentp->userlevel & PERM_DENYPOST) ))
         --ans;
   }

   return ans;
}

int
count_ulist()
{
   int ans = utmpshm->number;
   register user_info *uentp;
   int ch = 0;

   while (ch < USHM_SIZE) {
      uentp = &(utmpshm->uinfo[ch++]);
      if (uentp->pid && (
          is_rejected(uentp) & 2 && !HAS_PERM(PERM_SYSOP) ||
          uentp->invisible && !HAS_PERM(PERM_SEECLOAK) &!HAS_PERM(PERM_SYSOP) ||
          !PERM_HIDE(currutmp) && PERM_HIDE(uentp) ||
          cuser.uflag & FRIEND_FLAG && !is_friend(uentp)
         ))
         --ans;
   }

   return ans;
}

#endif



/*-------------------------------------------------------*/
/* .BOARDS cache                                         */
/*-------------------------------------------------------*/
boardheader *bcache;
int numboards = -1;
int brd_semid;

/* force re-caching */

void
touch_boards()
{
  time(&(brdshm->touchtime));
  numboards = -1;
}

void
reload_bcache()
{
   if (brdshm->busystate)
   {
     sleep(1);
   }
#if !defined(_BBS_UTIL_C_)
   else
   {
     int fd;

     brdshm->busystate = 1;
     sem_init(BRDSEM_KEY,&brd_semid);
     sem_lock(SEM_ENTER,brd_semid);

     if ((fd = open(fn_board, O_RDONLY)) > 0)
     {
       brdshm->number = read(fd, bcache, MAXBOARD * sizeof(boardheader))
         / sizeof(boardheader);
       close(fd);
     }

     /* 等所有 boards 資料更新後再設定 uptime */

     brdshm->uptime = brdshm->touchtime;
#if !defined(_BBS_UTIL_C_)
     log_usies("CACHE", "reload bcache");
#endif
     sem_lock(SEM_LEAVE,brd_semid);
     brdshm->busystate = 0;
   }
#endif
}

void
resolve_boards()
{
  if (brdshm == NULL)
  {
    brdshm = shm_new(BRDSHM_KEY, sizeof(*brdshm));
    if (brdshm->touchtime == 0)
      brdshm->touchtime = 1;
    bcache = brdshm->bcache;
  }

  while (brdshm->uptime < brdshm->touchtime)
     reload_bcache();

  numboards = brdshm->number;
}

int
getbnum(char *bname)
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i++ < numboards; bhdr++)
  {
#if !defined(_BBS_UTIL_C_)
  if (Ben_Perm(bhdr)) 
#endif
    if (!strcasecmp(bname, bhdr->brdname))
      return i;
  }
  return 0;
}

#if !defined(_BBS_UTIL_C_)
int
apply_boards(func)
  int (*func) (boardheader *);
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
  {
    if (Ben_Perm(bhdr))
      if ((*func) (bhdr) == QUIT)
        return QUIT;
  }
  return 0;
}

boardheader *
getbcache(char *bname)
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
  if (Ben_Perm(bhdr)) 
    if (!strcasecmp(bname, bhdr->brdname))
      return bhdr;
  return NULL;
}

char *
getbname(int bid)
{
  resolve_boards();

  if(bcache[bid-1].brdname[0] != '\0')
    return bcache[bid-1].brdname;
  else
    return " ";
}

int
haspostperm(char *bname)
{
  register int i;

/*
  if (currmode & MODE_DIGEST || currmode & MODE_ETC)
    return 0;
*/
  if (!strcasecmp(bname, DEFAULT_BOARD))
    return 1;

  if (!HAS_PERM(PERM_BASIC))
    return 0;

  if (!(i = getbnum(bname)))
    return 0;

/*  
  hialan: 原則上，如果在可見名單被設為壞人，是不應該進入該看板的:p
  
  if (hbflcheck(i, currutmp->uid) == 2)
    return 0;
*/

  /* 秘密看板特別處理 */

  if (bcache[i - 1].brdattr & BRD_HIDE)
    return 1;

  i = bcache[i - 1].level;
  return (HAS_PERM(i & ~PERM_POST));
}
#endif

/*-------------------------------------------------------*/
/* 看板文章總數 cache                                    */
/*-------------------------------------------------------*/

int
touchbtotal(int bid)
{
  resolve_boards();
  
  if(bid > 0)
  {
    brdshm->total[bid - 1] = 0;
    brdshm->lastposttime[bid - 1] = 0;
  }
  return 0;
}

#if !defined(_BBS_UTIL_C_)
int
getbtotal(int bid)
{
  resolve_boards();
  return brdshm->total[bid - 1];
}

void
setbtotal(int bid)
{
  boardheader *bh = &bcache[--bid];
  fileheader fhdr;
  char genbuf[PATHLEN];
  int num;

  if(!bh->brdname || (bh->brdattr & BRD_CLASS) || (bh->brdattr & BRD_GROUPBOARD))
  {
    brdshm->total[bid] = 0;
    brdshm->lastposttime[bid] = 0;
    return ;
  }
  
  setbdir(genbuf, bh->brdname);

  num = rec_num(genbuf, sizeof(fileheader));
  brdshm->total[bid] = num;

  if (num > 0)
  {
    if(rec_get(genbuf, &fhdr, sizeof(fhdr), num) != -1)
      brdshm->lastposttime[bid] = (time_t) atoi(fhdr.filename+2);
  }
  else
    brdshm->lastposttime[bid] = 0;
    
  return ;
}
#endif

#if !defined(_BBS_UTIL_C_)
/*-------------------------------------------------------*/
/* 看板可見名單 cache                                    */
/*-------------------------------------------------------*/

void
hbflreload(int bid)
{
  char genbuf[PATHLEN];
  PAL pal;
  HBFL hbfl;
  int fd, i;
  ushort uid;

  memset(hbfl.uid, 0, sizeof(hbfl.uid));
  memset(hbfl.type, 0, sizeof(hbfl.type));

  setbfile(genbuf, bcache[bid - 1].brdname, FN_LIST);

  if ((fd = open(genbuf, O_RDONLY)) > 0)
  {
    for (i = 0; i < MAX_FRIEND; i++)
    {
      if (read(fd, &pal, sizeof(pal)) == sizeof(pal))
      {
        if ((uid = (ushort) searchuser(pal.userid)) != 0)
        {
          hbfl.uid[i] = uid;
	  hbfl.type[i] = pal.ftype;
        }
	else
	  i--;
      }
      else
	break;
    }
    close(fd);
  }
  memcpy(brdshm->hbfl[bid - 1].uid, hbfl.uid, sizeof(hbfl.uid));
  memcpy(brdshm->hbfl[bid - 1].type, hbfl.type, sizeof(hbfl.type));

  time(&(brdshm->hbfl[bid - 1].lastloadtime));
}

int
hbflcheck(int bid, int uid)
{
  const int pos = bid - 1;
  int i, can = 0;

  if (bid > 0)
  {
    if (brdshm->hbfl[bid - 1].lastloadtime < login_start_time - HBFLexpire)
      hbflreload(bid);

    for (i = 0; brdshm->hbfl[pos].uid[i] != 0 && i < MAX_FRIEND; i++)
    {
      if (brdshm->hbfl[pos].uid[i] == uid)
      {
        if (brdshm->hbfl[pos].type[i] & M_BAD)
 	  can = 2;
        else
  	  can = 1;
        break;
      }
    }
  }
  return can;
}

int 
hbflcount(int bid, int mode)
{
  int i, count=0;
  const int pos = bid - 1;
  
  if(bid > 0)
  {
    for(i=0; brdshm->hbfl[pos].uid[i] != 0 && i < MAX_FRIEND; i++)
    {
      if(brdshm->hbfl[pos].type[i] & mode)
        count++;
    }
    
    return count;
  }
  
  return -1;
}

/*-------------------------------------------------------*/
/* FILM  cache                                           */
/*-------------------------------------------------------*/
#ifdef CAMERA

static FCACHE *fshm;

void
fshm_init()
{
  if (fshm == NULL)
    fshm = shm_new(FILMSHM_KEY, sizeof(FCACHE));
}


/* ----------------------------------------------------- */
/* itoc.020822.註解:                            `        */
/* ----------------------------------------------------- */
/* 第 0 ～ FILM_MOVIE-1 張是系統畫面及說明畫面           */
/* 第 FILM_MOVIE ～ fmax-1 張是動態看板                  */
/* ----------------------------------------------------- */
/* tag:                                                  */
/* 0 ～ fmax-1 → 撥放該張畫面                           */
/* >= fmax     → 撥放第 FILM_MOVIE 張                   */
/* ----------------------------------------------------- */
/* row:                                                  */
/* >=0 → 系統畫面，從 (row, 0) 開始印                   */
/* <0  → 說明畫面，從 (0, 0) 開始印，最後會 vmsg(NULL)  */
/* ----------------------------------------------------- */
/* return: 現在撥放的這張 tag 號碼                       */
/* ----------------------------------------------------- */

int
film_out(int tag, int row)
                  /* -1 : help */
{
  int fmax, len, *shot;
  char *film, buf[FILM_SIZ];

  len = 0;
  shot = fshm->shot;

  while (!(fmax = *shot))       /* util/camera.c 正在換片 */
  {
    sleep(5);
    if (++len > MOVIE_LINES)
      return FILM_MOVIE;
  }

  if (row <= 0)
    clear();
  else
  {
    move(row, 0);
    clrtobot();
  }

  /* sby: 目前還沒有動態看板 */
  if (tag >= FILM_MOVIE && fmax <= FILM_MOVIE)
    return FILM_MOVIE;

  if (tag >= fmax)		/* 超過了 fmax，從 FILM_MOVIE 開始重新撥放 */
    tag = FILM_MOVIE;

  film = fshm->film;

#if 0
  if (tag >= FILM_MOVIE)        /* random select */
  {
    if (tag >= fmax)
      tag = FILM_MOVIE + (time(0) % fshm->shot[0]);
    else {
      tag += (time(0) % fshm->shot[0]);
      if (tag >= fmax)
        tag = FILM_MOVIE + (time(0) % fshm->shot[0]);
    }
  } //FILM_MOVIE = 20;
   /* Thor.980804: 可能是故意的吧? 第一張 random select前10個其中一個 */
#endif

  if (tag)
  {
    len = shot[tag];
    film += len;
    len = shot[++tag] - len;
  }
  else
    len = shot[1];

  if (len >= FILM_SIZ - MOVIE_LINES)
    return tag;

  memcpy(buf, film, len);
  buf[len] = '\0';

  outs(Ptt_prints(buf, NO_RELOAD));

  if (row < 0)                  /* help screen */
    pressanykey(NULL);

  return tag;
}

char today_is[20];

void
feast_init()
{
  FILE *fp;
  char *chr;

  if ((fp = fopen("etc/today_is","r")) != NULL)
  {
    fgets(today_is, 19, fp);
    if ((chr = strchr(today_is, '\n')) != NULL) 
      *chr = 0;
    today_is[19] = 0;
    fclose(fp);
  }
}

extern char lenth[MOVIE2_LINES];

void
m2_out(int tag, int row)
{
  if (row == -1)
  {
    register int i;

    for (i = 0; i < MOVIE2_LINES; i++)
    {
      move(i + 3, lenth[i]);
      outs(fshm->movie2[tag * MOVIE2_LINES + i]);
    }
  }
  else
    outs(fshm->movie2[tag * MOVIE2_LINES + row]);
}

#endif	/* #ifdef CAMERA */
#endif  /* #if !defined(_BBS_UTIL_C_) */

/*-------------------------------------------------------*/
/* FROM cache                                            */
/*-------------------------------------------------------*/
/* cachefor from host 與最多上線人數 */
struct FROMCACHE *fcache;
int fcache_semid;

reload_fcache()
{
   if (fcache->busystate)
   {
     /*sleep(1);*/
   }
   else
   {
     FILE *fp;
     fcache->busystate = 1;
     sem_init(FROMSEM_KEY,&fcache_semid);
     sem_lock(SEM_ENTER,fcache_semid);
     bzero(fcache->domain, sizeof fcache->domain);
     if(fp=fopen(BBSHOME"/etc/domain_name_query","r"))
     {
       char buf[256],*po;
       fcache->top=0;
       while (fgets(buf,256,fp))
       {
         if(buf[0] && buf[0] != '#' && buf[0] != ' ' && buf[0] != '\n')
         {
           sscanf(buf,"%s",fcache->domain[fcache->top]);
           po = buf + strlen(fcache->domain[fcache->top]);
           while(*po == ' ') po++;
           strlcpy(fcache->replace[fcache->top],po,49);
           fcache->replace[fcache->top][strlen(fcache->replace[fcache->top])-1] = 0; 
           (fcache->top)++;
         }
       }
     }

     fcache->max_user=0;

     /* 等所有資料更新後再設定 uptime */
     fcache->uptime = fcache->touchtime;
#if !defined(_BBS_UTIL_C_)
     log_usies("CACHE", "reload fcache");
#endif
     sem_lock(SEM_LEAVE,fcache_semid);
     fcache->busystate = 0;
   }
}

void
resolve_fcache()
{
  if (fcache == NULL)
  {
    fcache = shm_new(FROMSHM_KEY, sizeof(*fcache));
    if (fcache->touchtime == 0)
      fcache->touchtime = 1;
  }
  while (fcache->uptime < fcache->touchtime) reload_fcache();
}

#if !defined(_BBS_UTIL_C_)
user_info* searchowner(userid)	/* alan.000422: owner online */
  char *userid;
{
  register int i, j;
  register user_info *uentp;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (!strcasecmp(userid, uentp->userid) && str_cmp(userid, cuser.userid))
    {
      if ((uentp->invisible && !HAVE_PERM(PERM_SEECLOAK))
         || PERM_HIDE(uentp) && (!PERM_HIDE(currutmp) || !HAS_PERM(PERM_SYSOP))
         || (is_rejected(uentp) && !HAS_PERM(PERM_SYSOP)))
        return NULL;
      return uentp;
    }
  }
  return NULL;
}
#endif
