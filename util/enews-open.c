/*-------------------------------------------------------*/
/* util/enews-open.c	( NTHU CS MapleBBS Ver 3.10 )	 */
/*-------------------------------------------------------*/
/* target : 奇摩新聞更新				 */
/* create : 02/01/29				     	 */
/* update :   /  /				       	 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#if 0

 本奇摩新聞更新程式需要配合 contab 去抓資料，crontab 如下：

# 每天 10:30AM 4:30PM 抓奇摩新聞
30 10,16 * * * bin/enews-open
 
 要使用 enews-open.c 必須裝有 lynx，注意 lynx 的路徑要和程式吻合。

 文章檔案放在 run/kimo/

 資料來源：Yahoo! 奇摩新聞 http://tw.news.yahoo.com/
 
#endif
  

#include "bbs.h"
#include "enews.h"


#ifdef HAVE_NETTOOL

/*-------------------------------------------------------*/
/* 轉錄文章					     	 */
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
    printf("%s 看板不存在, 請先建立!\n", BRD_ENEWS);
    return ;
  }
  
  sprintf(fname, "run/kimo/%c/%s", enews->kind, enews->xname);
  
  setbpath(xpath, BRD_ENEWS);
  stampfile(xpath, &fhdr);
  unlink(xpath);
  
  strcpy(fhdr.owner, "[奇摩新聞]");
  snprintf(fhdr.title, TTLEN, "[%s] %s", class[enews->kind - 'A'], enews->title);
  
  f_cp(fname, xpath, O_TRUNC);
  
  setbfile(xpath, BRD_ENEWS, ".DIR");
  rec_add(xpath, &fhdr, sizeof(fileheader));
  
  touchbtotal(bid);
  return;
}

/*-------------------------------------------------------*/
/* 分析 html					     	 */
/*-------------------------------------------------------*/

static int wlen;	/* 本行有多少字 */
static int slen;	/* 本行有多少半形字 */

static void
foutc(ch, fp)
  int ch;
  FILE *fp;
{
  static int in_tag = 0;	/* 1: 在 <tag> 中 */
  static int in_chi = 0;	/* 1: 前一碼是中文字 */

  if (ch == '<')
  {
    in_tag = 1;
    return;
  }

  if (!in_tag)
  {
    if (in_chi)			/* 前一個char是中文字的第一碼 */
      in_chi = 0;
    else if (ch & 0x80)		/* 前一個char不是中文字的第一碼，檢查這char是否為中文字的第一碼 */
      in_chi = 1;
    else			/* 如果都不是，表示這char是半形字 */
      slen++;

    if (wlen >= 60 - slen % 2)	/* 一行最多 60 字，若有奇數個半形字，該行只印 59 字 */
    {
      fputs("\n    ", fp);	/* 每行前面都空四格 */
      wlen = 0;
      /* slen = 0; */
      slen = !in_chi;		/* 若新的這行第一個char是半形字，slen=1 */
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
  fputs("\n    ", fp);		/* 每行前面都空四格 */

  while (ch = *str)
  {
    foutc(ch, fp);
    str++;
  }
}


static void
html_download(enews)		/* 下載文章並轉換為文字檔 */
  ENEWS *enews;
{
  char *strS, *strE;
  char buf[2048];	/* 假設文章每段不會超過 2048 字 */
  FILE *fpr, *fpw;
  int article_begin;

  /* 下載文章 */
  sprintf(buf, LYNX_PATH " %s > tmp/kimo_news", enews->link);
  system(buf);

  /* 轉換為文字檔 */
  if (fpr = fopen("tmp/kimo_news", "r"))
  {
    sprintf(buf, "run/kimo/%c/%s", enews->kind, enews->xname);
    if (fpw = fopen(buf, "w"))
    {
      /* 開頭加上標題 */
      fprintf(fpw, "%s %s (%s) %s %s\n", STR_AUTHOR1, STR_SYSOP, BOARDNAME, STR_POST2, "奇摩新聞");
      fprintf(fpw, "標題: %s\n時間: %s\n", enews->title, Now());

      /* html -> text */
      article_begin = 0;
      while (fgets(buf, sizeof(buf), fpr))
      {
	if (!article_begin)		/* 文章開始 */
	{
	  if (strstr(buf, "<!--end") && strstr(buf, "標題寄給朋友列印"))
	    article_begin = 1;
	  continue;
	}

	/* 文章結束 */
	if (strstr(buf, "<div style=\042clear:both;display:none"))
	  break;

	if ((strS = strstr(buf, "<big>")) && (strE = strstr(strS + 5, "</big>")))	/* 本文部分 */
	{
	  *strE = '\n';
	  *(strE + 1) = '\0';
	  fouts(strS, fpw);
	}
      }

      /* 結尾加上來源 */
      fprintf(fpw, "\n--\nYahoo!奇摩新聞\n%s\n\n\n", enews->link);
      fclose(fpw);
    }

    fclose(fpr);
    unlink("tmp/kimo_news");	/* 清除暫存檔 */
  }
}


static void
html_fetch(fpath, kind)	/* 將這檔案中有效的連結找出來 */
  char *fpath;
  char kind;		/* 種類 */
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
  unlink(folder);	/* 重建一個新的 */

#if 0
  處理 HTML 格式如下：
  <a href="/040520/44/nwtu.html" class="title">北市兩公車對撞  13名乘客送醫<!-- 20040520 --></a>
#endif

  article_num = 0;

  while (fgets(buf, 1024, fp) && article_num <= 16)
  {
    if ((str1 = strstr(buf, "<a href=\"/")) &&		/* 只處理連結 */
      (str2 = strstr(str1 + 10, "\" class=\"title\">")) &&
      (str3 = strstr(str2 + 16, "<!-- ")))
    {
      *str2 = '\0';
      *str3 = '\0';

      /* 加入 record */

      memset(&enews, 0, sizeof(ENEWS));
      enews.chrono = ++chrono;		/* 每篇文章一個編號 */
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
/* 主程式						 */
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
    printf("run/kimo 不存在, 建立\n");
    mkdir("run/kimo/", 0755);
  }

  /* 抓資料並分析之 */

  for (kind = 'A'; kind <= 'L'; kind++)
  {
    sprintf(cmd, LYNX_PATH " http://tw.news.yahoo.com/%s > tmp/kimo_index", class[kind - 'A']);
    system(cmd);

    sprintf(cmd, "run/kimo/%c/", kind);
    if(!dashd(cmd))
    {
      printf("%s 不存在, 建立\n", cmd);
        mkdir(cmd, 0755);
    }
      
    html_fetch("tmp/kimo_index", kind);
    unlink("tmp/kimo_index");		/* 清除暫存檔 */
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
