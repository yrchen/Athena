/*-------------------------------------------------------*/
/* util/bbsmail.c       ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : 由 Internet 寄信給 BBS 站內使用者            */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"
#include "cache.c"
#include "record.c"

#undef SPAM_
#ifdef SPAM_
  #define MAX_SPAM_WORD 5	/* 非法字忍受度 -- 最高可以出現幾次 */
#endif

#define LOG_FILE       "log/bbsmail.log"

char userid[IDLEN + 1];

/*copy from list.c by hialan 20020602*/
static int
belong_list(char *fname, char *userid)
{
  int fd, can = 0;
  PAL pal;
                                                                                
  if ((fd = open(fname, O_RDONLY)) >= 0)
  {
    while (read(fd, &pal, sizeof(pal)) == sizeof(pal))
    {
      if (!strcmp(pal.userid, userid))
      {
        if (pal.savemode & M_BAD)
          can = 0;
        else
          can = 1;
        break;
      }
    }
    close(fd);
  }
  return can;
}

#ifdef SPAM_
/* Dopin: HTML/Attachment check */
static int
check_content(char *str) 
{
  char *bad[] = 
  {
    "<HTML>",
    "<BR>",
    "<P>",
    "<a href",
    "<img ",
    "Content-Type",
    "text/html",
    "text/plain",
    "Content-disposition",
    "<script",
    "-=_NextPart_",
    "<body",
    "</HTML>",
    NULL};
                                                                                
  int i;
  static int score=0;
                                                                                
  for(i = 0 ; bad[i] ; i++) 
    if(strcasestr(str, bad[i])) 
      score++;
  
  if(score >= MAX_SPAM_WORD)
    return 1;
  else
    return 0;
}
#endif

static int
belong_spam(char *filelist, char *key)
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[STRLEN], *ptr;

    while (fgets(buf, STRLEN, fp))
    {
      if(buf[0] == '#') continue;
      if ((ptr = strtok(buf, " \t\n\r")) && strstr(key, ptr))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}

static void
mailog(char* mode, char* msg)
{
   FILE *fp;

   if (fp = fopen(LOG_FILE, "a")) 
   {
      time_t now;
      struct tm *p;

      time(&now);
      p = localtime(&now);
      fprintf(fp, "%02d/%02d/%02d %02d:%02d:%02d <%s> %s\n",
         p->tm_year % 100, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min,
         p->tm_sec, mode, msg);
      fclose(fp);
   }
}

static int
junk(char* reason)
{
   FILE* fp;
   int i;
   char msgbuf[256];

   sprintf(msgbuf, "<%s> %.100s", userid, reason);
   mailog("bbsmail", msgbuf);
   if (fp = popen("bin/bbspost junk", "w")) 
   {
      while (fgets(msgbuf, sizeof(msgbuf), stdin))
        fputs(msgbuf, fp);
      pclose(fp);
   }
   return 0;
}

static int
mail2bbs()
{
   fileheader mymail;
   char genbuf[2000], title[256], sender[256], *ip, *ptr;
   char fname[200];
   char firstline[100];
   struct stat st;
   time_t tmp_time;
   FILE *fout;

   /* check if the userid is in our bbs now */
   if (!searchuser(userid))
      return junk("not exist");

   sethomepath(genbuf, userid);

   if (!dashd(genbuf)) 
   {
      if (mkdir(genbuf, 0755) == -1)
        return junk("no mail box");
      else
        return junk("mail box error");
   }

   /* allocate a file for the new mail */

   stampfile(genbuf, &mymail);
   strcpy(fname, genbuf);

   /* copy the stdin to the specified file */

   if ((fout = fopen(genbuf, "w")) == NULL)
      return junk("can't open stamp file");

   /* parse header */

   title[0] = sender[0] = '\0';

   if (ip = getenv("SENDER"))
      strlcpy(sender, ip, sizeof(sender));

   while (fgets(genbuf, sizeof(genbuf), stdin)) 
   {
      if (!*sender && !strncasecmp(genbuf, "From: ", 6)) 
      {
         char mynick[128], myaddr[128];
         
         strlcpy(sender, genbuf+6, 256);  /* lib/str_decode 只能接受 decode 完 strlen < 256*/
         str_decode(sender);
         str_ansi(genbuf, sender, 128);
         
         str_from(genbuf, myaddr, mynick);

         if((*mynick) != '\0')
           sprintf(sender, "%s (%s)", myaddr, mynick);
         else
           strcpy(sender, myaddr);

         continue;
      }
      if (!memcmp(genbuf, "Subject: ", 9)) 
      {
         char tmp[256];
         
         strncpy(tmp, genbuf + 9, 255);
         str_decode(tmp);
         str_ansi(title, tmp, 128);
         
         continue;
      }
      if (genbuf[0] == '\n')
         break;
   }

   time(&tmp_time);
   if (fgets(genbuf, sizeof(genbuf), stdin))
      strlcpy(firstline, genbuf, sizeof(firstline));
      
   if (!title[0])
     sprintf(title, "來自 %.64s", sender);

   fprintf(fout, "作者: %s\n標題: %s\n時間: %s\n",
      sender, title, ctime(&tmp_time));


   fputs(genbuf, fout);
   while (fgets(genbuf, sizeof(genbuf), stdin) != NULL)
   { 
     str_decode(genbuf);
     fputs(genbuf, fout);

#ifdef SPAM_
     if(check_content(genbuf)) 
     {
       fclose(fout);
       unlink(fname);
       sprintf(genbuf, "SPAM-MAIL: form:%s title:%s", sender, title);
       mailog("bbsmail", genbuf);
       return -1;
     }
#endif
   }
   fclose(fout);

/*==== wildcat : anti-spam from etc/spam-list & user define spam=====*/
   sethomefile(genbuf, userid, "spam-list");
   if(belong_spam(BBSHOME"/etc/spam-list",sender) || belong_list(genbuf, sender))
   {
     unlink(fname);
     sprintf(genbuf,"SPAM: %s => %s",sender,userid);
     mailog("bbsmail", genbuf);
     return 0;
   }
       
   sprintf(genbuf, "%s => %s", sender, userid);
   mailog("bbsmail", genbuf);

   /* append the record to the MAIL control file */

   strncpy(mymail.title, title, 72);

   if (strtok(sender, " .@\t\n\r"))
     strcat(sender, ".");
   sender[IDLEN] = '\0';
   strcpy(mymail.owner, sender);
   sethomedir(genbuf, userid);
   return rec_add(genbuf, &mymail, sizeof(mymail));
}


int
main(int argc, char** argv)
{
   if (argc < 2) 
   {
      printf("Usage:\t%s <bbs_uid>\n", argv[0]);
      exit(-1);
   }
   setgid(BBSGID);
   setuid(BBSUID);
   chdir(BBSHOME);
   strlcpy(userid, argv[1], sizeof(userid));

   return mail2bbs();
}
