/*-------------------------------------------------------*/
/* favor.c       ( MCU CSIE MYTH-BBS Ver 1.02 )          */
/*-------------------------------------------------------*/
/* target : my favorite boards routines                  */
/* create : 03/08/09                                     */
/* update : 03/08/20                                     */
/*-------------------------------------------------------*/
/* author : sby.bbs@bbs.csie.mcu.edu.tw                  */
/* change : hialan					 */
/*-------------------------------------------------------*/

#include "bbs.h"

extern int numboards;
extern boardheader *bcache;
extern int list_move();
extern int cmpfilename();
extern char groupbrd[IDLEN +1];

#define B_FVR(bnum)      (&bcache[bnum])
#define FN_FAVOR	"favordir"
#define MAX_FAVOR	(128)

#define FAVOR_BOARD	00001
#define FAVOR_GROUP	00002

static char favorowner[IDLEN + 2];
static char favorpath[PATHLEN];
static char favordesc[TTLEN + 1];
static usint favortype;

static uschar favor_newflag = 0;
static uschar favor_cmpflag = 0;

static char *err_system = "¨t²Îµo¥Í¿ù»~!!";


/* sby: ­q°µµ¹ favor.c ªº getbnum, ¥h±¼Åv­­ÀË¬d, ¥[§ÖÅª¨ú³t«× */
static int
favor_getbnum(char *bname)
{
  register int i;
  register boardheader *bhdr;

  resolve_boards();
  for (i = 0, bhdr = bcache; i++ < numboards; bhdr++)
    if (!strcasecmp(bname, bhdr->brdname))
      return i;
  return 0;
}

static void
favortitle(int type)
{
  char buf[256];
  
  switch (type)
  {
    case RC_FULL:
    sprintf(buf,"%s [½u¤W %d ¤H]",BOARDNAME,count_ulist());
    showtitle("§Úªº³Ì·R", buf);

    prints("%s\033[30m¢«\033[37m¬ÝªO¢¨%s³Ì·R%s\033[37m¢©¤å³¹¢@ºëµØ°Ï\033[36;40m¢©\033[m                y)©Ò¦³¬ÝªO  a)·s¼W¬ÝªO  g)·s¼W¸s²Õ \n",
      COLOR1, COLOR3, COLOR1);
    prints("\033[0;47;30m  %s   %-12s Ãþ§O ÄÝ %-31s §ë¤H ªO    ¥D    \033[m",
      favor_newflag ? "Á`¼Æ" : "½s¸¹", "¬Ý    ªO", " ¤¤   ¤å   ±Ô   ­z");

    break;

  case RC_FOOT:
    move(b_lines, 0);
    prints("%s  §Úªº³Ì·R  %s                         ¡ö¡ô¡õ¡÷|PgUp|PgDn|Home|End)¾ÉÄý  h)»¡©ú \033[m",
      COLOR2, COLOR3);
    break;
  }
}


static int
favor_cmp(const void * arg1, const void * arg2)
{
  int type = 0;
  PAL *fhdr = (PAL *)arg1, *tmp= (PAL *)arg2;

  if (favor_cmpflag)		/* ¨Ì·ÓÃþ§O±Æ§Ç */
  {
    type = (tmp->ftype - fhdr->ftype) * 256;

    if(fhdr->ftype & tmp->ftype)
    {
      boardheader *fvrp1 = B_FVR(favor_getbnum(fhdr->owner) - 1);
      boardheader *fvrp2 = B_FVR(favor_getbnum(tmp->owner) - 1);

      type += strncmp(fvrp1->title, fvrp2->title, 4);
    }
    type *= 256;
  }

  type += strcasecmp(fhdr->owner, tmp->owner);	/* §_«h«ö·Óª©¦W±Æ§Ç */
  return type;
}


static int
favor_sort(int ent, PAL * fhdr, char *direct)
{
  int num;
  PAL *headers;

  favor_cmpflag = ~favor_cmpflag;

  num = rec_num(direct, sizeof(PAL));
  headers = (PAL *) calloc(num, sizeof(PAL));
  get_records(direct, headers, sizeof(PAL), 1, num);

  qsort(headers, num, sizeof(PAL), favor_cmp);
  substitute_record(direct, headers, sizeof(PAL) * num, 1);
  free(headers);

  return RC_NEWDIR;
}

static int
favor_check(void)
{
  /* §Úªº³Ì·R¤W­­ÀË¬d */
  if (strstr(currdirect, str_dotdir))
  {
    if (rec_num(currdirect, sizeof(PAL)) >= MAX_FAVOR)
    {
      pressanykey("§Úªº³Ì·R¶W¹L¤W­­¡I");
      return 1;
    }
  }

  return 0;
}


static int
favor_add_grp(int ent, PAL * fhdr, char *direct)
{
  PAL fvr;
  char fpath[PATHLEN];

  if (favor_check())
    return RC_FULL;

  memset(&fvr, 0, sizeof(PAL));

  setdirpath(fpath, direct, "");
  stampdir(fpath, (fileheader *) & fvr);

  if (!getdata(1, 0, "[·s¼W¥Ø¿ý] ½Ð¿é¤J¼ÐÃD¡G", fvr.owner, 13, DOECHO, 0))
  {
    rmdir(fpath);
    return RC_FULL;
  }
  getdata(1, 0, "¤¤¤å±Ô­z¡G", fvr.desc, 55, DOECHO, 0);

  fvr.ftype = FAVOR_GROUP;

  if (rec_add(direct, &fvr, sizeof(PAL)) == -1)
    pressanykey(err_system);

  return RC_FULL;
}

static int
favor_add_brd(int ent, PAL * fhdr, char *direct)
{
  char brdname[IDLEN + 1];
  PAL fvr;

  if (favor_check())
    return RC_FULL;

  move(1, 0);
  brdcomplete(msg_bid, brdname);

  if (brdname[0] && favor_getbnum(brdname))
  {
    memset(&fvr, 0, sizeof(PAL));
    strcpy(fvr.owner, brdname);
    fvr.ftype = FAVOR_BOARD;

    if (rec_add(direct, &fvr, sizeof(PAL)) == -1)
      pressanykey(err_system);
  }

  return RC_FULL;
}

void
favor_brd_add(char *brdname)
{
  char buf[200];
  PAL fvr;

  sprintf(buf, "½T©w­n¼W¥[¬ÝªO¡G%s ¨ì§Úªº³Ì·R¡H", brdname);

  if (getans2(1, 0, buf, 0, 2, 'y') != 'n')
  {
    memset(&fvr, 0, sizeof(PAL));
    strcpy(fvr.owner, brdname);
    fvr.ftype = FAVOR_BOARD;

    sethomefile(buf, cuser.userid, FN_FAVOR);
    if (!dashd(buf))
      mkdir(buf, 0755);

    setadir(buf, buf);

    if (rec_num(buf, sizeof(PAL)) >= MAX_FAVOR)
    {
      pressanykey("§Úªº³Ì·R¶W¹L¤W­­¡I");
      return;
    }

    if (rec_add(buf, &fvr, sizeof(PAL)) == -1)
      pressanykey(err_system);

    pressanykey("¬ÝªO¡G%s ¤w¸g¥[¤J¡I", brdname);
  }
}


static int
favor_del(int ent, PAL * fhdr, char *direct)
{
  char genbuf[PATHLEN];

  if (getans2(1, 0, msg_sure, 0, 2, 'n') == 'y')
  {
    if (fhdr->ftype & FAVOR_GROUP)
    {
      setdirpath(genbuf, direct, fhdr->userid);
      f_rm(genbuf);
    }
    strcpy(genbuf, strrchr(currdirect, '/') + 1);
    strcpy(currfile, fhdr->userid);
    delete_file(direct, sizeof(PAL), ent, cmpfilename);
  }

  return RC_NEWDIR;
}


static void
check_zapbuf(char *bname, int flag)
{
  int bid;
  extern int brc_list[];
  extern int brc_num;
  extern int brc_changed;
  extern int *zapbuf;

  brc_initial(bname);
  bid = favor_getbnum(bname);

  if (flag)
    zapbuf[bid] = brc_list[0] = 1;
  else
    zapbuf[bid] = time((time_t *) & brc_list[0]);

  brc_num = brc_changed = 1;
  brc_update();
}


static int
favor_zap(int ent, PAL * fhdr, char *direct)
{
  if (fhdr->ftype & FAVOR_GROUP)
    return RC_NONE;

  check_zapbuf(fhdr->owner, 0);
  return RC_NEWDIR;
}


static int
favor_unzap(int ent, PAL * fhdr, char *direct)
{
  if (fhdr->ftype & FAVOR_GROUP)
    return RC_NONE;

  check_zapbuf(fhdr->owner, 1);
  return RC_NEWDIR;
}


static int
favor_newtitle(int ent, PAL * fhdr, char *direct)
{
  char buf[32];

  if (fhdr->ftype & FAVOR_GROUP)
  {
    if (getdata(1, 0, "½Ð¿é¤J¼ÐÃD¡G", buf, 13, DOECHO, fhdr->owner))
    {
      strncpy(fhdr->owner, buf, 13);
      getdata(1, 0, "¤¤¤å±Ô­z¡G", fhdr->desc, 55, DOECHO, fhdr->desc);
      substitute_record(direct, fhdr, sizeof(PAL), ent);
    }
    return RC_FULL;
  }
  return RC_NONE;
}


static void
favor_copy(int ent, PAL * fhdr, char *direct)
{
  if (!(fhdr->ftype & (FAVOR_BOARD | FAVOR_GROUP)))
    return;

  if (fhdr->ftype & FAVOR_GROUP)
  {
    setdirpath(favorpath, direct, fhdr->userid);
    strcpy(favordesc, fhdr->desc);
  }

  strcpy(favorowner, fhdr->owner);
  favortype = fhdr->ftype;

  outz("ÀÉ®×¼Ð°O§¹¦¨¡C[ª`·N] «þ¨©«á¤~¯à§R°£­ì¤å!");
}


static int
favor_paste(int ent, PAL * fhdr, char *direct)
{
  char fpath[PATHLEN];
  char buf[PATHLEN * 2];
  int i;
  PAL fvr;

  if (favor_check())
    return RC_FULL;

  if (favorowner[0])
  {
    sprintf(buf, "½T©w­n«þ¨© [%s] ¶Ü¡H", favorowner);

    if (getans2(1, 0, buf, 0, 2, 'n') == 'y')
    {
      memset(&fvr, 0, sizeof(PAL));

      if (favortype & FAVOR_GROUP)
      {
	if (dashd(favorpath))
	{
	  for (i = 0; favorpath[i] && favorpath[i] == direct[i]; i++);

	  if (!favorpath[i])
	  {
	    pressanykey("±N¥Ø¿ý«þ¶i¦Û¤vªº¤l¥Ø¿ý¤¤¡A·|³y¦¨µL½a°j°é¡I");
	    return RC_NONE;
	  }
	}
	else
	{
	  pressanykey("µLªk«þ¨©¡I");
	  return RC_NONE;
	}

	setdirpath(fpath, direct, "");

	stampdir(fpath, (fileheader *) & fvr);
	/*
	 * sby: fvr ­n¦b³o¤§«á¤~§ó·s ¤£µM´N³Q stampdir() »\±¼¤F =.=
	 */
	sprintf(buf, "/bin/cp -r %s/* %s/.D* %s", favorpath, favorpath, fpath);
	system(buf);

	strcpy(fvr.desc, favordesc);
      }

      fvr.ftype = favortype;
      strcpy(fvr.owner, favorowner);

      if (rec_add(direct, &fvr, sizeof(PAL)) == -1)
	pressanykey(err_system);

      favorpath[0] = favordesc[0] = favorowner[0] = favortype = '\0';
    }
  }
  else
    pressanykey("½Ð¥ý°õ¦æ copy ©R¥O«á¦A paste¡I");

  return RC_FULL;
}


static int
favor_query(int ent, PAL * fhdr, char *direct)
{
  if (fhdr->ftype & FAVOR_BOARD && HAS_PERM(PERM_BASIC))
  {
    DL_func("SO/admin.so:va_m_mod_board", fhdr->owner);
    return RC_FULL;
  }

  return RC_NONE;
}


static int
favor_ch_flag()
{
  favor_newflag = ~favor_newflag;
  return RC_FULL;
}


static int
favor_desc(int ent, PAL * fhdr, char *direct)
{
  int bid, flag = ent > p_lines / 2 ? 1 : 0;

  if ((bid = favor_getbnum(fhdr->owner) - 1) < 0)
    return RC_NONE;

  if (flag)
  {
    clrchyiuan(3, 7);
    move(3, 0);
  }
  else
  {
    clrchyiuan(b_lines - 4, b_lines - 1);
    move(b_lines - 4, 0);
    prints("%s", msg_seperator);
    move(b_lines - 4, 2);
    outs("¬ÝªO»¡©ú");
    move(b_lines - 3, 0);
  }

  prints("%-80.80s\n%-80.80s\n%-80.80s\n",
    B_FVR(bid)->desc[0], B_FVR(bid)->desc[1], B_FVR(bid)->desc[2]);

  if (flag)
  {
    prints("%s", msg_seperator);
    move(6, 2);
    outs("¬ÝªO»¡©ú");
  }

  pressanykey(NULL);
  return RC_FULL;
}


static void
favordoent(int num, PAL * ent, int row, char *bar_color)
{
  static char *color[7] = {"[1;31m", "[1;35m", "[1;33m", "[1;30m", "[1;37m", "[1;36m", "[1;32m"};
  static char *unread[2] = {"  ", "[1;32m¡°\033[m"};
  char nusers[35];
  move(row, 0);
  clrtoeol();

  /* ¨Ï¥ÎªÌ¦Û©w¸s²Õ */
  if (ent->ftype & FAVOR_GROUP)
  {
    if (favor_newflag)
      prints("%6s", "");
    else
      prints("%6d", num);

    prints(" [0;33m£U%s%-12s[m [1;34m¸s²Õ [m¡° %-33.33s[m    %-12.12s",
      (bar_color) ? bar_color : "",
      ent->owner, ent->desc, "[¥Ø¿ý]");
  }
  else	
  {  /* §Úªº³Ì·R¬ÝªO */
    int bid; 
    boardheader *fvrp = NULL; 
    boardstat bstat;

    /* ÀË¬d¬ÝªO¬O§_¦s¦b*/ 
    if ((bid = getbnum(ent->owner)) == 0)
    {
      prints("  -----  %s%-12s\033[m %55s",
        (bar_color) ? bar_color : "",
        ent->owner, "<------ [1;37m[41m¦¹¬ÝªO¤w³Q§R°£©Î±z¨S¦³Åv­­Æ[¬Ý¡I[m");
      return;
    }

    fvrp = &bcache[bid-1];
    bstat.pos = bid-1;
    check_newpost(&bstat);

    if (favor_newflag)
    {
      prints((fvrp->brdattr & BRD_GROUPBOARD
             || fvrp->brdattr & BRD_CLASS) ? "      " : "%6d",
            brdshm->total[bid-1]);
    }
    else
      prints("%6d", num);

    prints("%c%s",
      !(fvrp->brdattr & BRD_HIDE) ? ' ' :
      (fvrp->brdattr & BRD_POSTMASK) ? ')' : '-',
      (fvrp->brdattr & BRD_GROUPBOARD) ? "[1;34m£U" :
      (fvrp->brdattr & BRD_CLASS) ? "[1;36m¡¼" :
      unread[(int) bstat.unread]);

    prints("%s%-12s[m %s%-5.5s[m%-35.35s[m",
      (bar_color) ? bar_color : "", ent->owner,
      color[(unsigned int)(fvrp->title[1] + fvrp->title[2] +
	  fvrp->title[3] + fvrp->title[0]) % 7],
      fvrp->title, fvrp->title + 5);

    prints("%s%s%-12.12s",           
           (fvrp->bvote == 1 ? "[1;33m¦³[m" : 
            fvrp->bvote == 2 ? "[1;37m¶}[m" : "  "),
            nusers_printer(nusers, bid-1), 
            fvrp->BM);
  }
}

static int favor_view(int, PAL *, char *);
extern int New();

static struct one_key favor_comm[] = {
  'a', favor_add_brd, 	0, "·s¼W¬ÝªO", 0,
  'r', favor_view, 	0, "¶i¤J/ÀËµø¬ÝªO", 0,
  'd', favor_del, 	0, "§R°£¬ÝªO", 0,
  'c', favor_ch_flag, 	0, "·s¤å³¹¼Ò¦¡", 0,
  'b', favor_desc, 	0, "¬ÝªO»¡©ú", 0,
  'Q', favor_query, 	0, "¬d¸ß¬ÝªO", 0,
  'g', favor_add_grp, 	0, "·s¼W¸s²Õ", 0,
  'T', favor_newtitle, 	0, "­×§ï¸s²Õ¦WºÙ/±Ô­z", 0,
  'S', favor_sort, 	0, "±Æ§Ç", 0,
  'v', favor_zap, 	0, "¼Ð°O¤wÅª", 0,
  'V', favor_unzap, 	0, "¼Ð°O¥¼Åª", 0,
  'm', list_move, 	0, "²¾°Ê¦ì¸m", 0,
  'y', New, 		0, "¯¸¤º©Ò¦³¬ÝªO", 0,
Ctrl('C'), favor_copy, 	0, "½Æ»s", 0,
Ctrl('P'), favor_paste, 0, "¶K¤W", 0,
  '\0', NULL, 0, NULL, 0};

static int
favor_view(int ent, PAL * fhdr, char *direct)
{
  int bid;
  char buf[PATHLEN];
  boardheader *fvrp = NULL;

  /* ¨Ï¥ÎªÌ¦Û©w¸s²Õ */
  if (fhdr->ftype & FAVOR_GROUP)
  {
    setdirpath(buf, direct, fhdr->userid);
    if (!dashd(buf))
      mkdir(buf, 0755);

    setadir(buf, buf);
    i_read(FAVORBRD, buf, favortitle, favordoent, favor_comm, 'a');

    return RC_FULL;
  }

  /* ÀË¬d¬ÝªO¬O§_¦s¦b */
  if ((bid = getbnum(fhdr->owner)) == 0)
  {
    pressanykey("¬ÝªOÅª¨ú¿ù»~¡A±zµLÅv­­¶i¤J¦¹¬ÝªO¡A©Î¦¹¬ÝªO¤w³Q§R°£¡I");
    return RC_FOOT;
  }

  fvrp = B_FVR(bid-1);

  /* ¤ÀÃþ¬ÝªO©Î¤l¿ï³æ */
  if (fvrp->brdattr & (BRD_GROUPBOARD | BRD_CLASS))
  {
    if (!strcmp(fhdr->owner, PERSONAL_ALL_BRD))
      choose_board(HAS_HABIT(HABIT_BOARDLIST), BRD_PERSONAL);

    else if (!strcmp(fhdr->owner, GROUP_ALL_BRD))
      choose_board(HAS_HABIT(HABIT_BOARDLIST), BRD_GROUP);

    else if (!strcmp(fhdr->owner, HIDE_ALL_BRD))
      choose_board(HAS_HABIT(HABIT_BOARDLIST), BRD_HIDE);

    else
    {
      int currmode0 = currmode;
      extern char *boardprefix;

      set_menu_BM(fvrp->BM);
      log_board3("USE", fvrp->brdname, 0);
      strlcpy(groupbrd, fvrp->brdname, sizeof(groupbrd));

      boardprefix = fvrp->title + 7;
      choose_board(HAS_HABIT(HABIT_BOARDLIST), 0);
      
      currmode = currmode0;
      groupbrd[0] = '\0';
    }
  }
  else  
  {
    /* ¾\Åª¦¹¬ÝªO */
    brc_initial(fhdr->owner);
    Read();
  }

  return RC_FULL;
}


int
Favor()
{
  char buf[PATHLEN];
  char groupbrd0[IDLEN +1];
  int currmode0=currmode;
 
  strlcpy(groupbrd0, groupbrd, sizeof(groupbrd0));
  groupbrd[0]= '\0';
  currmode &= ~MODE_MENU;
  
  sethomefile(buf, cuser.userid, FN_FAVOR);

  favor_newflag = HAS_HABIT(HABIT_BOARDLIST);

  if (!dashd(buf))
    mkdir(buf, 0755);

  setadir(buf, buf);
  i_read(FAVORBRD, buf, favortitle, favordoent, favor_comm, 'a');
  
  strlcpy(groupbrd, groupbrd0, sizeof(groupbrd));
  currmode = currmode0;
  return 0;
}
