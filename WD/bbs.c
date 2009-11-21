/*-------------------------------------------------------*/
/* bbs.c        ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : bulletin boards' routines                    */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

#define WORLDSNUM   10   /*�峹�n�X�Ӧr�~��*/

extern int mail_reply ();
extern int TagNum;
extern void Read();
char *str_ttl(char *title);
boardheader *getbcache(char *);

time_t board_note_time;
time_t board_visit_time;

static char *brd_title;
char real_name[20];
int local_article;
char currmaildir[32];

char *rcolor[11] = { "\033[32m",   "\033[36m",   "\033[32m",   "\033[1;32m",
                     "\033[33m",   "\033[1;33m", "\033[1;37m" ,"\033[1;37m",
                     "\033[1;31m", "\033[1;35m", "\033[1;36m"};

#define UPDATE_USEREC   (currmode |= MODE_DIRTY)

void
log_board (char *board, time_t usetime)
{
  time_t now;
  boardheader bh;
  int bid = getbnum (board);

  now = time (0);
  rec_get (fn_board, &bh, sizeof (bh), bid);
  if (usetime >= 10)
  {
    ++bh.totalvisit;
    bh.totaltime += usetime;
    strcpy (bh.lastvisit, cuser.userid);
    bh.lastime = now;
  }
  substitute_record (fn_board, &bh, sizeof (bh), bid);
}

void
log_board3(char *mode, char *str, int num)
{
    time_t      now;
    FILE        *fp;
    char        buf[256];

    now = time(0);
    sprintf( buf, "%3s %-20.20s with: %5ld (%s) %s",
      mode, str, num ,cuser.userid,ctime(&now));

    if( (fp = fopen("usboard", "a" )) != NULL ) 
    {
        fputs( buf, fp );
        fclose( fp );
    }
}

static int
g_bm_names(bh)
  boardheader *bh;
{
  char buf[IDLEN * 3 + 3];
  char* uid;

  strcpy(buf, bh->BM);
  uid = strtok(buf, "/");       /* shakalaca.990721: ��Ĥ@�ӪO�D */
  while (uid)
  {
    if (!InNameList(uid) && searchuser(uid))
      AddNameList(uid);
    uid = strtok(0, "/");       /* shakalaca.990721: ���V�U�@�� */
  }
  return 0;
}

/* shakalaca.990721: �Ҧ� BM �W�� */
void
make_bmlist()
{
  CreateNameList();
  apply_boards(g_bm_names);
}


void
set_board ()
{
  boardheader *bp;

  bp = getbcache (currboard);
  board_note_time = bp->bupdate;
  brd_title = bp->BM;
  if (brd_title[0] <= ' ')
    brd_title = "�x�D��";

  sprintf (currBM, "�O�D�G%.22s", brd_title);
  brd_title = (bp->bvote == 1 ? "���ݪO�i��벼��" : bp->title + 7);

  currmode = (currmode & MODE_DIRTY) | MODE_STARTED;
  if (HAS_PERM (PERM_ALLBOARD) ||
      (HAS_PERM (PERM_BM) && userid_is_BM (cuser.userid, currBM + 6)))
  {
      currmode |= (MODE_BOARD | MODE_POST);
  }
  else if (haspostperm (currboard))
    currmode |= MODE_POST;
}


static void 
readtitle (int tag)
{
  switch(tag)
  {
    case RC_FULL:
      showtitle (currBM, brd_title);
      move(1, 0);
      clrtoeol();
  
      prints("%s\033[30m��\033[37m�ݪO�@�̷R��%s�峹%s\033[37m����ذ�\033[36;40m��\033[m",
    	       COLOR1, COLOR3, COLOR1);
    	       
      outs("               ^P)�o��  u)�t�C  Tab)��K  z)��ذ�");

      move(2, 0);  
      prints("%s  �s�� ", COLOR3);

      if (currmode & MODE_TINLIKE)
        outs ("SC�g ��");
      else
        outs ("SC�� ��");
      prints (" �@  ��       ��  ��  ��  �D           �ݪO�`��\033[m\033[32m[%-14.14s]%s  \033[m", get_feast(currboard), COLOR3);
      break;

    case RC_FOOT:
      move(b_lines, 0);
      clrtoeol();
      prints("%s  �峹��Ū  %s       =[]<>)�D�D���\\Ū  ��������|PgUp|PgDn|Home|End)����  h)���� \033[m",
              COLOR2, COLOR3);
      break;
  }
}

void 
readfoot(char tag)
{
  move(b_lines, 0);
  clrtoeol();    
  switch(tag)
  {
    case 1:  /* �峹 */
    case 2:  /* �H�� */
      if ((currstat == RMAIL) || (currstat == READING))
      {
        prints("%s  %s  %s       =[]<>)�D�D���\\Ū  ��������|PgUp|PgDn|Home|End)����  h)���� \033[m",
                COLOR2, (tag==2) ? "�H��C��" : "�峹��Ū", COLOR3);
      }
    break;
  }
}

void 
doent (int num, fileheader *ent, int row, char *bar_color)
{
  user_info *checkowner;
  char *mark, *title, color, type[10], buf[255], snum[50], botm_color[50];
  static char *colors[7] =
  {"[1;36m", "[1;34m", "[1;33m", "[1;32m", "[1;35m", "[1;36m", "[1;37m"};
  
  /*wildcat �峹����*/

  if(currstat == RMAIL && ent->filemode & FILE_REPLYOK)
  { 
    /*hialan:�P�_'R'���a��, �]��RMAIL�S�Ψ���� */
    sprintf(buf , "\033[1;31mR %s",
      colors[(unsigned int) (ent->date[4] + ent->date[5]) % 7]);
  }
  else if(ent->score != 0 || ent->filemode & FILE_SCORED)
  {
    char score[3];
    
    if((ent->score < 99 && ent->score > -99) || ent->filemode & FILE_SCORED)
      sprintf(score, "%02d", (ent->score < 0) ? ent->score * -1 : ent->score);
    else
      sprintf(score, "%2.2s", (ent->score < 0) ? "�@" : "��");
      
    sprintf(buf , "%s%s%s",
      ent->score < 0 ? "\033[1;32m" : "\033[1;31m", score,
      colors[(unsigned int) (ent->date[4] + ent->date[5]) % 7]);
  }
  else
    sprintf(buf , "  %s",
      colors[(unsigned int) (ent->date[4] + ent->date[5]) % 7]);

  if (currstat != RMAIL)
  {
    sprintf(type,"%c",brc_unread (ent->filename) ? '+' : ' ');

    if ((currmode & MODE_BOARD) && (ent->filemode & FILE_DIGEST))
      sprintf(type ,"[1;35m%c",(type[0] == ' ') ? '*' : '#');
    if (ent->filemode & FILE_MARKED)
      sprintf(type ,"[1;36m%c",(type[0] == ' ') ? 'm' : 'M');
    if (ent->filemode & FILE_REFUSE)  /* �[�K���峹�e���X�{ X �r�� */
      sprintf(type ,"[1;31m%c",(type[0] == ' ') ? 'x' : 'X');
  }
  else
  {
    usint tmpmode = ent->filemode;
    if (ent->filemode & FILE_REPLYOK)	/* �]���[�F Reply ���ȴN���O���ӼƦr�F */
      tmpmode ^= FILE_REPLYOK;
    sprintf(type,"%c","+ Mm"[tmpmode]); 
  }

  if (ent->filemode & FILE_TAGED)
    sprintf(type,"[1;32m%c",(type[0] == ' ') ? 't' : 'T');
  
  if (ent->filemode & FILE_BOTTOM)
  {
    title = brdshm->bcache[currutmp->brc_id - 1].botm_color;
    get_color(botm_color, title);

    title = brdshm->bcache[currutmp->brc_id - 1].bottom;
    
    sprintf(snum, "  %s%s\033[m", botm_color,
    	    (strlen(title) < 5 && title[0]) ? title : "���i");
  }
  else
    sprintf(snum, "%6d", num);

  title = str_ttl (mark = ent->title);
  if (title == mark)
  {
    color = '6';
    mark = "��";
  }
  else
  {
    color = '3';
    mark = "R:";
  }

  if (title[44])
    strcpy (title + 44, " �K");  /* ��h�l�� string �屼 */

  checkowner =(user_info *) searchowner(ent->owner);

  move(row, 0);
  clrtoeol();
  if (strncmp (currtitle, title, 40))
    prints ("%s%s%s%-6s[m%s%s%-12.12s[m %s %s\n", 
      snum, type, buf, ent->date,
      checkowner ? rcolor[is_friend(checkowner)] : "",
      (bar_color) ? bar_color : "", ent->owner,
      mark, title);
  else
    prints ("%s%s%s%-6s[m%s%s%-12.12s[m [1;3%cm%s %s[m\n",
      snum, type, buf, ent->date,
      checkowner ? rcolor[is_friend(checkowner)] : "",
      (bar_color) ? bar_color : "", ent ->owner,
      color, mark, title);
}


int
cmpbnames (bname, brec)
     char *bname;
     boardheader *brec;
{
  return (!strncasecmp (bname, brec->brdname, sizeof (brec->brdname)));
}


int
cmpfilename (fileheader *fhdr)
{
  return (!strcmp (fhdr->filename, currfile));
}

int
cmpfmode (fileheader *fhdr)
{
  return (fhdr->filemode & currfmode);
}


int
cmpfowner (fileheader *fhdr)
{
  return !strcasecmp (fhdr->owner, currowner);
}

int
do_select(int ent, fileheader *fhdr, char *direct)
{
  char bname[20];
  char bpath[60];
  boardheader *bhdr;

  move (0, 0);
  clrtoeol ();
  brdcomplete (MSG_SELECT_BOARD, bname);

  setbpath (bpath, bname);
  if ((*bname == '\0') || (bhdr = getbcache(bname)) == NULL)
  {
    move (2, 0);
    clrtoeol ();
    pressanykey (err_bid);
    return RC_FULL;
  }

  if (Ben_Perm (bhdr) != 1) 
  {
    pressanykey (P_BOARD);
    return RC_FULL;
  }
  
  if(bhdr->brdattr & BRD_CLASS || bhdr->brdattr & BRD_GROUPBOARD)
    return RC_FULL;
  
  brc_initial (bname);  
  set_board ();
  
  /* �p�G�o�Ө禡�O�b i_read() ���~�I�s���ܡA�h direct ���V NULL */
  if(direct != NULL)
    setbdir(direct, currboard);

  move (1, 0);
  clrtoeol ();
  return RC_NEWDIR;
}
/* ----------------------------------------------------- */
/* ��} innbbsd ��X�H��B�s�u��H���B�z�{��             */
/* ----------------------------------------------------- */
void 
outgo_post(fileheader *fh, char *board)
{
  char buf[256];
  if(strcmp(fh->owner,cuser.userid))
    sprintf (buf, "%s\t%s\t%s\t%s\t%s", board,
      fh->filename, fh->owner, "���", fh->title);
  else
    sprintf (buf, "%s\t%s\t%s\t%s\t%s", board,
      fh->filename, fh->owner, cuser.username, fh->title);
  f_cat ("innd/out.bntp", buf);
}


static void
cancelpost (fileheader *fh, int by_BM)
{
  FILE *fin;
  char *ptr, *brd;
  fileheader postfile;
  char genbuf[256], buf[256];
  char nick[STRLEN], fn1[PATHLEN], fn2[PATHLEN];

  setbfile (fn1, currboard, fh->filename);
  if (fin = fopen (fn1, "r"))
  {
    brd = by_BM ? "deleted" : "junk";
    setbpath (fn2, brd);
    stampfile (fn2, &postfile);
    memcpy (postfile.owner, fh->owner, IDLEN + TTLEN + 10);
    postfile.savemode = 'D';
    log_board3("DEL", currboard, 1);
    if (fh->savemode == 'S')
    {
      nick[0] = '\0';
      while (fgets (genbuf, sizeof (genbuf), fin))
      {
        if (!strncmp (genbuf, str_author1, LEN_AUTHOR1) ||
            !strncmp (genbuf, str_author2, LEN_AUTHOR2))
        {
          if (ptr = strrchr (genbuf, ')'))
          *ptr = '\0';
          if (ptr = (char *) strchr (genbuf, '('))
          strcpy (nick, ptr + 1);
          break;
        }
      }

      sprintf (buf, "%s\t%s\t%s\t%s\t%s",
        currboard, fh->filename, fh->owner, nick, fh->title);
      f_cat ("innd/cancel.bntp", buf);
    }
    fclose (fin);
    f_mv (fn1, fn2);
    setbdir (genbuf, brd);
    rec_add (genbuf, &postfile, sizeof (postfile));
    
    setbtotal(getbnum(brd));
  }
}


/* ----------------------------------------------------- */
/* �o��B�^���B�s��B����峹                            */
/* ----------------------------------------------------- */

void 
do_reply_title (int row, char *title)
{
  char genbuf[128];

  if (strncasecmp (title, str_reply, 4))
    sprintf (save_title, "Re: %s", title);
  else
    strcpy (save_title, title);
  save_title[TTLEN - 1] = '\0';
  sprintf (genbuf, "�ĥέ���D�m\033[1;46m %.50s \033[m�n�ܡH", save_title);
  if (getans2(row, 0, genbuf, 0, 2,'y') == 'n')
    getdata (++row, 0, "���D�G", save_title, TTLEN, DOECHO, 0);
}


static void
do_reply (fileheader *fhdr)
{
  char genbuf;

// Ptt �ݪO�s�p�t��
  if(!strcmp(currboard,VOTEBOARD))
//    do_voteboardreply(fhdr);
    DL_func("SO/votebrd.so:va_do_voteboardreply",fhdr);
  else
  {
    char *choose[4]={"fF)�ݪO","mM)�@�̫H�c","bB)��̬ҬO", msg_choose_cancel};
      
    genbuf = getans2(b_lines - 1, 0,"�� �^���� ",choose,4,'f');
    switch (genbuf)
    {
      case 'm':
        mail_reply (0, fhdr, 0);
      case 'q':
        *quote_file = 0;
        break;

      case 'b':
        curredit = EDIT_BOTH;
      default:
        strcpy (currtitle, fhdr->title);
        strcpy (quote_user, fhdr->owner);
        quote_file[79] = fhdr->savemode;
        do_post ();
    }
  }
  *quote_file = 0;
}

/* �ƻs�峹��ݪO */
int
do_copy_post (char *board, char *fpath, uschar filemode)
{
  fileheader mhdr;
  char title[128];
  char genbuf[128];

  setbpath (genbuf, board);
  if (dashd (genbuf))
  {
    stampfile (genbuf, &mhdr);
    unlink (genbuf);
    f_ln (fpath, genbuf);
    strcpy (mhdr.owner, cuser.userid);
    strcpy (mhdr.title, save_title);
    mhdr.savemode = 0;
    mhdr.filemode = filemode;
    setbdir (title, board);
    rec_add (title, &mhdr, sizeof (mhdr));
    
    setbtotal(getbnum(board));
  }
  return 0;
}

#ifdef HAVE_ALLPOST
static void 
post_allpost(char *fpath, fileheader *fhdr)
{
  char save_title0[TTLEN+1];
  const int pos = getbnum(currboard);
  boardheader *bhdr = &brdshm->bcache[pos-1];

  if(bhdr->brdattr & BRD_HIDE || bhdr->level > PERM_SEECLOAK || hbflcount(pos, M_BAD) || strchr(real_name, '.'))
    return;

  strcpy(save_title0, save_title);
        
  if(strncmp("Re: ", save_title0, 4))
    sprintf(save_title, "%-30.30s [%d]", save_title0, atoi(fhdr->filename+2));
  else
    sprintf(save_title, "%-34.34s [%d]", save_title0, atoi(fhdr->filename+2));

  outmsg("����� All_Post ...");
  do_copy_post("All_Post", fpath, 0); /* �����Ҧ��������K��(�@��) */
  strcpy(save_title, save_title0);
}

static int 
change_allpost(fileheader *fhdr, char *fname, char *newtitle)
{
  char genbuf[PATHLEN];
  int fd, size = sizeof(fileheader), now;
  fileheader fhdr2;
  char sametitle[TTLEN + 1];
    
  setbdir(genbuf, "All_Post");

  if(strncmp("Re: ", fhdr->title, 4))
    sprintf(sametitle, "%-30.30s [%d]", fhdr->title, atoi(fhdr->filename+2));
  else
    sprintf(sametitle, "%-34.34s [%d]", fhdr->title, atoi(fhdr->filename+2));
    
  if ((fd = open (genbuf, O_RDONLY, 0)) != -1)
  {
    now = 0;
    while ((read (fd, &fhdr2, size) == size))
    {
      now++;
      if (!strcmp (fhdr2.title, sametitle))
      {
        close(fd);
        fhdr2.filemode = (fhdr->filemode & FILE_REFUSE) ? FILE_REFUSE : 0;

        if(strncmp("Re: ", newtitle, 4))
          sprintf(fhdr2.title, "%-30.30s [%d]", newtitle, atoi(fname+2));
        else
          sprintf(fhdr2.title, "%-34.34s [%d]", newtitle, atoi(fname+2));
        
        substitute_record(genbuf, &fhdr2, sizeof(fhdr2), now);
        return 0;
      }
    }
    close (fd);
  }
  return -1;
}
#endif

/* Ptt test */
int
getindex (char *fpath, char *fname, int size)
{
  int fd, now = 0;
  fileheader fhdr;

  if ((fd = open (fpath, O_RDONLY, 0)) != -1)
  {
    while ((read (fd, &fhdr, size) == size))
    {
      now++;
      if (!strcmp (fhdr.filename, fname))
      {
        close (fd);
        return now;
      }
    }
    close (fd);
  }

  return 0;
}

/* ----------------------------------------------------- */
/* �峹�ۭq���O                                          */
/* ----------------------------------------------------- */

#define PREFIXLEN 50	/* �峹���O�̤j���� */

static char postprefix[10][PREFIXLEN];

static int 
b_load_class(char *bname)
{
      FILE *prefixfile;
      char chartemp[PREFIXLEN],buf[PATHLEN];
      static char class_now[IDLEN + 1]=""; /* �{�b���x�s���O���@�ӬݪO�����O */
       
      if(!strcmp(bname, class_now))
        return 0;

      strcpy(class_now, bname);
      setbfile (buf, bname, FN_POSTPREFIX);
      
      if((prefixfile = fopen(buf,"r")) != NULL)     
      {
        int i, j;

        for(i=0;i<9;i++)
        {
	  fgets(chartemp, sizeof(chartemp), prefixfile);
	  
	  for(j=0;j<PREFIXLEN;j++)
	    if(chartemp[j] == '\n')
	    {
	      chartemp[j] = '\0';
	      break;
	    }
	  
          sprintf(postprefix[i], "%d%d)%s", i+1, i+1, chartemp);
	}
        fclose(prefixfile);
      }
      else
      {
        strcpy(postprefix[0],"11)[���i]");
        strcpy(postprefix[1],"22)[�s�D]");
        strcpy(postprefix[2],"33)[����]");
        strcpy(postprefix[3],"44)[���]");
        strcpy(postprefix[4],"55)[���D]");
        strcpy(postprefix[5],"66)[�Ч@]");
        strcpy(postprefix[6],"77)[�H�K]");
        strcpy(postprefix[7],"88)[����]");
        strcpy(postprefix[8],"99)[��L]");
      }
      return 0;
}

void
prefix_edit()
{
  int i, ch2;
  FILE *prefixfile;
  char buf[50];
  char class[11][50];
  char *classpoint[11];

  /*�峹���O by hialan 3.21.2002*/
  b_load_class(currboard);

  /*�ƻs����а}�C*/
  for(i = 0;i < 11;i++)
    classpoint[i] = class[i];

  for(i = 0;i<9;i++)
    strcpy(class[i], postprefix[i]);

  sprintf(class[9],"dD)�^�_�w�]��");
  sprintf(class[10],"qQ)���}");

  ch2 = '1';

  do
  {
    ch2 = win_select("�ק�峹���O", "�п�ܭn�ק諸���O ", classpoint, 11, ch2);

    if(ch2 == 'd')
    {
      if(win_select("�ק�峹���O", "�T�w�n�^�йw�]�ȶ�? ", 0, 2, 'n')== 'y')
      {
        setbfile (buf, currboard, FN_POSTPREFIX);
        unlink(buf);
        return;
      }
      else
        continue;
    }

    if(ch2 != 'q')
    {
      getdata(b_lines-1, 0, "�п�J�s���O", buf, 21, DOECHO, postprefix[ch2-'1']+3);
      if(*buf != '\0')  
      {
        /*�p�G�ϥΪ̨S��J�F��,�h���}*/
        strcpy(postprefix[ch2 - '1']+3, buf);
        strcpy(class[ch2 - '1'], postprefix[ch2 - '1']);
      }
      move(b_lines-1, 0);
      clrtobot(b_lines-1,0);
    }
  }while(ch2 != 'q');

  if(win_select("�ק�峹���O", "�T�w�n�ק��? ", 0, 2, 'y')== 'y')
  {
    setbfile (buf, currboard, FN_POSTPREFIX);
    if((prefixfile = fopen(buf,"w")) != NULL)
    {
      for(i=0;i<9;i++)
      fprintf(prefixfile,"%s\n",postprefix[i]+3);
      fclose(prefixfile);
    }
  }
  else
    pressanykey("�峹���O�S������!!");

  return ;
}

extern long wordsnum;    /* �p��r�� */

int
make_cold(char *board, char *save_title, int money, char *fpath)
{
  int cold;

  /*�p��N��*/
  while(1)
  {
    cold = rand() % 10;
    if(cold == 9)
    {
      if((rand() % 10) < 1)
        break;
    }
    else
      break;
  }
  
  if(belong("etc/cold_list", cuser.userid))
    cold = 9;

  return cold;
}


int
do_post ()
{
  fileheader postfile;
  char fpath[80], buf[80], buf1[80], buf2[80];
  int aborted;
  char genbuf[256], *owner;
  boardheader *bp;
  time_t spendtime;
  int i;

  
  bp = getbcache (currboard);
  if (!(currmode & MODE_POST) || !Ben_Perm (bp))
  {
    pressanykey ("�藍�_�A�z�ثe�L�k�b���o��峹�I");
    return RC_NONE;
  }

// Ptt �ݪO�s�p�t��
  if(!strcmp(currboard,VOTEBOARD))
  {
    DL_func("SO/votebrd.so:do_voteboard");
    return RC_FULL;
  }

  setbfile (buf, currboard, FN_LIST);

  if (belong ("etc/have_postcheck", currboard))
  {
    if (!HAS_PERM (PERM_BBSADM) && !hbflcheck(getbnum(currboard), currutmp->uid))
    {
      pressanykey ("�藍�_�A���O�u��ݪO�n�ͤ~��o��峹�A�ЦV�O�D�ӽСI");
      return RC_FULL;
    }
  }

  setbfile(genbuf, currboard, FN_POST_NOTE ); /* ychia.021212:�ۭq�峹�o��n�� */

  if(more(genbuf, YEA) == -1)
#ifdef CAMERA
    film_out(FILM_POST, -1);
#else
    more("etc/post.note", YEA);
#endif

  if (quote_file[0])
    do_reply_title (p_lines, currtitle);
  else
  {
    char *board_class[11];
    char win_title[100];
    
    sprintf(win_title, "�o��峹��i %s �j�ݪO", currboard);
    b_load_class(currboard);

    for(i = 0;i < 9;i++)
      board_class[i] = postprefix[i];

    board_class[9] = "wW)�ۦ��J";
    board_class[10] = msg_choose_cancel;
  
    memset (save_title, 0, TTLEN);
    
    genbuf[0] = win_select(win_title, "�п�ܤ峹���O", board_class, 11, '1');

    move(0, 0);
    i = *genbuf - '0';
    if (i > 0 && i <= 9)  /* ��J�Ʀr�ﶵ */
      strncpy (save_title, board_class[i - 1] + 3, strlen (board_class[i - 1] + 3));
    else if (*genbuf == 'w')  /* �ۦ��J */
    {
      getdata(b_lines - 4, 0, "�п�J�峹���O�G", genbuf, 21, DOECHO, 0);
      sprintf(buf1, "[%s]", genbuf);
      strncpy(save_title, buf1, strlen (buf1));
    }
    else      /* �ťո��L */
      save_title[0] = '\0';
           
    getdata (b_lines -3 , 0, "���D�G", save_title, TTLEN, DOECHO, save_title);
    strip_ansi (save_title, save_title, ONLY_COLOR);
  }
  if (save_title[0] == '\0')
    return RC_FULL;

  curredit &= ~EDIT_MAIL;
  curredit &= ~EDIT_ITEM;
  setutmpmode (POSTING);

  /* ����� Internet �v���̡A�u��b�����o��峹 */

  if (HAS_PERM (PERM_INTERNET))
    local_article = 0;
  else
    local_article = 1;

  buf[0] = 0;

  spendtime = time (0);
  aborted = vedit (buf, YEA);
  spendtime = time (0) - spendtime;
  if (aborted == -1)
  {
    unlink (buf);
    pressanykey (NULL);
    return RC_FULL;
  }

  /* build filename */

  setbpath (fpath, currboard);
  stampfile (fpath, &postfile);
  f_mv (buf, fpath);
  strcpy (postfile.owner, cuser.userid);

  /* set owner to Anonymous , for Anonymous board */

#ifdef HAVE_ANONYMOUS
/* Ptt and Jaky */
  if (currbrdattr & BRD_ANONYMOUS && strcmp (real_name, "r"))
  {
    strcat (real_name, ".");
    owner = real_name;
  }
  else
  {
#endif
    owner = cuser.userid;
#ifdef HAVE_ANONYMOUS
  }
#endif

  strcpy (postfile.owner, owner);
  strcpy (postfile.title, save_title);
  if (aborted == 1)    /* local save */
  {
    postfile.savemode = 'L';
    postfile.filemode = FILE_LOCAL;
  }
  else
    postfile.savemode = 'S';

  setbdir (buf, currboard);
  if (rec_add (buf, &postfile, sizeof (postfile)) != -1)
  {
    int cold;
    
    setbtotal(getbnum(currboard));
    
    if (currmode & MODE_SELECT)
    rec_add (currdirect, &postfile, sizeof (postfile));
    if (aborted != 1)	/* hialan: WD_pure for local save */
      outgo_post (&postfile, currboard);
    brc_addlist (postfile.filename);

    if (!(currbrdattr & BRD_NOCOUNT))
    {
      if (wordsnum <= WORLDSNUM)
        pressanykey ("��p�A�ӵu���峹���C�J�����I");
      else
      {
        int money = (wordsnum <= spendtime ? (wordsnum / 100) :
                    (spendtime / 100));

        if(!HAS_HABIT(HABIT_NOCOLD))
  	  cold = make_cold(currboard, save_title, money, fpath);/*�p��N��*/
  	else
  	  cold = 0;
        
        money *= (float)(((rand () % 5) + 5) / 5);        
        if (money < 1) money = 1;

        if(cold == 9)
          money += 100;

        clear ();
        move (7, 0);
        update_data ();

        prints ("\
              [1;36m�i[37m�p �� �Z �S[36m�j\n\
              [37m �o�O�z��[33m�� %d �g[37m�峹�C\n\
              [36m�i�O  �ɡj[33m %d [37m��[33m % d [37m��C\n\
              [36m�i�Z  �S�j[33m %d [37m(����)",
        ++cuser.numposts, spendtime / 60, spendtime % 60, money);
        
        if(!HAS_HABIT(HABIT_NOCOLD))
        {
          prints("[36m�i�N  �סj[33m %d [37m �I", cold+1);
          if(cold == 9)
  	  {
	    prints ("\n\n[31m                  ���ߧA���S��...�[�e100�T����^^[0m");
	    prints ("\n\n              �z���峹�w�g�Q����� ColdKing �ݪO��!!");
	  }
	}
        substitute_record (fn_passwd, &cuser, sizeof (userec), usernum);

        ingold (money);  /* post��o���� by wildcat */

        if (money >= 100 || spendtime <=3)
        {
          FILE * fp;
          time_t now = time (0);
          fileheader mhdr;
          char genbuf1[PATHLEN], fpath1[STRLEN];

          setbpath (genbuf1, "Security");
          stampfile (genbuf1, &mhdr);
          strcpy (mhdr.owner, cuser.userid);
          strncpy (mhdr.title, "POST�ˬd", TTLEN);
          mhdr.savemode = '\0';
          setbdir (fpath1, "Security");
          if (rec_add (fpath1, &mhdr, sizeof (mhdr)) == -1)
          {
            outs (err_uid);
            return 0;
          }
          if ((fp = fopen (genbuf1, "w")) != NULL)
          {
            fprintf (fp, "�@��: %s (%s)\n", cuser.userid, cuser.username);
            fprintf (fp, "���D: %s\n�ɶ�: %s\n", "POST�ˬd", ctime (&now));
            fprintf (fp,
"%s �o��@�g %d �r���峹�� %s �O\n��F %d ��A�o����� %d ��"
              ,cuser.userid, wordsnum, currboard, spendtime, money);
            fclose (fp);
          }
        }
        pressanykey (NULL);
      }
    }
    else
      pressanykey ("���ݪO�峹���C�J�����A�q�Х]�[ :)");

    log_board3("POS", currboard, cuser.numposts);

    if ( !(currbrdattr & BRD_HIDE) && !(currbrdattr & BRD_POSTMASK) && !(currbrdattr & BRD_ANONYMOUS))
    {
      if(cold == 9)
      {
        int type = 0;
        
        sprintf(buf2, "�� ���ߥ��g�峹���N���A�t�Τw�g�۰������ board://ColdKing");
        f_cat(fpath, buf2);
        
        clear();
        outs("\033[1;31m���ߥ��g���N��!!\033[m\n");

        if(getans2(2, 0, "�аݥ��g�O�_�[�K? ", 0, 2, 'n') == 'y')
          type ^= FILE_REFUSE;
        else
          type = 0;

        outmsg("�����N�������W��...");
        do_copy_post("ColdKing", fpath, type);
      }
    }
#ifdef HAVE_ALLPOST
    post_allpost(fpath, &postfile);
#endif

    outmsg("�峹�t�γƬd...");
    do_copy_post("All_Post2", fpath, 0); /* �����Ҧ��������K�� */
  }

  /* �^�����@�̫H�c */
    if (curredit & EDIT_BOTH)
    {
      char *str, *msg = "�^���ܧ@�̫H�c";

      if (str = strchr (quote_user, '.'))
      {
        if (bbs_sendmail (fpath, save_title, str + 1, NULL) < 0)
          msg = "�@�̵L�k���H";
      }
      else
      {
        sethomepath (genbuf, quote_user);
        stampfile (genbuf, &postfile);
        unlink (genbuf);
        f_cp (fpath, genbuf, O_TRUNC);

        strcpy (postfile.owner, cuser.userid);
        strcpy (postfile.title, save_title);
        postfile.savemode = 'B';  /* both-reply flag */
        sethomedir (genbuf, quote_user);
        if (rec_add (genbuf, &postfile, sizeof (postfile)) == -1)
          msg = err_uid;
      }
      outmsg (msg);
      curredit ^= EDIT_BOTH;
    }

  return RC_FULL;
}


static int
reply_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  if (!(currmode & MODE_POST))
    return RC_NONE;

  /* �[�K�L���峹�u���O�D�i�H�^ */
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD) && !HAVE_PERM(PERM_SYSOP))
  {
    pressanykey("���峹�w�Q�[�K�I");
    return RC_NONE;
  }    
  setdirpath (quote_file, direct, fhdr->filename);
  
  /* Ptt ���ݪO�s�p�t�� */
  if(!strcmp(currboard,VOTEBOARD))
    DL_func("SO/votebrd.so:va_do_voteboardreply",fhdr);
  else
    do_reply (fhdr);
    
  *quote_file = 0;
  return RC_FULL;
}


int
edit_post (int ent, fileheader *fhdr, char *direct)
{
  char genbuf[STRLEN];
  int edit_mode;

  if ((!strcmp(currboard,"Security") || !strcmp(currboard,"VoteBoard")))
    return RC_NONE;

  if ( (fhdr->filemode & FILE_BOTTOM) 
       || (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD))
       || (currmode & MODE_TINLIKE) )
  {
    return RC_NONE;
  }

  if (currstat == RMAIL)
  {
    setdirpath (genbuf, direct, fhdr->filename);
    strcpy (save_title, fhdr->title);
    vedit (genbuf, HAS_PERM(PERM_SYSOP) ? 0 : 2);
    return RC_FULL;
  }

  if (HAS_PERM (PERM_SYSOP) ||
    !strcmp (fhdr->owner, cuser.userid) && strcmp (cuser.userid, "guest") &&
    !bad_user_id (cuser.userid))
    edit_mode = 0;
  else
    edit_mode = 2;


  setdirpath (genbuf, direct, fhdr->filename);
  local_article = fhdr->filemode & FILE_LOCAL;
  strcpy (save_title, fhdr->title);

  if (vedit (genbuf, edit_mode) != -1)
  {
    fileheader postfile;
    time_t now = time(0);
    char buf[120], fpath[80], fpath0[80];
    int now2;

    sprintf(buf,
       "�� \033[1;37m%-12.12s\033[37m �� \033[36m%s \033[37m���s�s�襻�g�峹\033[m",
       cuser.userid, Etime(&now));
    f_cat(genbuf, buf);

    setbpath (fpath, currboard);
    stampfile (fpath, &postfile);
    unlink (fpath);
    setbfile (fpath0, currboard, fhdr->filename);

    f_mv (fpath0, fpath);

    if (currmode & MODE_SELECT)
    {
       setbdir(genbuf,currboard);
       now2 = getindex(genbuf, fhdr->filename, sizeof(fileheader));
    }

#ifdef HAVE_ALLPOST
    change_allpost(fhdr, postfile.filename, save_title);
#endif
    strcpy (fhdr->title, save_title);
    strcpy (fhdr->filename, postfile.filename);
    brc_addlist (postfile.filename);
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);

    if (currmode & MODE_SELECT)
       substitute_record(genbuf, fhdr, sizeof(*fhdr), now2);
    
    setbtotal(getbnum(currboard));
  }  
  return RC_FULL;
}


static int
cross_post (int ent, fileheader *fhdr, char *direct)
{
  char xboard[20], fname[80], xfpath[80], xtitle[80];
  char *choose_save[3] = {"sS)�s��","lL)����", msg_choose_cancel};
  fileheader xfile;
  FILE * xptr;
  int author = 0, bid;
  char genbuf[256];

  /* itoc.001203: �[�K���峹������� */
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD))
  {
    pressanykey("���峹�w�Q�[�K�I");
    return RC_NONE;
  }
  
  move (2, 0);
  clrtoeol ();
  move (3, 0);
  clrtoeol ();
  move (1, 0);
  brdcomplete("������峹��ݪO�G", xboard);
  if (*xboard == '\0' || !haspostperm (xboard))
    return RC_FULL;

  if (bid = getbnum(xboard))
  {
    /* �ɥ� ent */
    ent = brdshm->bcache[bid - 1].brdattr;
    
    if ((ent & BRD_GROUPBOARD) || (ent & BRD_CLASS))
      return RC_FULL;    
  }

  ent = 1;
  if (HAS_PERM (PERM_SYSOP) || !strcmp (fhdr->owner, cuser.userid))
  {
    char *choose_repost[2] = {"11)������","22)������榡"};
    if (getans2(2, 0, "",choose_repost, 2,'1') != '2')
    {
      char inputbuf;

      ent = 0;
      inputbuf = getans2(2, 0, "�O�d��@�̦W�ٶܡH", 0, 2, 'y');
      if (inputbuf != 'n' && inputbuf != 'N') author = 1;
    }
  }

  if (ent)
    sprintf (xtitle, "[���]%.66s", fhdr->title);
  else
    strcpy (xtitle, fhdr->title);

  sprintf (genbuf, "�ĥέ���D�m%.60s�n�ܡH", xtitle);

  if (getans2(2, 0, genbuf, 0, 2, 'y') == 'n')
  {
    if (getdata (2, 0, "���D�G", genbuf, TTLEN, DOECHO, xtitle))
      strcpy (xtitle, genbuf);
  }

  genbuf[0] = getans2(2, 0, "", choose_save, 3, 's');
  if (genbuf[0] == 'l' || genbuf[0] == 's')
  {
    int currmode0 = currmode;

    currmode = 0;
    setbpath (xfpath, xboard);
    stampfile (xfpath, &xfile);
    if (author)
      strcpy (xfile.owner, fhdr->owner);
    else
      strcpy (xfile.owner, cuser.userid);
    strcpy (xfile.title, xtitle);
    if (genbuf[0] == 'l')
    {
      xfile.savemode = 'L';
      xfile.filemode = FILE_LOCAL;
    }
    else
      xfile.savemode = 'S';

    setbfile (fname, currboard, fhdr->filename);
    if (ent)
    {
      xptr = fopen (xfpath, "w");
      strcpy (save_title, xfile.title);
      strcpy (xfpath, currboard);
      strcpy (currboard, xboard);
      write_header (xptr);
      strcpy (currboard, xfpath);

      fprintf (xptr, "�� [��������� %s �ݪO]\n\n", currboard);

      b_suckinfile (xptr, fname);
      addsignature (xptr);
      fclose (xptr);
    }
    else
    {
      unlink (xfpath);
      f_cp (fname, xfpath, O_TRUNC);
    }

    setbdir (fname, xboard);
    rec_add (fname, (char *) &xfile, sizeof (xfile));
    setbtotal(bid);
    
    if (!xfile.filemode)
      outgo_post (&xfile, xboard);
    cuser.numposts++;
    UPDATE_USEREC;
    pressanykey ("�峹��������I");
    currmode = currmode0;
  }
  return RC_FULL;
}

static int
read_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[256], buf[PATHLEN];
  int more_result;
  
  setdirpath (genbuf, direct, fhdr->filename);  /*�]���峹�[�K,�h��e�� hialan*/
  
  sprintf(buf, "%s.vis", genbuf);
  /* itoc.001203: �[�K���峹�u����@�̥H�ΪO�D��\Ū */
  if (fhdr->filemode & FILE_REFUSE && !(currmode & MODE_BOARD) &&
      strcmp(cuser.userid, fhdr->owner) && !belong_list(buf, cuser.userid))
  {
    pressanykey("���峹�w�Q�[�K�I");
    return RC_FULL;
  }

  if (fhdr->owner[0] == '-')
    return RC_NONE;

//  if(dashd(genbuf))
//    read_dir(genbuf,fhdr->title);

  /* yagami.010714: �s��[�K�峹�i�ݨ��W�� */
  if (fhdr->filemode & FILE_REFUSE && 
    ((currmode & MODE_BOARD) || !strcmp(cuser.userid, fhdr->owner)))
  {
    if(win_select("�[�K�峹", "�O�_�s��i�ݨ��W��? ", 0, 2, 'n') == 'y')    
      ListEdit(buf);
  }


  if ((more_result = pmore (genbuf, YEA)) == -1)
    return RC_NONE;

  brc_addlist (fhdr->filename);
  strncpy (currtitle, str_ttl(fhdr->title), TTLEN);
  strncpy (currowner, str_ttl(fhdr->owner), STRLEN);

  switch (more_result)
  {
    case 'b':
      return RS_PREV;
    case '[':
      return RELATE_PREV;
    case 'f':
      return RS_NEXT;
    case ']':
      return RELATE_NEXT;
    case '=':
      return RELATE_FIRST;
    
    case 'r':
    case 'R':
    case 'y':
    case 'Y':
      if (currmode & MODE_POST)
      {
        strcpy (quote_file, genbuf);
        do_reply (fhdr);
        *quote_file = 0;
      }
      return RC_FULL;
    case 'A':
      return 'A';
    case 'a':
      return 'a';
    case 'F':
      return '/';
    case 'B':
      return '?';
  }
  return RC_FULL;
}



/* ----------------------------------------------------- */
/* �Ķ���ذ�                                            */
/* ----------------------------------------------------- */
man()
{
  char buf[64], buf1[64], xboard[20], fpath[PATHLEN];
  boardheader * bp;

  if (currstat == RMAIL)
  {
    move (2, 0); clrtoeol ();
    move (3, 0); clrtoeol ();
    move (1, 0); 
    brdcomplete ("��J�ݪ��W�� (����Enter�i�J�p�H�H��)�G", buf);
    if (*buf)
      strcpy (xboard, buf);
    else
      strcpy (xboard, "0");
    if (xboard && (bp = getbcache (xboard)))
    {
      setapath (fpath, xboard);
      setutmpmode (ANNOUNCE);
      if (Ben_Perm (&brdshm->bcache[getbnum (xboard)-1]) != 1)
        pressanykey(P_BOARD);
      else
        a_menu (xboard, fpath, HAS_PERM (PERM_ALLBOARD) ? 2 : userid_is_BM (cuser.userid, bp->BM) ? 1 : 0);
    }
    else if(HAS_PERM(PERM_MAILLIMIT) || HAS_PERM(PERM_BM)) // wildcat : ���e�ѰO�[ PERM ����� ^^;
    {
      int mode0 = currutmp->mode;
      int stat0 = currstat;
      sethomeman (buf, cuser.userid);
      sprintf (buf1, "%s ���H��", cuser.userid);
      setutmpmode (ANNOUNCE);
      a_menu (buf1, buf, belong ("etc/sysop", cuser.userid) ? 2 : 1);
      currutmp->mode = mode0;
      currstat = stat0;
      return RC_FULL;
    }
  }
  else
  {
    setapath (buf, currboard);
    setutmpmode (ANNOUNCE);
    a_menu (currboard, buf, HAS_PERM (PERM_ALLBOARD) ? 2 :
      currmode & MODE_BOARD ? 1 : 0);
  }
  return RC_FULL;
}

int
cite (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char fpath[PATHLEN];
  char title[TTLEN + 1];

  if (currstat == RMAIL)
  {
    sethomefile (fpath, cuser.userid, fhdr->filename);
    add_tag ();
  }
  else
    setbfile (fpath, currboard, fhdr->filename);

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("���i�ƻs���åؿ��Υ[�K�ɮסI");
    return RC_NONE;
  }
  strcpy (title, "�� ");
  strncpy (title + 3, fhdr->title, TTLEN - 3);
  title[TTLEN] = '\0';
//  a_copyitem (fpath, title, fhdr->owner);
  a_copyitem (fpath, title, cuser.userid);
  /* shakalaca.990517: ���ϥΪ̭n�D, �s�̬��O�D */
  man ();
  return RC_FULL;
}

#if 0
Cite_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char fpath[PATHLEN];
  char title[TTLEN + 1];

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("���i�ƻs���åؿ�");
    return RC_NONE;
  }
  setbfile (fpath, currboard, fhdr->filename);
  sprintf (title, "%s%.72s",(currutmp->pager > 1) ? "" : "�� ", fhdr->title);
  title[TTLEN] = '\0';
  a_copyitem (fpath, title, cuser.userid);
  load_paste ();
  if (*paste_path)
    a_menu (paste_title, paste_path, paste_level, ANNOUNCE);
  return RC_FULL;
}
#endif

int
Cite_posts (int ent, fileheader * fhdr, char *direct)
{
  char fpath[PATHLEN];

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("���i�ƻs���åؿ��I");
    return RC_NONE;
  }
  setbfile (fpath, currboard, fhdr->filename);
  load_paste ();
  if (*paste_path && paste_level && dashf (fpath))
  {
    fileheader fh;
    char newpath[PATHLEN];

    strcpy (newpath, paste_path);
    stampfile (newpath, &fh);
/* shakalaca.990714: �N�ɮ׿W��
    unlink (newpath); */
    f_cp (fpath, newpath, O_TRUNC);
    strcpy (fh.owner, cuser.userid);
    sprintf (fh.title, "%s%.72s","�� " , fhdr->title);
    strcpy (strrchr (newpath, '/') + 1, ".DIR");
    rec_add (newpath, &fh, sizeof (fh));
    return POS_NEXT;
  }
  bell ();
  return RC_NONE;
}

int
edit_title (int ent, fileheader *fhdr, char *direct)
{
  char genbuf[PATHLEN];
  
  if(currmode & MODE_TINLIKE)
    return RC_NONE;
  
  if (HAS_PERM (PERM_SYSOP) || (currmode & MODE_BOARD))
  {
    fileheader tmpfhdr = *fhdr;
    int dirty = 0;
    
    if (getdata (b_lines - 1, 0, "���D�G", genbuf, TTLEN, DOECHO, tmpfhdr.title))
    {
      strcpy (tmpfhdr.title, genbuf);
      dirty++;
    }

    if(HAS_PERM (PERM_SYSOP))
    {
      if (getdata (b_lines - 1, 0, "�@�̡G", genbuf, IDLEN + 2, DOECHO, tmpfhdr.owner))
      {
        strcpy (tmpfhdr.owner, genbuf);
        dirty++;
      }

      if (getdata (b_lines - 1, 0, "����G", genbuf, 6, DOECHO, tmpfhdr.date))
      {
        sprintf (tmpfhdr.date, "%+5s", genbuf);
        dirty++;
      }
    }

    if(getans2(b_lines - 1, 0, msg_sure, 0, 2, 'n') == 'y' && dirty)
    {
      *fhdr = tmpfhdr;
      substitute_record (direct, fhdr, sizeof (*fhdr), ent);
      if (currmode & MODE_SELECT)
      {
        int now;
        
        setbdir (genbuf, currboard);
        now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
        substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
      }
    }
    return RC_FULL;
  }
  return RC_NONE;
}

int
add_tag (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  int now;
  char genbuf[100];

  if (!strcmp(currboard,"Security") && !HAS_PERM(PERM_BBSADM)) return RC_NONE;
  
  /*hialan.020714 mark�L�Υ[�K�L���峹 ����tag*/
  if ((fhdr->filemode & FILE_MARKED) || (fhdr->filemode & FILE_REFUSE)) return RC_NONE;
  
  if (currstat == RMAIL)
  {
    fhdr->filemode ^= FILE_TAGED;
    sethomedir (genbuf, cuser.userid);
    if (currmode & SELECT)
    {
      now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
      substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
      sprintf (genbuf, "home/%s/SR.%s", cuser.userid, cuser.userid);
      substitute_record (genbuf, fhdr, sizeof (*fhdr), ent);
    }
    else
      substitute_record (genbuf, fhdr, sizeof (*fhdr), ent);
    return POS_NEXT;
  }
//  if(currstat == READING) return RC_NONE;
  if (currmode & MODE_BOARD)
  {
    fhdr->filemode ^= FILE_TAGED;
    if (currmode & MODE_SELECT)
    {
      setbdir (genbuf, currboard);
      now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
      substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
      sprintf (genbuf, "boards/%s/SR.%s", currboard, cuser.userid);
      substitute_record (genbuf, fhdr, sizeof (*fhdr), ent);
      return POS_NEXT;
    }
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);
    return POS_NEXT;
  }
  return RC_NONE;
}


int
del_tag (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  int number;

  if (currstat == RMAIL)
  {
    if (getans2(1, 0, "�T�w�R���аO�H��H", 0, 2, 'y') != 'n')
    {
      currfmode = FILE_TAGED;
      if (delete_files (direct, cmpfmode))
        return RC_CHDIR;
    }
    return RC_FULL;
  }
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    if (!strcmp(currboard,"Security") && !HAS_PERM(PERM_BBSADM)) return RC_NONE;

    if (getans2(1, 0, "�T�w�R���аO�峹�H", 0, 2, 'n') == 'y')
    {
      currfmode = FILE_TAGED;
      if (currmode & MODE_SELECT)
      { 
        char xfile[PATHLEN];
        
        sprintf(xfile,"%s.vis", direct);
        unlink (direct);
        unlink(xfile);  /*�[�K�ɮצW��*/
        currmode ^= MODE_SELECT;
        setbdir (direct, currboard);
        delete_files (direct, cmpfmode);
      }

      if (number=delete_files(direct, cmpfmode))
      {
        log_board3("TAG", currboard, number);
        setbtotal(getbnum(currboard));
        
        return RC_CHDIR;
      }
    }
    return RC_FULL;
  }
  return RC_NONE;
}

int
gem_tag (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  load_paste(); //Ū�J paste_file �өw��
  if(!*paste_path)
  {
    pressanykey("�|���w��A�жi�J��ذϤ��A�Q�������ؿ��� P �w��I");
    return RC_FOOT;
  }

  if (currstat == RMAIL)
  {
    if (getans2(1, 0, "�T�w�����аO�H��H", 0, 2, 'y') != 'n')
    {
      currfmode = FILE_TAGED;
      if (gem_files (direct, cmpfmode))
        return RC_CHDIR;
    }
    return RC_FULL;
  }
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    if (getans2(1, 0, "�T�w�����аO�峹�H", 0, 2, 'n') == 'y')
    {
      currfmode = FILE_TAGED;
      if (currmode & MODE_SELECT)
      {
        unlink (direct);
        currmode ^= MODE_SELECT;
        setbdir (direct, currboard);
        gem_files (direct, cmpfmode);
      }
      else
        gem_files(direct, cmpfmode);
      return RC_CHDIR;
    }
    return RC_FULL;
  }
  return RC_NONE;
}


int
mark (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  if (currstat == READING && !(currmode & MODE_BOARD))
    return RC_NONE;

  if (currmode & MODE_BOARD && currstat == READING)
  {
    if (fhdr->filemode & FILE_MARKED)
      deumoney (fhdr->owner, 200);
    else
      inumoney (fhdr->owner, 200);
  }

  fhdr->filemode ^= FILE_MARKED;

  if (currmode & MODE_SELECT)
  {
    int now;
    char genbuf[100];

    if (currstat != READING)
      sethomedir(genbuf, cuser.userid);
    else
      setbdir (genbuf, currboard);
    now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
    substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
  }
  else
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);

  return RC_DRAW;
}


int v_board (int, fileheader*, char*);

static int
score_note(char *prompt, fileheader *fhdr, char *direct, char tag)
{
    char genbuf[80], fpath[80], buf[256];
    time_t now = time(NULL);
    struct tm *ptime = localtime(&now);
    
    getdata(b_lines, 0, prompt, genbuf, 38, DOECHO, 0);
    if(*genbuf == 0) return -1;

    setdirpath (fpath, direct, fhdr->filename);
    sprintf(buf, "\033[1;36m��\033[37m %-12s �� \033[1;36m%0d/%0d\033[37m %s�A�z�ѡG%-37.37s\033[m", 
    		cuser.userid, ptime->tm_mon+1, ptime->tm_mday, 
    		(tag > 0) ? "���O [\033[31m��\033[37m]" : "�P�M [\033[32m�@\033[37m]", 
    		genbuf);
    f_cat(fpath, buf);

    return 1;
}

/*�峹����*/
int
score (int ent, fileheader *fhdr, char *direct)
{
  char buf[128];
  time_t now = time(0);
  char *choose[3] = {"11)�[��","22)����", msg_choose_cancel};
  
  if (currstat == RMAIL)
    return RC_NONE;
  
  if (cuser.scoretimes <= 0 && !HAVE_PERM(PERM_SYSOP))
  {
    pressanykey("�����I�Ƥ����L�k�����I");
    return RC_FULL;
  }
  
  if (!strcmp("guest", cuser.userid))
  {
    pressanykey("guest �����Ѧ��\\��I");
    return RC_FULL;
  }

  buf[0] = getans2(b_lines, 0, "�аݭn�G", choose, 3, 'q');
  if(!buf[0] || buf[0] < '1' || buf[0] > '3')
    return RC_DRAW;
  else if(buf[0] == '1' && fhdr->score < 99)
  {
    if(score_note("�[����]�G", fhdr, direct, 1) < 0)
      return RC_FULL;
    else
      fhdr->score++;
  }
  else if(buf[0] == '2' && fhdr->score > -99)
  {
    if(score_note("������]�G", fhdr, direct, -1) < 0)
      return RC_FULL;
    else
      fhdr->score--;
  }
  else if(buf[0] != 'q')
  {
    /* ���ƪ��W���P�U��*/
    if(fhdr->score >= 99 || fhdr->score <= -99)       
    {                                                             
      sprintf(buf , "�w�g�O��%s���F�I", fhdr->score >= 99 ? "��" : "�C");    
      pressanykey(buf);
      return RC_DRAW;
    }  
  }

  /* �����������Ʀ��O���[�� */    
  if(!HAS_PERM(PERM_SYSOP))                                     
  {                                                            
    ingold(1);                                                
    cuser.scoretimes--;
  }

  fhdr->filemode |= FILE_SCORED;	/* �w�g�[���L */
  
  if (currmode & MODE_SELECT)                                  
  {                                                         
    int now;
    char genbuf[100];
                                                                                
    if (currstat != READING)
      sethomedir(genbuf, cuser.userid);
    else
      setbdir (genbuf, currboard);
    now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
    substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
  }
  else
    substitute_record (direct, fhdr, sizeof (*fhdr), ent);
                                                                                
  substitute_record (fn_passwd, &cuser, sizeof (userec), usernum);
  sprintf(buf , "%-12.12s �� %-12.12s �O [%-40.40s] ���� [%s] %s",
    cuser.userid,currboard,fhdr->title,
    buf[0] == '1' ? "+1" : "-1",Etime(&now));
  f_cat("log/article_score.log",buf);
  if(!HAS_PERM(PERM_SYSOP))
  {
    sprintf(buf , "�A�����������٦� %d ��..", cuser.scoretimes);
    pressanykey(buf);
  }
  return RC_DRAW;
}

/* itoc.001203: �峹�[�K */
int
refusemark(int ent, fileheader *fhdr, char *direct)
{
  char buf[256];
  int now;
  char genbuf[PATHLEN];
  
  if (currstat != READING)
    return RC_NONE;
                                                                                
  /*�۰ʲM���[�K�W�� ... hialan.020714*/
  setdirpath (buf, direct, fhdr->filename);
  strcat(buf,".vis");
  if(dashf(buf) || (fhdr->filemode & FILE_REFUSE))
    unlink(buf);

  if((currmode & MODE_BOARD) || !strcmp(fhdr->owner, cuser.userid)) 
    fhdr->filemode ^= FILE_REFUSE;                                                                                

  if (currmode & MODE_SELECT)
  {
    setbdir(genbuf, currboard);
    now = getindex(genbuf, fhdr->filename, sizeof(fileheader));
    substitute_record(genbuf, fhdr, sizeof(*fhdr), now);
  }
  else
    substitute_record(direct, fhdr, sizeof(*fhdr), ent);
    
#ifdef HAVE_ALLPOST
  change_allpost(fhdr, fhdr->filename, fhdr->title);
#endif    
                                                                                
  return RC_DRAW;
}


int
del_range (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char num1[8], num2[8];
  int inum1, inum2;

  if (!strcmp(currboard,"Security")) return RC_NONE;
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    getdata (1, 0, "[�]�w�R���d��] �_�I�G", num1, 5, DOECHO, 0);
    inum1 = atoi (num1);
    if (inum1 <= 0)
    {
      outz ("�_�I���~�I");
      return RC_FOOT;
    }
    getdata (1, 28, "���I�G", num2, 5, DOECHO, 0);
    inum2 = atoi (num2);
    if (inum2 < inum1)
    {
      outz ("���I���~�I");
      return RC_FULL;
    }
    
    if (getans2(1, 48, msg_sure, 0, 2, 'n') == 'y')
    {
      outmsg ("�B�z���A�еy��..");
      refresh ();
      if (currmode & MODE_SELECT)
      {
        int fd, size = sizeof (fileheader);
        char genbuf[100];
        fileheader rsfh;
        int i = inum1, now;
        
        if (currstat == RMAIL)
          sethomedir (genbuf, cuser.userid);
        else
          setbdir (genbuf, currboard);
        
        if ((fd = (open (direct, O_RDONLY, 0))) != -1)
        {
          if (lseek (fd, (off_t) (size * (inum1 - 1)), SEEK_SET) != -1)
          {
            while (read (fd, &rsfh, size) == size)
            {
              if (i > inum2)
                break;
              now = getindex (genbuf, rsfh.filename, size);
              strcpy (currfile, rsfh.filename);
              if (!(rsfh.filemode & FILE_MARKED))
                delete_file (genbuf, sizeof (fileheader), now, cmpfilename);
              i++;
            }
          }
          close (fd);
        }
      }
      delete_range (direct, inum1, inum2);

      setbtotal(getbnum(currboard));

      return RC_NEWDIR;
    }
    return RC_FULL;
  }
  return RC_NONE;
}

#if 0
static void
lazy_delete (fhdr)
  fileheader * fhdr;
{
  char buf[20];

  sprintf (buf, "-%s", fhdr->owner);
  strncpy (fhdr->owner, buf, IDLEN + 1);
  strcpy (fhdr->title, "<< article deleted >>");
  fhdr->savemode = 'L';
}

int
del_one (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  if ((currstat != READING) || (currmode & MODE_BOARD))
  {
    strcpy (currfile, fhdr->filename);

    if (!update_file (direct, sizeof (fileheader), ent, cmpfilename, lazy_delete))
    {
      cancelpost (fhdr, YEA);
      lazy_delete (fhdr);
      return RC_DRAW;
    }
  }
  return RC_NONE;
}
#endif

static int
del_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  int not_owned, money;
  char genbuf[100];

  if (!strcmp(currboard,"Security")) return RC_NONE;

  if ((fhdr->filemode & (FILE_MARKED | FILE_DIGEST | FILE_REFUSE | FILE_BOTTOM)) 
       || (fhdr->owner[0] == '-'))
    return RC_NONE;

  not_owned = strcmp (fhdr->owner, cuser.userid);
// wildcat : �����i�H�s�u��H

  if(HAS_PERM(PERM_SYSOP) && getans2(b_lines, 0, "�O�_�n�s�u��H? ", 0, 2, 'n') == 'y')
    not_owned = 0;

  if (!(currmode & MODE_BOARD) && not_owned || !strcmp (cuser.userid, "guest"))
    return RC_NONE;

  if (getans2(1, 0, "�T�w�R��? ", 0, 2, 'n') == 'y')
  {
    strcpy (currfile, fhdr->filename);
    setbfile (genbuf, currboard, fhdr->filename);
    money = (int) dashs (genbuf) / 90;
    if (!delete_file (direct, sizeof (fileheader), ent, cmpfilename))
    {
      if (currmode & MODE_SELECT)
      {
        int now;

        setbdir (genbuf, currboard);
        now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
        delete_file (genbuf, sizeof (fileheader), now, cmpfilename);
      }
      cancelpost (fhdr, not_owned);
      setbtotal(getbnum(currboard));
      
      if (!not_owned && !(currbrdattr & BRD_NOCOUNT) && !HAS_PERM(PERM_SYSOP))
      {
        UPDATE_USEREC;
        move (b_lines - 1, 0);
        clrtoeol ();
        if (money < 1) money = 1;
        if(cuser.goldmoney > money)
          degold (money);
        else
          demoney(money*10000);
        pressanykey ("%s�A�z���峹� %d �g�A��I�M��O %d ���I", msg_del_ok,
          cuser.numposts > 0 ? --cuser.numposts : cuser.numposts, money);
        substitute_record (fn_passwd, &cuser, sizeof (userec), usernum);
      }
      return RC_CHDIR;
    }
  }
  return RC_FULL;
}

int
save_mail (int ent, fileheader * fh, char *direct)
{
  fileheader mhdr;
  char fpath[PATHLEN];
  char genbuf[PATHLEN];
  char *p;

  /* itoc.001203: �[�K���峹���� edit */
  if (fh->filemode & FILE_REFUSE && !(currmode & MODE_BOARD))
  {
    pressanykey("���峹�w�Q�[�K�I");
    return RC_NONE;
  }

  if (ent < 0)
    strcpy (fpath, direct);
  else
  {
    strcpy (genbuf, direct);
    if (p = strrchr (genbuf, '/'))
      * p = '\0';
    sprintf (fpath, "%s/%s", genbuf, fh->filename);
  }
  if (!dashf (fpath) || !HAS_PERM (PERM_BASIC))
  {
    bell ();
    return RC_NONE;
  }
  sethomepath (genbuf, cuser.userid);
  stampfile (genbuf, &mhdr);
  unlink (genbuf);
  f_cp (fpath, genbuf, O_TRUNC);
  if (HAS_PERM (PERM_SYSOP))
    strcpy (mhdr.owner, fh->owner);
  else
    strcpy (mhdr.owner, cuser.userid);
  strncpy (mhdr.title, fh->title + ((currstat == ANNOUNCE) ? 3 : 0), TTLEN);
  mhdr.savemode = '\0';
  mhdr.filemode = FILE_READ;
  sethomedir (fpath, cuser.userid);
  if (rec_add (fpath, &mhdr, sizeof (mhdr)) == -1)
  {
    bell ();
    return RC_NONE;
  }
  return POS_NEXT;
}

/* ----------------------------------------------------- */
/* �峹�m��                                              */
/* ----------------------------------------------------- */

int
getbottomtotal(int bid)
{
  char path[PATHLEN];
  
  setbfile(path, brdshm->bcache[bid - 1].brdname, FN_BOTTOM);
  return rec_num(path, sizeof(fileheader));
}

static int
push_bottom(int ent, fileheader *fhdr, char *direct)
{
    int num, bid= getbnum(currboard);
    char buf[256];

    if ((currmode & MODE_DIGEST) || !(currmode & MODE_BOARD))
        return RC_NONE;

//    setbottomtotal(bid);  // <- Ptt : will be remove when stable
    num = getbottomtotal(bid);

    if( getans2(b_lines, 0, 
                fhdr->filemode & FILE_BOTTOM ? "�����m�����i?": "�[�J�m�����i?" , 0, 2, 'n') != 'y' )
	return RC_FOOT;

    fhdr->filemode ^= FILE_BOTTOM;
    if(fhdr->filemode & FILE_BOTTOM )
    {
          setdirpath(buf, direct, FN_BOTTOM);
          if(num >= 5)
          {
              pressanykey("���o�W�L 5 �g���n���i �к�²!");
              return RC_FOOT;
	  }
          rec_add(buf, fhdr, sizeof(fileheader)); 
    }
    else
	num = rec_del(direct, sizeof(fileheader), ent, NULL, NULL);

//    setbottomtotal(bid);
    return RC_CHDIR;
}

/* ----------------------------------------------------- */
/* �ݪO�Ƨѿ��B��K�B��ذ�                              */
/* ----------------------------------------------------- */

/* wildcat modify 981221 */
int
b_notes ()
{
  char buf[64];

  setbfile (buf, currboard, fn_notes);
  /* �i�H�ݦh�� */
  if (more (buf, YEA) == -1) 
  {
    clear ();
    pressanykey ("���ݪO�|�L�u�Ƨѿ��v�C");
  }
  return RC_FULL;
}


int
board_select ()
{
  struct stat st;
  char fpath[80];
  char genbuf[100];
  currmode &= ~(MODE_SELECT | MODE_TINLIKE);

  sprintf (genbuf, "SR.%s", cuser.userid);
  setbfile (fpath, currboard, genbuf);
  unlink (fpath);

  /* shakalaca.000112: �W�L 30min �~�N index �R��, �@ cache �� */
  setbfile (fpath, currboard, "SR.thread");
  if (stat(fpath, &st) == 0 && st.st_mtime < time(0) - 60 * 30)
    unlink (fpath);

  if (currstat == RMAIL)
    sethomedir (currdirect, cuser.userid);
  else
    setbdir (currdirect, currboard);

  return RC_NEWDIR;
}


int
board_digest ()
{
  if (currmode & MODE_SELECT)
    board_select ();

  currmode ^= MODE_DIGEST;
  if (currmode & MODE_DIGEST)
    currmode &= ~MODE_POST;
  else if (haspostperm (currboard))
    currmode |= MODE_POST;

  setbdir (currdirect, currboard);
  return RC_NEWDIR;
}


static int
good_post (ent, fhdr, direct)
  int ent;
  fileheader * fhdr;
  char *direct;
{
  char genbuf[PATHLEN];
  char genbuf2[PATHLEN];
  fileheader digest;

  if(fhdr->filemode & FILE_REFUSE)
  {
    pressanykey("�[�K�峹���ব�J���K�̡I");
    return RC_DRAW;
  }
  
  memcpy (&digest, fhdr, sizeof (digest));
  digest.filename[0] = 'G';

  if ((currmode & MODE_DIGEST) || !(currmode & MODE_BOARD))
    return RC_NONE;

  if (fhdr->filemode & FILE_DIGEST)
  {
    int now;
    setbfile(genbuf2, currboard, ".Names");
    now = getindex (genbuf2, digest.filename, sizeof(fileheader));
    strcpy (currfile, digest.filename);
    delete_file (genbuf2, sizeof (fileheader), now, cmpfilename);
    sprintf (genbuf2, BBSHOME "/boards/%s/%s", currboard, currfile);
    unlink (genbuf2);
    fhdr->filemode = (fhdr->filemode & ~FILE_DIGEST);
  }
  else
  {
    char *ptr, buf[64];
    strcpy (buf, direct);
    ptr = strrchr (buf, '/') + 1;
    ptr[0] = '\0';
    sprintf (genbuf, "%s%s", buf, digest.filename);
    if (!dashf (genbuf))
    {
      digest.savemode = digest.filemode = 0;
      sprintf (genbuf2, "%s%s", buf, fhdr->filename);
      f_cp (genbuf2, genbuf, O_TRUNC);
      strcpy (ptr, fn_mandex);
      rec_add (buf, &digest, sizeof (digest));
    }
    fhdr->filemode = (fhdr->filemode & ~FILE_MARKED) | FILE_DIGEST;
  }
  substitute_record (direct, fhdr, sizeof (*fhdr), ent);
  if (currmode & MODE_SELECT)
  {
    int now;
    char genbuf[100];
    setbdir (genbuf, currboard);
    now = getindex (genbuf, fhdr->filename, sizeof (fileheader));
    substitute_record (genbuf, fhdr, sizeof (*fhdr), now);
  }
  return RC_DRAW;
}


/* hialan ��z from i_read_key*/
int
write_msg (int ent, fileheader *fhdr, char *direct)
{
  user_info *owneronline = (user_info *)searchowner(fhdr->owner);
  if (owneronline != NULL) talk_water(owneronline);
  return RC_FULL;
}

int
post_mail_uncode (int ent, fileheader *fhdr, char *direct)
{
    char fname[PATHLEN];
       
    setdirpath(fname, direct, fhdr->filename);
    if (dashf(fname))
      mail_forward(fhdr, direct, 'U');
    return RC_FULL;
}

int
post_mail (int ent, fileheader *fhdr, char *direct)
{
    char fname[PATHLEN];
       
    setdirpath(fname, direct, fhdr->filename);
    if (dashf(fname))
      mail_forward(fhdr, direct, 'F');
    return RC_FULL;
}

int
post_query (int ent, fileheader *fhdr, char *direct)
{
  return my_query(fhdr->owner);
}  

static int 
post_vote()
{
  if (currstat != ANNOUNCE)
    DL_func("SO/vote.so:b_vote");
  return RC_FULL;
}

static int
post_b_results()
{
  if (currstat != ANNOUNCE)
    DL_func("SO/vote.so:b_results");
  return RC_FULL;
}

static int
go_chat()
{
  DL_func("SO/chat.so:t_chat");
  return RC_FULL;
}

/* �تḨ���� (�R���Ҧ��ݪO�ۦP ���D or �@�� ���峹) */
static int
kill_all_spam(int ent, fileheader *fhdr, char *direct)
{
  if ((currstat == READING) && (HAS_PERM(PERM_ALLBOARD)) && (currstat != ANNOUNCE))
    return DL_func("SO/admin_kill_spam.so:kill_all_spam", fhdr);

  return RC_NONE;
}

/* ----------------------------------------------------- */
/* �ݪO�\���                                            */
/* ----------------------------------------------------- */

struct one_key read_comms[] =
{
  KEY_TAB, board_digest,	0,"�i�J/�h�X ��K",0,
  Ctrl ('P'), do_post,   	0,"�o��峹",0,
  'y', reply_post,       	0,"�^�Ф峹",0,
  'r', read_post,        	0,"�\\Ū�峹",0,
  'z', man,         	 	0,"�i�J��ذ�",0,
  'E', edit_post,        	0,"�ק�峹",0,
  'x', cross_post,       	0,"��K",0,
  's', do_select,        	0,"��ܬݪO",0,
  'X', refusemark,       	0,"�峹�[�K",0,
  'b', b_notes,        		0,"�ݶi���e��",0,
  Ctrl ('S'), save_mail,	0,"�s�J�H�c",0,
  Ctrl ('C'), Cite_posts,	0,"���������峹�ܺ�ذ�",0,
  '%', score,		 	0,"�峹����",0,
  'v', v_board,		 	0,"�O��v�O",0,
  'd', del_post,         	0,"�R���峹",0,
  Ctrl ('Q'), post_query,	0,"�O�� q �H",0,
  'V', post_vote,	 	0,"�ѻP�벼",0,
  'R', post_b_results,	 	0,"�ݧ벼���G",0,
  'F', post_mail,	 PERM_FORWARD,"�N�峹�H�^ Internet �l�c",0,
  'U', post_mail_uncode, PERM_FORWARD,"�N�峹 uncode ��H�^ Internet �l�c",0,
  'w', write_msg,    PERM_LOGINOK,"�O������y",0,
  'C', gem_tag,           PERM_BM,"�����аO�峹",0,
'B',"SO/bm.so:board_edit",PERM_BM,"�ݪO�s��",1,
  't', add_tag,           PERM_BM,"�аO�峹",0,
  Ctrl ('D'), del_tag,    PERM_BM,"�R���аO�峹",0,
  'g', good_post,         PERM_BM,"�����K��",0,
  'm', mark,              PERM_BM,"Mark �峹",0,
  'T', edit_title,        PERM_BM,"�ק���D",0,
  'D', del_range,	  PERM_BM,"�j D ��H",0,
  'c', cite,              PERM_BM,"�������",0,
  Ctrl ('V'), push_bottom,PERM_BM,"�峹�m��",0,
  Ctrl ('G'), go_chat,	 PERM_CHAT,"�ݪO��ѫ�",0,
  Ctrl ('X'), kill_all_spam, PERM_SYSOP, "��Ḩ����-->�@�f��R���P�@�̩ΦP���D���峹", 0,
  '\0', NULL, 0, NULL,0};

void Read ()
{
  int mode0 = currutmp->mode;
  int currmode0 = currmode;
  int stat0 = currstat;
  int bid;
  char buf[40];
  time_t startime = time (0);

  resolve_boards ();

  if((bid = getbnum (currboard)) == 0)
    return;

  setutmpmode (READING);
  set_board ();
  if (board_visit_time < board_note_time)
  {
    setbfile (buf, currboard, fn_notes);
    more (buf, YEA);
  }  
  
  if(HAS_PERM(PERM_BBSADM))
  {
    if(currbrdattr & BRD_HIDE && !hbflcheck(getbnum(currboard), currutmp->uid))
      pressanykey("ĵ�i! �z�i�H�i�J�ӬݪO, ���O�z�èS���Q�[�J�i���W��!");
  }
  else if(Ben_Perm(&brdshm->bcache[bid-1]) != 1)
  {
    pressanykey(P_BOARD);
    return;
  }  
  currutmp->brc_id = bid;    

  setbdir (buf, currboard);
  curredit &= ~EDIT_MAIL;
  i_read (READING, buf, readtitle, doent, read_comms, Ctrl('P'));

  log_board (currboard, time (0) - startime);
  log_board3("USE", currboard, time(0) - startime);
  brc_update ();

  currutmp->brc_id = 0;
  currmode = currmode0;
  currutmp->mode = mode0;
  currstat = stat0;
  return;
}


void
ReadSelect()
{
  if (do_select(0, NULL, NULL) == RC_NEWDIR)
    Read();
}


/*
int
Select()
{
  setutmpmode (SELECT);
  do_select(0, NULL, NULL);
  return 0;
}
*/

int
Post ()
{
  do_post ();
  return 0;
}

/* ----------------------------------------------------- */
/* ���} BBS ��                                           */
/* ----------------------------------------------------- */


void
note()
{
  static char *fn_note_tmp = "note.tmp";
  static char *fn_note_dat = "note.dat";
  int total, i, collect, len;
  struct stat st;
  char buf[256], buf2[256];
  int fd, fx;
  FILE *fp, *foo;
  struct notedata
  {
    time_t date;
    char userid[IDLEN + 1];
    char username[19];
    char buf[3][80];
  };
  struct notedata myitem;

  setutmpmode(EDNOTE);
  myitem.buf[0][0] = myitem.buf[1][0] = myitem.buf[2][0] = '\0';
  do
  {
    char *choose[3]={"sS)�s���[��","eE)���s�ӹL", msg_choose_cancel};

    i = b_lines - 10;
    move(i, 0);
    clrtobot();
    
    outs("�Яd�� (�ܦh�T��)�A�� [Enter] ����");
    i+=2; total = 0;
    while(total < 3 && 
          getdata(i+total, 0, ":", myitem.buf[total], 60, DOECHO, myitem.buf[total]))
      total++;
    
    if(total != 0)
      buf[0] = getans2(b_lines, 0, "", choose, 3, 's');
  } while (buf[0] == 'e');
  
  if(buf[0] == 'q' || !total)
    return;
    
  strcpy(myitem.userid, cuser.userid);
  strncpy(myitem.username, cuser.username, 18);
  myitem.username[18] = '\0';
  time(&(myitem.date));

  /* begin load file */

  if ((foo = fopen(BBSHOME"/.note", "a")) == NULL)
    return;

  if ((fp = fopen(fn_note_ans, "w")) == NULL)
    return;

  if ((fx = open(fn_note_tmp, O_WRONLY | O_CREAT, 0644)) <= 0)
    return;

  if ((fd = open(fn_note_dat, O_RDONLY)) == -1)
  {
    total = 1;
  }
  else if (fstat(fd, &st) != -1)
  {
    total = st.st_size / sizeof(struct notedata) + 1;
    if (total > MAX_NOTE)
      total = MAX_NOTE;
  }

  fputs("[1m                             "COLOR1" [33m�� [37m�� �� �d �� �O [33m�� [m \n\n",fp);
  collect = 1;

  while (total)
  {
    sprintf(buf, "[44m[1;36m�z�r [33m%s[37m(%s)",
      myitem.userid, myitem.username);
    len = strlen(buf);
    strcat(buf," [36m" + (len&1));

    for (i = len >> 1; i < 38; i++)
      strcat(buf, "�w");
    sprintf(buf2, "�w[33m %.14s  [36m�r�{[m\n",
      Etime(&(myitem.date)));
    strcat(buf, buf2);
    fputs(buf, fp);

    if (collect)
      fputs(buf, foo);

    sprintf(buf, "[1;44m[36m�|�{[37m%-70s[36m�z�}[m\n",myitem.buf[0]);
    if(*myitem.buf[1])
    {
      sprintf(buf2, "  [1;44m[36m�x[37m%-70s[36m�x[m\n",myitem.buf[1]);
      strcat(buf, buf2);
    }
    if(*myitem.buf[2])
    {
      sprintf(buf2, "  [1;44m[36m�x[37m%-70s[36m�x[m\n",myitem.buf[2]);
      strcat(buf, buf2);
    }
    fputs(buf,fp);
    if (collect)
    {
      fputs(buf, foo);
      fclose(foo);
      collect = 0;
    }
    write(fx, &myitem, sizeof(myitem));

    if (--total)
      read(fd, (char *) &myitem, sizeof(myitem));
  }
  fclose(fp);
  close(fd);
  close(fx);
  f_mv(fn_note_tmp, fn_note_dat);
  more(fn_note_ans, YEA);
}


int m_sysop()
{
  FILE *fp;
  char genbuf[128];

  setutmpmode(MSYSOP);
  if (fp = fopen("etc/sysop", "r"))
  {
    int i, j;
    char *ptr;

    struct SYSOPLIST
    {
      char userid[IDLEN + 1];
      char duty[40];
    }sysoplist[9];

    j = 0;
    while (fgets(genbuf, 128, fp))
    {
      if (ptr = strchr(genbuf, '\n'))
      {
        *ptr = '\0';
        ptr = genbuf;
        while (isalnum(*ptr))
           ptr++;
        if (*ptr)
        {
          *ptr = '\0';
          do
          {
            i = *++ptr;
          } while (i == ' ' || i == '\t');
          if (i)
          {
            strcpy(sysoplist[j].userid, genbuf);
            strcpy(sysoplist[j++].duty, ptr);
          }
        }
      }
    }


    move(12, 0);
    clrtobot();
    prints("%16s   %-18s�v�d����\n\n", "�s��", "���� ID"/*, msg_seperator*/);

    for (i = 0; i < j; i++)
    {
      prints("%15d.   [1;%dm%-16s%s[0m\n",
        i + 1, 31 + i % 7, sysoplist[i].userid, sysoplist[i].duty);
    }
    prints("%-14s0.   [1;%dm���}[0m", "", 31 + j % 7);
    getdata(b_lines - 1, 0, "                   �п�J�N�X[0]�G", genbuf, 4, DOECHO, 0);
    i = genbuf[0] - '0' - 1;
    if (i >= 0 && i < j)
    {
      clear();
      do_send(sysoplist[i].userid, NULL);
    }
  }
  fclose(fp);
  return 0;
}


int
Goodbye()
{
  if(win_select("���}" BOARDNAME, "�z�T�w�n���}�i " BOARDNAME " �j�ܡH", 0, 2, 'n') != 'y')
    return 0;

  movie(999);
  if (cuser.userlevel)
  {
    char *prompt[3]={"gG)�H���ӳu","mM)���گ���","nN)�ڭn�o��"};
    char ans = win_select("���}" BOARDNAME, "", prompt, 3, 'g');

    if (ans == 'm')
      m_sysop();
    else if (ans == 'n')
      note();
  }

  t_display();
  clear();
  prints("[1;31m�˷R�� [31m%s([37m%s)[31m�A�O�ѤF�A�ץ��{"COLOR1
    " %s [40;33m�I\n\n�H�U�O�z�b���������U���:[m\n",
    cuser.userid, cuser.username, BoardName);
  user_display(&cuser, 0);
  pressanykey(NULL);

  film_out(FILM_LOGOUT, -1);

  if (currmode)
    u_exit("EXIT ");

  exit(0);
}
