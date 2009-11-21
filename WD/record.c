/*-------------------------------------------------------*/
/* record.c     ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : binary record file I/O routines              */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef SYSV
int
flock(fd, op)
  int fd, op;
{
  switch (op)
  {
  case LOCK_EX:
    return lockf(fd, F_LOCK, 0);
  case LOCK_UN:
    return lockf(fd, F_ULOCK, 0);
  default:
    return -1;
  }
}
#endif


int 
get_sum_records(char* fpath, int size)
{
   struct stat st;
   long ans = 0;
   FILE* fp;
   fileheader fhdr;
   char buf[PATHLEN], *p;

   if (!(fp = fopen(fpath, "r")))
      return -1;

   strcpy(buf, fpath);
   p = strrchr(buf, '/') + 1;

   while (fread(&fhdr, size, 1, fp) == 1) 
   {
      strcpy(p, fhdr.filename);
      if (stat(buf, &st) == 0 && S_ISREG(st.st_mode) && st.st_nlink == 1)
         ans += st.st_size;
   }
   fclose(fp);
   return ans / 1024;
}

int 
get_records(char *fpath, void *rptr, int size, int id, int number)
{
  int fd;

  if ((fd = open(fpath, O_RDONLY, 0)) == -1)
    return -1;

  if (lseek(fd, (off_t)(size * (id - 1)), SEEK_SET) == -1)
  {
    close(fd);
    return 0;
  }
  if ((id = read(fd, rptr, size * number)) == -1)
  {
    close(fd);
    return -1;
  }
  close(fd);

  return (id / size);
}


int substitute_record(char *fpath, void *rptr, int size, int id)
{
  int fd;
  static short substitute_flag = 1;               /* 判斷重入的旗標 */

  if (substitute_flag)
  {                                               /* 旗標為真才可以進入 */
    if (id < 1)  
      return -1;

    if (fpath[1] == 'P' || fpath[1] == 'B') 
    {
      substitute_flag = 0;            

      /*檢查數值上限 hialan.020714*/
      if(fpath[1] == 'P')
      {
        if(id < 1 || id > MAXUSERS)
        {
            substitute_flag = 1;                  /* 離開時設回來 */
            return -1;
        }
      }

    }
    if ((fd = open(fpath, O_WRONLY | O_CREAT, 0644)) == -1)
    {
      substitute_flag = 1;                        /* 離開時設回來 */
      return -1;
    }
    flock(fd, LOCK_EX);
    lseek(fd, (off_t) (size * (id - 1)), SEEK_SET);
    write(fd, rptr, size);
    flock(fd, LOCK_UN);
    close(fd);
    substitute_flag = 1;           /* 離開時設回來 */
    return 0;
  }
  return -1;	/* itoc:substitute_flag = 0 時要傳回 -1 */
}

#if !defined(_BBS_UTIL_C_)

void
nolfilename(nol *n, char *fpath)
{
  sprintf(n->newfn, "%s.new", fpath);
  sprintf(n->oldfn, "%s.old", fpath);
  sprintf(n->lockfn, "%s.lock", fpath);
}


int
rec_del(fpath, size, id)
  char fpath[];
  int size, id;
{
  nol my;
  char abuf[1024];
  int fdr, fdw, fd;
  int count;

  nolfilename(&my, fpath);
  if ((fd = open(my.lockfn, O_RDWR | O_CREAT | O_APPEND, 0644)) == -1)
    return -1;
  flock(fd, LOCK_EX);
  if ((fdr = open(fpath, O_RDONLY, 0)) == -1)
  {
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }

  if ((fdw = open(my.newfn, O_WRONLY | O_CREAT | O_EXCL, 0644)) == -1)
  {
    flock(fd, LOCK_UN);
    close(fd);
    close(fdr);
    return -1;
  }

  count = 1;
  while (read(fdr, abuf, size) == size)
  {
    if (id != count++ && (write(fdw, abuf, size) == -1))
    {
      unlink(my.newfn);
      close(fdr);
      close(fdw);
      flock(fd, LOCK_UN);
      close(fd);
      return -1;
    }
  }
  close(fdr);
  close(fdw);
  if (f_mv(fpath, my.oldfn) == -1 || f_mv(my.newfn, fpath) == -1)
  {
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }
  flock(fd, LOCK_UN);
  close(fd);
  return 0;
}


int
delete_range(fpath, id1, id2)
  char *fpath;
  int id1, id2;
{
  fileheader fhdr;
  nol my;
  char fullpath[STRLEN], *t;
  int fdr, fdw, fd;
  int count,number=0;

  nolfilename(&my, fpath);

  if ((fd = open(my.lockfn, O_RDWR | O_CREAT | O_APPEND, 0644)) == -1)
    return -1;
  flock(fd, LOCK_EX);

  if ((fdr = open(fpath, O_RDONLY, 0)) == -1)
  {
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }

  if ((fdw = open(my.newfn, O_WRONLY | O_CREAT | O_EXCL, 0644)) == -1)
  {
    close(fdr);
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }
  count = 1;
  strcpy(fullpath, fpath);
  t = (char *) strrchr(fullpath, '/') + 1;

  while (read(fdr, &fhdr, sizeof(fileheader)) == sizeof(fileheader))
  {
    strcpy(t, fhdr.filename);
    if (count < id1 || count > id2 || 
         (fhdr.filemode & FILE_MARKED) ||
         (fhdr.filemode & FILE_DIGEST) || dashd(fullpath))
    {
      if ((write(fdw, &fhdr, sizeof(fileheader)) == -1))
      {
        close(fdr);
        close(fdw);
        unlink(my.newfn);
        flock(fd, LOCK_UN);
        close(fd);
        return -1;
      }
    }
    else
    {
      number++;
      unlink(fullpath);
    }
    count++;
  }
  close(fdr);
  close(fdw);
  log_board3("PUR",currboard,number);
  if (f_mv(fpath, my.oldfn) == -1 || f_mv(my.newfn, fpath) == -1)
  {
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }
  flock(fd, LOCK_UN);
  close(fd);
  return 0;
}

/* woju */

int
search_rec(char* dirname, int (*filecheck)())
{
   fileheader fhdr;
   FILE *fp;
   int ans = 0;

   if (!(fp = fopen(dirname, "r")))
      return 0;

   while (fread(&fhdr, sizeof(fhdr), 1, fp)) 
   {
     ans++;
     if ((*filecheck) (&fhdr)) 
     {
        fclose(fp);
        return ans;
     }
   }
   fclose(fp);
   return 0;
}

int
delete_files(char* dirname, int (*filecheck)())
{
   fileheader fhdr;
   FILE *fp, *fptmp;
   int ans = 0;
   char tmpfname[100];
   char genbuf[PATHLEN];

   if (!(fp = fopen(dirname, "r")))
      return ans;

   strcpy(tmpfname, dirname);
   strcat(tmpfname, "_tmp");

   if (!(fptmp = fopen(tmpfname, "w"))) 
   {
      fclose(fp);
      return ans;
   }

   while (fread(&fhdr, sizeof(fhdr), 1, fp))
     if ((*filecheck) (&fhdr)) 
     {
        ans++;
        setdirpath(genbuf, dirname, fhdr.filename);
        unlink(genbuf);
     }
     else
        fwrite(&fhdr, sizeof(fhdr), 1, fptmp);

   fclose(fp);
   fclose(fptmp);
   unlink(dirname);
   f_mv(tmpfname, dirname);

   return ans;
}




int
delete_file(dirname, size, ent, filecheck)
  char *dirname;
  int size, ent;
  int (*filecheck) ();
{
  char abuf[1024];
  int fd;
  struct stat st;
  long numents;

  if ((fd = open(dirname, O_RDWR)) == -1)
    return -1;
  flock(fd, LOCK_EX);
  fstat(fd, &st);
  numents = ((long) st.st_size) / size;
  if (((long) st.st_size) % size)
    fprintf(stderr, "align err\n");
  if (lseek(fd,(off_t)( size * (ent - 1)), SEEK_SET) != -1)
  {
    if (read(fd, abuf, size) == size)
      if ((*filecheck) (abuf))
      {
        int i;

        for (i = ent; i < numents; i++)
        {
          if (lseek(fd, (off_t)((i) * size), SEEK_SET) == -1)
            break;
          if (read(fd, abuf, size) != size)
            break;
          if (lseek(fd, (off_t)((i - 1) * size), SEEK_SET) == -1)
            break;
          if (write(fd, abuf, size) != size)
            break;
        }
        ftruncate(fd, (off_t) size * (numents - 1));
        flock(fd, LOCK_UN);
        close(fd);
        return 0;
      }
  }
  lseek(fd, 0, SEEK_SET);
  ent = 1;
  while (read(fd, abuf, size) == size)
  {
    if ((*filecheck) (abuf))
    {
      int i;

      for (i = ent; i < numents; i++)
      {
        if (lseek(fd, (off_t)((i + 1) * size), SEEK_SET) == -1)
          break;
        if (read(fd, abuf, size) != size)
          break;
        if (lseek(fd, (off_t)(i * size), SEEK_SET) == -1)
          break;
        if (write(fd, abuf, size) != size)
          break;
      }
      ftruncate(fd, (off_t) size * (numents - 1));
      flock(fd, LOCK_UN);
      close(fd);
      return 0;
    }
    ent++;
  }
  flock(fd, LOCK_UN);
  close(fd);
  return -1;
}

int
rec_search(fpath, rptr, size, fptr, farg)
  char *fpath;
  char *rptr;
  int size;
  int (*fptr) ();
int farg;
{
  int fd;
  int id = 1;

  if ((fd = open(fpath, O_RDONLY, 0)) == -1)
    return 0;
  while (read(fd, rptr, size) == size)
  {
    if ((*fptr) (farg, rptr))
    {
      close(fd);
      return id;
    }
    id++;
  }
  close(fd);
  return 0;
}
#endif                          /* !defined(_BBS_UTIL_C_) */

/* ------------------------------------------ */
/* mail / post 時，依據時間建立檔案，加上郵戳 */
/* ------------------------------------------ */
/* Input: fpath = directory; Output: fpath = full path; */

void stampfile(char *fpath, fileheader *fh)
{
  register char *ip = fpath;
  time_t dtime;
  struct tm *ptime;
  int fp;

#if 1
  if (access(fpath, X_OK | R_OK | W_OK))
    mkdir(fpath, 0755);
#endif

  time(&dtime);
  while (*(++ip));
  *ip++ = '/';
  do
  {
    sprintf(ip, "M.%d.A", ++dtime );
  } while ((fp = open(fpath, O_CREAT | O_EXCL | O_WRONLY, 0644)) == -1);
  close(fp);
  memset(fh, 0, sizeof(fileheader));
  strcpy(fh->filename, ip);
  ptime = localtime(&dtime);

#if 0// !defined(_BBS_UTIL_C_)
  {//shakalaca.000428: find out the BBSHOME/M.* problem.. :p
    char genbuf[80];
    
    sprintf(genbuf, "MODE:%d", currstat);
    debug(genbuf);
  }
#endif
  sprintf(fh->date, "%2d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
}


/*
    ===== Added by mgtsai, Sep 10th, '96 =====
*/
void stampdir(char *fpath, fileheader *fh)
{
  register char *ip = fpath;
  time_t dtime;
  struct tm *ptime;

#if 1
  if (access(fpath, X_OK | R_OK | W_OK))
    mkdir(fpath, 0755);
#endif

  time(&dtime);
  while (*(++ip));
  *ip++ = '/';
  do
  {
    sprintf(ip, "D%X", ++dtime & 07777);
  } while (mkdir(fpath, 0755) == -1);
  memset(fh, 0, sizeof(fileheader));
  strcpy(fh->filename, ip);
  ptime = localtime(&dtime);
  sprintf(fh->date, "%2d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
}


void stamplink(char *fpath, fileheader *fh)
{
  register char *ip = fpath;
  time_t dtime;
  struct tm *ptime;

#if 1
  if (access(fpath, X_OK | R_OK | W_OK))
    mkdir(fpath, 0755);
#endif

  time(&dtime);
  while (*(++ip));
  *ip++ = '/';
  do
  {
    sprintf(ip, "S%X", ++dtime );
  } while (symlink("temp", fpath) == -1);
  memset(fh, 0, sizeof(fileheader));
  strcpy(fh->filename, ip);
  ptime = localtime(&dtime);
  sprintf(fh->date, "%2d/%02d", ptime->tm_mon + 1, ptime->tm_mday);
}


/* ===== end ===== */
#if !defined(_BBS_UTIL_C_)
int
gem_files(char* dirname, int (*filecheck)())
{
   fileheader fhdr;
   FILE *fp;
   int c = 0 ,ans = 0;
   char genbuf[PATHLEN];

   if (!(fp = fopen(dirname, "r")))
      return ans;

   /* wildcat.991216 : 詢問是否消除 tag */
   c = answer("是否要消除標記  Y)確定  N)取消？");

   while (fread(&fhdr, sizeof(fhdr), 1, fp))
   {
     if ((*filecheck) (&fhdr)) 
     {
        char title[TTLEN+1] = "◇ "; //精華區 title 前三格是符號
        char copypath[PATHLEN];
        fileheader item;
        int now;
        
        ans++;
        setdirpath(genbuf, dirname, fhdr.filename);

        strcpy(copypath, paste_path);
        stampfile(copypath, &item);
        unlink(copypath);

// wildcat : 改用 f_cp 
        f_cp(genbuf, copypath, O_TRUNC); 

// wildcat : owner 該是收錄者的 id ?!
	strcpy (item.owner, /* fhdr.owner*/ cuser.userid);
        /* shakalaca.990616: stamp 目的檔, 再將原始檔拷過去,
         *                   之前沒這麼做, 結果收錄後原標題也改了.. :(
         */
                          	
        strncpy(title + 3,fhdr.title, TTLEN-3);//把 fhdr.title 往後移三個space
        strcpy(item.title,title);              //再co回來
        /* shakalaca.990616: 改目的標題, 而非原始標題
         *                   其實原來寫法也沒錯, 只是我後面有用 substitude_rec
         */
                          
        setadir (genbuf, paste_path); 
        rec_add(genbuf,&item,sizeof(item));    //maple2的append_record

	/* wildcat.991216 : 詢問是否消除 tag */
	if(c != 'n' && c != 'N')
	  fhdr.filemode ^= FILE_TAGED;
        now = getindex (dirname, fhdr.filename, sizeof (fileheader));
        substitute_record (dirname, &fhdr, sizeof (fileheader), now);
    }
  }  

  fclose(fp);
  return ans;
}

#endif

