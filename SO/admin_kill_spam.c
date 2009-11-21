#include "bbs.h"
#include <stdarg.h>
#include <sys/mman.h>
char *str_ttl(char *title);
static void
cancel_post(fileheader *fhdr, char *fpath)
{
#define NICK_LEN    80
  int fd;

  if ((fhdr->savemode == 'S') &&/* 外轉信件 */
    ((fd = open(fpath, O_RDONLY)) >= 0))
  {
    char *ptr, *left, *right, nick[NICK_LEN];
    FILE *fout;
    int ch;

    ptr = nick;
    ch = read(fd, ptr, NICK_LEN);
    close(fd);
    ptr[ch] = '\0';
    if (!strncmp(ptr, str_author1, LEN_AUTHOR1) ||
      !strncmp(ptr, str_author2, LEN_AUTHOR2))
    {
      if (left = (char *) strchr(ptr, '('))
      {
        right = NULL;
        for (ptr = ++left; ch = *ptr; ptr++)
        {
          if (ch == ')')
            right = ptr;
          else if (ch == '\n')
            break;
        }

        if (right != NULL)
        {
          *right = '\0';
          log_board3("DEL", currboard, 1);

          if (fout = fopen(BBSHOME"/innd/cancel.bntp", "a"))
          {
            fprintf(fout, "%s\t%s\t%s\t%s\t%s\n",
              currboard, fhdr->filename, fhdr->owner    /* cuser.userid */
              ,left, fhdr->title);
            fclose(fout);
          }
        }
      }
    }
#undef  NICK_LEN
  }
}

extern int TagNum;           /* tag's number */


#define	BATCH_SIZE	65536

static int
TagThread(char *direct, char *search, int type)
                                      /* 0: title, 1: owner */
{
  caddr_t fimage, tail;
  off_t off;

  int fd, fsize, count;
  struct stat stbuf;
  fileheader *head;
  char *title;

  if ((fd = open(direct, O_RDONLY)) < 0)
    return RC_NONE;

  fstat(fd, &stbuf);
  fsize = stbuf.st_size;

  fimage = NULL;
  off = count = 0;
  do
  {
    fimage = mmap(fimage, BATCH_SIZE, PROT_READ, MAP_SHARED, fd, off);
    if (fimage == (char *) -1)
    {
      outs("MMAP ERROR!!!!");
      close(fd); //hialan:是不是缺少這個..@@""
      abort_bbs();
    }

    tail = fimage + BMIN(BATCH_SIZE, fsize - off);

    for (head = (fileheader *) fimage; (caddr_t) head < tail; head++)
    {
      int tmplen;

      count++;

      if (type == 1)
      {
        title = head->owner;
        tmplen = IDLEN+1;
      }
      else
      {
        title = str_ttl(head->title);
        tmplen = TTLEN;
      }

      if (!strncmp(search, title, tmplen))
      {
	if (!Tagger(atoi(head->filename + 2) , count, TAG_INSERT))
	{
	  off = fsize;
	  break;
	}
      }
    }

    off += BATCH_SIZE;
// wildcat : 會越吃越多記憶體?
  munmap(fimage, BATCH_SIZE);
  } while (off < fsize);
  close(fd);
  return RC_DRAW;
}

/* ----------------------------------------------------- */
/* id1:							 */
/* 0 ==> 依據 TagList 連鎖刪除			         */
/* !0 ==> 依據 range [id1, id2] 刪除		         */
/* ----------------------------------------------------- */

static int
delete_range2(char *fpath, int id1, int id2)
{
  fileheader fhdr;
  nol my;
  char fullpath[STRLEN], *t;
  int fdr, fdw, fd;
  register int count;

  nolfilename(&my, fpath);

  if ((fd = open(my.lockfn, O_RDWR | O_CREAT | O_APPEND, 0644)) < 0)
    return -1;
  flock(fd, LOCK_EX);

  if ((fdr = open(fpath, O_RDONLY, 0)) < 0)
  {
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }

  if ((fdw = open(my.newfn, O_WRONLY | O_CREAT | O_EXCL, 0644)) < 0)
  {
    close(fdr);
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }
  strcpy(fullpath, fpath);
  t = (char *) strrchr(fullpath, '/') + 1;

  count = 0;
  while (read(fdr, &fhdr, sizeof(fileheader)) == sizeof(fileheader))
  {
    count++;
    if ((fhdr.filemode & FILE_MARKED) ||	/* 標記 */
      (id1 && (count < id1 || count > id2)) ||	/* range */
      (!id1 && Tagger(atoi(fhdr.filename+2), count, TAG_COMP)))	/* TagList */
    {
      if ((write(fdw, &fhdr, sizeof(fileheader)) < 0))
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
      strcpy(t, fhdr.filename);

      /* 若為看板就連線砍信 */

      if (currstat == READING)
      {
	cancel_post(&fhdr, fullpath);
      }

      unlink(fullpath);
    }
  }
  close(fdr);
  close(fdw);
  count = rename(fpath, my.oldfn);
  if (!count)
  {
    if (count = rename(my.newfn, fpath))
      rename(my.oldfn, fpath);	/* 萬一出鎚，再救回來 */
  }
  flock(fd, LOCK_UN);
  close(fd);
  return count;
}

int 
kill_all_spam(va_list pvar)
{
        char *choose[3] = {"11)作者", "22)標題", msg_choose_cancel};
        char buf[128], mode[3];
        int i;
        boardheader *bp;
        fileheader *fhdr = NULL;
        extern boardheader *bcache;
        extern int numboards;
        
        fhdr = va_arg(pvar, fileheader *);
        
        mode[0] = getans2(b_lines, 0, "刪除此 ", choose, 3, 'q');
        if(mode[0] == '1')
          sprintf(buf, "終結所有看板中的 [%.40s] 嗎？ ", fhdr->owner);
        else if(mode[0] == '2')
          sprintf(buf, "終結所有看板中的 [%.40s] 嗎？ ", fhdr->title);
        else
          return RC_FOOT; 
          
        if (getans2(b_lines, 0, buf, 0, 2, 'n') != 'y')
          return RC_FOOT;
  
        resolve_boards();
        for (bp = bcache, i = 0; i < numboards; i++, bp++)
        {
          TagNum = 0;
          setbdir(buf, bp->brdname);
          outmsg(buf);
          refresh();
          if(mode[0] != '1') 
            TagThread(buf,fhdr->title, 0);
          else
            TagThread(buf, fhdr->owner, 1);
            
          if (TagNum)
          {
            delete_range2(buf, 0, 0); 
            setbtotal(i);
          }
        }
        if(mode[0] != '1') 
          log_usies("SPAM title ", currtitle);
        else
          log_usies("SPAM user", currtitle);
        TagNum = 0;
        return RC_CHDIR;
}

