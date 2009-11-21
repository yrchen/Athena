/* $Id: camera.c,v 1.6 2003/12/28 15:29:00 sby Exp $ */

/* target : 建立 [動態看板] [小看板] 和其他經常讀取的畫面 cache */

#include "bbs.h"

static char *film_list[] =
{
  "etc/Welcome0", 
  "etc/Welcome1", 
  "etc/Welcome2", 
  "etc/Welcome3",
  "etc/Welcome4", 
  "etc/Welcome_login", 
  "etc/Welcome_birth", 
  "etc/Logout", 
  "log/day", 
  "etc/post.note", 
  "etc/register",

  "etc/help/ANNOUNCE.help",
  "etc/help/MORE.help",

NULL};

static char *m2_list[] =
{
  "m2/1",
  "m2/2",
  "m2/3",
  "m2/4",
  "m2/5",
  "m2/6",
  "m2/7",
  "m2/8",
  "m2/9",
  "m2/10",

NULL};

static FCACHE image;
static int number;
static int tail;


static int	/* 0:撥放失敗  !=0:已撥放了幾篇 */
play(char *data, int mode, int size)
{
  int line, ch;
  char *head;

  head = data;

  if (mode)		/* 動態看板，要限制行數 */
  { 
    line = 0;
    while ((ch = *data) != '\0')	/* at most MOVIE_LINES lines */
    {
      data++;
      if (ch == '\n')
      {
        if (++line >= MOVIE_LINES)
	  break;
      }
    }

#if 0
    *data++ = 27;
    *data++ = '[';
    *data++ = 'm';
#endif

    while (line < MOVIE_LINES)	/* at lease MOVIE_LINES lines */
    {
      *data++ = '\n';
      line++;
    }

    *data = '\0';
    size = data - head;
  }

  ch = size + 1; /* Thor.980804: +1 將結尾的 '\0' 也算入 */
         
  line = tail + ch;
  if (line >= MOVIE_SIZE)	/* 若加入這篇會超過全部容量，則不 mirror */
    return 1;			/* overflow */

  data = image.film + tail;
  memcpy(data, head, ch);
  image.shot[++number] = tail = line;
  return number;
}

static int		       /* return: 0)mirror失敗  !=0)mirror成功 */
mirror(char *fpath, int mode)  /* mode :  1)動態看板，要限制列數  0)一般文件，不限制列數 */
{
  int fd, size;
  char buf[FILM_SIZ + 1];

  /* 若已經超過最大篇數，則不 mirror */

  if ((number < MOVIE_MAX - 1) && (fd = open(fpath, O_RDONLY)) >= 0)
  {
    /* 讀入檔案 */
    size = read(fd, buf, FILM_SIZ);
    close(fd);

    if (size > 0)
    {
      buf[size] = '\0';
      return play(buf, mode, size);
    }
  }

  return 0;
}

static void
m2_mirror(FCACHE *fshm, char *fpath, int id)
{
  FILE *fp;
  register int i;

  if ((fp = fopen(fpath, "r")) != NULL)
  {
    id *= MOVIE2_LINES;

    for (i = 0; i < MOVIE2_LINES; i++)
    {
      if (fgets(fshm->movie2[id + i], 256, fp) == NULL)
	break;
    }

    fclose(fp);
  }
}

int
main()
{
  int i, num;
  char fpath[PATHLEN];
  char pbuf[256], dbuf[256];
  FCACHE *fshm;
  FILE *fp;
  fileheader hdr;

  FILE *fp1;
  int mo, da, j = 0;

  time_t now = time(NULL);
  struct tm *ptime = localtime(&now);

  /* --------------------------------------------------- */
  /* mirror pictures					 */
  /* --------------------------------------------------- */

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);

  /* 用mirror拍下來再print */

  for (i = 0; film_list[i]; i++)
  {
    mirror(film_list[i], 0);
  }

  /* --------------------------------------------------- */
  /* visit all films					 */
  /* --------------------------------------------------- */

  setapath(pbuf, "Note");
  setadir(dbuf, pbuf);
  num = rec_num(dbuf, sizeof hdr);

  for (i = 1; i <= num; i++)
  {
    if (rec_get(dbuf, &hdr, sizeof hdr, i) != -1)
      if (hdr.title[3] == '<' && hdr.title[8] == '>')
      {
        char path[PATHLEN], buf1[PATHLEN];
        int num1, k;
        fileheader subitem;

        setapath(path, "Note");
        sprintf(buf1, "%s/%s", path, hdr.filename);
        setadir(path, buf1);
        num1 = rec_num(path, sizeof hdr);

        for (k = 1; k <= num1; k++)
        {
          if (rec_get(path, &subitem, sizeof hdr, k) != -1)
          {
            sprintf(fpath, "man/boards/Note/%s/%s",
		hdr.filename, subitem.filename);

            if (!mirror(fpath, 1))
              break;
          }
        }
      }
  }
 
  if ((fp = fopen("etc/feast", "r")) != NULL)
  {
    while (fscanf(fp, "%d %d %s\n", &mo, &da, dbuf) != EOF)
    {
      if ((ptime->tm_mday == da) && (ptime->tm_mon + 1 == mo))
      {
        j = 1;

        if ((fp1 = fopen("etc/today_is","w")) != NULL)
	{
          fprintf(fp1,"%-16.16s", dbuf);
          fclose(fp1);
	}
      }
    }
    
    fclose(fp);
    
    if (j == 0)
    {
      if ((fp1 = fopen("etc/today_is","w")) != NULL)
      {
        if ((fp = fopen("etc/today_boring","r")) != NULL)
        {
          while (fgets(dbuf, 250, fp))
          {
            if (strlen(dbuf) > 3)
            {
              dbuf[strlen(dbuf) - 1] = 0;
              fprintf(fp1, "%-16s\n", dbuf);
            }
          }
          fclose(fp);
        }
        else
          fprintf(fp1,"本日節日徵求中");

        fclose(fp1);
      }
    }
  }


  /* --------------------------------------------------- */
  /* resolve shared memory				 */
  /* --------------------------------------------------- */

  fshm = (FCACHE *) shm_new(FILMSHM_KEY, sizeof(FCACHE));
  memcpy(fshm, &image, sizeof(image.shot) + tail);
                       /* Thor.980805: 再加上 shot的部分 */
  fshm->shot[0] = number;	/* 總共有幾片 ? */

#if 0
  fd = open("run/camera.img", O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd >= 0)
  {
    write(fd, &image, tail);
    close(fd);
  }
#endif

  if ((fp = fopen("log/camera.log", "a")) != NULL)
  {
    fprintf(fp, "%d/%d films, %d/%d bytes\n",
	number, MOVIE_MAX, tail, MOVIE_SIZE);
    fclose(fp);
  }

  /* 小看板 */
  for (i = 0; m2_list[i]; i++)
  {
    m2_mirror(fshm, m2_list[i], i);
  }

  return 0;
}

