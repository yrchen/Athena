/*-------------------------------------------------------*/
/* util/poststat.c      ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : ²Î­p¤µ¤é¡B¶g¡B¤ë¡B¦~¼öªù¸ÜÃD                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"
#include "cache.c"

char *myfile[] = {"day", "week", "month", "year"};
int mycount[4] = {7, 4, 12};
int mytop[] = {10, 50, 100, 100};
char *mytitle[] = {"¤é¤Q", "¶g¤­¤Q", "¤ë¦Ê", "¦~«×¦Ê"};

#define HASHSIZE 1024
#define TOPCOUNT 200

struct postrec
{
  char author[13];              /* author name */
  char board[13];               /* board name */
  char title[66];               /* title name */
  time_t date;                  /* last post's date */
  int number;                   /* post number */
  struct postrec *next;         /* next rec */
}      *bucket[HASHSIZE];


/* 100 bytes */
struct posttop
{
  char author[13];              /* author name */
  char board[13];               /* board name */
  char title[66];               /* title name */
  time_t date;                  /* last post's date */
  int number;                   /* post number */
}       top[TOPCOUNT], *tp;


int
hash(char *key)
{
  int i, value = 0;

  for (i = 0; key[i] && i < 80; i++)
    value += key[i] < 0 ? -key[i] : key[i];

  value = value % HASHSIZE;
  return value;
}


/* ---------------------------------- */
/* hash structure : array + link list */
/* ---------------------------------- */


void
search(struct posttop *t)
{
  struct postrec *p, *q, *s;
  int i, found = 0;

  i = hash(t->title);
  q = NULL;
  p = bucket[i];
  while (p && (!found))
  {
    if (!strcmp(p->title, t->title) && !strcmp(p->board, t->board))
      found = 1;
    else
    {
      q = p;
      p = p->next;
    }
  }
  if (found)
  {
    p->number += t->number;
    if (p->date < t->date)      /* ¨ú¸ûªñ¤é´Á */
      p->date = t->date;
  }
  else
  {
    s = (struct postrec *) malloc(sizeof(struct postrec));
    memcpy(s, t, sizeof(struct posttop));
    s->next = NULL;
    if (q == NULL)
      bucket[i] = s;
    else
      q->next = s;
  }
}


int
sort(struct postrec *pp, int count)
{
  int i, j;

  for (i = 0; i <= count; i++)
  {
    if (pp->number > top[i].number)
    {
      if (count < TOPCOUNT - 1)
        count++;
      for (j = count - 1; j >= i; j--)
        memcpy(&top[j + 1], &top[j], sizeof(struct posttop));

      memcpy(&top[i], pp, sizeof(struct posttop));
      break;
    }
  }
  return count;
}


void
load_stat(char *fname)
{
  FILE *fp;

  if (fp = fopen(fname, "r"))
  {
    int count = fread(top, sizeof(struct posttop), TOPCOUNT, fp);
    fclose(fp);
    while (count)
      search(&top[--count]);
  }
}

int
belong(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[STRLEN], *ptr;

    while (fgets(buf, STRLEN, fp))
    {
      if ((ptr = strtok(buf, " \t\n\r")) && !strcasecmp(ptr, key))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}


int
filter(board)
  char *board;
{
  boardheader bh;
  int bid;
  int i;

  bid = getbnum(board);
  if (rec_get(".BOARDS", &bh, sizeof(bh), bid) == -1)
     return 1;
/*
  if (!(bh.level & PERM_POST))
     return 1;
*/
  if (belong("etc/NoStatBoards", board))
     return 1;
  if (bh.level & PERM_POSTMASK)
     return 0;
  return (bh.level & ~PERM_POSTMASK) >= 32;
}


void
poststat(int mytype)
{
  static char *logfile = ".post";
  static char *oldfile = ".post.old";

  FILE *fp;
  char buf[40], curfile[40] = "log/day.0", *p;
// CityLion Patch : ±N old postrec ¼È¦s¦Ü qq
  struct postrec *pp , *qq;
  int i, j;

  if (mytype < 0)
  {
    /* --------------------------------------- */
    /* load .post and statictic processing     */
    /* --------------------------------------- */

    remove(oldfile);
    f_mv(logfile, oldfile);
    if ((fp = fopen(oldfile, "r")) == NULL)
      return;

    load_stat(curfile);

    while (fread(top, sizeof(struct posttop), 1, fp))
      search(top);
    fclose(fp);

    mytype = 0;
  }
  else
  {
    /* ---------------------------------------------- */
    /* load previous results and statictic processing */
    /* ---------------------------------------------- */

    i = mycount[mytype];
    p = myfile[mytype];
    while (i)
    {
      sprintf(buf, "log/%s.%d", p, i);
      sprintf(curfile, "log/%s.%d", p, --i);
      load_stat(curfile);
      f_mv(curfile, buf);
    }
    mytype++;
  }

  /* ---------------------------------------------- */
  /* sort top 100 issue and save results            */
  /* ---------------------------------------------- */

  memset(top, 0, sizeof(top));
  for (i = j = 0; i < HASHSIZE; i++)
  {

    for (pp = bucket[i]; pp; pp = pp->next)
    {

#ifdef  DEBUG
      printf("Title : %s, Board: %s\nPostNo : %d, Author: %s\n"
        ,pp->title
        ,pp->board
        ,pp->number
        ,pp->author);
#endif

      j = sort(pp, j);
    }
  }

  p = myfile[mytype];
  sprintf(curfile, "log/%s.0", p);
  if (fp = fopen(curfile, "w"))
  {
    fwrite(top, sizeof(struct posttop), j, fp);
    fclose(fp);
  }

  sprintf(curfile, "log/%s", p);
  if (fp = fopen(curfile, "w"))
  {
    int max, cnt;

    fprintf(fp, "           [1;34m-----[37m=====[41m ¥»%s¤j¼öªù¸ÜÃD [40m=====[34m-----[m «ö [1;32m[TAB][m ¥iª½±µ¶i¤J¸Ó¬ÝªO\n\n", mytitle[mytype]);

    max = mytop[mytype];
    p = buf + 4;
    for (i = cnt = 0; (cnt < max) && (i < j); i++)
    {
      tp = &top[i];
      if (filter(tp->board))
              continue;

      strcpy(buf, ctime(&(tp->date)));
      buf[20] = 0;
      fprintf(fp,
        "[1;31m%3d. [33m¬ÝªO : [32mboard://%-16s[35m¡m %s¡n[36m%4d ½g[33m%+14s\n"
        "     [33m¼ÐÃD : [0;44;37m%-60.60s      [40m\n"
        ,++cnt, tp->board, p, tp->number, tp->author, tp->title);
    }
    fclose(fp);
  }

  /* free statistics */

  for (i = 0; i < HASHSIZE; i++)
  {
    for (pp = bucket[i]; pp; pp = qq)
    {
      qq = pp->next;
      free(pp);
    }
    bucket[i] = NULL;
  }
}

int 
main(int argc, char *argv[])
{
  time_t now;
  struct tm *ptime;

  if (argc < 2)
  {
    printf("Usage:\t%s bbshome [day]\n", argv[0]);
    return (-1);
  }

  chdir(argv[1]);

  if (argc == 3)
  {
    poststat(atoi(argv[2]));
    return (0);
  }
  time(&now);
  ptime = localtime(&now);
  if (ptime->tm_hour == 0)
  {
    if (ptime->tm_mday == 1)
      poststat(2);
    if (ptime->tm_wday == 0)
      poststat(1);
    poststat(0);
  }
  poststat(-1);

  return 0;
}
