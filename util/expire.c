/*-------------------------------------------------------*/
/* util/expire.c        ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : �۰ʬ�H�u��{��                             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
/* syntax : expire [day] [max] [min]                     */
/*-------------------------------------------------------*/

#include "bbs.h"
#include <strings.h>

#include "cache.c"

typedef int (*FPTR)(const void*, const void*);

char *bpath = "boards";

struct life
{
  char bname[IDLEN+1];          /* board ID */
  int days;                     /* expired days */
  int maxp;                     /* max post */
};
typedef struct life life;


void
expire(life *brd)
{
  fileheader head;
  struct stat state;
  char lockfile[128], tmpfile[128], bakfile[128];
  char fpath[128], index[128], *fname;
  int total;
  int fd, fdr, fdw, done, keep;
  int duetime, ftime;

  printf("%s\n", brd->bname);

#if 0
  if (brd->days < 1)
  {
    printf(":Err: expire time must more than 1 day.\n");
    return;
  }
  else if (brd->maxp < 100)
  {
    printf(":Err: maxmum posts number must more than 100.\n");
    return;
  }
#endif
//  sprintf(index, "%s/%s/.DIR", bpath, brd->bname);

  setbfile(index, brd->bname, ".DIR");
  sprintf(lockfile, "%s.lock", index);
  if ((fd = open(lockfile, O_RDWR | O_CREAT | O_APPEND, 0644)) == -1)
    return;
  flock(fd, LOCK_EX);

  strcpy(fpath, index);
  fname = (char *) strrchr(fpath, '.');

  duetime = time(NULL) - brd->days * 24 * 60 * 60;
  done = 0;
  if ((fdr = open(index, O_RDONLY, 0)) > 0)
  {
    fstat(fdr, &state);
    total = state.st_size / sizeof(head);
    sprintf(tmpfile, "%s.new", index);
    unlink(tmpfile);
    if ((fdw = open(tmpfile, O_WRONLY | O_CREAT | O_EXCL, 0644)) > 0)
    {
      while (read(fdr, &head, sizeof head) == sizeof head)
      {
        done = 1;
        ftime = atoi(head.filename + 2);        
        if (head.owner[0] == '-')        
          keep = 0;
        else if (head.filemode & FILE_MARKED || head.filemode & FILE_DIGEST)
          keep = 1;
#if 0
        /* �o�O���F�ٵw�Юɭԥ� */
        else if(head.filemode & FILE_DIGEST && ftime < (duetime - 50 * 24 * 60 * 60))
          keep = 0;
#endif
        else if (brd->maxp == 0)
        {
          if(brd->days == 0)
            keep = 1;
          else if(ftime < duetime)
            keep = 0;
          else
            keep = 1;
        }
        else if (brd->days == 0)
        {
          if(brd->maxp == 0)
            keep = 1;
          else if(total > brd->maxp)
            keep = 0;
          else
            keep = 1;
        }
        else if (ftime < duetime || total > brd->maxp)
          keep = 0;
        else
          keep = 1;
        if (keep)
        {
          if (write(fdw, &head, sizeof head) == -1)
          {
            done = 0;
            break;
          }
        }
        else
        {
          strcpy(fname, head.filename);
          unlink(fpath);
          
          strcat(fname, ".vis\0");  /*��[�K�W��*/
          unlink(fpath);
          
          printf("\t%s\n", fname);
          total--;
        }
      }
      close(fdw);
    }
    close(fdr);
  }

  if (done)
  {
    sprintf(bakfile, "%s.old", index);
    if (f_mv(index, bakfile) != -1)
    {
      f_mv(tmpfile, index);
      touchbtotal(getbnum(brd->bname));
    }
  }
  flock(fd, LOCK_UN);
  close(fd);
}

int
main(int argc, char *argv[])
{
  FILE *fin;
  int number, count;
  life db, table[MAXBOARD], *key;
  struct dirent *de;
  DIR *dirp;
  char *ptr, buf[256];
  boardheader bh;

  setgid(BBSGID);
  setuid(BBSUID);
  chdir(BBSHOME);
  
  db.days = ((argc > 1) && (number = atoi(argv[1])) > 0) ? number : DEF_MAXT;
  db.maxp = ((argc > 2) && (number = atoi(argv[2])) > 0) ? number : DEF_MAXP;

  /* --------------- */
  /* load expire.ctl */
  /* --------------- */

//  sprintf(buf,BBSHOME"/.BOARDS");
  for(count=0;count<=MAXBOARD;count++)
  {
    memset (&bh, 0, sizeof (boardheader));
    rec_get(FN_BOARD, &bh, sizeof(boardheader), count);
    strcpy(table[count].bname,bh.brdname);
    table[count].maxp = bh.maxpost;
    table[count].days = bh.maxtime;
    printf("%-4d. %-13.13s %d %d\n",
      count,table[count].bname,table[count].maxp,table[count].days);
  }
    
  if (count > 1)
  {
    qsort(table, count, sizeof(life), (FPTR)strcasecmp);
  }

  /* ---------------- */
  /* visit all boards */
  /* ---------------- */

  if (!(dirp = opendir(bpath)))
  {
    printf(":Err: unable to open %s\n", bpath);
    return -1;
  }

  while (de = readdir(dirp))
  {
    ptr = de->d_name;
    if (ptr[0] > ' ' && ptr[0] != '.')
    {
      if (count)
        key = (life *) bsearch(ptr, table, count, sizeof(life), (FPTR)strcasecmp);
      else
        key = NULL;
      if (!key)
        key = &db;
      strcpy(key->bname, ptr);
      expire(key);
    }
  }
  closedir(dirp);
}
