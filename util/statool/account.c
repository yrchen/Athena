/*-------------------------------------------------------*/
/* util/account.c       ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : ¤W¯¸¤H¦¸²Î­p¡B¨t²Î¸ê®Æ³Æ¥÷                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

#include "record.c"

#define MAX_LINE        16
#define ADJUST_M        6	/* adjust back 5 minutes */

/* Ptt about share memory */
struct UCACHE *uidshm;
struct FROMCACHE *fcache;

static void
attach_err(shmkey, name)
  int shmkey;
  char *name;
{
  fprintf(stderr, "[%s error] key = %x\n", name, shmkey);
  exit(1);
}
static void *
attach_shm(shmkey, shmsize)
  int shmkey, shmsize;
{
  void *shmptr;
  int shmid;

  shmid = shmget(shmkey, shmsize, 0);
  if (shmid < 0)
  {
    shmid = shmget(shmkey, shmsize, IPC_CREAT | 0600);
    if (shmid < 0)
      attach_err(shmkey, "shmget");
    shmptr = (void *)shmat(shmid, NULL, 0);
    if (shmptr == (void *)-1)
      attach_err(shmkey, "shmat");
    memset(shmptr, 0, shmsize);
  }
  else
  {
    shmptr = (void *)shmat(shmid, NULL, 0);
    if (shmptr == (void *)-1)
      attach_err(shmkey, "shmat");
  }
  return shmptr;
}
void
resolve_ucache()
{
  if (uidshm == NULL)
  {
    uidshm = attach_shm(UIDSHM_KEY, sizeof(*uidshm));
  }
}

void
resolve_fcache()
{
  if (fcache == NULL)
  {
    fcache = attach_shm(FROMSHM_KEY, sizeof(*fcache));
    if (fcache->touchtime == 0)
      fcache->touchtime = 1;
  }
  fcache->uptime = 0;
}

void
keeplog(char *fpath, char *board, char *title)
{
  fileheader fhdr;
  char genbuf[256];
  char *flog;

  if (!board)
    board = "Record";

  setbpath(genbuf, board);
  stampfile(genbuf, &fhdr);
  f_mv(fpath, genbuf);

  strcpy(fhdr.title, title);
  strcpy(fhdr.owner, "[¾ú¥vªº¤µ¤Ñ]");
  setbfile(genbuf, board, ".DIR");
  rec_add(genbuf, &fhdr, sizeof(fhdr));
}


void
outs(fp, buf, mode)
  FILE *fp;
  char buf[], mode;
{
  static char state = '0';

  if (state != mode)
    fprintf(fp, "[3%cm", state = mode);
  if (buf[0])
  {
    fprintf(fp, buf);
    buf[0] = 0;
  }
}


void
gzip(char *source, char *target, char *stamp)
{
  char buf[128];
  sprintf(buf, "/bin/gzip -9n adm/%s%s", target, stamp);
  f_mv(source, &buf[14]);
  system(buf);
}

int
main()
{
  int hour, max, item, total, i, j, mo, da, max_user = 0, max_login = 0,
    max_reg = 0, mahour, k;
  char *act_file = ".act";
  char *log_file = "usies";
  char buf[256], buf1[256], *p;
  FILE *fp, *fp1;
  int act[27];			/* ¦¸¼Æ/²Ö­p®É¶¡/pointer */
  time_t now;
  struct tm *ptime;

  chdir(BBSHOME);

  now = time(NULL) - ADJUST_M * 60;	/* back to ancent */
  ptime = localtime(&now);
  memset(act, 0, sizeof(act));

  printf("¦¸¼Æ/²Ö­p®É¶¡\n");
  if ((ptime->tm_hour != 0) && (fp = fopen(act_file, "r")))
  {
    fread(act, sizeof(act), 1, fp);
    fclose(fp);
  }
  if ((fp = fopen(log_file, "r")) == NULL)
  {
    printf("cann't open usies\n");
    if (fp = fopen(act_file, "w"))
    {
      memset(act, 0, sizeof(act));
      fwrite(act, sizeof(act), 1, fp);
      fclose(fp);
    }
    return 1;
  }
  if (act[26])
    fseek(fp, (off_t) (act[26]), 0);

  while (fgets(buf, 256, fp))
  {
    hour = atoi(buf + 11);
    if (hour < 0 || hour > 23)
    {
      continue;
    }
    if (!strncmp(buf + 24, "ENTER", 5))
    {
      act[hour]++;
      continue;
    }
    if (p = (char *)strstr(buf + 43, "Stay:"))
    {
      if (hour = atoi(p + 5))
      {
	act[24] += hour;
	act[25]++;
      }
      continue;
    }
  }
  act[26] = ftell(fp);
  fclose(fp);
  for (i = max = total = 0; i < 24; i++)
  {
    total += act[i];
    if (act[i] > max)
    {
      max_user = max = act[i];
      mahour = i;
    }
  }
  item = max / MAX_LINE + 1;

  if (!ptime->tm_hour)
    keeplog("etc/today", "Record", "[¶®¨å·¹·½] ¤W¯¸¤H¦¸²Î­p");

  if ((fp = fopen("etc/today", "w")) == NULL)
  {
    printf("cann't open etc/today\n");
    return 1;
  }
  fprintf(fp, "\t\t\t[1m[33;46m ¨C¤p®É¤W¯¸¤H¦¸²Î­p [%02d/%02d/%02d] [40m\n\n", ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);
  for (i = MAX_LINE + 1; i > 0; i--)
  {
    strcpy(buf, "   ");
    for (j = 0; j < 24; j++)
    {
      max = item * i;
      hour = act[j];
      if (hour && (max > hour) && (max - item <= hour))
      {
	outs(fp, buf, '7');
	fprintf(fp, "%-3d", hour);
      }
      else if (max <= hour)
      {
	outs(fp, buf, '6');
	fprintf(fp, "¢i ");
      }
      else
	strcat(buf, "   ");
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "   [33m"
	  "0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21 22 23\n\n"
	  "\t[33mÁ`¦@¤W¯¸¤H¦¸¡G[37m%-9d[33m¥­§¡¨Ï¥Î¤H¼Æ¡G[37m%-5d[33m¥­§¡¨Ï¥Î®É¶¡¡G[37m %d [33m¤À[m \n"
	  ,total, total / 24, act[24] / act[25] + 1);
  fclose(fp);

  if (fp = fopen(act_file, "w"))
  {
    fwrite(act, sizeof(act), 1, fp);
    fclose(fp);
  }

  /* -------------------------------------------------------------- */

  sprintf(buf, "-%02d%02d%02d",
	  ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);

  now += ADJUST_M * 60;		/* back to future */



  /* Ptt ¾ú¥v¨Æ¥ó³B²z */
  if (fp = fopen("etc/history.data", "r"))
  {				/* ³Ì¦h¦P®É¤W½u */
    if (fscanf(fp, "%d %d %d %d", &max_login, &max, &max_reg, &k))
    {
      int a;
      resolve_fcache();
      printf("¦¹®É¬q³Ì¦h¦P®É¤W½u:%d ¹L¥h:%d\n", a = fcache->max_user, k);
      if (a > k)
      {
	time_t now = time(0);
	ptime = localtime(&fcache->max_time);
	if (fp1 = fopen("etc/history", "a"))
	{
	  fprintf(fp1,
		  "¡· ¡i%02d/%02d/%02d %02d:%02d¡j"
		  "[32m¦P®É¦b¯¸¤º¤H¼Æ[m­º¦¸¹F¨ì [1;36m%d[m ¤H¦¸\n",
		  ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_year % 100,
		  ptime->tm_hour, ptime->tm_min, a);
	  fclose(fp1);
	}
	fclose(fp);

	if (fp = fopen("etc/history.data", "w"))
	{
	  fprintf(fp, "%d %d %d %d", max_login, max, max_reg, a);
	}
      }
    }
    fclose(fp);
  }

  ptime = localtime(&now);

  if (!ptime->tm_hour)
  {
    keeplog(BBSHOME "/.note", "Record", "[¹Ð®J¬ö¿ý] ¤ß±¡¯d¨¥ª©");
    keeplog(BBSHOME "/log/GNP", "Record", "[ª÷¿Ä¤¤¤ß] ¥Í²£¤òÃB²Î­p");
    keeplog(BBSHOME "/log/counter/¤W¯¸¤H¦¸", "Record", "[¨t²Î³ø§i] ¥»¤é¤H¼Æ­p¼Æ¾¹");

    keeplog("usies", "Security", "[¨t²Î³ø§i] ¨Ï¥ÎªÌ¤W½u¬ö¿ý");
    keeplog("usboard", "Security", "[¨t²Î³ø§i] ¬ÝªO¨Ï¥Î¬ö¿ý");
    keeplog(BBSHOME "/log/article_score.log", "Security",
	    "[¨t²Î³ø§i] ¤å³¹µû¤À¬ö¿ý");
    keeplog(BBSHOME "/log/bighome", "Security", "[¨t²Î³ø§i] ¶W¹L 500K ªº userhome");
    keeplog(BBSHOME "/log/bigboard", "Security", "[¨t²Î³ø§i] ¶W¹L 3000K ªº board");
    keeplog(BBSHOME "/log/board.log", "Record", "[¨t²Î³ø§i] ¬ÝªO¨Ï¥Î¬ö¿ý");
    keeplog(BBSHOME "/log/personal.log", "Record", "[¨t²Î³ø§i] ­Ó¤HªO¨Ï¥Î¬ö¿ý");
    keeplog(BBSHOME "/log/admin.log", "Security", "[¨t²Î³ø§i] ¤µ¤é¨t²Î¬ö¿ý");
    keeplog(BBSHOME "/log/func.log", "Security", "[¨t²Î³ø§i] ¤µ¤é¥\\¯à¨Ï¥Î¬ö¿ý");
    keeplog(BBSHOME "/log/bm_check", "Record", "[¨t²Î³ø§i] ªO¥D¨ì¯¸¬ö¿ý");
    keeplog(BBSHOME "/log/bank.log", "Security", "[ª÷¿Ä¤¤¤ß] ª÷¿ú¬y°Ê¬ö¿ý");
    keeplog(BBSHOME "/log/board_edit", "Security", "[¨t²Î³ø§i] ¬ÝªO§ó°Ê¬ö¿ý");
    unlink("log/dlog");

    system("/bin/cp etc/today etc/yesterday");

    /* ¨C¤é¤W¯¸¤H¼ÆÂk¹s */
    system("/bin/cp -f ~log/counter/¤W¯¸¤H¦¸.new ~/log/counter/¤W¯¸¤H¦¸");

    unlink("note.dat");
    gzip(log_file, "usies", buf);


    /* Ptt ¾ú¥v¨Æ¥ó³B²z */
    now = time(NULL) - ADJUST_M * 60;	/* back to ancent */
    ptime = localtime(&now);

    resolve_ucache();
    if (fp = fopen("etc/history.data", "r"))
    {				/* ³æ¤é³Ì¦h¦¸¤H¦¸,¦P®É¤W½u,µù¥U */
      if (fscanf(fp, "%d %d %d %d", &max_login, &max, &max_reg, &k))
      {
	fp1 = fopen("etc/history", "r+");
	fseek(fp1, (off_t) 0, 2);
	if (max_user > max)
	{
	  fprintf(fp1, "¡º ¡i%02d/%02d/%02d %02d¡j   "
		  "[1;32m³æ¤@¤p®É¤W½u¤H¦¸[m­º¦¸¹F¨ì [1;35m%d[m ¤H¦¸ \n"
		  ,ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_year % 100, mahour, max_user);
	  max = max_user;
	}
	if (total > max_login)
	{
	  fprintf(fp1, "¡» ¡i%02d/%02d/%02d¡j      "
		  "[1;32m³æ¤é¤W½u¤H¦¸[m­º¦¸¹F¨ì[1;33m %d[m ¤H¦¸   \n"
	   ,ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_year % 100, total);
	  max_login = total;
	}

	if (uidshm->number > max_reg + max_reg / 10)
	{
	  fprintf(fp1, "¡¹ ¡i%02d/%02d/%02d¡j      "
		  "[1;32mÁ`µù¥U¤H¼Æ[m´£¤É¨ì[1;31m %d[m ¤H   \n"
		  ,ptime->tm_mon + 1, ptime->tm_mday, ptime->tm_year % 100, uidshm->number);
	  max_reg = uidshm->number;
	}

	fclose(fp1);
      }
      fclose(fp);
      fp = fopen("etc/history.data", "w");
      fprintf(fp, "%d %d %d %d", max_login, max, max_reg, k);
      fclose(fp);
    }
    now += ADJUST_M * 60;	/* back to future */
    ptime = localtime(&now);

    if (ptime->tm_wday == 0)
    {
      keeplog("log/week", "Record", "[¹Ð®J¬ö¿ý] ¥»¶g¼öªù¸ÜÃD");
      keeplog("log/trade.log", "Record", "[ª÷¿Ä¤¤¤ß] °Ó«~³c½æª¬ªp");

      keeplog("etc/game.log", "LocalGame", "[¹C¼Ö³õ] ¥»¶g¹CÀ¸¬ö¿ý");

      gzip("bbslog", "bntplink", buf);
      gzip("innd/bbslog", "innbbsd", buf);
      gzip("log/bbsmail.log", "mailog", buf);
    }

    if (ptime->tm_mday == 1)
    {
      keeplog("log/month", "Record", "[¹Ð®J¬ö¿ý] ¥»¤ë¼öªù¸ÜÃD");
      keeplog("log/topboard", "Record", "[¹Ð®J¬ö¿ý] ¥»¤ë¬ÝªO±Æ¦æ");
      keeplog("log/toppersonal", "Record", "[¹Ð®J¬ö¿ý] ¥»¤ë­Ó¤HªO±Æ¦æ");
    }
    if (ptime->tm_yday == 1)
      keeplog("log/year", "Record", "[¹Ð®J¬ö¿ý] ¦~«×¼öªù¸ÜÃD");
  }
  else if (ptime->tm_hour == 3 && ptime->tm_wday == 6)
  {
    char *fn1 = "tmp";
    char *fn2 = "suicide";
    f_mv(fn1, fn2);
    mkdir(fn1, 0755);
    sprintf(buf, "/bin/gtar cfz adm/%s-%02d%02d%02d.tgz %s",
	 fn2, ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday, fn2);
    system(buf);
    sprintf(buf, "/bin/rm -fr %s", fn2);
    system(buf);
  }
}
