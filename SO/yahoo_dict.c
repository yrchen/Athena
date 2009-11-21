/*-------------------------------------------------------*/
/* dreye.c    (YZU WindTopBBS Ver 3.02 )                 */
/*-------------------------------------------------------*/
/* author : statue.bbs@bbs.yzu.edu.tw                    */
/* change : hialan.bbs@venice.twbbs.org                  */
/* target : Yahoo線上字典                                */
/* create : 01/07/09                                     */
/*-------------------------------------------------------*/
/*
普通
http://tw.dictionary.yahoo.com/word/hello
片語
http://tw.dictionary.yahoo.com/word/hello?t=i
同義字/反義字
http://tw.dictionary.yahoo.com/word/hello?t=s
*/
#include "bbs.h"

#define mouts(y,x,s)    { move(y,x); outs(s); }

#define HTTP_PORT       80
#define SERVER_yahoo    "tw.dictionary.yahoo.com"
#define CGI_yahoo       "/word/"
#define REF             "http://tw.dictionary.yahoo.com/"

static void
write_file(int sockfd, FILE *fp)
{
  static char pool[2048];
  int cc, i;
  char *xhead, *xtail;
  int show, start_show;
  int space;	//在 html 中，連續的 space 只會算一次

  char *start_str[]={
        "<!-- view content_start-->",
        "<!-- end show top-->",
        "<!--end 650 footer-->",
        NULL};  		

  char *stop_str[]={
        "<!-- show top-->",
        "<!--begin 650 footer-->",
        NULL};

  char *newline_str[]={		//取代換行字元的符號
        "<br>",
        "</td>",
        "</li>",
        "<blockquote>",
        NULL};
  /* parser return message from web server */
  xhead = pool;
  xtail = pool;
  show = 1;
  start_show=0;
  space = 0;

  for (;;xhead++)
  {
    if (xhead >= xtail)
    {
      xhead = pool;
      cc = read(sockfd, xhead, sizeof(pool));
      if (cc <= 0)
        break;
      xtail = xhead + cc;
    }

    if(!start_show)
    {
      for(i=0;start_str[i] != NULL;i++)
      {
        if(!strncasecmp(xhead, start_str[i], strlen(start_str[i])))
        {
          start_show = 1;
          xhead+=strlen(start_str[i]);
          break;
        }
      }
    }
    else if(start_show)
    {
      for(i=0;stop_str[i] != NULL;i++)
      {
        if(!strncasecmp(xhead, stop_str[i], strlen(stop_str[i])))
        {
          start_show = 0;
          xhead+=strlen(stop_str[i]);
          break;
        }
      }
    }    
    
    if(!start_show)
      continue;
      
    for(i=0;newline_str[i] != NULL;i++)
    {
      if(!strncasecmp(xhead, newline_str[i], strlen(newline_str[i])))
      {
        fputc('\n', fp);
        xhead+=strlen(newline_str[i]);
        space=0;
        break;
      }
    }

    if(!strncasecmp(xhead, "&nbsp;", 6))
    {
      fputc(' ', fp);
      xhead+=6;
      space=0;
    }

    if(!strncasecmp(xhead, "&#169;", 6))
    {
      fputs("(C)", fp);
      xhead+=6;
      space=0;
    }
    
    if(!strncasecmp(xhead, "<li>", 4))
    {
      fputs("  ˙", fp);
      xhead+=4;
      space=0;
    }

    /* 標籤略過 */

    cc = *xhead;
    switch(cc)
    {
    case '<':
      show = 0;
      continue;
    case '>':
      show = 1;
      continue;
    case '\n':
    case '\r':
      continue;
    case ' ':
      if(space)
        continue;
      space = 1;
    }
    
    if(show)
      fputc(cc, fp);

    if(cc != ' ')
      space = 0;    
  }
  fputc('\n', fp);
}

static int
http_conn(char *server, char *s)
{
  int sockfd;
  FILE *fp;
  char fname[PATHLEN], *str;

  if ((sockfd = dns_open(server, HTTP_PORT)) < 0)
  {
    pressanykey("無法與伺服器取得連結，查詢失敗");
    return 0;
  }
  else
  {
    mouts(22, 0, "正在連接伺服器，請稍後(按任意鍵離開).............");
    refresh();
  }
  write(sockfd, s, strlen(s));
  shutdown(sockfd, 1);

  /* usr_fpath(fname, cuser.userid,Msg_File); */
  sprintf(fname, BBSHOME"/tmp/%s.yahoo_dict", cuser.userid);

  fp = fopen(fname, "w");

  str = strchr(s+4, ' ');
  
  if(str)
    *str = '\0';
  fprintf(fp, "該頁連結: http://%s%s\n\n", server, s+4);
  
  write_file(sockfd, fp);
  fclose(fp);

  close(sockfd);
  more(fname);
  unlink(fname);
  return 0;
}

static int
yahoo_dict(char *word, char *ans)
{
  char atrn[256], sendform[512];
  char ue_word[90];
  char d;

  url_encode(ue_word, word);

  if(ans[0] == '3')
    d = 'i';
  else if(ans[0] == '2')
    d = 's';
  else
    d = '\0';

  if(d)
    sprintf(atrn, "%s?t=%c", ue_word, d);
  else
    sprintf(atrn, "%s", ue_word);

  sprintf(sendform, "GET %s%s HTTP/1.0\n\n", CGI_yahoo, atrn);

  http_conn(SERVER_yahoo, sendform);
  return 0;
}

int 
main_dict()
{
  char ans[2];
  char word[30];
  int mode0=currutmp->mode;
  char *choose[4]={"11)意義", "22)片語", "33)同意字/反義字", msg_choose_cancel};

  setutmpmode(DICT);
  ans[0] = '1';
  do 
  {
    clear();
    move(0, 23);
    outs("\033[1;37;44m◎ Yahoo! 線上字典 v0.1 ◎\033[m");
    move(3, 0);
    outs("此字典來源為 Yahoo! 線上字典。\n");
    prints("WWW: %s\n", REF);
    outs("author: statue.bbs@bbs.yzu.edu.tw\n");
    outs("移植WD: gorilla.bbs@bbs.thitsrc.net");
    getdata(8, 0, "word: ", word, 30, LCECHO,0);

    if(word[0] == '\0')
      break;

    ans[0] = getans2(9, 0, "", choose, 4, '1');

    if(ans[0] != 'q')
      yahoo_dict(word, ans);
  } while(ans[0] != 'q');
  
  currutmp->mode = mode0;
  return 0;
}
