/*-------------------------------------------------------*/
/* enews.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : Yahoo! �_���s�D				 */
/* create : 02/01/27					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


/* ���_���s�D�{���ݭn�t�X enews-open �h���ơAcrontab �p�U�G */
/* # �C�� 10:30AM 4:30PM ��_���s�D */
/* 30 10,16 * * * bin/enews-open */

/* �n�ϥ� enews-open.c �����˦� lynx�C */
/* �峹�ɮש�b run/kimo/ */
/* ��ƨӷ��GYahoo! �_���s�D http://tw.news.yahoo.com/ */

#include "bbs.h"
#include "enews.h"

#define FEETER_ENEWS	\
COLOR2 "  �_���s�D  " COLOR3 "  ��V)����  �r��/##)����/����  ENTER)�s��  F|x)��H/���  q)���} \033[m"

#ifdef HAVE_NETTOOL


/*-------------------------------------------------------*/
/* variable						 */
/*-------------------------------------------------------*/


/* itoc.020129: �ثe���䴩½���A�ҥH�C�ؤ����s�D�̦h�]�O 16 ���Ӥw */

#define MAX_ENEWS	16	/* �@���̦h�� 16 �� */

static char title[MAX_ENEWS][TTLEN + 1];	/* �������s�D���Ҧ����D */
static char xname[MAX_ENEWS][10];	/* �������s�D���Ҧ��ɮ׸��| */
static int maxitem;		/* ���������@���X�g */

#define mouts(x,y,s)    { move(x, y); clrtoeol(); outs(s); }

static void
enews_fpath(fpath, kind, fname)
  char *fpath;
  char kind;
  char *fname;
{
  sprintf(fpath, "run/kimo/%c/%s", kind, fname);
}


/*-------------------------------------------------------*/
/* E-News ���						 */
/*-------------------------------------------------------*/


static void
enews_item(num, cc)
  int num;
  int cc;
{
  char bar_color[50];

  get_lightbar_color(bar_color);

  move(6 + num, 0);
  clrtoeol();

  prints("%15s %2d %s %s \033[m",
	 "", num + 1, (cc == num) ? bar_color : "", title[num]);

  move(b_lines, 0);
}


static void
enews_body(kind, cc)
  char kind;
  int cc;
{
  int num;

  if (!maxitem)
    return;

  for (num = 0; num < maxitem; num++)
    enews_item(num, cc);

  for (; num < MAX_ENEWS; num++)
  {
    move(6 + num, 0);
    clrtoeol();
  }
}


static void
enews_neck(kind, cc)
  char kind;
  int cc;
{
  int i;

  move(4, 4);
  clrtoeol();
  for (i = 'A'; i <= 'L'; i++)
    prints(i == kind ? "\033[1;36m%c\033[37;41m%s\033[m " : "\033[1;36m%c\033[m%s ", i, class[i - 'A']);

  enews_body(kind, cc);
}


static void
enews_head(kind, cc)
  char kind;
  int cc;
{
  clear();

  mouts(0, 8, "\033[1;37;42m  �������� ��������������������������������������������������  \033[m");
  mouts(1, 8, "\033[1;37;42m  ��  �����~�����~  ���~����     \033[33m����I�_���s�D\033[37m            ��  \033[m");
  mouts(2, 8, "\033[1;37;42m  ��  ����������������������     \033[33mhttp://tw.news.yahoo.com/\033[37m ��  \033[m");
  mouts(3, 8, "\033[1;37;42m  �������������������������� ��������������������������������  \033[m");

  move(b_lines, 0);
  outs(FEETER_ENEWS);

  enews_neck(kind, cc);
}


static void
enews_load(kind)
  char kind;
{
  char fpath[64];
  FILE *fp;
  ENEWS enews;

  maxitem = 0;

  enews_fpath(fpath, kind, ".ENEWS");
  if (!(fp = fopen(fpath, "r")))
    return;

  while (fread(&enews, sizeof(ENEWS), 1, fp) == 1 && maxitem < MAX_ENEWS)
  {
    strcpy(title[maxitem], enews.title);
    strcpy(xname[maxitem], enews.xname);
    maxitem++;
  }
  fclose(fp);
}


/*-------------------------------------------------------*/
/* ��L�\��						 */
/*-------------------------------------------------------*/


static int			/* �^�ǳ̫�Ū����@�g */
enews_browse(kind, cc)
  char kind;
  int cc;
{
  int key;
  char fpath[64];

  for (;;)
  {
    if (cc >= maxitem)
      break;

    enews_fpath(fpath, kind, xname[cc]);

    /* Thor.990204: ���Ҽ{more �Ǧ^�� */
    if ((key = more(fpath, YEA)) < 0)
      break;

    if (!key)			/* �wŪ�� */
      key = igetkey();

    switch (key)
    {
    case 'q':
    case 'f':
    case ' ':
    case 'j':
      if (cc < maxitem - 1)
      {
	cc++;
	continue;
      }
      break;

    case 'b':
      if (cc > 0)
      {
	cc--;
	continue;
      }
      break;
    }

    break;
  }

  return cc;
}


static void
enews_cross(kind, cc)		/* ���ѳ�g�����ݪO */
  char kind;
  int cc;
{
  char xboard[IDLEN + 1];
  int bid, tmp;
  fileheader xfile;
  char *choose_save[3] = {"sS)�s��", "lL)����", msg_choose_cancel};
  char fpath[PATHLEN], xfpath[PATHLEN];

  if (!cuser.userlevel)
    return;

  move(1, 0);
  brdcomplete("������峹��ݪO�G", xboard);

  if (bid = getbnum(xboard) && haspostperm(xboard))
  {
    tmp = brdshm->bcache[bid - 1].brdattr;
    
    if (!(tmp & BRD_GROUPBOARD) && !(tmp & BRD_CLASS))
    {

      tmp = getans2(2, 0, "", choose_save, 3, 'q');

      if (tmp == 'l' || tmp == 's')
      {
	setbpath(fpath, xboard);
	stampfile(fpath, &xfile);

	strcpy(xfile.owner, cuser.userid);
	strcpy(xfile.title, title[cc]);
	if (tmp == 'l')
	{
	  xfile.savemode = 'L';
	  xfile.filemode = FILE_LOCAL;
	}
	else
	  xfile.savemode = 'S';

	unlink(fpath);
	enews_fpath(xfpath, kind, xname[cc]);
	f_cp(xfpath, fpath, O_TRUNC);

	setbdir(fpath, xboard);
	rec_add(fpath, &xfile, sizeof(fileheader));
	setbtotal(bid);

	if (!xfile.filemode)
	  outgo_post(&xfile, xboard);

	pressanykey("�������");
      }
    }
  }
  enews_head(kind, cc);
}


static int
enews_forward(kind, cc)		/* ���ѳ�g��H */
  char kind;
  int cc;
{
  fileheader fhdr;
  char fname[PATHLEN];
  
  strcpy(fhdr.filename, xname[cc]);
  strcpy(fhdr.owner, "[�_���s�D]");
  strcpy(fhdr.title, title[cc]);
  
  enews_fpath(fname, kind, xname[cc]);
  
  mail_forward(&fhdr, fname, 'F');
  
  enews_head(kind, cc);
  return RC_NONE;
}


/*-------------------------------------------------------*/
/* �D�{��						 */
/*-------------------------------------------------------*/


int
main_enews()
{
  int cc, ch;
  char kind;

  kind = 'A';			/* ���� */
  cc = 0;			/* �ĴX�g */

  enews_load(kind);
  enews_head(kind, cc);

  while (ch = igetkey())
  {
    if (ch >= 'a' && ch <= 'l')
    {
      kind = ch - 32;		/* �ܤj�g */
      cc = 0;
      enews_load(kind);
      enews_neck(kind, cc);
      continue;
    }

    if (ch >= '1' && ch <= '9')
    {
      char ans[5];

      ans[0] = ch;
      ans[1] = '\0';
      getdata(b_lines, 0, "���ܲĴX���G", ans, 4, DOECHO, "");
      move(b_lines, 0);
      clrtoeol();
      outs(FEETER_ENEWS);	/* ��ø feet */

      ch = atoi(ans);
      if (ch > 0 && ch <= maxitem)
      {
	enews_item(cc, 12345);	/* ��ø��Ө��� */
	cc = ch - 1;
	enews_item(cc, cc);
      }
      continue;
    }

    switch (ch)
    {
    case 'q':
    case 'e':
      return 0;

    case KEY_RIGHT:
      kind = kind < 'L' ? kind + 1 : 'A';
      cc = 0;
      enews_load(kind);
      enews_neck(kind, cc);
      break;

    case KEY_LEFT:
      kind = kind > 'A' ? kind - 1 : 'L';
      cc = 0;
      enews_load(kind);
      enews_neck(kind, cc);
      break;

    case KEY_DOWN:
      enews_item(cc, 12345);	/* ��ø��Ө��� */
      cc = cc < maxitem - 1 ? cc + 1 : 0;
      enews_item(cc, cc);
      break;

    case KEY_UP:
      enews_item(cc, 12345);	/* ��ø��Ө��� */
      cc = cc > 0 ? cc - 1 : maxitem - 1;
      enews_item(cc, cc);
      break;

    case '\r':
    case '\n':
      cc = enews_browse(kind, cc);
      enews_head(kind, cc);
      break;

    case '0':
    case KEY_HOME:
      if (cc != 0)
      {
	enews_item(cc, 12345);	/* ��ø��Ө��� */
	cc = 0;
	enews_item(cc, cc);
      }
      break;

    case KEY_END:
      if (cc != maxitem - 1)
      {
	enews_item(cc, 12345);	/* ��ø��Ө��� */
	cc = maxitem - 1;
	enews_item(cc, cc);
      }
      break;

    case 'x':
      enews_cross(kind, cc);
      break;

    case 'F':
      if (enews_forward(kind, cc) == RC_FOOT)
      {
	move(b_lines, 0);
	clrtoeol();
	outs(FEETER_ENEWS);
      }
      break;
    }
  }
}

#endif				/* HAVE_NETTOOL */
