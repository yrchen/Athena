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
/* �O�� friend �� user number */
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
  static char *notonline="���b���W";
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
        sprintf(modestr, "�^�� %s", getuserid(uentp->destuid));
     else
        sprintf(modestr, "�^���I�s");
  }

  else if (!mode && *uentp->chatid == 3)
     sprintf(modestr, "���y�ǳƤ�");
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
    if (is_hidden(getuserid(uentp->destuid)))    /* Leeym ���(����)���� */
      sprintf(modestr, "%s", "�ۨ��ۻy��"); /* Leeym �j�a�ۤv�o���a�I */
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

int     /* Leeym �q FireBird ���ӧ�g�L�Ӫ� */
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
        return 0;       /* ��� xxx */
  else
        return 1;       /* �ۨ��ۻy */
}

/* ------------------------------------- */
/* routines for Talk->Friend             */
/* ------------------------------------- */

int is_friend(user_info *ui)
{
  register ushort unum, hit, *myfriends;

  /* �P�_���O�_���ڪ��B�� ? */

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

  /* �ݪO�n�� */

  if(currutmp->brc_id && ui->brc_id == currutmp->brc_id)
  {
    hit |= 1;
    bfriends_number++;
  }

  /* �P�_�ڬO�_����誺�B�� ? */

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

  /* �Q�ڵ� */

int
is_rejected(user_info *ui)
{
  register ushort unum, hit, *myrejects;

  if (PERM_HIDE(ui))
     return 0;

  /* �P�_���O�_���ڪ����H ? */

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
    
  /* �P�_�ڬO�_����誺���H ? */

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
/* �u��ʧ@                              */
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
    outz("��X�h�o");
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
  char *money[10] = {"�^��","���h","�M�H","���q","�p�d",
                     "�p�I��","���I��","�j�I��","�I�i�İ�","�񺸤��E"};

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
    prints("[ �b  �� ]%-30.30s[ ��  �� ]%s\n",muser.userid,muser.username);
    if (pal_type(muser.userid, cuser.userid) || HAS_PERM(PERM_SYSOP)
        || !strcmp(muser.userid, cuser.userid) )
    {
      char *sex[8] = { MSG_BIG_BOY, MSG_BIG_GIRL,
                       MSG_LITTLE_BOY, MSG_LITTLE_GIRL,
                       MSG_MAN, MSG_WOMAN, MSG_PLANT, MSG_MIME };
      prints("[ ��  �O ]%-30.30s",sex[muser.sex%8]);
    }

    prints("[ ��  �� ][1;33m%s[m\n",muser.feeling);
    uentp = (user_info *) search_ulist(cmpuids, tuid);
    if (uentp && !(PERM_HIDE(currutmp) ||
      is_rejected(uentp) & HRM && is_friend(uentp) & 2) && PERM_HIDE(uentp))
      prints("[�ثe�ʺA][1;30m���b���W                      [m\n");
    else
      prints("[�ثe�ʺA][1;36m%-30.30s[m",
         uentp ? modestring(uentp, 0) : "[1;30m���b���W");

    prints("[�s�H��Ū]");
    sethomedir(currmaildir, muser.userid);
    outs(chkmail(1) || muser.userlevel & PERM_SYSOP ? "[1;5;33m��" : "[1;30m�L");
    sethomedir(currmaildir, cuser.userid);
    chkmail(1);
    outs("\033[m\n");
    if (HAS_PERM(PERM_SYSOP))  
      prints("[�u��m�W]%s\n", muser.realname);

    prints("[1;36m%s[m\n", msg_seperator);
    prints("[�W���a�I]%-30.30s",muser.lasthost[0] ? muser.lasthost : "(����)");
    prints("[�W���ɶ�]%s\n",Etime(&muser.lastlogin));
    prints("[�W������]%-30d",muser.numlogins);
    prints("[�o��峹]%d �g\n",muser.numposts);
    prints("[�H�����]%-30d[�n�_����]%d\n",muser.bequery,muser.toquery);
    prints("[���y����]�� %d / �o %d \n",muser.receivemsg,muser.sendmsg);
    if(HAS_PERM(PERM_SYSOP))
      prints("[�e���d��]%-30.30s[�Q�d��]%s\n",muser.toqid,muser.beqid);

    prints("[1;36m%s[m\n", msg_seperator);

    prints("[�g�٪��p]%s\n",money[i]);
    if (HAS_PERM(PERM_SYSOP) || !strcmp(muser.userid, cuser.userid))
      prints("[�����ƶq]%-30ld[�ȹ��ƶq]%-21ld\n[�ȹ��W��]%ld\n"
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
  char *choose_ques[4]={"yY)���", "cC)�H��", "dD)�t��", msg_choose_cancel};

  ch = uin->mode;
  strcpy(currutmp->chatid,uin->userid);
  strcpy(currauthor, uin->userid);

  if (ch == EDITING || ch == TALK || ch == CHATING
      || ch == PAGE || ch == MAILALL || ch == FIVE || ch == IDLE   //IDLE by hialan
      || !ch && (uin->chatid[0] == 1 || uin->chatid[0] == 3))
    pressanykey("�H�a�b����");
  else if (!HAS_PERM(PERM_SYSOP) && (!uin->pager && !pal_type(uin->userid, cuser.userid)))
    pressanykey("��������I�s���F");
  else if (!HAS_PERM(PERM_SYSOP) && uin->pager == 2)
    pressanykey("���ޱ��I�s���F");
  else if (!HAS_PERM(PERM_SYSOP) && !(is_friend(uin) & 2) && uin->pager == 4)
    pressanykey("���u�����n�ͪ��I�s");
  else if (!(pid = uin->pid) || (kill(pid, 0) == -1))
  {
    resetutmpent();
    pressanykey(msg_usr_left);
  }
  else
  {
    switch(genbuf = getans2(2, 0, "��L ", choose_ques, 4, 'q'))
    {
      case 'y':		//���
        uin->dark_turn=0;
        uin->turn = 0;
        log_usies("TALK ", uin->userid);
        break;

      case 'd':		//�t��
        uin->dark_turn = 1;
        currutmp->turn = 0;
        uin->turn = 1;
        break;

      case 'c':		//�H��
        uin->dark_turn = 2;
        currutmp->turn = 0;
        uin->turn = 1;
        break;
        
      case 'f':		//���l��
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
    prints("���I�s %s.....\n��J Ctrl-D ����....", uin->userid);

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
          outmsg("���^����...");
        }
        else if (ch == EDITING || ch == TALK || ch == CHATING
             || ch == PAGE || ch == MAILALL || ch == FIVE
             || !ch && (uin->chatid[0] == 1 || uin->chatid[0] == 3))
        {
          add_io(0, 0);
          close(sock);
          currutmp->sockactive = currutmp->destuid = 0;
          pressanykey("�H�a�b����");
          return;
        }
        else
        {
#ifdef LINUX
          add_io(sock, 20);       /* added 4 linux... achen */
#endif
          move(0, 0);
          outs("�A");
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
      outs("�i�^���j ");
      switch (c)
      {
      case 'a':
        outs("�ڲ{�b�ܦ��A�е��@�|��A call �ڡA�n�ܡH");
        break;
      case 'b':
        outs("�藍�_�A�ڦ��Ʊ������A talk....");
        break;
      case 'c':
        outs("�Ф��n�n�ڦn�ܡH");
        break;
      case 'd':
        outs("��ڦ��ƶܡH�Х��ӫH��....");
        break;
      case 'e':
      {
        char msgbuf[60];
        read(msgsock, msgbuf, 60);
        outs("�藍�_�A�ڲ{�b�����A talk�A�]��\n");
        move(10,18);
        outs(msgbuf);
      }
      break;
      default:
        outs("�ڲ{�b���Q talk ��.....:)");
      }
    }
    close(msgsock);
  }
  currutmp->mode = mode0;
  currutmp->destuid = 0;
}


/* ------------------------------------- */
/* ��榡��Ѥ���                        */
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

  namecomplete("�п�J�ϥΪ̩m�W�G", genbuf);
  
#if 0
  getdata(b_lines - 1, 0, "�п�J�ϥΪ̩m�W�G", genbuf, IDLEN + 1, DOECHO,0);
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

    /* itoc.010529: �n�ͦW���ˬd�H�ƤW�� */
    sethomefile(fpath, cuser.userid, FN_PAL);
    if (rec_num(fpath, sizeof(fileheader)) >= MAX_FRIEND)
    {
      pressanykey("�n�ͤH�ƶW�L�W���I");
      return;
    }

    pal.ftype = 0;
    strcpy(pal.userid, uident);
    sprintf(fpath, "��� %s ���y�z�G", uident); /* �� fpath �Τ@�U */
    getdata(2, 0, fpath, buf, 22, DOECHO, 0);
    strncpy(pal.desc, buf, 21);

    if (getans2(2, 0,"�a�H�ܡH", 0, 2, 'n') != 'y')
    {
      pal.ftype |= M_PAL;

      if (strcmp(uident, cuser.userid) && getans2(2, 0, "�[�J�W���q���ܡH", 0, 2, 'y')!= 'n')
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

  sprintf(fpath, "�T�w�����n�� %s�H", uident);
  if(getans2(2, 0, fpath, 0, 2, 'n') == 'n') return ;
  
  sethomefile(fpath, cuser.userid, FN_PAL);
  pos = rec_search(fpath, &pal, sizeof(pal), cmpuname, (int) uident);

  if (pos)
  {
    rec_del(fpath, sizeof(PAL), pos, NULL, NULL);

    sethomefile(fpath, uident, FN_ALOHA);  /* �W���q�� */
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
  char *prompt[6]={"11)�n��","22)�N��","33)�ʺA","44)�o�b","55)�G�m","66)�ݪO"};
  char ans = getans2(b_lines-1, 0,"�ƧǤ覡�G",prompt, 6, pickup_way + '1');
  if(!ans) return US_REDRAW;

  pickup_way = ans - '1';
  if(pickup_way > 5 || pickup_way < 0) pickup_way = 0;
  return US_PICKUP;
}

int 
talk_chusername()
{
  char buf[100];
  sprintf(buf, "�ʺ� [%s]�G", currutmp->username);
  if (!getdata(1, 0, buf, currutmp->username, 17, DOECHO,0))
     strcpy(currutmp->username, cuser.username);
  return US_PICKUP;
}

int
talk_chmood()
{
  char buf[64];
  sprintf(buf, "�߱� [%s]�G", currutmp->feeling);
  if (!getdata(1, 0, buf, currutmp->feeling, 5, DOECHO, currutmp->feeling))
    strcpy(currutmp->feeling, cuser.feeling);
  return US_PICKUP;
}

int
talk_chhome()
{
  char buf[50];
  if(getans2(1, 0, "�аݬO�_�ק�G�m�H", 0, 2, 'n') == 'y')
  {  
    sprintf(buf, "�G�m [%s]�G", currutmp->from);
    if (getdata(1, 0, buf, buf, 17, DOECHO,currutmp->from))
    {
      if(!HAS_PERM(PERM_SYSOP) && !HAS_PERM(PERM_FROM))
      {
        if(check_money(5,GOLD)) return US_PICKUP;

        degold(5);
        pressanykey("�ק�G�m��h���� 5 ���I");
      }
      currutmp->from_alias=0;
      strcpy(currutmp->from, buf);
    }
  }
  return US_PICKUP;
}

static int
talk_switch()  /* ��ܤ��� */
{
  char *choose[6]={"sS)�n�ʹy�z",
  		   "bB)�ϥάݪO",
  		   "rR)�u��m�W",
  		   "uU)UID", 
  		   "yY)TTY",
  		   "iI)PID"};
  int ch = getans2(b_lines, 0, "�����G", choose, HAS_PERM(PERM_SYSOP) ? 6 : 1, '1');
  		   
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
  sprintf(buf, "�N�� [%s]�G", currutmp->userid);
  if (!getdata(1, 0, buf, currutmp->userid, IDLEN + 1, DOECHO,0))
    strcpy(currutmp->userid, cuser.userid);
  return US_PICKUP;
}

static int 
talk_chfriend()  /* ������ܦn��/�@�몬�A */
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
talk_friendlist()  /*�s��n�ͦW��*/
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
    if (!getdata(0, 0, "�s���T���G", genbuf + 1, 60, DOECHO,0)) return US_REDRAW;
    genbuf[0] = HAS_PERM(PERM_SYSOP) ? 2 : 0;
    if(getans2(0, 0, "�T�w�s���H", 0, 2, 'y') == 'n') return US_REDRAW;
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
    my_write(uentp->pid, "���y���u�G");
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
  stand_title("�H  �H");
  prints("���H�H�G%s", uentp->userid);
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
  stand_title("�ϥΪ̳]�w");
  move(1, 0);
  if (id = getuser(uentp->userid))
  {
    memcpy(&muser, &xuser, sizeof(muser));
    uinfo_query(&muser, 1, id);
  }
  return US_PICKUP;
}

int
talk_kick(uentp) /* ��H */
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
talk_ask(uentp)  /* �n�D��ѵ��s�u�Ʃy */
  user_info *uentp;
{
  if (uentp->pid != currpid)
  {
    char genbuf[PATHLEN];
    clear();
    stand_title("��ѿﶵ");
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
KEY_TAB, change_talk_type,  0, "�����ƧǤ�k�C",0,
'N',	  talk_chusername,  PERM_BASIC, "�ק�ʺ١C",0,
'M',	  talk_chmood,	    PERM_BASIC, "�ק�߱��C",0,
'F',	  talk_chhome,	    PERM_BASIC, "�ק�G�m�C",0,
'b',	  talk_broadcast,   PERM_PAGE, "�s���C",0,
's',	  talk_switch,	    PERM_LOGINOK, "��ܦn�ʹy�z�C",0,
't',	  talk_ask, 	    PERM_PAGE, "��ѵ������\\��C",0,
'w',	  talk_water,       PERM_PAGE, "����y�C",0,
'i',	  u_cloak, 	    PERM_CLOAK, "���ΡC",0,
'a',	  talk_editfriend,  PERM_LOGINOK, "�[�J/�R�� �n�͡C",0,
'd',	  talk_editfriend,  PERM_LOGINOK, "�[�J/�R�� �n�͡C",0,
'o',	  talk_friendlist,  PERM_LOGINOK, "�s��n�ͦW��C",0,
'f',	  talk_chfriend,    0, "������ܦn��/�@�몬�A",0,
'q',	  talk_query, 	    0, "query �H",0,
'm',	  talk_sendmail,    PERM_BASIC, "�H�H",0,
'r',	  m_read, 	    PERM_BASIC, "Ū�H",0,
'l',	  t_bmw, 	    PERM_BASIC, "���y�^�U",0,
'p',	  t_pager,          PERM_BASIC, "����������A�C",0,
'H',	  talk_sysophide,   PERM_SYSOP, "�������������C�����M��!!",0,
'D',	  talk_chuser,      PERM_SYSOP, "�Ȯɤ����ϥΪ̡C�����M��!!",0,
'u',	  talk_edituser,    PERM_ACCOUNTS, "�ק�ϥΪ̪���ơC�����M��!!",0,
'K',	  talk_kick,	    PERM_SYSOP, "������H�C�����M��!!",0,
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
      uentp->birth ? "�جP" : uentp->feeling[0] ? uentp->feeling : "����" ,
      buf);

  return state;
}

/*Add End*/

static void
pickup_user()
{
  static int num = 0;

  register user_info *uentp;
  register pid_t pid0=0;  /* Ptt �w��                */
  register int   id0;   /*     US_PICKUP�ɪ���Х� */
  register int state = US_PICKUP, ch;
  register int actor, head, foot;
  char bar_color[50];
  int badman;
  int savemode = currstat;
  time_t diff, freshtime;
  pickup pklist[USHM_SIZE];                     /* parameter Ptt�� */
                                                /* num : �{�b����Ц� */
  						/* actor:�@���h��user */
  						/* foot: �������}�} */
  char *msg_pickup_way[PICKUP_WAYS] =
  { "�١I�B��",
    "���ͥN��",
    "���ͰʺA",
    "�o�b�ɶ�",
    "�Ӧۦ��",
    "�ϥάݪO"
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

            /*Ctrl-U �|�_�u...��o��*/
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
        if (getans2(b_lines, 0, "�A���B���٨S�W���A�n�ݬݤ@����ͶܡH", 0, 2, 'y') != 'n')
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
      sprintf(tmpbuf,"%s [�u�W %d �H]",BOARDNAME,count_ulist());
      showtitle((cuser.uflag & FRIEND_FLAG)? "�n�ͦC��": "�𶢲��", tmpbuf);
      
      prints(" ��)���}  �Ƨ�[[1;36;44m%s[0m] �W���H�� %d  [1;32m�ڪ��B�� %-2d "
        "[33m�P�ڬ��� %-2d [36m�O�� %-2d [31m�a�H %-3d[m\n",
        msg_pickup_way[pickup_way], count_ulist(), 
	(cuser.uflag & FRIEND_FLAG)?friends_number/2:friends_number,
	(cuser.uflag & FRIEND_FLAG)?override_number/2:override_number,
	(cuser.uflag & FRIEND_FLAG)?bfriends_number/2:bfriends_number,
	badman);
	
      prints("%s  %sTP%c�N��         %-17s%-15s S %-12s%-10s[m\n",
	COLOR3,
#ifdef SHOWUID
        show_uid ? "UID" :
#endif
        "No.",
        (HAS_PERM(PERM_SEECLOAK) || HAS_PERM(PERM_SYSOP)) ? 'C' : ' ',

#ifdef REALINFO
        real_name ? "�m�W" :
#endif

        "�ʺ� (N)", 
        show_friend ? "�n�ʹy�z (s)" : show_board ? "�ϥάݪO" : "�G�m (F)",

#ifdef SHOWTTY
        show_tty ? "TTY " :
#endif
        "�ʺA",
#ifdef SHOWPID
        show_pid ? "       PID" :
#endif
#ifdef SHOW_IDLE_TIME
        " �߱�(M) ��"
#else
        " �߱�(M)"
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
    prints("%s  �n�ͦC��  %s       Tab|f)�Ƨ�|�n��  y)��s  w)���y  a|d|o)���  m)�H�H  h)���� \033[m",
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
          /* �����ϥΪ� FRIEND_FLAG ���A */        
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
        {                       /* Thor: �i�H���Ʀr����ӤH */
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


/* talk list ���� */

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
  char *reason[8]={"00.�o�b","11.���q��","22.�V��","33.���O��","44.�˦�","55.ù��","66.��L","qQ.�S��"};

  setutmpmode(IDLE);

  genbuf[0] = getans2(b_lines, 0,"�z�ѡG",reason,8,'0');
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
    if (!cuser.userlevel || !getdata(b_lines, 0, "�o�b���z�ѡG", currutmp->chatid, 11, DOECHO, 0))
      currutmp->destuid = 0;
  {
    char buf[80], passbuf[PASSLEN];
    do
    {
      move(b_lines - 1, 0);
      clrtoeol();
      sprintf(buf, "(��w�ù�)�o�b��]�G%s", (currutmp->destuid != 6) ?
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

  stand_title("�d�ߺ���");
  usercomplete(msg_uid, uident);
  if (getuser(uident) == 0)
    pressanykey("�o�̨S�o�ӤH�I");
  else
  {
    if (uident[0])
      showplans(uident);
  }
  return 0;
}

/* ------------------------------------- */
/* ���H�Ӧ���l�F�A�^���I�s��            */
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
  currstat = XMODE;             /* �קK�X�{�ʵe */

  clear();
  outs("\n\
       (Y) ���ڭ� talk �a�I     (A) �ڲ{�b�ܦ��A�е��@�|��A call ��\n\
       (N) �ڲ{�b���Q talk      (B) �藍�_�A�ڦ��Ʊ������A talk\n\
       (C) �Ф��n�n�ڦn�ܡH     (D) ���ƶܡH�Х��ӫH\n\
       (E) [1;33m�ڦۤv��J�z�Ѧn�F...[m\n\n");

  getuser(uip->userid);
  currutmp->msgs[0].last_pid = uip->pid;
  strcpy(currutmp->msgs[0].last_userid, uip->userid);
  strcpy(currutmp->msgs[0].last_call_in, "�I�s�B�I�s�Ať��Ц^��");
  prints("���Ӧ� [%s]�A�@�W�� %d ���A�峹 %d �g\n",
    uip->from, xuser.numlogins, xuser.numposts);
  show_last_call_in();
  sprintf(genbuf, "�A�Q�� %s %s�ܡH�п��(Y/N/A/B/C/D/E)[Y] ",
    page_requestor, !currutmp->turn ? "���" :
    		    currutmp->dark_turn == 1 ? "�t��" :
    		    currutmp->dark_turn == 2 ? "�H��" :
    		    currutmp->dark_turn == 3 ? "���l��" : "");
    		    
  getdata(0, 0, genbuf, buf, 4, LCECHO,0);

  if (uip->mode != PAGE) 
  {
     pressanykey("%s�w����I�s", page_requestor);
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
    if (!getdata(b_lines, 0, "���� talk ����]�G", genbuf, 60, DOECHO,0))
      strcpy(genbuf, "���i�D�A�� !! ^o^");
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

/* shakalaca.000814: �H�U�o��Ө禡�b .so �����Ψ� :pp */
int
lockutmpmode(int unmode)
{
  if (count_multiplay(unmode))
  {
   char buf[80];
   sprintf(buf,"��p�A�z�w����L�u�ۦP�� ID ���b %s",ModeTypeTable[unmode]);
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

