/*-------------------------------------------------------*/
/* util/bbsmail.c	( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : 由 Internet 寄信給 BBS 看板                  */
/* create : 95/03/29                                     */
/* update : 99/01/19                                     */
/*-------------------------------------------------------*/
#include "bbs.h"
#include "cache.c"
#include "record.c"

#define	LOG_FILE	("log/bbspost.log")

char *fn_board = FN_BOARD;
extern boardheader *bcache;
extern int numboards;

void
outgo_post(fileheader * fh, char *board)
{
  char buf[256];
  sprintf(buf, "%s\t%s\t%s\t%s\t%s", board,
	  fh->filename, fh->owner, "轉出", fh->title);
  f_cat("innd/out.bntp", buf);
}

void
postlog(char *msg)
{
  FILE *fp;

  if (fp = fopen(LOG_FILE, "a"))
  {
    time_t now;
    struct tm *p;

    time(&now);
    p = localtime(&now);
    fprintf(fp, "%02d/%02d/%02d %02d:%02d:%02d <bbspost> %s\n",
	    p->tm_year % 100, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec,
	    msg);
    fclose(fp);
  }
}

int
post2bbs(char *bid)
{
  fileheader mypost;
  boardheader bh;
  char genbuf[256], title[80], sender[80], *ip, *ptr, buf[80];
  time_t tmp_time;
  struct stat st;
  FILE *fout;
  int  bnum;

  if (!(bnum = getbnum(bid)))
  {
    sprintf(buf, "No Such board [%s]", bid);
    postlog(buf);
    return -1;
  }

  rec_get(fn_board, &bh, sizeof(boardheader), bnum);

  if (bh.level > PERM_LOGINOK || bh.brdattr & BRD_HIDE)
  {
    sprintf(buf, "Permission denied - [%s]", bh.brdname);
    postlog(buf);
    return -1;
  }

  setbpath(genbuf, bh.brdname);
  printf("dir: %s\n", genbuf);

  /* allocate a file for the new mail */

  stampfile(genbuf, &mypost);
  printf("file: %s\n", genbuf);

  /* copy the stdin to the specified file */

  if ((fout = fopen(genbuf, "w")) == NULL)
  {
    printf("Cannot open %s\n", genbuf);
    return -1;
  }

  /* parse header */

  while (fgets(genbuf, sizeof(genbuf), stdin))
  {
    if (!strncmp(genbuf, "From: ", 6))
    {
      char mynick[128], myaddr[128];

      /* lib/str_decode 只能接受 decode 完 strlen < 256*/
      strlcpy(sender, genbuf + 6, 256);
      str_decode(sender);
      str_ansi(genbuf, sender, 128);

      str_from(genbuf, myaddr, mynick);

      if ((*mynick) != '\0')
	sprintf(sender, "%s (%s)", myaddr, mynick);
      else
	strcpy(sender, myaddr);

      continue;
    }
    if (!strncmp(genbuf, "Subject: ", 9))
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

  if (!title[0])
    sprintf(title, "來自 %.64s", sender);

  fprintf(fout, "作者: %s 看板: %s\n標題: %s\n時間: %s\n",
	  sender, bh.brdname, title, ctime(&tmp_time));

  while (fgets(genbuf, sizeof(genbuf), stdin))
  {
    str_decode(genbuf);
    fputs(genbuf, fout);
  }

  fclose(fout);

  sprintf(genbuf, "%s => %s", sender, bh.brdname);
  postlog(genbuf);

  /* append the record to the MAIL control file */

  strncpy(mypost.title, title, 72);

  if (strtok(sender, " .@\t\n\r"))
    strcat(sender, ".");
  sender[IDLEN + 1] = '\0';
  strcpy(mypost.owner, sender);

  if (!(bh.brdattr & BRD_NOTRAN))
    outgo_post(&mypost, bh.brdname);
  setbfile(genbuf, bh.brdname, ".DIR");
  rec_add(genbuf, &mypost, sizeof(mypost));

  touchbtotal(bnum);
  return 0;
}


int
main(int argc, char *argv[])
{
  char receiver[256];

  /* argv[1] is userid in bbs   */

  if (argc < 2)
  {
    printf("Usage:\t%s <bbs_bid>\n", argv[0]);
    return -1;
  }

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  strcpy(receiver, argv[1]);

  post2bbs(receiver);


  /* eat mail queue ?? */
  /*
   * if (post2bbs(receiver)) { while (fgets(receiver, sizeof(receiver),
   * stdin)); }
   */
  return 0;
}
