/*-------------------------------------------------------*/
/* visio.c           ( AT-BBS/WD_hialan BBS )            */
/*-------------------------------------------------------*/
/* target : VIrtual Screen Input Output routines         */
/*	    term.c + screen.c + io.c			 */
/* ¦X  ¨Ö : hialan					 */
/*-------------------------------------------------------*/
#include "bbs.h"
#include <arpa/telnet.h>
#include <stdarg.h>
#include <sys/ioctl.h>

/* ----------------------------------------------------- */
/* output routines  (io.c)                               */
/* ----------------------------------------------------- */
#define OBUFSIZE  (4096)
#define IBUFSIZE  (512)

#define INPUT_ACTIVE    0
#define INPUT_IDLE      1
static int i_mode = INPUT_ACTIVE;

static char inbuf[IBUFSIZE];
static int ibufsize = 0;
static int icurrchar = 0;

static char outbuf[OBUFSIZE];
static int obufsize = 0;

void oflush()
{
  if (obufsize)
  {
     write(1, outbuf, obufsize);
     obufsize = 0;
  }
}

static void
output(char *s, int len)
{
  /* Invalid if len >= OBUFSIZE */
  if (obufsize + len > OBUFSIZE)
    oflush();

  memcpy(outbuf + obufsize, s, len);
  obufsize += len;
}

void
ochar(char c)
{
  if (obufsize > OBUFSIZE - 1)
    oflush();

  outbuf[obufsize++] = c;
}

/* ----------------------------------------------------- */
/* init tty control code                                 */
/* ----------------------------------------------------- */
#define TERMCOMSIZE (40)

static int automargins;

char *outp;
int *outlp;

void
do_move(int destcol, int destline)
{
  char buf[16], *p;

  sprintf(buf, "\33[%d;%dH", destline + 1, destcol + 1);
  for(p = buf; *p; p++)
    ochar(*p);
}

void
save_cursor()
{
  ochar('\33');
  ochar('7');
}

void restore_cursor()
{
  ochar('\33');
  ochar('8');
}

/*-------------------------------------------------------*/
/* screen.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : ANSI/Chinese screen display routines 	 */
/*-------------------------------------------------------*/
#define o_ansi(x)       output(x, sizeof(x)-1)
#define o_clear()       o_ansi("\033[H\033[J")
#define o_cleol()       o_ansi("\033[K")
#define o_standup()     o_ansi("\033[7m")
#define o_standdown()   o_ansi("\033[m")
#define o_scrollrev()   o_ansi("\033M")

#define scr_cols ANSILINELEN

static uschar scr_lns, cur_ln = 0, cur_col = 0;
static uschar docls, standing = NA;
static char roll = 0;
static int scrollcnt, tc_col, tc_line;

static screenline *big_picture = NULL;

void initscr()
{
  extern void *calloc();

  scr_lns = t_lines;
  big_picture = (screenline *) calloc(scr_lns, sizeof(screenline));
  docls = YEA;
}

/* Add by hialan for resize screen */
static void
chscr(int new_lines)
{
  t_lines = new_lines;
  b_lines = new_lines - 1;
  p_lines = new_lines - 4;
  
  if(big_picture != NULL)
  {
    if(scr_lns != t_lines)
    {
      size_t sclsize = sizeof(screenline);
      screenline *new_bp = (screenline *) calloc(new_lines, sclsize);
      memcpy(new_bp, big_picture, sclsize * BMIN(new_lines, scr_lns));
      free(big_picture);
      big_picture = new_bp;
      
      scr_lns = t_lines;
    }
  }
}

void
move(int y, int x)
{
  cur_col = x;
  cur_ln = y;
}

void
getyx(int *y, int *x)
{
  *y = cur_ln;
  *x = cur_col;
}

static void
rel_move(int new_col, int new_ln)
{
  int was_col = tc_col;
  int was_ln = tc_line;

  if (new_ln >= t_lines || new_col >= scr_cols)
    return;

  tc_col = new_col;
  tc_line = new_ln;
  if (new_col == 0)
  {
    if (new_ln == was_ln)
    {
      if (was_col)
	ochar('\r');
      return;
    }
    else if (new_ln == was_ln + 1)
    {
      ochar('\n');
      if (was_col)
	ochar('\r');
      return;
    }
  }

  if (new_ln == was_ln)
  {
    if (was_col == new_col)
      return;

    if (new_col == was_col - 1)
    {
      ochar(Ctrl('H'));
      return;
    }
  }
  do_move(new_col, new_ln);
}

static void
standoutput(char *buf, int ds, int de, int sso, int eso)
{
  if (eso <= ds || sso >= de)
    output(buf + ds, de - ds);
  else
  {
    int st_start, st_end;

    st_start = BMAX(sso, ds);
    st_end = BMIN(eso, de);
    if (sso > ds)
      output(buf + ds, sso - ds);
    o_standup();
    output(buf + st_start, st_end - st_start);
    o_standdown();
    if (de > eso)
      output(buf + eso, de - eso);
  }
}

void
redoscr()
{
  register screenline *bp;
  register int i, j, len;

  o_clear();
  for (tc_col = tc_line = i = 0, j = roll; i < scr_lns; i++, j++)
  {
    if (j >= scr_lns)
      j = 0;
    bp = &big_picture[j];
    if (len = bp->len)
    {
      rel_move(0, i);

      if (bp->mode & STANDOUT)
	standoutput(bp->data, 0, len, bp->sso, bp->eso);
      else
	output(bp->data, len);
      tc_col += len;
      if (tc_col >= scr_cols)
      {
	if (automargins)
	  tc_col = scr_cols - 1;
	else
	{
	  tc_col -= scr_cols;
	  tc_line++;
	  if (tc_line >= t_lines)
	    tc_line = b_lines;
	}
      }
      bp->mode &= ~(MODIFIED);
      bp->oldlen = len;
    }
  }
  rel_move(cur_col, cur_ln);
  docls = scrollcnt = 0;
  oflush();
}

void
refresh()
{
  register screenline *bp = big_picture;
  register int i, j, len;

  if (icurrchar != ibufsize)
    return;

  if ((docls) || (abs(scrollcnt) >= (scr_lns - 3)))
  {
    redoscr();
    return;
  }

  if (scrollcnt < 0)
  {
    rel_move(0, 0);
    do
    {
      o_scrollrev();
    } while (++scrollcnt);
  }
  else if (scrollcnt > 0)
  {
    rel_move(0, b_lines);
    do
    {
      ochar('\n');
    } while (--scrollcnt);
  }

  for (i = 0, j = roll; i < scr_lns; i++, j++)
  {
    if (j >= scr_lns)
      j = 0;
    bp = &big_picture[j];
    len = bp->len;
    if (bp->mode & MODIFIED && bp->smod < len)
    {
      bp->mode &= ~(MODIFIED);
      if (bp->emod >= len)
	bp->emod = len - 1;
      rel_move(bp->smod, i);

      if (bp->mode & STANDOUT)
	standoutput(bp->data, bp->smod, bp->emod + 1, bp->sso, bp->eso);
      else
	output(&bp->data[bp->smod], bp->emod - bp->smod + 1);
      tc_col = bp->emod + 1;
      if (tc_col >= scr_cols)
      {
	if (automargins)
	  tc_col = scr_cols - 1;
	else
	{
	  tc_col -= scr_cols;
	  tc_line++;
	  if (tc_line >= t_lines)
	    tc_line = b_lines;
	}
      }
    }

    if (bp->oldlen > len)
    {
      rel_move(len, i);
      o_cleol();
    }
    bp->oldlen = len;
  }
  rel_move(cur_col, cur_ln);
  oflush();
}

void
clear()
{
  int i;
  screenline *slp;

  docls = YEA;
  cur_ln = cur_col = roll = i = 0;
  slp = big_picture;
  while (i++ < t_lines)
    memset(slp++, 0, 9);
}

void
clrchyiuan(int x,int y)
{
    register screenline *slp;
    register int i, j;

    for (i = x, j = i + roll; i < y; i++, j++)
    {
      if (j >= scr_lns)
        j -= scr_lns;
      slp = &big_picture[j];
      slp->mode = slp->len = 0;
      if (slp->oldlen)
        slp->oldlen = 255;
    }
}

void
clrtobot()
{
  clrchyiuan(cur_ln, scr_lns);
}

void
clrtoeol()
{
    register screenline *slp;
    register int ln;

    standing = NA;
    if ((ln = cur_ln + roll) >= scr_lns)
      ln -= scr_lns;
    slp = &big_picture[ln];
    if (cur_col <= slp->sso)
      slp->mode &= ~STANDOUT;

    if (cur_col > slp->oldlen)
    {
      for (ln = slp->len; ln <= cur_col; ln++)
	slp->data[ln] = ' ';
    }

    if (cur_col < slp->oldlen)
    {
      for (ln = slp->len; ln >= cur_col; ln--)
	slp->data[ln] = ' ';
    }

    slp->len = cur_col;
}

void 
outc(uschar c)
{
  register screenline *slp;
  register int i;

  if ((i = cur_ln + roll) >= scr_lns)
    i -= scr_lns;
  slp = &big_picture[i];

    if (c == '\n' || c == '\r')
    {
      if (standing)
      {
	slp->eso = BMAX(slp->eso, cur_col);
	standing = NA;
      }

      if ((i = cur_col - slp->len) > 0)
	memset(&slp->data[slp->len], ' ', i + 1);

      slp->len = cur_col;
      cur_col = 0;
      if (cur_ln < scr_lns)
	cur_ln++;
      return;
    }

  if (cur_col >= slp->len)
  {
    for (i = slp->len; i < cur_col; i++)
      slp->data[i] = ' ';
    slp->data[cur_col] = '\0';
    slp->len = cur_col + 1;
  }

  if (slp->data[cur_col] != c)
  {
    slp->data[cur_col] = c;
    if (!(slp->mode & MODIFIED))
      slp->smod = slp->emod = cur_col;
    slp->mode |= MODIFIED;
    if (cur_col > slp->emod)
      slp->emod = cur_col;
    if (cur_col < slp->smod)
      slp->smod = cur_col;
  }

#if 1
  {
    if(cur_col >= (uschar) scr_cols)
      outc('\n');
    else
      ++cur_col;
  }
#else
  /* Always false ? from ptt */
  if (++cur_col >= scr_cols)
  {
    if (standing && (slp->mode & STANDOUT))
    {
      standing = 0;
      slp->eso = BMAX(slp->eso, cur_col);
    }
    cur_col = 0;
    if (cur_ln < scr_lns)
      cur_ln++;
  }
#endif
}

void
edit_outs(const char *text)
{
  register int column = 0;
  register char ch;

  while ((ch = *text++) && (++column < t_columns))
    outc(ch == KEY_ESC ? '*' : ch);
  
  return ;
}

void
outs(const char *str)
{
  while (*str)
    outc(*str++);
}

void
outmsg(register char *msg)
{
  move(b_lines, 0);
  clrtoeol();
  while (*msg)
    outc(*msg++);
  
  refresh();
}

void
outz(register char *msg)
{
  outmsg(msg);
  sleep(1);
}

void
prints(char *fmt, ...)
{
  va_list args;
  char buff[1024];

  va_start(args, fmt);
  vsprintf(buff, fmt, args);
  va_end(args);
  outs(buff);
}

void scroll()
{
    scrollcnt++;
    if (++roll >= scr_lns)
      roll = 0;
    move(b_lines, 0);
    clrtoeol();
}

void rscroll()
{
    scrollcnt--;
    if (--roll < 0)
      roll = b_lines;
    move(0, 0);
    clrtoeol();
}


/* 		do_talk.c		    */

static void 
change_scroll_range(int top, int bottom)
{ 
  char buf[TERMCOMSIZE];
  
  sprintf(buf, "\033[%d;%dr", top+1, bottom+1);
  output(buf, strlen(buf));
}

static void
scroll_forward()
{
  output("\033D", 2);
}

void
region_scroll_up(int top, int bottom)
{
   int i;

   if (top > bottom) 
   {
      i = top; 
      top = bottom;
      bottom = i;
   }

   if (top < 0 || bottom >= scr_lns)
     return;

   for (i = top; i < bottom; i++)
      big_picture[i] = big_picture[i + 1];
   memset(big_picture + i, 0, sizeof(*big_picture));
   memset(big_picture[i].data, ' ', scr_cols);
   save_cursor();
   change_scroll_range(top, bottom);
   do_move(0, bottom);
   scroll_forward();
   change_scroll_range(0, scr_lns - 1);
   restore_cursor();
   refresh();
}

screenline *
get_currscl(int line)
{
  return (big_picture + line);
}

/*    		end do_talk.c		*/

void standout()
{
  if (!standing /*&& !dumb_term && strtstandoutlen*/)
  {
    register screenline *slp;

    slp = &big_picture[((cur_ln + roll) % scr_lns)];
    standing = YEA;
    slp->sso = slp->eso = cur_col;
    slp->mode |= STANDOUT;
  }
}


void standend()
{
  if (standing /*&& !dumb_term && strtstandoutlen*/)
  {
    register screenline *slp;

    slp = &big_picture[((cur_ln + roll) % scr_lns)];
    standing = NA;
    slp->eso = BMAX(slp->eso, cur_col);
  }
}

#define VS_STACK_SIZE 10
static int vs_stack_ptr = -1;          /* CityLion */                 
static int old_roll[VS_STACK_SIZE];                                   
static int my_newfd[VS_STACK_SIZE],x[VS_STACK_SIZE],y[VS_STACK_SIZE]; 
static int i_newfd = 0;
static int mode0[VS_STACK_SIZE],stat0[VS_STACK_SIZE];                 
                                                               
int vs_save(screenline *screen)                                                
{                                                              
  vs_stack_ptr++;                                              
  old_roll[vs_stack_ptr] = roll;
  
  if(currutmp)
  {
    mode0[vs_stack_ptr] = currutmp->mode;                        
    stat0[vs_stack_ptr] = currstat;                              
  }
  getyx(&y[vs_stack_ptr],&x[vs_stack_ptr]);                    
  memcpy(screen, big_picture, t_lines * sizeof(screenline));   
  my_newfd[vs_stack_ptr] = i_newfd;                            
  i_newfd = 0;
  return old_roll[vs_stack_ptr];	/* itoc.030723: ¶Ç¦^¥Ø«eªº roll */
}                                                           
                                                            
void vs_restore(screenline *screen)
{                                                           
  roll = old_roll[vs_stack_ptr];                            
  i_newfd = my_newfd[vs_stack_ptr];
  if(currutmp)
  {
    currstat = stat0[vs_stack_ptr];
    currutmp->mode = mode0[vs_stack_ptr];
  }
  memcpy(big_picture, screen, t_lines * sizeof(screenline));
  move(y[vs_stack_ptr],x[vs_stack_ptr]);                    
  vs_stack_ptr--;                                           
  free(screen);                                             
  redoscr();                                                
  refresh();                                                
}                                                           

void capture_screen()
{
   char fname[PATHLEN];
   FILE* fp;
   int i;

   sethomefile(fname, cuser.userid, "buf.0");
   if (fp = fopen(fname, "w")) 
   {
      for (i = 0; i < scr_lns; i++)
         fprintf(fp, "%.*s\n", big_picture[i].len, big_picture[i].data);
      fclose(fp);
   }
}

/*-------------------------------------------------------*/
/* io.c         ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : basic console/screen/keyboard I/O routines   */
/*-------------------------------------------------------*/
#ifdef AIX
#include <sys/select.h>
#endif

void passwd_outs(char *text)
{
  register int column = 0;
  register char ch;
  while ((ch = *text++) && (++column < 80))
  {
    outc('*');
  }
}

/* ----------------------------------------------------- */
/* ©w®ÉÅã¥Ü°ÊºA¬ÝªO                                      */
/* ----------------------------------------------------- */
#define STAY_TIMEOUT    (30*60)

static void
hit_alarm_clock()
{
  static int stay_time = 0;
  static int idle_time = 0;

  time_t now = time(0);
  char buf[100]="\0";

  if(currutmp->pid != currpid)
    setup_utmp(XMODE);   /* ­«·s°t¸m shm */

#ifdef  DOTIMEOUT
  if((idle_time = now - currutmp->lastact) > IDLE_TIMEOUT && !HAS_PERM(PERM_RESEARCH))
#else
  if((idle_time = now - currutmp->lastact) > !HAS_PERM(PERM_RESEARCH))
#endif
  {
    outmsg("¶W¹L¶¢¸m®É¶¡¡I½ð¥X¥hÅo¡K¡K");
    abort_bbs();
  }

  if (HAS_HABIT(HABIT_MOVIE) && (currstat && (currstat < CLASS || currstat == MAILALL)))
    movie(0);

  alarm(MOVIE_INT);
  stay_time += MOVIE_INT;

#ifdef  DOTIMEOUT
  if(idle_time > IDLE_TIMEOUT - 60 && !HAS_PERM(PERM_RESEARCH)) 
#else
  if(idle_time > 60 && !HAS_PERM(PERM_RESEARCH))
#endif
    sprintf(buf, "[1;5;37;41mÄµ§i¡G±z¤w¶¢¸m¹L¤[¡A­YµL¦^À³¡A¨t²Î§Y±N¤ÁÂ÷¡I¡I[m");
  else if(stay_time > 10 * 60 && chkmail(0)) 
  {
    sprintf(buf, "\033[1;33;41m[%s] «H½cùØÁÙ¦³¨S¬Ý¹Lªº«H­ò\033[m",
      Etime(&now));
    stay_time = 0 ;
  }
  else if(stay_time > STAY_TIMEOUT && HAS_HABIT(HABIT_ALARM))
  {
    /* ¦b³o¸Ì´£¥Ü user ¥ð®§¤@¤U */
    char *msg[10] = {
    "¦ù¦ù¸y, ´|´|²´, ³Ü¤f¯ù....³Ý¤f®ð...¦AÄ~Äò...!",
    "¤@Ãä¬O¤Í±¡ ¤@Ãä¬O·R±¡ ¥ª¥kªº¬G¨Æ¬°ÃøµÛ¦Û¤v...",
    "¬O§_¦³¤HÁA¸Ñ±z¤º¤ßªº©t±I? ¤j®a¨Ótalk talk§a.. ",
    "¥ª¤T°é,¥k¤T°é,²ä¤l§á§á§¾ªÑ§á§á ¤j®a¨Ó§@¹B°Ê­ò~",
    "§ÚÄé..§ÚÄé..§ÚÄéÄéÄé! Äé¨ìµwºÐÃz±¼...",
    "¥Î¡E¥\\¡E°á¡E®Ñ",
    "®Ñ©À§¹¤F¨S°Ú....^.^",
    "©ú¤Ñ¦³¨S¦³¦Ò¸Õ°Ú...©À®Ñ­«­n­ò...!",
    "¬Ý¡ã¬y¬P¡I",
    "¡E®Ñ¦b¤ß¤¤®ð¦Û¬Ó¡EÅª®Ñ¥h¡E"};
    int i = rand() % 10;

    sprintf(buf, "[1;33;41m[%s] %s[m", Etime(&now), msg[i]);
    stay_time = 0 ;
  }

  if(buf[0]) 
  {
    outmsg(buf);
    bell();
  }
}

void init_alarm()
{
  alarm(0);
  signal(SIGALRM, hit_alarm_clock);
  alarm(MOVIE_INT);
}

/* ----------------------------------------------------- */
/* input routines                                        */
/* ----------------------------------------------------- */
static struct timeval i_to, *i_top = NULL;

void add_io(int fd, int timeout)
{
  i_newfd = fd;
  if (timeout)
  {
    i_to.tv_sec = timeout;
    i_to.tv_usec = 16384;	/* Ptt: §ï¦¨16384 Á×§K¤£«ö®Éfor loop¦Ycpu
    				 * time 16384 ¬ù¨C¬í64¦¸ */
    i_top = &i_to;
  }
  else
    i_top = NULL;
}

int
num_in_buf(void)
{
  return ibufsize - icurrchar;
}

static int
iac_count(char *current) 
{
  switch (*(current + 1) & 0xff) 
  {
    case DO:
    case DONT:
    case WILL:
    case WONT:
      return 3;

    case SB:                     /* loop forever looking for the SE */
    {
      register char *look = current + 2;

      /* fuse.030518: ½u¤W½Õ¾ãµe­±¤j¤p¡A­«§ì b_lines */

      if ((*look & 0xff) == TELOPT_NAWS)
      {
        t_lines = *(look + 4) & 0xff;
	if (t_lines < 24)
	  t_lines = 24;

        chscr(t_lines);
      }

      for (;;) 
      {
        if((*look++ & 0xff) == IAC) 
        {
          if((*look++ & 0xff) == SE) 
            return look - current;
        }
      }
    }

    default:
      return 1;
  }
}

static int
dogetch()
{
  int ch;

  if(currutmp) 
    time(&currutmp->lastact);

  for (;;)
  {
    if (ibufsize == icurrchar)
    {
      fd_set readfds;
      struct timeval to;
      register int nfds;

      FD_ZERO(&readfds);
      FD_SET(0, &readfds);

      if (i_newfd)
      {
        FD_SET(i_newfd, &readfds);
        nfds = i_newfd + 1;
      }
      else
        nfds = 1;

      to.tv_sec = to.tv_usec = 0;
      if ((ch = select(nfds, &readfds, NULL, NULL, &to)) <= 0)
      {
          refresh();

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        if (i_newfd)
          FD_SET(i_newfd, &readfds);

        while ((ch = select(nfds, &readfds, NULL, NULL, i_top)) < 0)
        {
          /*
          if (errno == EINTR)
            continue;
          else
          */
          if(errno != EINTR)
          {
            perror("select");
            return -1;
          }
        }
        if (ch == 0)
          return I_TIMEOUT;
      }
      if (i_newfd && FD_ISSET(i_newfd, &readfds))
        return I_OTHERDATA;

      for(;;) 
      {
        ch = read(0, inbuf, IBUFSIZE);
        if(ch > 0) break;
        if((ch < 0) && (errno == EINTR)) continue;

        /* longjmp(byebye, -1); */
	raise(SIGHUP);	/* jochang: don't know if this is necessary */
      }

      icurrchar = (*inbuf & 0xff) == IAC ? iac_count(inbuf) : 0;
      if(icurrchar >= ch)
      {
        icurrchar = ibufsize;
        continue;
      }
      ibufsize = ch;
      i_mode = INPUT_ACTIVE;
    }

    ch = inbuf[icurrchar++];
    return (ch);
  }
}

extern char oldmsg_count;            /* pointer */    

int igetch()
{
    register int ch;
    while(ch = dogetch())
    {
     switch (ch)
      {
       case Ctrl('L'):
         redoscr();
         continue;
       case Ctrl('I'):
         if(currutmp != NULL && currutmp->mode == MMENU)
         {
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
           vs_save(screen);
           t_idle();
           vs_restore(screen);
           continue;
         }
         else return(ch);

       case Ctrl('Q'):  /* wildcat : §Ö³tÂ÷¯¸ :p */
         if(currutmp->mode && currutmp->mode != READING)
         {
           if(answer("\033[0;30;47m  ½T©w­nÂ÷¯¸?? (y/N)  \033[m") != 'y')
             return(ch);
           update_data();
           u_exit("ABORT");
           pressanykey("ÁÂÁÂ¥úÁ{, °O±o±`¨Ó³á !");
           exit(0);
         }
         else return (ch);

       case Ctrl('U'):
         resetutmpent();
         if(currutmp != NULL && currutmp->mode != EDITING &&
            currutmp->mode != LUSERS && currutmp->mode)
         {
           int mode0 = currutmp->mode;
           int stat0 = currstat;
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

           vs_save(screen);
           t_users();
           vs_restore(screen);

           currutmp->mode = mode0;
           currstat = stat0;

           continue;
         }
         else return (ch);

        case Ctrl('R'):
        {
          if(currutmp == NULL) return (ch);
          else if(watermode > 0)
          {
            watermode = (watermode + oldmsg_count)% oldmsg_count + 1;
            t_display_new(0);
            continue;
          }
          else if (!currutmp->mode && (currutmp->chatid[0] == 2 ||
               currutmp->chatid[0] == 3) && oldmsg_count && !watermode)
          {
            watermode=1;
            t_display_new(1);
            continue;
          }
          else if (currutmp->msgs[0].last_pid && currutmp->mode)
          {
            screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
            vs_save(screen);
            watermode=1;
            t_display_new(1);
            my_write(currutmp->msgs[0].last_pid, "¤ô²y¥á¦^¥h¡G");
            vs_restore(screen);
            continue;
          }
          else return (ch);
        }

        case '\n':   /* Ptt§â \n®³±¼ */
           continue;
        case Ctrl('T'):
          if(watermode > 0 )
          {
            watermode = (watermode + oldmsg_count - 2 )% oldmsg_count + 1;
            t_display_new(0);
            continue;
          }

        default:
          return (ch);
       }
    }
}

int offset_count(char *prompt)   /* Robert Liu 20010813 */
{
  int i=0, j=0, off=0;

  for(;prompt[i];i++)
  {
    if(prompt[i]==27)
      off=1;
    if(off==0) 
      j++;
    if(*prompt=='m' && off==1) 
      off=0;
  }
  
  return (i-j);
}

int 
getdata(int line, int col, char *prompt, char *buf, int len, int echo, char *ans)
{
  register int ch;
  int clen;
  int x, y;
  int off_set = 0; /*add color*/
#define MAXLASTCMD 6
  static char lastcmd[MAXLASTCMD][80];

  if (prompt)
  {
    move(line, col);
    clrtoeol();
    outs(prompt);
    off_set=offset_count(prompt); /*add color*/
  }
  else
    clrtoeol();

  if (echo == NOECHO)
  {
    len--;              /* ¤U­±³o¬qµ{¦¡½X¬O¨S¦³¤Ï¥Õ (!echo) */
    clen = 0;
    while ((ch = igetkey()) != '\r')
    {
      if (ch == '\n')
        break;
      if (ch == KEY_BKSP || ch == Ctrl('H'))
      {
        if (!clen)
          bell();
        else
          clen--;
          
        continue;
      }

#ifdef BIT8
      if (!isprint2(ch))
#else
      if (!isprint(ch))
#endif
      {
        continue;
      }

      if (clen >= len)
        continue;

      buf[clen++] = ch;
    }
    buf[clen] = '\0';
    outc('\n');
    oflush();
  }
  else
  {
   int cmdpos = MAXLASTCMD -1;
   int currchar = 0;
   int keydown;
   int dirty, i;

    getyx(&y, &x);
    x=x-off_set;  /*add color by hialan*/
    standout();
    for (clen = len; clen; clen--)
      outc(' ');
    standend();

    if (ans != NULL) 
    {
       str_ansi(buf, ans, len);
       move(y, x);
       edit_outs(buf);
       clen = currchar = strlen(buf);
    }
    
    /* ¦]¬° buf ¦³¥i¯à·|©M ans ¬Û¦P , ©Ò¥H¥ý§âªì©l¦r¦ê§¹¦¨«á */
    /* ¦A®Ú¾Ú clen ±N¦r¦ê«á­±²MªÅ */
    memset(buf+clen, 0, len-clen);

    len--;
    dirty = 0;
    while (move(y, x + currchar), (ch = igetkey()) != '\r')
    {
       keydown = 0;
       switch (ch) 
       {
       case Ctrl('Y'): 
       {
          if (clen && dirty) 
          {
             for (i = MAXLASTCMD - 1; i; i--)
                strcpy(lastcmd[i], lastcmd[i - 1]);
             strlcpy(lastcmd[0], buf, sizeof(lastcmd[0]) );
          }

          move(y, x);
          for (clen = len; clen; clen--)
            outc(' ');
          memset(buf, '\0', len);
          clen = currchar = 0;
          continue;
       }

       case KEY_DOWN:
       case Ctrl('N'):
          keydown = 1;
       case Ctrl('P'):
       case KEY_UP: 
       {
          if (clen && dirty) 
          {
             for (i = MAXLASTCMD - 1; i; i--)
                strcpy(lastcmd[i], lastcmd[i - 1]);
             strlcpy(lastcmd[0], buf, sizeof(lastcmd[0]) );
          }

          i = cmdpos;
          do 
          {
             if (keydown)
                --cmdpos;
             else
                ++cmdpos;
             if (cmdpos < 0)
                cmdpos = MAXLASTCMD - 1;
             else if (cmdpos == MAXLASTCMD)
                cmdpos = 0;
          } while (cmdpos != i && (!*lastcmd[cmdpos] || !strncmp(buf, lastcmd[cmdpos], len)));
          if (cmdpos == i)
             continue;

          strncpy(buf, lastcmd[cmdpos], len);
          buf[len] = 0;

          move(y, x);                   /* clrtoeof */
          for (i = 0; i <= clen; i++)
             outc(' ');
          move(y, x);

          if (echo == PASS)
            passwd_outs(buf);
          else
            edit_outs(buf);
          clen = currchar = strlen(buf);
          dirty = 0;
          continue;
       }
       case KEY_ESC:
         if(currutmp)
         {
           if (KEY_ESC_arg == 'c')
              capture_screen();
              
           if (KEY_ESC_arg == 'n')
              edit_note();
         }
         continue;

       /* yagami.000504 : ´å¼Ð¥i¨ì³Ì«e©Î³Ì«á */ 
       case Ctrl('A'):
       case KEY_HOME:
         currchar = 0;
         break;
       case Ctrl('E'):
       case KEY_END:
         currchar = clen;
         break;

       case KEY_LEFT:
          if (currchar)
             --currchar;
          continue;
       case KEY_RIGHT:
          if (buf[currchar])
             ++currchar;
          continue;
       case Ctrl('K'):
       {
         buf[currchar] = 0;
         move(y, x + currchar);
         for (i = currchar; i < clen; i++)
            outc(' ');
         clen = currchar;
         dirty = 1;
         continue;
       }
 
       case Ctrl('D'): 
       {
         if (buf[currchar]) 
         {
            clen--;
            for (i = currchar; i <= clen; i++)
               buf[i] = buf[i + 1];
            move(y, x + clen);
            outc(' ');
            move(y, x);
            if (echo == PASS)
              passwd_outs(buf);
            else
              edit_outs(buf);
            dirty = 1;
         }
         continue;
       }       
       
       case KEY_BKSP:
       case Ctrl('H'):
       {
         if (currchar) 
         {
            currchar--;
            clen--;
            for (i = currchar; i <= clen; i++)
               buf[i] = buf[i + 1];
            move(y, x + clen);
            outc(' ');
            move(y, x);
            if (echo == PASS)
              passwd_outs(buf);
            else
              edit_outs(buf);
            dirty = 1;
         }
         continue;
       }       
       
       case Ctrl('I'):
         if(currstat != IDLE && !(currutmp->mode == 0 &&
            (currutmp->chatid[0] == 2 || currutmp->chatid[0] == 3))) 
         {
           t_idle();
         }
         continue;

       } /* switch(ch) */

      if (ch == '\n' || ch == '\r')
         break;

      if (!(isprint2(ch)) || clen >= len || x + clen >= scr_cols)
        continue;

      if (buf[currchar]) 
      {     		                /* insert */
         for (i = currchar; buf[i] && i < len && i < 80; i++)
            ;
         buf[i + 1] = 0;
         for (; i > currchar; i--)
            buf[i] = buf[i - 1];
      }
      else                              /* append */
         buf[currchar + 1] = '\0';

      buf[currchar] = ch;
      move(y, x + currchar);
      if (echo == PASS)
        passwd_outs(buf + currchar);
      else
      /* shakalaca.990422: ­ì¥»¥u¦³¤U­±¨º¦æ, ³o¬O¬°¤F¿é¤J passwd ¦³¤Ï¥Õ */
        edit_outs(buf + currchar);
      currchar++;
      clen++;
      dirty = 1;
    }
    buf[clen] = '\0';

    if (clen > 1 && echo != PASS) 
    /* shaklaaca.990514: ^^^^^^^ ¤£Åý¿é¤Jªº password ¯d¤U¬ö¿ý */
    {
       for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
          strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
       strlcpy(lastcmd[0], buf, sizeof(lastcmd[0]) );
    }

    move(y, x + clen);
    outc('\n');
    refresh();
  }

  if(echo == LCECHO)
    buf[0] = tolower(buf[0]);

  return clen;
}

/*  getans2 ¶Ç­È¸ÑÄÀ  by hialan 2002/4/14
                                                                                
line    --> ²Ä´X¦C
col     --> ²Ä´X­Ó¦r
prompt  --> ´£¥Ü
s       --> ¿ï¶µ®æ¦¡
  ¨Ò¦p:
    char *test[2] = {"yY.¬O","nN.§_"};
  ²Ä¤@­Ó¤p¼gy¬O§Ö³tÁä
  ²Ä¤G­Ó¥H«á¥Î¨ÓÅã¥Ü
many    --> ¦³´X­Ó¿ï¶µ
def     --> ¹w³]­È
  ¦P¤W¨Ò:
    getans2(b_line,0,"", test, 2, 'y');
                                                                                
*/
                                                                                
char 
getans2(int line, int col, char *prompt, char **s, int many, char def)
{
  int point = 0;        /*«ü¼Ð*/
  int i, ch;
  char bar_color[50], buf2[128];
  char *p;      	/*°T®§ªº«ü¼Ð*/

  /*¹w³]­È*/
  if(!s) s = msg_choose;
  
  if(many <= 0) 
    return 0;	/* ¿ù»~: ¨S¦³¿ï¶µ */
  else if(many == 1) 
    return *(s[0]);	/* ¥u¦³¤@­Ó¿ï¶µ¿ï¤°»ò? */
  
  for(i = 0;i < many;i++)
  {
    p = s[i];
    if(def == *p)
    {
      point = i;
      break;
    }
  }
  
  /* §ï¥Î getdata */                                                                              
  if(!HAVE_HABIT(HABIT_LIGHTBAR))
  {
    char tmp[128],tmp2[2];
                                                                                
    buf2[0] = '\0';
    tmp2[0] = (char) toupper(*(s[point]));
    tmp2[1] = '\0';
                                                                                
    strcpy(tmp,prompt);
                                                                                
    for(i = 0; i < many; i++)
    {
      p = s[i];
      strcat(tmp, p+1);
      strcat(tmp, " ");
    }
                                                                                
    sprintf(tmp,"%s[%c] ", tmp, tmp2[0]);
    getdata(line, col, tmp, buf2, 4, LCECHO, 0);
                                                                                
    if(*buf2 == '\0')
      return *tmp2;
    else
      return *buf2;
  }
                                                                                
  get_lightbar_color(bar_color);
  
  /*²MªÅ¿Ã¹õ*/
  move(line, 0);
  clrtoeol();
                                                                                
  do
  {
    move(line, col);
    clrtoeol();
    if (prompt) outs(prompt);
    
    for(i = 0;i < many;i++)
    {
      p = s[i];
      if(i == point)
        prints("\033[1m[\033[m%s%s\033[0;1m]\033[m",bar_color,p+1);
      else
        prints(" %s ",p+1);
    }

    move(b_lines,0);
                                                                                
    ch = igetkey();
    switch(ch)
    {
      case KEY_TAB:
      case KEY_DOWN:
      case KEY_RIGHT:
        point++;
        if(point >= many)
          point = 0;
        break;
      case KEY_UP:
      case KEY_LEFT:
        point--;
        if(point < 0)
          point = many - 1;
        break;
                                                                                
      default:
        ch = tolower(ch);
        
        for(i = 0;i < many;i++)
        {
          p = s[i];
          if(ch == tolower(*p))
            point = i;
        }
        break;
    }
  }while(ch != '\r');
                                                                                
  return *(s[point]);
}

int igetkey()
{
  int mode;
  int ch, last;

  mode = last = 0;
  while (1)
  {
    ch = igetch();
    if (mode == 0)
    {
      if (ch == KEY_ESC)
        mode = 1;
      else
        return ch;              /* Normal Key */
    }
    else if (mode == 1)
    {                           /* Escape sequence */
      if (ch == '[' || ch == 'O')
        mode = 2;
      else if (ch == '1' || ch == '4')
        mode = 3;
      else
      {
        KEY_ESC_arg = ch;
        return KEY_ESC;
      }
    }
    else if (mode == 2)
    {                           /* Cursor key */
      if (ch >= 'A' && ch <= 'D')
        return KEY_UP + (ch - 'A');
      else if (ch >= '1' && ch <= '6')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 3)
    {                           /* Ins Del Home End PgUp PgDn */
      if (ch == '~')
        return KEY_HOME + (last - '1');
      else
        return ch;
    }
    last = ch;
  }
}


/* term.c */
#include <termios.h>
#include <syslog.h>

int             tgetent(const char *bp, char *name);
char           *tgetstr(const char *id, char **area);
int             tgetflag(const char *id);
int             tgetnum(const char *id);
int             tputs(const char *str, int affcnt, int (*putc) (int));
char           *tparm(const char *str,...);
char           *tgoto(const char *cap, int col, int row);

/* ----------------------------------------------------- */
/* basic tty control                                     */
/* ----------------------------------------------------- */
void
init_tty()
{
    struct termios tty_state, tty_new;

    if (tcgetattr(1, &tty_state) < 0) {
	syslog(LOG_ERR, "tcgetattr(): %m");
	return;
    }
    memcpy(&tty_new, &tty_state, sizeof(tty_new));
    tty_new.c_lflag &= ~(ICANON | ECHO | ISIG);
    /*
     * tty_new.c_cc[VTIME] = 0; tty_new.c_cc[VMIN] = 1;
     */
    tcsetattr(1, TCSANOW, &tty_new);
    system("stty raw -echo");
}

/* ----------------------------------------------------- */
/* init tty control code                                 */
/* ----------------------------------------------------- */


#define TERMCOMSIZE (40)

#if 0
static char    *outp;
static int     *outlp;

static int
outcf(int ch)
{
    if (*outlp < TERMCOMSIZE) {
	(*outlp)++;
	*outp++ = ch;
    }
    return 0;
}
#endif

static void
term_resize(int sig)
{
    struct winsize  newsize;

    signal(SIGWINCH, SIG_IGN);	/* Don't bother me! */
    ioctl(0, TIOCGWINSZ, &newsize);

    /* make sure reasonable size */
    newsize.ws_row = BMAX(24, BMIN(100, newsize.ws_row));
    newsize.ws_col = BMAX(80, BMIN(200, newsize.ws_col));

    chscr(newsize.ws_row);

    signal(SIGWINCH, term_resize);
}

int
shell_term_init()
{
    signal(SIGWINCH, term_resize);
    return YEA;
}

