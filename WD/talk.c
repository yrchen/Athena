/*-------------------------------------------------------*/
/* talk.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : talk/quety/friend routines                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#define _MODES_C_

#include "bbs.h"
#include "type.h"

#ifdef lint
#include <sys/uio.h>
#endif

#define IRH 1	/* I Rejected Him */
#define HRM 2	/* He Rejected Me */

extern int bind();
extern int cmpuname();
/* extern char currdirect[]; */

/* -------------------------- */
/* 記錄 friend 的 user number */
/* -------------------------- */

#define PICKUP_WAYS     6
int pickup_way = 0;
int friendcount;
int friends_number;
int override_number;
int rejected_number;
int bfriends_number;
char *fcolor[11] = {"[m","[36m","[32m","[1;32m",
                   "[33m","[1;33m","[1;37m" ,"[1;37m",
                   "[1;31m", "[1;35m", "[1;36m"};
char *talk_uent_buf;
char save_page_requestor[40];
char page_requestor[40];


void friend_load();

int is_rejected(user_info *ui);

char *
modestring(user_info *uentp, int simple)
{
  static char modestr[40];
  static char *notonline="不在站上";
  register int mode = uentp->mode;
  register char *word;

  word = ModeTypeTable[mode];

  if (!(HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_SEECLOAK)) &&
      (uentp->invisible || (is_rejected(uentp) & HRM)))
    return (notonline);
  else if (mode == EDITING) 
  {
     sprintf(modestr, "E:%s",
         ModeTypeTable[uentp->destuid < EDITING ? uentp->destuid : EDITING]);
     word = modestr;
  }
  else if (!mode && *uentp->chatid == 1)
  {
     if (!simple)
        sprintf(modestr, "回應 %s", getuserid(uentp->destuid));
     else
        sprintf(modestr, "回應呼叫");
  }

  else if (!mode && *uentp->chatid == 3)
     sprintf(modestr, "水球準備中");
  else if (!mode)
    return (uentp->destuid == 6) ? uentp->chatid :
      IdleTypeTable[(0 <= uentp->destuid & uentp->destuid < 6) ?
                    uentp->destuid: 0];
  else if (simple)
    return (word);

  else if (uentp->in_chat & mode == CHATING)
    sprintf(modestr, "%s (%s)", word, uentp->chatid);
  else if (mode == TALK)
  {
    if (is_hidden(getuserid(uentp->destuid)))    /* Leeym 對方(紫色)隱形 */
      sprintf(modestr, "%s", "自言自語中"); /* Leeym 大家自己發揮吧！ */
    else
      sprintf(modestr, "%s %s", word, getuserid(uentp->destuid));
  }
  else if (mode != PAGE && mode != QUERY)
    return (word);
  else
    sprintf(modestr, "%s %s", word, getuserid(uentp->destuid));

  return (modestr);
}


int
cmpuids(uid, urec)
  int uid;
  user_info *urec;
{
  return (uid == urec->uid);
}

int
cmppids(pid, urec)
  pid_t pid;
  user_info *urec;
{
  return (pid == urec->pid);
}

int     /* Leeym 從 FireBird 移植改寫過來的 */
is_hidden(char *user)
{
    int tuid;
    user_info *uentp;

  if ((!(tuid = getuser(user)))
  || (!(uentp = (user_info *) search_ulist(cmpuids, tuid)))
  || ((!uentp->invisible|| HAS_PERM(PERM_SYSOP)||HAS_PERM(PERM_SEECLOAK))
        && (((!PERM_HIDE(uentp) && !PERM_HIDE(currutmp)) ||
        PERM_HIDE(currutmp))
        && !(is_rejected(uentp) & HRM && !(is_friend(uentp) & 2)))))
        return 0;       /* 交談 xxx */
  else
        return 1;       /* 自言自語 */
}

/* ------------------------------------- */
/* routines for Talk->Friend             */
/* ------------------------------------- */

int is_friend(user_info *ui)
{
  register ushort unum, hit, *myfriends;

  /* 判斷對方是否為我的朋友 ? */

  unum = ui->uid;
  myfriends = currutmp->friend;
  while (hit = *myfriends++)
  {
    if (unum == hit)
    {
      hit = 3;
      friends_number++;
      break;
    }
  }

  /* 看板好友 */

  if(currutmp->brc_id && ui->brc_id == currutmp->brc_id)
  {
    hit |= 1;
    bfriends_number++;
  }

  /* 判斷我是否為對方的朋友 ? */

  myfriends = ui->friend;
  while (unum = *myfriends++)
  {
    if (unum == currutmp->uid)
    {
      override_number++;
      hit |= 5;
      break;
    }
  }
  return hit;
}

  /* 被拒絕 */

int
is_rejected(user_info *ui)
{
  register ushort unum, hit, *myrejects;

  if (PERM_HIDE(ui))
     return 0;

  /* 判斷對方是否為我的仇人 ? */

  unum = ui->uid;
  myrejects = currutmp->reject;
  while (hit = *myrejects++)
  {
    if (unum == hit)
    {
      hit = IRH;
      rejected_number++;
      break;
    }
  }

  if(hit != IRH)
    hit = 0;
    
  /* 判斷我是否為對方的仇人 ? */

  myrejects = ui->reject;
  while (unum = *myrejects++)
  {
    if (unum == currutmp->uid)
    {
      if (hit & IRH)
         --rejected_number;
      hit |= HRM;
      break;
    }
  }
  return hit;
}


/* ------------------------------------- */
/* 真實動作                              */
/* ------------------------------------- */

static void my_kick(user_info *uentp)
{
  if (getans2(1, 0, msg_sure, 0, 2, 'n') == 'y')
  {
    char genbuf[200];  
    sprintf(genbuf, "%s (%s)", uentp->userid, uentp->username);
    log_usies("KICK ", genbuf);
    if ((kill(uentp->pid, SIGHUP) == -1) && (errno == ESRCH))
      memset(uentp, 0, sizeof(user_info));
    /* purge_utmp(uentp); */
    outz("踢出去囉");
  }
  else
    outz(msg_cancel);
}

int 
my_query(char *uident)
{
  extern char currmaildir[];
  int tuid,i;
  unsigned long int j;
  user_info *uentp;
  userec muser;
  char *money[10] = {"乞丐","赤貧","清寒","普通","小康",
                     "小富翁","中富翁","大富翁","富可敵國","比爾丐錙"};

  if (tuid = getuser(uident))
  {
    memcpy(&muser, &xuser, sizeof(userec));
    move(0, 0);
    clrtobot();
    move(1, 0);
    setutmpmode(QUERY);
    currutmp->destuid = tuid;

    j = muser.silvermoney + (10000 * muser.goldmoney);
    for(i=0;i<10 && j>10;i++) j /= 100;
    prints("[ 帳  號 ]%-30.30s[ 暱  稱 ]%s\n",muser.userid,muser.username);
    if (pal_type(muser.userid, cuser.userid) || HAS_PERM(PERM_SYSOP)
        || !strcmp(muser.userid, cuser.userid) )
    {
      char *sex[8] = { MSG_BIG_BOY, MSG_BIG_GIRL,
                       MSG_LITTLE_BOY, MSG_LITTLE_GIRL,
                       MSG_MAN, MSG_WOMAN, MSG_PLANT, MSG_MIME };
      prints("[ 性  別 ]%-30.30s",sex[muser.sex%8]);
    }

    prints("[ 心  情 ][1;33m%s[m\n",muser.feeling);
    uentp = (user_info *) search_ulist(cmpuids, tuid);
    if (uentp && !(PERM_HIDE(currutmp) ||
      is_rejected(uentp) & HRM && is_friend(uentp) & 2) && PERM_HIDE(uentp))
      prints("[目前動態][1;30m不在站上                      [m\n");
    else
      prints("[目前動態][1;36m%-30.30s[m",
         uentp ? modestring(uentp, 0) : "[1;30m不在站上");

    prints("[新信未讀]");
    sethomedir(currmaildir, muser.userid);
    outs(chkmail(1) || muser.userlevel & PERM_SYSOP ? "[1;5;33m有" : "[1;30m無");
    sethomedir(currmaildir, cuser.userid);
    chkmail(1);
    outs("\033[m\n");
    if (HAS_PERM(PERM_SYSOP))  
      prints("[真實姓名]%s\n", muser.realname);

    prints("[1;36m%s[m\n", msg_seperator);
    prints("[上站地點]%-30.30s",muser.lasthost[0] ? muser.lasthost : "(不詳)");
    prints("[上站時間]%s\n",Etime(&muser.lastlogin));
    prints("[上站次數]%-30d",muser.numlogins);
    prints("[發表文章]%d 篇\n",muser.numposts);
    prints("[人氣指數]%-30d[好奇指數]%d\n",muser.bequery,muser.toquery);
    prints("[水球次數]收 %d / 發 %d \n",muser.receivemsg,muser.sendmsg);
    if(HAS_PERM(PERM_SYSOP))
      prints("[前次查詢]%-30.30s[被查詢]%s\n",muser.toqid,muser.beqid);

    prints("[1;36m%s[m\n", msg_seperator);

    prints("[經濟狀況]%s\n",money[i]);
    if (HAS_PERM(PERM_SYSOP) || !strcmp(muser.userid, cuser.userid))
      prints("[金幣數量]%-30ld[銀幣數量]%-21ld\n[銀幣上限]%ld\n"
             ,muser.goldmoney,muser.silvermoney,MAXMONEY(muser));

    if(strcmp(muser.beqid,cuser.userid))
    {
      strcpy(muser.beqid,cuser.userid);
      ++muser.bequery;
      substitute_record(fn_passwd, &muser,sizeof(userec), tuid);
    }

    if(strcmp(muser.userid,cuser.toqid))
    {
      update_data();
      strcpy(cuser.toqid,muser.userid);
      ++cuser.toquery;
      substitute_record(fn_passwd, &cuser, sizeof(userec), currutmp->uid);
    }
    pressanykey(NULL);
    return RC_FULL;
  }
  update_data();
  return RC_NONE;
  /* currutmp->destuid = 0; */
}


static void
my_talk(user_info *uin)
{
  int sock, msgsock, length, ch;
  struct sockaddr_in sin;
  pid_t pid;
  char c, genbuf;
  uschar mode0 = currutmp->mode;
  char *choose_ques[4]={"yY)聊天", "cC)象棋", "dD)暗棋", msg_choose_cancel};

  ch = uin->mode;
  strcpy(currutmp->chatid,uin->userid);
  strcpy(currauthor, uin->userid);

  if (ch == EDITING || ch == TALK || ch == CHATING
      || ch == PAGE || ch == MAILALL || ch == FIVE || ch == IDLE   //IDLE by hialan
      || !ch && (uin->chatid[0] == 1 || uin->chatid[0] == 3))
    pressanykey("人家在忙啦");
  else if (!HAS_PERM(PERM_SYSOP) && (!uin->pager && !pal_type(uin->userid, cuser.userid)))
    pressanykey("對方關掉呼叫器了");
  else if (!HAS_PERM(PERM_SYSOP) && uin->pager == 2)
    pressanykey("對方拔掉呼叫器了");
  else if (!HAS_PERM(PERM_SYSOP) && !(is_friend(uin) & 2) && uin->pager == 4)
    pressanykey("對方只接受好友的呼叫");
  else if (!(pid = uin->pid) || (kill(pid, 0) == -1))
  {
    resetutmpent();
    pressanykey(msg_usr_left);
  }
  else
  {
    switch(genbuf = getans2(2, 0, "找他 ", choose_ques, 4, 'q'))
    {
      case 'y':		//聊天
        uin->dark_turn=0;
        uin->turn = 0;
        log_usies("TALK ", uin->userid);
        break;

      case 'd':		//暗棋
        uin->dark_turn = 1;
        currutmp->turn = 0;
        uin->turn = 1;
        break;

      case 'c':		//象棋
        uin->dark_turn = 2;
        currutmp->turn = 0;
        uin->turn = 1;
        break;
        
      case 'f':		//五子棋
        uin->dark_turn = 3;
        currutmp->turn = 0;
        uin->turn = 1;
        break;
                             
      default:
        return;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
      return;

#if     defined(__OpenBSD__)                    /* lkchu */

    if (!(h = gethostbyname(MYHOSTNAME)))
      return -1;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = 0;
    memcpy(&sin.sin_addr, h->h_addr, h->h_length);

#else

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = 0;
    memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

    length = sizeof(sin);
    if (bind(sock, (struct sockaddr *) &sin, length) < 0 || getsockname(sock, (struct sockaddr *) &sin, &length) < 0)
    {
      close(sock);
      return;
    }

    currutmp->sockactive = YEA;
    currutmp->sockaddr = sin.sin_port;
    currutmp->destuid = uin->uid;

    setutmpmode(PAGE);

    uin->destuip = currutmp;
    kill(pid, SIGUSR1);
    clear();
    prints("正呼叫 %s.....\n鍵入 Ctrl-D 中止....", uin->userid);

    listen(sock, 1);
    add_io(sock, 20);
    while (1)
    {
      ch = igetch();
      if (ch == I_TIMEOUT)
      {
        ch = uin->mode;
        if (!ch && uin->chatid[0] == 1 && uin->destuip == currutmp)
        {
          bell();
          outmsg("對方回應中...");
        }
        else if (ch == EDITING || ch == TALK || ch == CHATING
             || ch == PAGE || ch == MAILALL || ch == FIVE
             || !ch && (uin->chatid[0] == 1 || uin->chatid[0] == 3))
        {
          add_io(0, 0);
          close(sock);
          currutmp->sockactive = currutmp->destuid = 0;
          pressanykey("人家在忙啦");
          return;
        }
        else
        {
#ifdef LINUX
          add_io(sock, 20);       /* added 4 linux... achen */
#endif
          move(0, 0);
          outs("再");
          bell();
          uin->destuip = currutmp;

          if (kill(pid, SIGUSR1) == -1)
          {
#ifdef LINUX
            add_io(sock, 20);       /* added 4 linux... achen */
#endif
            pressanykey(msg_usr_left);
            refresh();
            return;
          }
          continue;
        }
      }

      if (ch == I_OTHERDATA)
        break;

      if (ch == '\004')
      {
        add_io(0, 0);
        close(sock);
        currutmp->sockactive = currutmp->destuid = 0;
        return;
      }
    }

    msgsock = accept(sock, (struct sockaddr *) 0, (int *) 0);

    if (msgsock == -1)
    {
      perror("accept");
      return;
    }
    add_io(0, 0);
    close(sock);
    currutmp->sockactive = NA;
    /* currutmp->destuid = 0 ; */
    read(msgsock, &c, sizeof c);

    if (c == 'y')
    {
      sprintf(save_page_requestor, "%s (%s)", uin->userid, uin->username);
      if (uin->turn)
      {
        switch(uin->dark_turn)
        {
          case 1:
            DL_func("SO/dark.so:va_dark",msgsock, uin, 1);
            break;
	  case 2:
            DL_func("SO/chess.so:va_chc",msgsock,uin);
            break;
          case 3:
            DL_func("SO/five.so:va_gomoku",msgsock);
            break;
        }
      }
      else if(genbuf == 'y' )
        DL_func("SO/do_talk.so:va_do_talk", msgsock);
    }
    else
    {
      move(9, 9);
      outs("【回音】 ");
      switch (c)
      {
      case 'a':
        outs("我現在很忙，請等一會兒再 call 我，好嗎？");
        break;
      case 'b':
        outs("對不起，我有事情不能跟你 talk....");
        break;
      case 'c':
        outs("請不要吵我好嗎？");
        break;
      case 'd':
        outs("找我有事嗎？請先來信唷....");
        break;
      case 'e':
      {
        char msgbuf[60];
        read(msgsock, msgbuf, 60);
        outs("對不起，我現在不能跟你 talk，因為\n");
        move(10,18);
        outs(msgbuf);
      }
      break;
      default:
        outs("我現在不想 talk 啦.....:)");
      }
    }
    close(msgsock);
  }
  currutmp->mode = mode0;
  currutmp->destuid = 0;
}


/* ------------------------------------- */
/* 選單式聊天介面                        */
/* ------------------------------------- */


#define US_PICKUP       1234
#define US_RESORT       1233
//#define US_ACTION       1232
#define US_REDRAW       1231

static int
search_pickup(int num, int actor, pickup pklist[])
{
  char genbuf[IDLEN + 2];
  int n;
  
  move(1,0);
  clrtoeol();
  CreateNameList();
  for(n = 0;n < actor;n++)
    AddNameList(pklist[n].ui->userid);

  namecomplete("請輸入使用者姓名：", genbuf);
  
#if 0
  getdata(b_lines - 1, 0, "請輸入使用者姓名：", genbuf, IDLEN + 1, DOECHO,0);
  move(b_lines - 1, 0);
  clrtoeol();
#endif

  if (genbuf[0])
  {
    n = (num + 1) % actor;
    str_lower(genbuf, genbuf);
    while (n != num)
    {
      if (strstr_lower(pklist[n].ui->userid, genbuf))
        return n;
      if (++n >= actor)
        n = 0;
    }
  }
  return -1;
}


static int
pickup_cmp(i, j)
  pickup *i, *j;
{
  switch (pickup_way)
  {
  case 0:
    {
      register int friend;

      if (friend = j->friend - i->friend)
        return friend;
    }
  case 1:
    return strcasecmp(i->ui->userid, j->ui->userid);
  case 2:
    return (i->ui->mode - j->ui->mode);
  case 3:
    return (i->idle - j->idle);
  case 4:
    return strcasecmp(i->ui->from, j->ui->from);
  case 5:
    return (j->ui->brc_id - i->ui->brc_id);
  }
}

int
pal_type(userid, whoask)
/* return value :
 * 0  : no such user
 * 1  : friend
 * 2  : bad user
 * 4  : aloha
 */
  char *userid;
  char *whoask;
{
  char buf[STRLEN];
  int fd, can = 0;
  PAL pal;

  sethomefile(buf, userid, FN_PAL);
  if ((fd = open(buf, O_RDONLY)) >= 0)
  {
    while (read(fd, &pal, sizeof(pal)) == sizeof(pal))
    {
      if (!strcmp(pal.userid, whoask))
      {
        can = pal.ftype;
        break;
      }
    }
    close(fd);
  }

  return can;
}

void
friend_add(uident)
  char *uident;
{
  time_t now = time(NULL);
  struct tm *ptime = localtime(&now);

  if (uident[0] > ' ')
  {
    char fpath[80];
    char buf[22];
    PAL pal;

    /* itoc.010529: 好友名單檢查人數上限 */
    sethomefile(fpath, cuser.userid, FN_PAL);
    if (rec_num(fpath, sizeof(fileheader)) >= MAX_FRIEND)
    {
      pressanykey("好友人數超過上限！");
      return;
    }

    pal.ftype = 0;
    strcpy(pal.userid, uident);
    sprintf(fpath, "對於 %s 的描述：", uident); /* 借 fpath 用一下 */
    getdata(2, 0, fpath, buf, 22, DOECHO, 0);
    strncpy(pal.desc, buf, 21);

    if (getans2(2, 0,"壞人嗎？", 0, 2, 'n') != 'y')
    {
      pal.ftype |= M_PAL;

      if (strcmp(uident, cuser.userid) && getans2(2, 0, "加入上站通知嗎？", 0, 2, 'y')!= 'n')
      {
        PAL aloha;

        pal.ftype |= M_ALOHA;
        strcpy(aloha.userid, cuser.userid);
        sethomefile(fpath, uident, FN_ALOHA);
        rec_add(fpath, &aloha, sizeof(aloha));
      }
    }
    else
      pal.ftype |= M_BAD;

    sprintf(pal.date, "%02d/%02d",  ptime->tm_mon + 1, ptime->tm_mday);
    sethomefile(fpath, cuser.userid, FN_PAL);
    rec_add(fpath, &pal, sizeof(pal));
  }
}


int
cmpuname(userid, pal)
  char *userid;
  PAL *pal;
{
  return (!str_ncmp(userid, pal->userid, sizeof(pal->userid)));
}


static void
friend_delete(uident)
  char *uident;
{
  char fpath[80];
  PAL pal;
  int pos;

  sprintf(fpath, "確定移除好友 %s？", uident);
  if(getans2(2, 0, fpath, 0, 2, 'n') == 'n') return ;
  
  sethomefile(fpath, cuser.userid, FN_PAL);
  pos = rec_search(fpath, &pal, sizeof(pal), cmpuname, (int) uident);

  if (pos)
  {
    rec_del(fpath, sizeof(PAL), pos, NULL, NULL);

    sethomefile(fpath, uident, FN_ALOHA);  /* 上站通知 */
    pos = rec_search(fpath, &pal, sizeof(pal), cmpuname, (int) cuser.userid);
    while (pos)
    {
      rec_del(fpath, sizeof(PAL), pos, NULL, NULL);
      pos = rec_search(fpath, &pal, sizeof(pal), cmpuname, (int) cuser.userid);
    }
  }
}


void friend_load()
{
  ushort myfriends[MAX_FRIEND];
  ushort myrejects[MAX_REJECT];
  char genbuf[200];
  PAL pal;
  int fd;

  memset(myfriends, 0, sizeof(myfriends));
  memset(myrejects, 0, sizeof(myrejects));
  friendcount = rejected_number = 0;

  sethomefile(genbuf, cuser.userid, FN_PAL);
  if ((fd = open(genbuf, O_RDONLY)) > 0)
  {
    ushort unum;

    while (read(fd, &pal, sizeof(pal)))
    {
      if (unum = searchuser(pal.userid))
      {
        if ((pal.ftype & M_PAL) && friendcount<MAX_FRIEND-1)
          myfriends[friendcount++] = (ushort) unum;
        else if ((pal.ftype & M_BAD) && rejected_number<MAX_REJECT-1)
          myrejects[rejected_number++] = (ushort) unum;
      }
    }
    close(fd);
  }
  memcpy(currutmp->friend, myfriends, sizeof(myfriends));
  memcpy(currutmp->reject, myrejects, sizeof(myrejects));
}

static char *       /* Kaede show friend description */
friend_descript(char *uident)
{
  static char *space_buf="                    ";
  static char desc_buf[80];
  char fpath[80];
  int pos;
  PAL pal;

  sethomefile(fpath, cuser.userid, FN_PAL);
  pos = rec_search(fpath, &pal, sizeof(pal), cmpuname, (int) uident);

  if (pos)
  {
    strcpy (desc_buf, pal.desc);
    return desc_buf;
  }
  else
    return space_buf;
}


/*-------------------------------------*/
/* talk list			       */
/*-------------------------------------*/
/*Move From  pickup_user() */
static int real_name = 0;
static int show_friend = 0;
static int show_board = 0;
static int show_uid = 0;
static int show_tty = 0;
static int show_pid = 0;

int t_bmw();
int m_read();
int u_cloak();

static int 
change_talk_type()
{
  char *prompt[6]={"11)好友","22)代號","33)動態","44)發呆","55)故鄉","66)看板"};
  char ans = getans2(b_lines-1, 0,"排序方式：",prompt, 6, pickup_way + '1');
  if(!ans) return US_REDRAW;

  pickup_way = ans - '1';
  if(pickup_way > 5 || pickup_way < 0) pickup_way = 0;
  return US_PICKUP;
}

int 
talk_chusername()
{
  char buf[100];
  sprintf(buf, "暱稱 [%s]：", currutmp->username);
  if (!getdata(1, 0, buf, currutmp->username, 17, DOECHO,0))
     strcpy(currutmp->username, cuser.username);
  return US_PICKUP;
}

int
talk_chmood()
{
  char buf[64];
  sprintf(buf, "心情 [%s]：", currutmp->feeling);
  if (!getdata(1, 0, buf, currutmp->feeling, 5, DOECHO, currutmp->feeling))
    strcpy(currutmp->feeling, cuser.feeling);
  return US_PICKUP;
}

int
talk_chhome()
{
  char buf[50];
  if(getans2(1, 0, "請問是否修改故鄉？", 0, 2, 'n') == 'y')
  {  
    sprintf(buf, "故鄉 [%s]：", currutmp->from);
    if (getdata(1, 0, buf, buf, 17, DOECHO,currutmp->from))
    {
      if(!HAS_PERM(PERM_SYSOP) && !HAS_PERM(PERM_FROM))
      {
        if(check_money(5,GOLD)) return US_PICKUP;

        degold(5);
        pressanykey("修改故鄉花去金幣 5 元！");
      }
      currutmp->from_alias=0;
      strcpy(currutmp->from, buf);
    }
  }
  return US_PICKUP;
}

static int
talk_switch()  /* 顯示切換 */
{
  char *choose[6]={"sS)好友描述",
  		   "bB)使用看板",
  		   "rR)真實姓名",
  		   "uU)UID", 
  		   "yY)TTY",
  		   "iI)PID"};
  int ch = getans2(b_lines, 0, "切換：", choose, HAS_PERM(PERM_SYSOP) ? 6 : 1, '1');
  		   
  if(ch == 's') show_friend ^= 1;
  else if(HAS_PERM(PERM_SYSOP))
  {
    switch(ch)
    {
      case 'b':
        show_board ^= 1;      
        break;

#ifdef  REALINFO        
      case 'r': {real_name ^= 1;break;}
#endif
#ifdef  SHOWUID        
      case 'u': {show_uid ^= 1;break;}
#endif
#ifdef  SHOWTTY
      case 'y': {show_tty ^= 1;break;}
#endif
#ifdef  SHOWPID
      case 'i': {show_pid ^= 1;break;}
#endif
    }
  }
  return US_PICKUP;
}

static int 
talk_sysophide()
{
  currutmp->userlevel ^= PERM_DENYPOST;
  return US_PICKUP;
}

int
talk_chuser()
{
  char buf[100];
  sprintf(buf, "代號 [%s]：", currutmp->userid);
  if (!getdata(1, 0, buf, currutmp->userid, IDLEN + 1, DOECHO,0))
    strcpy(currutmp->userid, cuser.userid);
  return US_PICKUP;
}

static int 
talk_chfriend()  /* 切換顯示好友/一般狀態 */
{
  cuser.uflag ^= FRIEND_FLAG;
  return US_PICKUP;
}

int
t_pager()
{
  currutmp->pager = (currutmp->pager + 1) % 5;
  return US_PICKUP;
}

int
talk_friendlist()  /*編輯好友名單*/
{
  char buf[PATHLEN];
  sethomefile(buf, cuser.userid, FN_PAL);
  ListEdit(buf);
  return US_PICKUP;
}

int 
talk_broadcast(uentp, actor, pklist)
  pickup *pklist;
  int actor;
  user_info *uentp;  
{
  char genbuf[200];
  
  if(HAS_PERM(PERM_SYSOP) || cuser.uflag & FRIEND_FLAG)
  {
    if (!getdata(0, 0, "廣播訊息：", genbuf + 1, 60, DOECHO,0)) return US_REDRAW;
    genbuf[0] = HAS_PERM(PERM_SYSOP) ? 2 : 0;
    if(getans2(0, 0, "確定廣播？", 0, 2, 'y') == 'n') return US_REDRAW;
    while (actor)
    {
      uentp = pklist[--actor].ui;
      if (uentp->pid &&
         currpid != uentp->pid &&
         kill(uentp->pid, 0) != -1 &&
         (HAS_PERM(PERM_SYSOP) || (uentp->pager != 3 &&
         (uentp->pager != 4 || is_friend(uentp) & 4))))
           my_write(uentp->pid, genbuf);
    }
  }
  return US_PICKUP;
}

int 
talk_water(user_info *uentp)
{
  if ((uentp->pid != currpid) &&
      (HAS_PERM(PERM_SYSOP) || uentp->pager < 3 ||
      (pal_type(uentp->userid, cuser.userid) && uentp->pager == 4) ))
  {
    my_write(uentp->pid, "水球熱線：");
  }
  return US_PICKUP;
}

int 
talk_query(uentp)
  user_info *uentp;  
{
  strcpy(currauthor, uentp->userid);
  showplans(uentp->userid);
  return US_PICKUP;
}

int
talk_sendmail(uentp)
  user_info *uentp;  
{
  stand_title("寄  信");
  prints("收信人：%s", uentp->userid);
  my_send(uentp->userid);
  return US_PICKUP;
}

int
talk_edituser(uentp)
  user_info *uentp;  
{
  int id;
  userec muser;
  
  strcpy(currauthor, uentp->userid);
  stand_title("使用者設定");
  move(1, 0);
  if (id = getuser(uentp->userid))
  {
    memcpy(&muser, &xuser, sizeof(muser));
    uinfo_query(&muser, 1, id);
  }
  return US_PICKUP;
}

int
talk_kick(uentp) /* 踢人 */
  user_info *uentp;  
{
  if (uentp->pid && (kill(uentp->pid, 0) != -1))
  {
    clear();
    move(2, 0);
    my_kick(uentp);
  }        
  return US_PICKUP;
}

int 
talk_ask(uentp)  /* 要求聊天等連線事宜 */
  user_info *uentp;
{
  if (uentp->pid != currpid)
  {
    char genbuf[PATHLEN];
    clear();
    stand_title("聊天選項");
    sethomefile(genbuf, uentp->userid, fn_plans);
    if (dashf(genbuf)) show_file(genbuf, 4, 20, ONLY_COLOR);
    move(3, 0);
    my_talk(uentp);
  }
  return US_PICKUP;
}

int
talk_editfriend(uentp)
  user_info *uentp;
{
  if (!pal_type(cuser.userid, uentp->userid))
    friend_add(uentp->userid, FRIEND_OVERRIDE);
  else
    friend_delete(uentp->userid);
  friend_load();
  return US_PICKUP;
}

struct one_key talklist_key[]={
KEY_TAB, change_talk_type,  0, "切換排序方法。",0,
'N',	  talk_chusername,  PERM_BASIC, "修改暱稱。",0,
'M',	  talk_chmood,	    PERM_BASIC, "修改心情。",0,
'F',	  talk_chhome,	    PERM_BASIC, "修改故鄉。",0,
'b',	  talk_broadcast,   PERM_PAGE, "廣播。",0,
's',	  talk_switch,	    PERM_LOGINOK, "顯示好友描述。",0,
't',	  talk_ask, 	    PERM_PAGE, "聊天等相關功\能。",0,
'w',	  talk_water,       PERM_PAGE, "丟水球。",0,
'i',	  u_cloak, 	    PERM_CLOAK, "隱形。",0,
'a',	  talk_editfriend,  PERM_LOGINOK, "加入/刪除 好友。",0,
'd',	  talk_editfriend,  PERM_LOGINOK, "加入/刪除 好友。",0,
'o',	  talk_friendlist,  PERM_LOGINOK, "編輯好友名單。",0,
'f',	  talk_chfriend,    0, "切換顯示好友/一般狀態",0,
'q',	  talk_query, 	    0, "query 人",0,
'm',	  talk_sendmail,    PERM_BASIC, "寄信",0,
'r',	  m_read, 	    PERM_BASIC, "讀信",0,
'l',	  t_bmw, 	    PERM_BASIC, "水球回顧",0,
'p',	  t_pager,          PERM_BASIC, "切換摳機狀態。",0,
'H',	  talk_sysophide,   PERM_SYSOP, "切換站長隱身。站長專用!!",0,
'D',	  talk_chuser,      PERM_SYSOP, "暫時切換使用者。站長專用!!",0,
'u',	  talk_edituser,    PERM_ACCOUNTS, "修改使用者的資料。站長專用!!",0,
'K',	  talk_kick,	    PERM_SYSOP, "站長踢人。站長專用!!",0,
0, NULL, 0, NULL,0};

int
pklist_doent(pklist, ch, row, bar_color)
  pickup pklist;
  int ch, row;
  char *bar_color;
{
  register user_info *uentp;
  time_t diff;
  char buf[20];
  int state = US_PICKUP, hate;
  char pagerchar[4] = "* o ";
  char sex_tag[8] = "MFMFMF  ";
    
#ifdef WHERE
  extern struct FROMCACHE *fcache;
#endif    
    
      uentp = pklist.ui;
      if (!uentp->pid) return US_PICKUP;

#ifdef SHOW_IDLE_TIME
      diff = pklist.idle;
      if (diff > 3600)
        sprintf(buf, "%3dh%02d", diff/3600, (diff % 3600) / 60);
      else if(diff > 0)
        sprintf(buf, "%3d'%02d", diff / 60, diff % 60);
      else
        buf[0] = '\0';
#else
      buf[0] = '\0';
#endif
#ifdef SHOWPID
      if (show_pid)
        sprintf(buf, "%6d", uentp->pid);
#endif
      state = (currutmp == uentp) ? 10 : pklist.friend;
      if (PERM_HIDE(uentp) && HAS_PERM(PERM_SYSOP))
         state = 9;
      hate = is_rejected(uentp);
      diff = uentp->pager & !(hate & HRM);
      
      move (row, 0);
      clrtoeol();

      prints("%5d %c%c%s%-12s[0m %s%-17.16s[m%-15.15s %c %-12.12s %s%-4.4s%s[m",

#ifdef SHOWUID
      show_uid ? uentp->uid :
#endif
      (ch + 1),
      (hate & HRM)? 'X' :
      (uentp->pager == 4) ? 'f' : (uentp->pager == 3) ? 'W' :
      (uentp->pager == 2) ? '-' : pagerchar[(state & 2) | diff],
      (uentp->invisible ? ')' : ' '),
      (bar_color && HAVE_HABIT(HABIT_LIGHTBAR)) ? bar_color : (hate & IRH)? fcolor[8] : fcolor[state],
      uentp->userid,
      (hate & IRH)? fcolor[8] : fcolor[state],
#ifdef REALINFO
      real_name ? uentp->realname :
#endif
      uentp->username,
      show_friend ? friend_descript(uentp->userid) :
      show_board ? (char *)getbname(uentp->brc_id) :
      ((uentp->pager != 2 && uentp->pager != 3 && diff || HAS_PERM(PERM_SYSOP)) ?
#ifdef WHERE
      uentp->from_alias ? fcache->replace[uentp->from_alias] : uentp->from
#else
      uentp->from
#endif
      : "*" ),
      (uentp->sex < 8) ? sex_tag[uentp->sex] : sex_tag[7],
#ifdef SHOWTTY
      show_tty ? uentp->tty :
#endif
      modestring(uentp, 0),
      uentp->birth ? fcolor[8] : fcolor[0],
      uentp->birth ? "壽星" : uentp->feeling[0] ? uentp->feeling : "不明" ,
      buf);

  return state;
}

/*Add End*/

static void
pickup_user()
{
  static int num = 0;

  register user_info *uentp;
  register pid_t pid0=0;  /* Ptt 定位                */
  register int   id0;   /*     US_PICKUP時的游標用 */
  register int state = US_PICKUP, ch;
  register int actor, head, foot;
  char bar_color[50];
  int badman;
  int savemode = currstat;
  time_t diff, freshtime;
  pickup pklist[USHM_SIZE];                     /* parameter Ptt註 */
                                                /* num : 現在的游標位 */
  						/* actor:共有多少user */
  						/* foot: 此頁的腳腳 */
  char *msg_pickup_way[PICKUP_WAYS] =
  { "嗨！朋友",
    "網友代號",
    "網友動態",
    "發呆時間",
    "來自何方",
    "使用看板"
  };

  get_lightbar_color(bar_color);
  
#ifdef WHERE
  resolve_fcache();
#endif
  while (1)
  {
    if (state == US_PICKUP) freshtime = 0;
    if (utmpshm->uptime > freshtime)
    {
      time(&freshtime);
      bfriends_number =  friends_number = override_number =
      rejected_number = actor = ch = 0;

      while (ch < USHM_SIZE)
      {
        uentp = &(utmpshm->uinfo[ch++]);
        if (uentp->pid)
        {
          if (uentp->userid[0] == 0) continue;  /* Ptt's bug */

          if ((uentp->invisible && !(HAS_PERM(PERM_SYSOP) || HAS_PERM(PERM_SEECLOAK)))
            || ((is_rejected(uentp) & HRM) && !HAS_PERM(PERM_SYSOP)))
            continue;           /* Thor: can't see anyone who rejects you. */

          if (!PERM_HIDE(currutmp) && PERM_HIDE(uentp)) continue;
          head = is_friend(uentp) ;
          if ( (cuser.uflag & FRIEND_FLAG) && (!head || is_rejected(uentp)) )
            continue;

#ifdef SHOW_IDLE_TIME
          {
            if(!uentp->lastact) uentp->lastact = time(0);
            diff = freshtime - uentp->lastact;

#ifdef DOTIMEOUT
            /* prevent fault /dev mount from kicking out users */

            /*Ctrl-U 會斷線...改這裡*/
            if ((diff > IDLE_TIMEOUT) && (diff < 60 * 60 * 24 * 5)
            		&& !HAS_PERM(PERM_NOTIMEOUT))
            {
              if ((kill(uentp->pid, SIGHUP) == -1) && (errno == ESRCH))
                memset(uentp, 0, sizeof(user_info));
              continue;
            }
#endif
          }
          pklist[actor].idle = diff;
#endif

          pklist[actor].friend = head;
          pklist[actor].ui = uentp;

          actor++;
        }
      }
      badman = rejected_number;

      state = US_PICKUP;
      if (!actor)
      {
        if (getans2(b_lines, 0, "你的朋友還沒上站，要看看一般網友嗎？", 0, 2, 'y') != 'n')
        {
          cuser.uflag ^= FRIEND_FLAG;
          continue;
        }
        return;
      }
    }

    if (state >= US_RESORT) 
      qsort(pklist, actor, sizeof(pickup), pickup_cmp);

    if (state >= US_REDRAW)
    {
      sprintf(tmpbuf,"%s [線上 %d 人]",BOARDNAME,count_ulist());
      showtitle((cuser.uflag & FRIEND_FLAG)? "好友列表": "休閒聊天", tmpbuf);
      
      prints(" ←)離開  排序[[1;36;44m%s[0m] 上站人數 %d  [1;32m我的朋友 %-2d "
        "[33m與我為友 %-2d [36m板友 %-2d [31m壞人 %-3d[m\n",
        msg_pickup_way[pickup_way], count_ulist(), 
	(cuser.uflag & FRIEND_FLAG)?friends_number/2:friends_number,
	(cuser.uflag & FRIEND_FLAG)?override_number/2:override_number,
	(cuser.uflag & FRIEND_FLAG)?bfriends_number/2:bfriends_number,
	badman);
	
      prints("%s  %sTP%c代號         %-17s%-15s S %-12s%-10s[m\n",
	COLOR3,
#ifdef SHOWUID
        show_uid ? "UID" :
#endif
        "No.",
        (HAS_PERM(PERM_SEECLOAK) || HAS_PERM(PERM_SYSOP)) ? 'C' : ' ',

#ifdef REALINFO
        real_name ? "姓名" :
#endif

        "暱稱 (N)", 
        show_friend ? "好友描述 (s)" : show_board ? "使用看板" : "故鄉 (F)",

#ifdef SHOWTTY
        show_tty ? "TTY " :
#endif
        "動態",
#ifdef SHOWPID
        show_pid ? "       PID" :
#endif
#ifdef SHOW_IDLE_TIME
        " 心情(M) 閒"
#else
        " 心情(M)"
#endif

        );
    }
    else
    {
      move(3, 0);
      clrtobot();
    }

    if(pid0)
    {
       for (ch = 0; ch < actor; ch++)
        {
          if(pid0 == (pklist[ch].ui)->pid &&
           id0  == 256 * pklist[ch].ui->userid[0] + pklist[ch].ui->userid[1])
            {
               num = ch;
            }
        }
     }

    if (num < 0)
      num = 0;
    else if (num >= actor)
      num = actor - 1;

    head = (num / p_lines) * p_lines;
    foot = head + p_lines;
    if (foot > actor) foot = actor;

    for (ch = head; ch < foot; ch++)
    {
      uentp = pklist[ch].ui;
      if (!uentp->pid)
      {
         state = US_PICKUP;
         break;
      }
      state = pklist_doent(pklist[ch], ch, ch + 3 - head, 0, 0);
    }

    if (state == US_PICKUP)  continue;

    move(b_lines, 0);
    clrtoeol();
    prints("%s  好友列表  %s       Tab|f)排序|好友  y)更新  w)水球  a|d|o)交友  m)寄信  h)說明 \033[m",
    	   COLOR2, COLOR3);
    
    state = 0;
    while (!state)
    {
      if(HAVE_HABIT(HABIT_LIGHTBAR))
      {
        pklist_doent(pklist[num], num, num + 3 - head, bar_color);
        cursor_show(num + 3 - head, 0);
        ch = igetkey();
        pklist_doent(pklist[num], num, num + 3 - head, 0);
      }
      else
        ch = cursor_key(num + 3 - head, 0);
      
      if (ch == KEY_RIGHT || ch == '\n' || ch == '\r') ch = 't';
      switch (ch)
      {
        case KEY_LEFT:
        case 'e':
        case 'E':
        {
          /* 紀錄使用者 FRIEND_FLAG 狀態 */        
          int unum = do_getuser(cuser.userid, &xuser);

          if((cuser.uflag & FRIEND_FLAG) != (xuser.uflag & FRIEND_FLAG))
          {
            xuser.uflag ^= FRIEND_FLAG;
            substitute_record(fn_passwd, &xuser, sizeof(userec), unum);
          }
          
          return;
        }

      case KEY_DOWN:
      case 'n':
        if (++num < actor)
        {
          if (num >= foot)
            state = US_REDRAW;
          break;
        }

      case '0':
      case KEY_HOME:
        num = 0;
        if (head)
          state = US_REDRAW;
        break;

      case ' ':
      case KEY_PGDN:
      case Ctrl('F'):
        if (foot < actor)
        {
          num += p_lines;
          state = US_REDRAW;
          break;
        }
        if (head)
          num = 0;
        state = US_PICKUP;
        break;

      case KEY_UP:
        if (--num < head)
        {
          if (num < 0)
          {
            num = actor - 1;
            if (actor == foot)
              break;
          }
          state = US_REDRAW;
        }
        break;

      case KEY_PGUP:
      case Ctrl('B'):
      case 'P':
        if (head)
        {
          num -= p_lines;
          state = US_REDRAW;
          break;
        }

      case KEY_END:
      case '$':
        num = actor - 1;
        if (foot < actor)
          state = US_REDRAW;
        break;

      case '/':
        {
          int tmp;
          if ((tmp = search_pickup(num, actor, pklist)) >= 0)
            num = tmp;
          state = US_REDRAW;
        }
        break;

      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        {                       /* Thor: 可以打數字跳到該人 */
          int tmp;
          if ((tmp = search_num(ch, actor - 1)) >= 0)
            num = tmp;
          state = US_REDRAW;
        }
        break;

      case 'h': //help
        i_read_helper(talklist_key);
        state = US_PICKUP;
        break;
      case KEY_ESC:
         if (KEY_ESC_arg == 'c')
            capture_screen();
         else if (KEY_ESC_arg == 'n') 
         {
            edit_note();
            state = US_PICKUP;
         }
         break;
      default:
        {
          int tmp = 0;
          state = US_PICKUP;
          for(tmp = 0;talklist_key[tmp].key != 0;tmp++)
          {
            if(talklist_key[tmp].level && !HAS_PERM(talklist_key[tmp].level))
              continue;
            if(ch == talklist_key[tmp].key && talklist_key[tmp].fptr != NULL)
            {
              cursor_show(num-head + 3, 0);
              state = (*((int (*)())talklist_key[tmp].fptr)) (pklist[num].ui, actor, pklist);
              if(!state) state = US_PICKUP;
            }
          }
        }
      }
    }

    pid0 = 0;
    setutmpmode(savemode);
  }
}


/* talk list 結束 */

int
t_users()
{
  int destuid0 = currutmp->destuid;

  if (chkmailbox())
    return;

  setutmpmode(LUSERS);
  pickup_user();
  currutmp->destuid = destuid0;
  return 0;
}

int
t_idle()
{
  int destuid0 = currutmp->destuid;
  int mode0 = currutmp->mode;
  int stat0 = currstat;
  char genbuf[20];
  char *reason[8]={"00.發呆","11.接電話","22.覓食","33.打瞌睡","44.裝死","55.羅丹","66.其他","qQ.沒事"};

  setutmpmode(IDLE);

  genbuf[0] = getans2(b_lines, 0,"理由：",reason,8,'0');
  if (genbuf[0] == 'q')
  {
    currutmp->mode = mode0;
    currstat = stat0;
    return 0;
  }
  else if (genbuf[0] >= '1' && genbuf[0] <= '6')
    currutmp->destuid = genbuf[0] - '0';
  else
    currutmp->destuid = 0;

  if (currutmp->destuid == 6)
    if (!cuser.userlevel || !getdata(b_lines, 0, "發呆的理由：", currutmp->chatid, 11, DOECHO, 0))
      currutmp->destuid = 0;
  {
    char buf[80], passbuf[PASSLEN];
    do
    {
      move(b_lines - 1, 0);
      clrtoeol();
      sprintf(buf, "(鎖定螢幕)發呆原因：%s", (currutmp->destuid != 6) ?
         IdleTypeTable[currutmp->destuid] : currutmp->chatid);
      outs(buf);
      refresh();
      getdata(b_lines, 0, MSG_PASSWD, passbuf, PASSLEN, PASS, 0);
      passbuf[8]='\0';
    } while(!chkpasswd(cuser.passwd, passbuf) && strcmp(STR_GUEST,cuser.userid));
  }
  currutmp->mode = mode0;
  currutmp->destuid = destuid0;
  currstat = stat0;

  return 0;
}


int
t_query()
{
  char uident[STRLEN];

  stand_title("查詢網友");
  usercomplete(msg_uid, uident);
  if (getuser(uident) == 0)
    pressanykey("這裡沒這個人！");
  else
  {
    if (uident[0])
      showplans(uident);
  }
  return 0;
}

/* ------------------------------------- */
/* 有人來串門子了，回應呼叫器            */
/* ------------------------------------- */

void talkreply()
{
  int a;
  struct hostent *h;
  char hostname[STRLEN],buf[80];
  struct sockaddr_in sin;
  char genbuf[200];
  user_info *uip;

  uip = currutmp->destuip;
  sprintf(page_requestor, "%s (%s)", uip->userid, uip->username);
  currutmp->destuid = uip->uid;
  currstat = XMODE;             /* 避免出現動畫 */

  clear();
  outs("\n\
       (Y) 讓我們 talk 吧！     (A) 我現在很忙，請等一會兒再 call 我\n\
       (N) 我現在不想 talk      (B) 對不起，我有事情不能跟你 talk\n\
       (C) 請不要吵我好嗎？     (D) 有事嗎？請先來信\n\
       (E) [1;33m我自己輸入理由好了...[m\n\n");

  getuser(uip->userid);
  currutmp->msgs[0].last_pid = uip->pid;
  strcpy(currutmp->msgs[0].last_userid, uip->userid);
  strcpy(currutmp->msgs[0].last_call_in, "呼叫、呼叫，聽到請回答");
  prints("對方來自 [%s]，共上站 %d 次，文章 %d 篇\n",
    uip->from, xuser.numlogins, xuser.numposts);
  show_last_call_in();
  sprintf(genbuf, "你想跟 %s %s嗎？請選擇(Y/N/A/B/C/D/E)[Y] ",
    page_requestor, !currutmp->turn ? "聊天" :
    		    currutmp->dark_turn == 1 ? "暗棋" :
    		    currutmp->dark_turn == 2 ? "象棋" :
    		    currutmp->dark_turn == 3 ? "五子棋" : "");
    		    
  getdata(0, 0, genbuf, buf, 4, LCECHO,0);

  if (uip->mode != PAGE) 
  {
     pressanykey("%s已停止呼叫", page_requestor);
     return;
  }

  currutmp->msgcount = 0;
  strcpy(save_page_requestor, page_requestor);
  memset(page_requestor, 0, sizeof(page_requestor));
  gethostname(hostname, STRLEN);
  if (!(h = gethostbyname(hostname)))
  {
    perror("gethostbyname");
    return;
  }
  memset(&sin, 0, sizeof sin);
  sin.sin_family = h->h_addrtype;
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);
  sin.sin_port = uip->sockaddr;
  a = socket(sin.sin_family, SOCK_STREAM, 0);
  if ((connect(a, (struct sockaddr *) & sin, sizeof sin)))
  {
    perror("connect err");
    return;
  }
  if (!buf[0] || !strchr("abcdefn", buf[0]))
    buf[0] = 'y';

  write(a, buf, 1);
  if (buf[0] == 'e')
  {
    if (!getdata(b_lines, 0, "不能 talk 的原因：", genbuf, 60, DOECHO,0))
      strcpy(genbuf, "不告訴你咧 !! ^o^");
    write(a, genbuf, 60);
  }

  if (buf[0] == 'y')
  {
    strcpy(currutmp->chatid, uip->userid);
    if (currutmp->turn)
    {
      switch(currutmp->dark_turn)
      {
        case 1:
          DL_func("SO/dark.so:va_dark",a, uip, 0);
          break;
        case 2:
          DL_func("SO/chess.so:va_chc",a,uip);
          break;
        case 3:
          DL_func("SO/five.so:va_gomoku",a);
          break;
      }
    }
    else
      DL_func("SO/do_talk.so:va_do_talk", a);
  }
  else
    close(a);

  clear();
}

/* shakalaca.000814: 以下這兩個函式在 .so 中有用到 :pp */
int
lockutmpmode(int unmode)
{
  if (count_multiplay(unmode))
  {
   char buf[80];
   sprintf(buf,"抱歉，您已有其他線相同的 ID 正在 %s",ModeTypeTable[unmode]);
   pressanykey(buf);
   return 1;
  }
  setutmpmode(unmode);
  currutmp->lockmode = unmode;
  return 0;
}


int
unlockutmpmode()
{
  currutmp->lockmode = 0;
}

