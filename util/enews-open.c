/*-------------------------------------------------------*/
/* util/enews-open.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : �_���s�D��s				 */
/* create : 02/01/29				     	 */
/* update :   /  /				       	 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

 ���_���s�D��s�{���ݭn�t�X contab �h���ơAcrontab �p�U�G

# �C�� 10:30AM 4:30PM ��_���s�D
30 10,16 * * * bin/enews-open
 
 �n�ϥ� enews-open.c �����˦� lynx�A�`�N lynx �����|�n�M�{���k�X�C

 �峹�ɮש�b run/kimo/

 ��ƨӷ��GYahoo! �_���s�D http://tw.news.yahoo.com/
 
#endif
  

#include "bbs.h"
#include "enews.h"


#ifdef HAVE_NETTOOL

/*-------------------------------------------------------*/
/* ����峹					     	 */
/*-------------------------------------------------------*/
#include "cache.c"
#include "record.c"

#define BRD_ENEWS "KimoNews"

static void
cross2board(ENEWS *enews)
{
  char xpath[PATHLEN], fname[PATHLEN];
  fileheader fhdr;
  int bid;
  
  if(!(bid = getbnum(BRD_ENEWS)))
  {
    printf("%s �ݪO���s�b, �Х��إ�!\n", BRD_ENEWS);
    return ;
  }
  
  sprintf(fname, "run/kimo/%c/%s", enews->kind, enews->xname);
  
  setbpath(xpath, BRD_ENEWS);
  stampfile(xpath, &fhdr);
  unlink(xpath);
  
  strcpy(fhdr.owner, "[�_���s�D]");
  snprintf(fhdr.title, TTLEN, "[%s] %s", class[enews->kind - 'A'], enews->title);
  
  f_cp(fname, xpath, O_TRUNC);
  
  setbfile(xpath, BRD_ENEWS, ".DIR");
  rec_add(xpath, &fhdr, sizeof(fileheader));
  
  touchbtotal(bid);
  return;
}

/*-------------------------------------------------------*/
/* ���R html					     	 */
/*-------------------------------------------------------*/

static int wlen;	/* ���榳�h�֦r */
static int slen;	/* ���榳�h�֥b�Φr */

static void
foutc(ch, fp)
  int ch;
  FILE *fp;
{
  static int in_tag = 0;	/* 1: �b <tag> �� */
  static int in_chi = 0;	/* 1: �e�@�X�O����r */

  if (ch == '<')
  {
    in_tag = 1;
    return;
  }

  if (!in_tag)
  {
    if (in_chi)			/* �e�@��char�O����r���Ĥ@�X */
      in_chi = 0;
    else if (ch & 0x80)		/* �e�@��char���O����r���Ĥ@�X�A�ˬd�ochar�O�_������r���Ĥ@�X */
      in_chi = 1;
    else			/* �p�G�����O�A��ܳochar�O�b�Φr */
      slen++;

    if (wlen >= 60 - slen % 2)	/* �@��̦h 60 �r�A�Y���_�ƭӥb�Φr�A�Ӧ�u�L 59 �r */
    {
      fputs("\n    ", fp);	/* �C��e�����ť|�� */
      wlen = 0;
      /* slen = 0; */
      slen = !in_chi;		/* �Y�s���o��Ĥ@��char�O�b�Φr�Aslen=1 */
    }

    fputc(ch, fp);
    wlen++;
  }
  else
  {
    if (ch == '>')
      in_tag = 0;
  }
}


static void
fouts(str, fp)
  uschar *str;
  FILE *fp;
{
  int ch;

  wlen = 0;
  slen = 0;
  fputs("\n    ", fp);		/* �C��e�����ť|�� */

  while (ch = *str)
  {
    foutc(ch, fp);
    str++;
  }
}


static void
html_download(enews)		/* �U���峹���ഫ����r�� */
  ENEWS *enews;
{
  char *strS, *strE;
  char buf[2048];	/* ���]�峹�C�q���|�W�L 2048 �r */
  FILE *fpr, *fpw;
  int article_begin;

  /* �U���峹 */
  sprintf(buf, LYNX_PATH " %s > tmp/kimo_news", enews->link);
  system(buf);

  /* �ഫ����r�� */
  if (fpr = fopen("tmp/kimo_news", "r"))
  {
    sprintf(buf, "run/kimo/%c/%s", enews->kind, enews->xname);
    if (fpw = fopen(buf, "w"))
    {
      /* �}�Y�[�W���D */
      fprintf(fpw, "%s %s (%s) %s %s\n", STR_AUTHOR1, STR_SYSOP, BOARDNAME, STR_POST2, "�_���s�D");
      fprintf(fpw, "���D: %s\n�ɶ�: %s\n", enews->title, Now());

      /* html -> text */
      article_begin = 0;
      while (fgets(buf, sizeof(buf), fpr))
      {
	if (!article_begin)		/* �峹�}�l */
	{
	  if (strstr(buf, "<!--end") && strstr(buf, "���D�H���B�ͦC�L"))
	    article_begin = 1;
	  continue;
	}

	/* �峹���� */
	if (strstr(buf, "<div style=\042clear:both;display:none"))
	  break;

	if ((strS = strstr(buf, "<big>")) && (strE = strstr(strS + 5, "</big>")))	/* ���峡�� */
	{
	  *strE = '\n';
	  *(strE + 1) = '\0';
	  fouts(strS, fpw);
	}
      }

      /* �����[�W�ӷ� */
      fprintf(fpw, "\n--\nYahoo!�_���s�D\n%s\n\n\n", enews->link);
      fclose(fpw);
    }

    fclose(fpr);
    unlink("tmp/kimo_news");	/* �M���Ȧs�� */
  }
}


static void
html_fetch(fpath, kind)	/* �N�o�ɮפ����Ī��s����X�� */
  char *fpath;
  char kind;		/* ���� */
{
  static int chrono = 0;  
  FILE *fp;
  char folder[64], buf[1024];
  char *str1, *str2, *str3;
  ENEWS enews;
  int article_num;

  if (!(fp = fopen(fpath, "r")))
    return;

  sprintf(folder, "run/kimo/%c/.ENEWS", kind);
  unlink(folder);	/* ���ؤ@�ӷs�� */

#if 0
  �B�z HTML �榡�p�U�G
  <a href="/040520/44/nwtu.html" class="title">�_���⤽���Ｒ  13�W���Ȱe��<!-- 20040520 --></a>
#endif

  article_num = 0;

  while (fgets(buf, 1024, fp) && article_num <= 16)
  {
    if ((str1 = strstr(buf, "<a href=\"/")) &&		/* �u�B�z�s�� */
      (str2 = strstr(str1 + 10, "\" class=\"title\">")) &&
      (str3 = strstr(str2 + 16, "<!-- ")))
    {
      *str2 = '\0';
      *str3 = '\0';

      /* �[�J record */

      memset(&enews, 0, sizeof(ENEWS));
      enews.chrono = ++chrono;		/* �C�g�峹�@�ӽs�� */
      enews.kind = kind;
      sprintf(enews.xname, "A%06d%c", chrono, kind);
      sprintf(enews.link, "http://tw.news.yahoo.com/%s", str1 + 10);
      str_ncpy(enews.title, str2 + 16, sizeof(enews.title));
      if (!rec_add(folder, &enews, sizeof(ENEWS)))
	html_download(&enews);

      article_num++;
      
      cross2board(&enews);
    }
  }

  fclose(fp);
}


/*-------------------------------------------------------*/
/* �D�{��						 */
/*-------------------------------------------------------*/


int
main()
{
  char kind;
  char cmd[256];

  char *class[12] = 
  {
    "polity", "society", "international", "twoshore", 
    "finance", "entertain", "sports", "leisure", 
    "relaxation", "health", "technology", "odd"
  };

  chdir(BBSHOME);
  
  if(!dashd("run/kimo/"))
  {
    printf("run/kimo ���s�b, �إ�\n");
    mkdir("run/kimo/", 0755);
  }

  /* ���ƨä��R�� */

  for (kind = 'A'; kind <= 'L'; kind++)
  {
    sprintf(cmd, LYNX_PATH " http://tw.news.yahoo.com/%s > tmp/kimo_index", class[kind - 'A']);
    system(cmd);

    sprintf(cmd, "run/kimo/%c/", kind);
    if(!dashd(cmd))
    {
      printf("%s ���s�b, �إ�\n", cmd);
        mkdir(cmd, 0755);
    }
      
    html_fetch("tmp/kimo_index", kind);
    unlink("tmp/kimo_index");		/* �M���Ȧs�� */
  }

  return 0;
}

#else
int
main()
{
  printf("You should define HAVE_NETTOOL first.\n");
  return -1;
}
#endif	/* HAVE_NETTOOL */
