/*-------------------------------------------------------*/
/* board.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : ¬ÝªO¡B¸s²Õ¥\¯à                               */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

#define BRC_MAXSIZE     24576
#define BRC_MAXNUM      80
#define BRC_ITEMSIZE    (BRC_STRLEN + 1 + BRC_MAXNUM * sizeof( int ))

#define         MAX_FAVORITE    64      /* ­­©w³Ì¦h¥i¥H¦³¦h¤Ö­Ó§Úªº³Ì·R */

int brc_changed = 0;
time_t brc_list[BRC_MAXNUM];
int brc_num;

static int brc_size;

static char brc_buf[BRC_MAXSIZE];
static char brc_name[BRC_STRLEN];

static time_t brc_expire_time;

extern int numboards;
extern boardheader *bcache;

static boardstat *nbrd;

#define B_TOTAL(bptr)		(brdshm->total[(bptr)->pos])
#define B_LASTPOSTTIME(bptr)	(brdshm->lastposttime[(bptr)->pos])
#define	B_BH(bptr)		(&bcache[(bptr)->pos])
#define B_NUM_BH(bnum)		(bcache[nbrd[(bnum)].pos])

int *zapbuf;
int brdnum, yank_flag = 0;

char *boardprefix;

static char *str_local_board = "¡¸¡³¡º¡·¡µ";  /* ¥Nªí local board class */
static char *str_good_board = "¡¸¡¹";  /* ¥Nªí good board class */

/* ----------------------------------------------------- */
/* home/userid/.boardrc maintain                         */
/* ----------------------------------------------------- */

static char *fn_boardrc = ".boardrc";

static char *
brc_getrecord(char *ptr, char *name, int *pnum, time_t *list)
{
  int num;
  char *tmp;

  strncpy(name, ptr, BRC_STRLEN);
  ptr += BRC_STRLEN;
  num = (*ptr++) & 0xff;
  tmp = ptr + num * sizeof(int);
  if (num > BRC_MAXNUM)
    num = BRC_MAXNUM;
  *pnum = num;
  memcpy(list, ptr, num * sizeof(int));
  return tmp;
}


static char *
brc_putrecord(char *ptr, char *name, int num, time_t *list)
{
  if (num > 0 && list[0] > brc_expire_time)
  {
    if (num > BRC_MAXNUM)
      num = BRC_MAXNUM;

    while (num > 1 && list[num - 1] < brc_expire_time)
      num--;

    strncpy(ptr, name, BRC_STRLEN);
    ptr += BRC_STRLEN;
    *ptr++ = num;
    memcpy(ptr, list, num * sizeof(int));
    ptr += num * sizeof(int);
  }
  return ptr;
}


void
brc_update()
{
  if (brc_changed && cuser.userlevel)
  {
    char dirfile[PATHLEN], *ptr;
    char tmp_buf[BRC_MAXSIZE - BRC_ITEMSIZE], *tmp;
    char tmp_name[BRC_STRLEN];
    time_t tmp_list[BRC_MAXNUM];
    int tmp_num;
    int fd, tmp_size;

    ptr = brc_buf;
    if (brc_num > 0)
      ptr = brc_putrecord(ptr, brc_name, brc_num, brc_list);

    sethomefile(dirfile, cuser.userid, fn_boardrc);
    if ((fd = open(dirfile, O_RDONLY)) != -1)
    {
      tmp_size = read(fd, tmp_buf, sizeof(tmp_buf));
      close(fd);
    }
    else
      tmp_size = 0;

    tmp = tmp_buf;
    while (tmp < &tmp_buf[tmp_size] && (*tmp >= ' ' && *tmp <= 'z'))
    {
      tmp = brc_getrecord(tmp, tmp_name, &tmp_num, tmp_list);
      if (strncmp(tmp_name, currboard, BRC_STRLEN))
        ptr = brc_putrecord(ptr, tmp_name, tmp_num, tmp_list);
    }
    brc_size = (int) (ptr - brc_buf);

    if ((fd = open(dirfile, O_WRONLY | O_CREAT, 0644)) != -1)
    {
      ftruncate(fd, 0);
      write(fd, brc_buf, brc_size);
      close(fd);
    }
    brc_changed = 0;
  }
}

static void
read_brc_buf()
{
  char dirfile[PATHLEN];
  int fd;

  if (brc_buf[0] == '\0')
  {
    sethomefile(dirfile, cuser.userid, fn_boardrc);
    if ((fd = open(dirfile, O_RDONLY)) != -1)
    {
      brc_size = read(fd, brc_buf, sizeof(brc_buf));
      close(fd);
    }
    else
      brc_size = 0;
  }
}


int
brc_initial(char *boardname)
{
  char *ptr;

  if (strcmp(currboard, boardname) == 0)
    return brc_num;

  brc_update();
  strcpy(currboard, boardname);
  currbrdattr = bcache[getbnum(currboard)-1].brdattr;  
  read_brc_buf();

  ptr = brc_buf;
  while (ptr < &brc_buf[brc_size] && (*ptr >= ' ' && *ptr <= 'z'))
  {
    ptr = brc_getrecord(ptr, brc_name, &brc_num, brc_list);
    if (strncmp(brc_name, currboard, BRC_STRLEN) == 0)
        return brc_num;
  }
  strncpy(brc_name, boardname, BRC_STRLEN);
  brc_num = brc_list[0] = 1;

  return 0;
}


void
brc_addlist(char *fname)
{
  int ftime, n, i;

  if (!cuser.userlevel)
    return;

  ftime = (time_t) atoi(&fname[2]);
  if (ftime <= brc_expire_time
     /* || fname[0] != 'M' || fname[1] != '.' */ )
  {
    return;
  }

  if (brc_num <= 0)
  {
    brc_list[0] =ftime;
    brc_changed = 1;
    brc_num = 1;
    return;
  }
  if ((brc_num == 1) && (ftime < brc_list[0]))
    return;
  for (n = 0; n < brc_num; n++)
  {
    if (ftime == brc_list[n])
      return;
    else if (ftime > brc_list[n])
    {
      if (brc_num < BRC_MAXNUM)
        brc_num++;
      for (i = brc_num - 1; --i >= n; brc_list[i + 1] = brc_list[i]);
      brc_list[n] = ftime;
      brc_changed = 1;
      return;
    }
  }
  /* (by scw) These lines are no used. Since if it reachs here, this file
   * is already been labeled read.  
  
  if (brc_num < BRC_MAXNUM)
  {
    brc_list[brc_num++] = ftime;
    brc_changed = 1;
  }
  
  */
}


static int
brc_unread_time(time_t ftime, int bnum, time_t *blist)
{
  register int n;
  
  if (ftime <= brc_expire_time)
    return 0;

  if (bnum <= 0)
    return 1;
    
  for (n = 0; n < bnum; n++)
  {
    if (ftime > blist[n])
      return 1;    
    else if (ftime == blist[n])      
      return 0;
  }
  return 0;
}


int
brc_unread(char *fname)
{
  int ftime, n;

  ftime = atoi(&fname[2]);
  if (ftime <= brc_expire_time
     /* || fname[0] != 'M' || fname[1] != '.' */ )
    return 0;
  if (brc_num <= 0)
    return 1;
  for (n = 0; n < brc_num; n++)
  {
    if (ftime > brc_list[n])
      return 1;
    else if (ftime == brc_list[n])
      return 0;
  }
  return 0;
}


/* ----------------------------------------------------- */
/* .bbsrc processing                                     */
/* ----------------------------------------------------- */

char *str_bbsrc = ".bbsrc";

static void
load_zapbuf()
{
  register int n, size;
  char fname[60];

  /* MAXBOARDS ==> ¦Ü¦h¬Ý±o¨£ 4 ­Ó·sªO */

  n = numboards + 4;
  size = n * sizeof(int);
  zapbuf = (int *) malloc(size);
  while (n)
    zapbuf[--n] = login_start_time;
  sethomefile(fname, cuser.userid, str_bbsrc);
  if ((n = open(fname, O_RDONLY, 0600)) != -1)
  {
    read(n, zapbuf, size);
    close(n);
  }
  if (!nbrd)
    nbrd = (boardstat *) malloc(MAXBOARD * sizeof(boardstat));
  brc_expire_time = login_start_time - 365 * 86400;
}


static void
save_zapbuf()
{
  register int fd, size;
  char fname[60];

  sethomefile(fname, cuser.userid, str_bbsrc);
  if ((fd = open(fname, O_WRONLY | O_CREAT, 0600)) != -1)
  {
    size = numboards * sizeof(int);
    write(fd, zapbuf, size);
    close(fd);
  }
}

/*
woju
Ref: bbs.c: brdperm(char* brdname, char* userid)
*/

int 
Ben_Perm(boardheader *bptr)
{
  register int level,brdattr;
  register char *ptr;
  int brdfriend;

  level = bptr->level;
  brdattr = bptr->brdattr;

  if (HAS_PERM(PERM_BBSADM))
    return 1;

  ptr = bptr->BM;
  if (userid_is_BM(cuser.userid, ptr))
    return 1;

  /* ¬ÝªOªý¾×¦W³æ */
  brdfriend = hbflcheck((int)(bptr - bcache) + 1, currutmp->uid);
  if(brdfriend == 2)	/* ³]¬°Ãa¤H*/
    return 0;

  /* ¯¦±K¬ÝªO */

  if (brdattr & BRD_HIDE) /* ¨p¤H/ÁôÂÃ */
  {
    if (brdfriend != 1)
    {
      if(brdattr & BRD_POSTMASK)  /* ÁôÂÃ */
        return 0;
      else
        return 2;
    }
    else return 1;
  }
                                         /* ­­¨î¾\ÅªÅv­­ */
//  if((brdattr & BRD_PERSONAL) && !(brdattr & BRD_HIDE))
//    return 1;
  if(!(brdattr & BRD_POSTMASK) && HAS_PERM(level))
    return 1;
  else if (!(brdattr & BRD_POSTMASK) && !HAS_PERM(level))
    return 0;
  return 1;
}

static void 
load_boards(char *bname , usint mode)
{
  boardheader *bptr;
  boardstat *ptr;
  char brdclass[5];
  int n;
  register char state;

  resolve_boards();
  if (!zapbuf)
    load_zapbuf();

  brdnum = 0;
  for (n = 0; n < numboards; n++)
  {
    bptr = &bcache[n];
    if (bptr->brdname[0] == '\0')
      continue;
    if (bname)
    {
      if(strcmp(bname,bptr->brdname)) 
        continue;
      ptr = &nbrd[brdnum++];
      ptr->pos = n;
      ptr->zap = (zapbuf[n] == 0);
      check_newpost(ptr);
      return;
    }
    
    if (mode == BRD_POPULAR)
    {
      if (brdshm->nusers[n] <= 0)
	continue;
    }
    else if(mode)
    {
      if(!(bptr->brdattr & mode))
        continue;
    }
    else if (boardprefix)
    {
      if (boardprefix == str_local_board || boardprefix == str_good_board)
      {
        strncpy(brdclass, bptr->title + 5, 2);
        brdclass[2] = '\0';
      }
      else
      {
        strncpy(brdclass, bptr->title, 4);
        brdclass[4] = '\0';
      }
      if (strstr(boardprefix, brdclass) == NULL)
        continue;
    }
    else if(bptr->brdattr & BRD_GROUPBOARD || bptr->brdattr & BRD_CLASS)
      continue;

    if ((state = Ben_Perm(bptr)) && (yank_flag == 1 || (yank_flag == 2 &&
         ((bptr->brdattr & BRD_GROUPBOARD || bptr->brdattr & BRD_CLASS || have_author(bptr->brdname))))
         || yank_flag != 2 && zapbuf[n]))
    {
      ptr = &nbrd[brdnum++];

      ptr->pos = n;
      ptr->zap = (zapbuf[n] == 0);
      check_newpost(ptr);
    }
  }

  /* ¦pªG user ±N©Ò¦³ boards ³£ zap ±¼¤F */

  if (!brdnum && !boardprefix)
     if (yank_flag == 0)
        yank_flag = 1;
     else if (yank_flag == 2)
        yank_flag = 0;
}


static int 
search_board()
{
  char genbuf[IDLEN + 2];
  int num;

     move(0,0);
     clrtoeol();
     CreateNameList();
     for (num = 0; num < brdnum; num++)
        AddNameList(B_NUM_BH(num).brdname);
     namecomplete(MSG_SELECT_BOARD, genbuf);
     
     for (num = 0; num < brdnum; num++)
        if (!strcmp(B_NUM_BH(num).brdname, genbuf))
           return num;

  return -1;
}

int 
check_newpost(boardstat *ptr)
{
  int tbrc_num;
  char bname[BRC_STRLEN];
  char *po;
  time_t ftime, tbrc_list[BRC_MAXNUM];

  ptr->unread = 0;

  if (B_BH(ptr)->brdattr & (BRD_GROUPBOARD | BRD_CLASS))
    return 0;

  if (B_TOTAL(ptr) == 0)
    setbtotal((ptr->pos) + 1);

  if (B_TOTAL(ptr) == 0)
    return 0;

  ftime = B_LASTPOSTTIME(ptr);

  read_brc_buf();
  po = brc_buf;
  while (po < &brc_buf[brc_size] && (*po >= ' ' && *po <= 'z'))
  {
    po = brc_getrecord(po, bname, &tbrc_num, tbrc_list);
    if (strncmp(bname, B_BH(ptr)->brdname, BRC_STRLEN) == 0)
    {
      if (brc_unread_time(ftime, tbrc_num, tbrc_list))
        ptr->unread = 1;
      return 1;
    }
  }
  ptr->unread = 1;
  return 1;
}

static int 
unread_position(dirfile, ptr)
  char *dirfile;
  boardstat *ptr;
{
  fileheader fh;
  char fname[FNLEN];
  register int num, fd, step, total;

  total = B_TOTAL(ptr);
  num = total + 1;
  if (ptr->unread && (fd = open(dirfile, O_RDWR)) > 0)
  {
    if (!brc_initial(B_BH(ptr)->brdname))
      num = 1;
    else
    {
      num = total - 1;
      step = 4;
      while (num > 0)
      {
        lseek(fd, (off_t)(num * sizeof(fh)), SEEK_SET);
        if (read(fd, fname, FNLEN) <= 0 || !brc_unread(fname))
          break;
        num -= step;
        if (step < 32)
          step += step >> 1;
      }
      if (num < 0)
        num = 0;
      while (num < total)
      {
        lseek(fd, (off_t)(num * sizeof(fh)), SEEK_SET);
        if (read(fd, fname, FNLEN) <= 0 || brc_unread(fname))
          break;
        num++;
      }
    }
    close(fd);
  }
  if (num < 0)
    num = 0;
  return num;
}

int
have_author(char* brdname)
{
   char dirname[100];
   extern cmpfowner();

   sprintf(dirname, "¥¿¦b·j´M§@ªÌ[33m%s[m ¬ÝªO:[1;33m%s[0m.....",
           currauthor,brdname);
   move(b_lines, 0);
   clrtoeol();
   outs(dirname);
   refresh();
   setbdir(dirname, brdname);
   str_lower(currowner, currauthor);
   return search_rec(dirname, cmpfowner);
}

char *
nusers_printer(char *buf, int bid)
{
  if (brdshm->nusers[bid] >= 15)
    sprintf(buf, "[1;31mHOT[m");
  else if (brdshm->nusers[bid] >= 10)
    sprintf(buf, "[1;31m%2d[m ", brdshm->nusers[bid]);
  else if (brdshm->nusers[bid] >= 5)
    sprintf(buf, "[1;33m%2d[m ", brdshm->nusers[bid]);
  else if (brdshm->nusers[bid] > 0)
    sprintf(buf, "%2d ", brdshm->nusers[bid]);
  else
    sprintf(buf, "   ");

  return buf;
}

static void 
show_brdlist_line(int headx, int row, int clsflag, int newflag, char *bar_color)
{
  boardstat *ptr;
  int head = headx;
  static char *color[7]={"[1;31m","[1;35m","[1;33m","[1;30m","[1;37m","[1;36m","[1;32m"};
  static char *unread[2]={"  ","[1;32m¡°\033[m"};
  char attrbuf[35], brdtype, nusers[35];

      move(row, 0);
      clrtoeol();
      if (head < brdnum)
      {
        ptr = &nbrd[head++];

        if(B_TOTAL(ptr) == 0)
          check_newpost(ptr);
        
        brdtype = !(B_BH(ptr)->brdattr & BRD_HIDE) ? ' ':
                 (B_BH(ptr)->brdattr & BRD_POSTMASK) ? ')' : '-';
                 
        if (yank_flag == 2)
        {
          prints("%6d%c%c ",
               head,B_BH(ptr)->brdattr & BRD_HIDE ? ')':' ',
               (B_BH(ptr)->brdattr & BRD_GROUPBOARD || B_BH(ptr)->brdattr & BRD_CLASS) ? ' ':'A');
        }
        else if (!newflag)
        {
          prints("%6d%c%s",
                 head,brdtype,
                 ptr->zap ? "--" :
                 (B_BH(ptr)->brdattr & BRD_GROUPBOARD) ? "[0;33m£U" :
                 (B_BH(ptr)->brdattr & BRD_CLASS) ? "[0;36m¡¼" :
                 unread[ptr->unread]);
        }
        else if (ptr->zap)
          outs("  ------");
        else if (newflag)
        {
          prints((B_BH(ptr)->brdattr & BRD_GROUPBOARD
            || B_BH(ptr)->brdattr & BRD_CLASS) ? "         "
            :"%6d%c%s", (B_TOTAL(ptr)), brdtype, unread[ptr->unread]
          );
        }

        if(clsflag != 2)  /* «Ø¥ß title ©ÎÄÝ©Ê */
          strcpy(attrbuf, B_BH(ptr)->title+5);
        else
        {
          sprintf(attrbuf," %s%s%s%s%s%s%s%s ",
            B_BH(ptr)->brdattr & BRD_NOZAP ?  "¢æ" : "£¾",
	    B_BH(ptr)->brdattr & BRD_NOCOUNT ? "¢æ" : "£¾",
	    B_BH(ptr)->brdattr & BRD_NOTRAN ? "¢æ" : "£¾",
	    B_BH(ptr)->brdattr & BRD_HIDE ?  "£¾" : "¢æ",
	    (B_BH(ptr)->brdattr & BRD_HIDE) && ( B_BH(ptr)->brdattr & BRD_POSTMASK) ? "£¾" : "¢æ",
	    B_BH(ptr)->brdattr & BRD_ANONYMOUS ? "£¾" : "¢æ",
	    B_BH(ptr)->brdattr & BRD_GOOD ? 	 "£¾" : "¢æ",
	    B_BH(ptr)->brdattr & BRD_PERSONAL ? "£¾" : "¢æ");
        }

        prints("%s%-12s[m %s%-5.5s[m%-35.35s%s%s%-12.12s[m",
          (bar_color) ? bar_color : "", B_BH(ptr)->brdname,
          color[(unsigned int)(B_BH(ptr)->title[1]+B_BH(ptr)->title[2]+
                 B_BH(ptr)->title[3]+B_BH(ptr)->title[0])%7],
          B_BH(ptr)->title, attrbuf,
          (B_BH(ptr)->bvote == 1 ? "[1;33m¦³[m" : 
           B_BH(ptr)->bvote == 2 ? "[1;37m¶}[m" : "  "),
          nusers_printer(nusers, ptr->pos),
          B_BH(ptr)->BM);        
      }
}                                                                                

static void 
brdlist_foot()
{
  move(b_lines, 0);
  clrtoeol();
  prints("%s  ¿ï¾Ü¬ÝªO  %s           ¡ö¡ô¡õ¡÷|PgUp|PgDn|Home|End)¾ÉÄý  c)·s¤å³¹¼Ò¦¡  h)»¡©ú [m", 
    	 COLOR2, COLOR3);
}

static void 
show_brdlist(int head, int clsflag, int newflag)
{
  if (clsflag)
  {
    sprintf(tmpbuf,"%s [½u¤W %d ¤H]",BOARDNAME,count_ulist());
    showtitle("¬ÝªO¦Cªí", tmpbuf);

    move(1,0);
    clrtoeol();
    prints("%s¢«¬ÝªO\033[0;37m%s¢©³Ì·R¢@¤å³¹¢@ºëµØ°Ï\033[30m¢ª\033[m                   S)±Æ§Ç  v|V)¼Ð°O¤wÅª/¥¼Åª  %s", 
    	   COLOR3, COLOR1, yank_flag ? "¥þ³¡" : "­q¾\\");
    
    move(2, 0);
    clrtoeol();
    prints("%s%-20s  Ãþ§O ÄÝ %-31s §ë¤H ªO    ¥D    [m",
      COLOR3, 
      newflag ? "  Á`¼Æ   ¬Ý    ªO" : "  ½s¸¹   ¬Ý    ªO", 
      clsflag == 1 ? " ¤¤   ¤å   ±Ô   ­z" : " ¢è²ÎÂà¨pÁô°ÎÀu­Ó ");

    brdlist_foot();
  }

  if (brdnum > 0)
  {
    int myrow;  /* Ptt add color */
    int endrow = (HAS_HABIT(HABIT_NOBRDHELP)) ? b_lines : b_lines - 4;

    myrow = 2;
    while (++myrow < endrow)
      show_brdlist_line(head++ , myrow, clsflag, newflag, 0);
  }
}

/* ®Ú¾Ú title ©Î name °µ sort */

static int 
cmpboard(brd, tmp)
  boardstat *brd, *tmp;
{
  register int type = 0;

  if(B_BH(brd)->brdattr & BRD_CLASS)
    type -= BRD_CLASS;
  if(B_BH(brd)->brdattr & BRD_GROUPBOARD)
    type -= BRD_GROUPBOARD;
  if(B_BH(tmp)->brdattr & BRD_CLASS)
    type += BRD_CLASS;
  if(B_BH(tmp)->brdattr & BRD_GROUPBOARD)
    type += BRD_GROUPBOARD;    

  if(type)
    return type;

  if (!(cuser.uflag & BRDSORT_FLAG))
    type = strcasecmp(B_BH(brd)->brdname, B_BH(tmp)->brdname);
  else
  {
    type = strncmp(B_BH(brd)->title, B_BH(tmp)->title, 4);
    type *= 256;
    type += strcasecmp(B_BH(brd)->brdname, B_BH(tmp)->brdname);
  }

  return type;
}

/* ®Ú¾Ú¬ÝªO¤H®ð°µ sort */
static int
cmppopular(brd, tmp)
  boardstat *brd, *tmp;
{
  return brdshm->nusers[tmp->pos] - brdshm->nusers[brd->pos];
}

void 
set_menu_BM(char *BM)
{
  if (HAS_PERM(PERM_ALLBOARD) || userid_is_BM(cuser.userid, BM))
    currmode |= MODE_MENU;
}

/*By hialan ¬ÝªO¤º«öv */
int v_board (int ent, fileheader *fhdr, char *direct)
{
  boardstat *ptr;
  char ans;
  char *choose[3] = {"vV)¤wÅª","uU)¥¼Åª",msg_choose_cancel};
  
  ans = getans2(b_lines - 1, 0,"³]©w¬ÝªO¡G", choose, 3, 'v');
  move(b_lines - 1, 0);
  clrtobot();
  outs(" ");
  
  if(ans == 'q') 
    return RC_NONE;
  
  load_boards(currboard,0);
  ptr = &nbrd[0];
  check_newpost(ptr);
  brc_initial(B_BH(ptr)->brdname);

  if (ans == 'v')
  {
    ptr->unread = 0;
    zapbuf[ptr->pos] = time((time_t *) &brc_list[0]);
  }
  else
  zapbuf[ptr->pos] = brc_list[0] = ptr->unread = 1;

  brc_num = brc_changed = 1;
  brc_update();
    
  brdnum = -1;
  return RC_DRAW;
}

struct one_key board_comms[] =
{
 'e', NULL, 	     0, "Â÷¶}",0,
 'A', NULL, PERM_SYSOP, "¦C¥X¬ÝªOÄÝ©Ê", 0,
 'a', NULL, 	     0, "·j´M§@ªÌ", 0,
 'f', NULL, 	     0, "±N¬ÝªO¥[¤J§Úªº³Ì·R", 0,
 'F', NULL,  	     0, "±N¬ÝªO¥[¤J§Úªº³Ì·R", 0,
Ctrl('A'), NULL,     0, "¾\\Åª¥»¯¸¤Ñ¦aºëµØ", 0,
 '/', NULL,	     0, "·j´M¬ÝªO", 0,
 's', NULL,	     0, "·j´M¬ÝªO", 0, 
 'S', NULL,	     0, "¤Á´«¦Cªí±Æ§Ç¤è¦¡", 0,
 'y', NULL,	     0, "¤Á´«¬O§_¦C¥X­q¾\\¬ÝªO", 0,
 'z', NULL, PERM_BASIC, "­q¾\\/¨ú®ø­q¾\\¬ÝªO", 0,
 'Z', NULL, PERM_BASIC, "§ó·s¬ÝªO¤å³¹¬O§_¾\\Åªª¬ºA?", 0,
 'v', NULL, 	     0, "±N¬ÝªO³]¦¨¤w¾\\Åª", 0,
 'V', NULL,	     0, "±N¬ÝªO³]¦¨¥¼¾\\Åª", 0,
 'Q', NULL, PERM_BASIC, "¬d¸ß¬ÝªO¸ê°T", 0,
 'B', NULL, PERM_SYSOP, "«Ø¥ß¬ÝªO", 0,
 'r', NULL, 	     0, "¶i¤J¦h¥\\¯à¾\\Åª¿ï³æ", 0,
 'b', NULL,    PERM_BM, "ºÞ²z¸s²Õ¥\\¯à", 0,
'\0', NULL, 0, NULL, 0};

char groupbrd[IDLEN + 1] = "";

void
choose_board(int newflag,usint mode)
{
#define SHOW_BRDLIST() show_brdlist(head, clsflag, newflag)

  static int num = 0;
  int attmode = 1;
  boardstat *ptr;
  int head, ch, tmp,tmp1;
  int clsflag;
  char genbuf[200],*prefixtmp;
  extern time_t board_visit_time;
  char bar_color[50];
  int page_lines;    /*¤@­¶¦³´X¦æ*/  
  
  /* Åã¥Ü¶iªOµe­± */
  if(groupbrd[0] != '\0')
  {
    setbfile (genbuf, groupbrd, fn_notes);
    more (genbuf, YEA);
  }

  if(HAS_HABIT(HABIT_NOBRDHELP))
    page_lines = b_lines - 4 + 1;
  else
    page_lines = b_lines - 8 + 1; /* ¦]¬°¦æ¼Æ±q0¶}©l */
    
  get_lightbar_color(bar_color);

  setutmpmode(newflag ? READNEW : READBRD);
  brdnum = 0;
  if (!cuser.userlevel)         /* guest yank all boards */
    yank_flag = 1;

  do
  {
    if (brdnum <= 0)
    {
      load_boards(NULL,mode);
      if (brdnum <= 0)
        break;
      if(mode == BRD_POPULAR)
        qsort(nbrd, brdnum, sizeof(boardstat), cmppopular);
      else
        qsort(nbrd, brdnum, sizeof(boardstat), cmpboard);
      head = -1;
    }

    if (num < 0)
      num = 0;
    else if (num >= brdnum)
      num = brdnum - 1;

    if (head < 0)
    {
      if (newflag)
      {
        tmp = num;
        while (num < brdnum)
        {
          ptr = &nbrd[num];
          if(B_TOTAL(ptr) == 0)
            check_newpost(ptr);
          if (ptr->unread)
            break;
          num++;
        }
        if (num >= brdnum)
          num = tmp;
      }
      head = (num / page_lines) * page_lines;
      clsflag = 1;
      SHOW_BRDLIST();
    }
    else if (num < head || num >= head + page_lines)
    {
      head = (num / page_lines) * page_lines;
      clsflag = 0;
      SHOW_BRDLIST();
    }
    
    if(!HAS_HABIT(HABIT_NOBRDHELP))
    {
      clrchyiuan(b_lines - 4, b_lines - 1);
      move(b_lines - 4, 0);
      prints("\033[0m%s\033[m\n", msg_seperator);
      
      move(b_lines - 4, 6);
      outs("¬ÝªO»¡©ú");
      
      move(b_lines - 3, 0);
      clrtoeol();
      outs( B_NUM_BH(num).desc[0]);
      
      move(b_lines - 2, 0);
      clrtoeol();
      outs( B_NUM_BH(num).desc[1]);
      
      move(b_lines - 1, 0);
      clrtoeol();
      outs( B_NUM_BH(num).desc[2]);
      {
        int c;

        c = page_lines - (num % page_lines);
        if(num+c < brdnum && B_NUM_BH(num+c).brdname)
        {
          move(b_lines - 4, 64);
          prints("[¤U­¶ÁÙ¦³¬ÝªO¡I]");
        }
      }
    }

    if( HAVE_HABIT(HABIT_LIGHTBAR) )
    {
      show_brdlist_line(num, 3 + num - head, clsflag, newflag, bar_color);  
      move(3+num-head, 0);
      ch = igetkey();
      show_brdlist_line(num, 3 + num - head, clsflag, newflag, 0);      
    }
    else
      ch = cursor_key(3 + num - head, 0);

    if(ch == 'h')
    {
      ch = i_read_helper(board_comms);
      clsflag = 1;
      SHOW_BRDLIST();
    }
      
    switch (ch)
    {
      case Ctrl('Z'):	/* ¸Ñ¨M ¿ï §Ö³t¿ï³æ-->¬ÝªO¦Cªí-->¦^¨Ó¦Cªí¿ù¶Ã */
        brdnum = -1;
        break;
        
      case 'e':
      case KEY_LEFT:
      case EOF:
        ch = 'q';
      case 'q':
        break;

     case 'c':
      if (yank_flag == 2) 
      {
         newflag = yank_flag = 0;
         brdnum = -1;
      }
      show_brdlist(head, 1, newflag ^= 1);
      clsflag = 1;
      break;


     /* wildcat : show board attr in list */
     case 'A':
      if(HAS_PERM(PERM_SYSOP))
      {
        if(attmode >= 2) attmode = 1;
        else attmode = 2;
        clsflag = attmode;
        SHOW_BRDLIST();
      }
      break;

     case 'a': 
     {
       if (yank_flag != 2 ) 
       {
         sprintf(genbuf, "%s", currauthor);
         if (getdata(1, 0,"§@ªÌ¡G", genbuf, IDLEN + 2, DOECHO,currauthor))
            strncpy(currauthor, genbuf, IDLEN + 2);
         if (*currauthor)
           yank_flag = 2;
         else
           yank_flag= 0;
        }
        else
          yank_flag = 0;
          brdnum = -1;
          clsflag = 1;
          SHOW_BRDLIST();
          break;
      }

      case 'f':
      case 'F':
        ptr = &nbrd[num];
        favor_brd_add(B_BH(ptr)->brdname);
        break;

      case KEY_PGUP:
      case 'P':
      case Ctrl('B'):
        if (num)
        {
          num -= page_lines;
          break;
        }

      case KEY_END:
      case '$':
        num = brdnum - 1;
        break;

      case ' ':
      case KEY_PGDN:
      case 'N':
      case Ctrl('F'):
        if (num == brdnum - 1)
          num = 0;
        else
          num += page_lines;
        break;

      case KEY_ESC: 
        if (KEY_ESC_arg == 'n') 
        {
          edit_note();
          clsflag = 1;
          SHOW_BRDLIST();
        }
        break;

      case KEY_UP:
      case 'p':
      case 'k':
        if (num-- <= 0)
          num = brdnum - 1;
        break;

      case KEY_DOWN:
      case 'n':
      case 'j':
        if (++num < brdnum)
          break;

      case '0':
      case KEY_HOME:
        num = 0;
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
        if ((tmp = search_num(ch, brdnum)) >= 0)
          num = tmp;
        brdlist_foot();
        break;

      case Ctrl('A'):
        Announce();
        clsflag = 1;
        SHOW_BRDLIST();
        break;

      case 's':
      case '/':
        if ((tmp = search_board()) >= 0)
          num = tmp;
        clsflag = 1;
        SHOW_BRDLIST();
        break;

      case 'S':
        cuser.uflag ^= BRDSORT_FLAG;
        substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* °O¿ý */
        move(3,0);
        clrtobot();
        qsort(nbrd, brdnum, sizeof(boardstat), cmpboard);
        head = 999;
        move(b_lines,0);
        brdlist_foot();
        break;

      case 'y':
        if (yank_flag == 2)
          yank_flag = 0;
        else
          yank_flag ^= 1;
        brdnum = -1;
        break;

      case 'z':                   /* opus: no perm check ? */
        if (HAS_PERM(PERM_BASIC))
        {
          ptr = &nbrd[num];
          ptr->zap = !ptr->zap;
          if(B_BH(ptr)->brdattr & BRD_NOZAP) ptr->zap = 0;
          if(!ptr->zap) check_newpost(ptr);
          zapbuf[ptr->pos] = (ptr->zap ? 0 : login_start_time);
          head = 999;
        }
        break;

      case 'Z':                   /* opus: no perm check ? */
        if (HAS_PERM(PERM_BASIC))
        {
          int i;
          for(i=0;i < MAXBOARD;i++)
          {
            ptr = &nbrd[i];
            ptr->zap = 0;
            check_newpost(ptr);
            zapbuf[ptr->pos] = (ptr->zap ? 0 : login_start_time);
            head = 999;
          }
        }
        break;

      case 'v':
      case 'V':
        ptr = &nbrd[num];
        brc_initial(B_BH(ptr)->brdname);
        if (ch == 'v')
        {
          ptr->unread = 0;
          zapbuf[ptr->pos] = time((time_t *) &brc_list[0]);
        }
        else
          zapbuf[ptr->pos] = brc_list[0] = ptr->unread = 1;
        brc_num = brc_changed = 1;
        brc_update();
        clsflag = 0;
        SHOW_BRDLIST();
        break;

      case 'Q':
        if (HAS_PERM(PERM_BASIC) || (currmode & MODE_MENU))
        {
          ptr = &nbrd[num];
          move(1,1);
          clrtobot();
          DL_func("SO/admin.so:va_m_mod_board", B_BH(ptr)->brdname);
          brdnum = -1;
        }
        break;
      case 'B':
        if (HAS_PERM(PERM_SYSOP) || (currmode & MODE_MENU)) 
        {
          DL_func("SO/admin.so:m_newbrd");
          brdnum = -1;
        }
        break;
      
      case 'b':
        if (HAS_PERM(PERM_SYSOP) || (currmode & MODE_MENU))
        {
          char *choose[3] = {"eE)½s¿è¸s²Õ¶iªOµe­±", "dD)§R°£¸s²Õ¶iªOµe­±", msg_choose_cancel};
          
          if(groupbrd[0] != '\0')
            setbfile(genbuf, groupbrd, fn_notes);
          else
            break;

          switch(win_select(groupbrd, "½Ð¿ï¾Ü©Ò»Ýªº°Ê§@", choose, 3, 'e'))
          {
            case 'e':
            {
              char mode = dashf(genbuf);
              
              if (vedit (genbuf, NA) == -1)
              {
                pressanykey (msg_cancel);
                if(!mode)
                  unlink(genbuf);
              }
              
              break;
            }
            case 'd':
              unlink(genbuf);
              break;
          }
          SHOW_BRDLIST();
        }
        break;
      
      case KEY_RIGHT:
      case '\n':
      case '\r':
      case 'r':
      {
        char buf[STRLEN];

        ptr = &nbrd[num];

        if (!(B_BH(ptr)->brdattr & BRD_GROUPBOARD || B_BH(ptr)->brdattr & BRD_CLASS)) /* «Dsub class */
        {

/*
   wildcat 000121 : ¥u­n§PÂ_¨p¤HªO´N¦n , ÁôÂÃªO¬Ýªº¨ìªº´N¬Ýªº¨ì , ¬Ý¤£¨ìªº
                    ÁÙ¬O¬Ý¤£¨ì :Q , ¥»¨Ó¦³¬q©_©Çªº FN_APPLICATION ´N®³±¼§a
                    ¥i¬O«ç»òÄ±±o¬O¦b­«½Æ Ben_Perm °µªº§PÂ_?
*/
          if((B_BH(ptr)->brdattr & BRD_HIDE && !(B_BH(ptr)->brdattr & BRD_POSTMASK))
             && (!HAS_PERM(PERM_SYSOP) && !userid_is_BM(cuser.userid, B_BH(ptr)->BM)))
          {
            if(!hbflcheck(ptr->pos + 1, currutmp->uid))
            {
              pressanykey(P_BOARD);
              break;
            }
          }

          brc_initial(B_BH(ptr)->brdname);

          if (yank_flag == 2)
          {
            setbdir(buf, currboard);
            tmp = have_author(currboard) - 1;
            head = tmp - t_lines / 2;
            getkeep(buf, head > 1 ? head : 1, -(tmp + 1));
          }
          else if (newflag)
          {
            setbdir(buf, currboard);
            tmp = unread_position(buf, ptr);
            head = tmp - t_lines / 2;
            getkeep(buf, head > 1 ? head : 1, tmp + 1);
          }
          board_visit_time = zapbuf[ptr->pos];
          if (!ptr->zap)
            time((time_t *) &zapbuf[ptr->pos]);
          Read();
          check_newpost(ptr);
          head = -1;
          setutmpmode(newflag ? READNEW : READBRD);
        }
        else                                   /* sub class */
        {
            int currmode0 = currmode;
            char groupbrd0[IDLEN + 1];
            
            strlcpy(groupbrd0, groupbrd, sizeof(groupbrd0));
            prefixtmp = boardprefix;
            tmp1=num; num=0;
            boardprefix = B_BH(ptr)->title+7;

            strlcpy(groupbrd, B_BH(ptr)->brdname, sizeof(groupbrd));
            set_menu_BM(B_BH(ptr)->BM);
            log_board3("USE", B_BH(ptr)->brdname, 0);

            if(!strcmp(B_BH(ptr)->brdname, PERSONAL_ALL_BRD))
              choose_board(HAS_HABIT(HABIT_BOARDLIST), BRD_PERSONAL);

            else if(!strcmp(B_BH(ptr)->brdname, GROUP_ALL_BRD))
              choose_board(HAS_HABIT(HABIT_BOARDLIST), BRD_GROUP);

            else if(!strcmp(B_BH(ptr)->brdname, HIDE_ALL_BRD))
              choose_board(HAS_HABIT(HABIT_BOARDLIST), BRD_HIDE);

            else
              choose_board(HAS_HABIT(HABIT_BOARDLIST), 0);

            strlcpy(groupbrd, groupbrd0, sizeof(groupbrd));
            currmode = currmode0;
            num=tmp1;
            boardprefix = prefixtmp;
            brdnum = -1;
         }
      }
    }
  } while (ch != 'q');
  save_zapbuf();
  
  return;
}


int
board()
{
  choose_board(cuser.habit & HABIT_BOARDLIST,0);
  return 0;
}


int
local_board()
{
  boardprefix = str_local_board;
  choose_board(cuser.habit & HABIT_BOARDLIST,0);
  return 0;
}

int
good_board()
{
  boardprefix = str_good_board;
  choose_board(cuser.habit & HABIT_BOARDLIST,0);
  return 0;
}

int
popularboard()
{
  int mode0 = currutmp->mode;
  int stat0 = currstat;

  choose_board(cuser.habit & HABIT_BOARDLIST, BRD_POPULAR);
  currutmp->mode = mode0;
  currstat = stat0;
  return 0;
}

int
Boards()
{
  boardprefix = NULL;
  choose_board(cuser.habit & HABIT_BOARDLIST,0);
  return 0;
}


int
New()
{
  int mode0 = currutmp->mode;
  int stat0 = currstat;

  boardprefix = NULL;

  choose_board(cuser.habit & HABIT_BOARDLIST,0);

  currutmp->mode = mode0;
  currstat = stat0;
  
  return RC_FULL;
}

void force_board(char *bname)
{
  boardstat *ptr;
  char buf[80];

  setbpath(buf ,bname);
  if(!dashd(buf)) return;
  brdnum = 0;
  load_boards(bname,0);
  ptr = &nbrd[0];
  check_newpost(ptr);
  while(ptr->unread && cuser.userlevel) /* guest skip force read */
  {
    char buf[80];
    sprintf(buf," %s ªO¦³·s¤å³¹¡A½Ð¾\\Åª§¹·s¤å³¹«á¦AÂ÷¶}¡I",bname);
    pressanykey(buf);
    brc_initial(B_BH(ptr)->brdname);
    Read();
    check_newpost(ptr);
  }
}

void 
voteboard()
{
  boardstat *ptr;
  char buf[80];

  setbpath(buf ,VOTEBOARD);
  if(!dashd(buf)) return;
  brdnum = 0;
  load_boards(VOTEBOARD,0);
  ptr = &nbrd[0];
  brc_initial(B_BH(ptr)->brdname);
  Read();
}

/*-------------------------------------------------------*/
/* festival.c     ( AT-BBS/WD_hialan 	V1.5)            */
/*-------------------------------------------------------*/
/* author : hialan.nation@infor.org                      */
/* target : Board Festival                               */
/* create : 2002/08/24                                   */
/*-------------------------------------------------------*/
#define FRESH_TIME  2*60*60		//§ó·s®É¶¡

static int
check_feast(int bid)
{
  time_t now=time(0);
  time_t brd_ori = brdshm->feast_update[bid-1];
  int len = strlen(brdshm->festival[bid-1]);
  
  if((now - brd_ori > FRESH_TIME) || !brd_ori || len > 14 || !len)
    return 1;
  else
    return 0;
}

/* From Athenaeum by ychia.nation@infor.org*/
/* 1:¦blist¤¤ 0:¤£¦blist¤¤ */
static int 
get_belong_list(char *filelist, char *key, char *from)
{
  FILE *fp;
  char buf[80], *site, *desc;
  int rc;

  rc = 0;
  if (fp = fopen(filelist, "r"))
  {
    while (fgets(buf, 80, fp) && buf[0])
    {
      if (buf[0] == '#')
        continue;

      site = (char *) strtok(buf, " \t\r\n");
      if (site && *site)
      {
        if (strstr(key, site))
        {
          desc = (char *) strtok(NULL, " \t\r\n");
          if (desc && *desc)
          {
            strcpy(from, desc);
            rc = 1;
            break;
          }
        }
      }
    }
    fclose(fp);
  }
  return rc;
}

void 
load_feast(char *brdname)
{
  int bid = getbnum(brdname);
  char fpath[PATHLEN];
  char buf[16], key[6];
  time_t now=time(0);
  struct tm *ptime=localtime(&now);

  resolve_boards();
  setbfile(fpath, brdname, FN_BRDFEAST);
  sprintf(key, "%02d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
  if(get_belong_list(fpath, key, buf))
  {
    strncpy(brdshm->festival[bid-1], buf, 14);
    brdshm->festival[bid-1][14] = '\0';
    brdshm->feast_update[bid-1] = now;
  }
  else
    strcpy(brdshm->festival[bid-1], "§Ö¸òªO¥D¥Ó½Ð§a");
  
  return;
}

char *
get_feast(char *brdname)
{
  int bid = getbnum(brdname);
  char fpath[PATHLEN];

  setbfile(fpath, brdname, FN_BRDFEAST);

  if(dashf(fpath))
  {
    if(check_feast(bid))
      load_feast(brdname);
  }
  else
    strcpy(brdshm->festival[bid-1], "§Ö¸òªO¥D¥Ó½Ð§a");
  
  return brdshm->festival[bid-1];
}
