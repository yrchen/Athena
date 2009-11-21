/*-------------------------------------------------------*/
/* stuff.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : utility routines                             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"
#include <stdarg.h>
void bell();
void 
setbdir(char *buf, char *boardname)
{
  sprintf(buf, "boards/%s/%s", boardname,
    currmode & MODE_DIGEST ? fn_mandex : ".DIR");
}

int 
invalid_fname(char *str)
{
  char ch;

  if (strspn(str, ".") == strlen(str))
     return 1;

  while (ch = *str++)
  {
    if (not_alnum(ch) && !strchr("@[]-._", ch))
      return 1;
  }
  return 0;
}


int
invalid_pname(str)
  char *str;
{
  char *p1, *p2, *p3;

  p1 = str;
  while (*p1) 
  {
    if (!(p2 = strchr(p1, '/')))
      p2 = str + strlen(str);
    
    if (p1 + 1 > p2 || p1 + strspn(p1, ".") == p2)
      return 1;
    
    for (p3 = p1; p3 < p2; p3++)
      if (not_alnum(*p3) && !strchr("@[]-._", *p3))
        return 1;
    
    p1 = p2 + (*p2 ? 1 : 0);
  }
  return 0;
}



int
valid_ident(ident)
  char *ident;
{
  static char *invalid[] = {"unknown@", "root@", "gopher@", "bbs@",
  "@bbs", "guest@", "@ppp", "@slip", NULL};
  char buf[128];
  int i;

//  str_lower(buf, ident);
  for (i = 0; invalid[i]; i++)
    if (strcasestr(buf, invalid[i]))
      return 0;
  return 1;
}


/*
woju
*/
int userid_is_BM(char *userid, char *list)     /* ªO¥D¡GBM list */
{
  register int ch, len;

  ch = list[0];
  if ((ch > ' ') && (ch < 128))
  {
    len = strlen(userid);
    do
    {
      if (!strncasecmp(list, userid, len))
      {
        ch = list[len];
        if ((ch == 0) || (ch == '/') || (ch == ']'))
          return 1;
      }
      while (ch = *list++)
      {
        if (ch == '/')
          break;
      }
    } while (ch);
  }
  return 0;
}

/* ----------------------------------------------------- */
/* ÀÉ®×ÀË¬d¨ç¼Æ¡GÀÉ®×¡B¥Ø¿ý¡BÄÝ©ó                        */
/* ----------------------------------------------------- */
off_t
dashs(fname)
  char *fname;
{
  struct stat st;

  if (!stat(fname, &st))
        return (st.st_size);
  else
        return -1;
}


long
dasht(fname)
  char *fname;
{
  struct stat st;

  if (!stat(fname, &st))
        return (st.st_mtime);
  else
        return -1;
}


int
dashl(fname)
  char *fname;
{
  struct stat st;

  return (lstat(fname, &st) == 0 && S_ISLNK(st.st_mode));
}

/*
dashf(fname)
  char *fname;
{
  return S_ISREG(f_mode(fname));
}


int
dashd(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}
*/

int
belong(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[80], *ptr;

    while (fgets(buf, 80, fp))
    {
      if ((ptr = strtok(buf, str_space)) && !strcasecmp(ptr, key))
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
belong_spam(filelist, key)
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


char *
Cdatelite(clock)
  time_t *clock;
{
  static char foo[18];
  struct tm *mytm = localtime(clock);

  strftime(foo, 18, "%D %T", mytm);
  return (foo);
}


char *
Cdate(clock)
  time_t *clock;
{
  static char foo[18];
  struct tm *mytm = localtime(clock);

  strftime(foo, 18, "%D %T %a", mytm);
  return (foo);
}


void
pressanykey_old(char *fmt, ... )
{
  va_list ap;
  char msg[128];
  int ch;

  msg[0]=0;
  
  if(fmt)
  {
    va_start(ap, fmt);
    vsprintf(msg, fmt, ap);
    va_end(ap);
  }
  
  if (msg[0])
  {
    move(b_lines, 0); clrtoeol();
    prints(COLOR1"[1m¡¹ [37m%-54s  "COLOR2"[ªÅ¥Õ]©Î ESC_c¼È¦s [m", msg);
  }
  else
  {
    outmsg("\033[1;36m                                            \
 ¢j¢k¢l¢m¢n¢o¢p \033[1;37m½Ð«ö¥ô·NÁäÄ~Äò \033[1;36m¢p\033[m");
  }
  
  do
  {
    ch = igetkey();
    if (ch == KEY_ESC && KEY_ESC_arg == 'c')
      capture_screen();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  move(b_lines, 0);
  clrtoeol();
  refresh();
}

void
pressanykey(char *fmt, ... )
{
  va_list ap;
  char msg[128];
  int ch;
  screenline *nowslt = (screenline *)calloc(t_lines, sizeof(screenline));

  msg[0]=0;
  
  if(fmt)
  {
    va_start(ap, fmt);
    vsprintf(msg, fmt, ap);
    va_end(ap);
  }

  if (!*msg)
  {
    pressanykey_old(NULL);
    return;
  }

  msgbox(nowslt, "«ö¥ô·NÁäÄ~Äò....", msg);
  
  do
  {
    ch = igetkey();
    if (ch == KEY_ESC && KEY_ESC_arg == 'c')
      capture_screen();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  vs_restore(nowslt);
  refresh();
}

void stand_title(char *title)
{
  clear();
  prints("%s[1m¡i %s ¡j[m\n", COLOR1, title);
}

/* opus : cursor position */
void cursor_show(int row, int column)
{
  int i, len;

  if((len = strlen(cuser.cursor))< 2)
  {
    strcpy(cuser.cursor, STR_CURSOR);
    len = strlen(cuser.cursor);
  }
  
  i = (rand()%(len/2))*2;
  move(row, column);
  prints("%c%c", cuser.cursor[i], cuser.cursor[i+1]);
  move(row, column + 1);
}

void
cursor_clear(int row, int column)
{
  move(row, column);
  outs(STR_UNCUR);
}

int
cursor_key(int row, int column)
{
  int ch;

  cursor_show(row, column);
  ch = igetkey();
  cursor_clear(row, column);
  return ch;
}

/* wildcat 981218 */
#define INTEREST_TIME	86400*7	/* wildcat:7¤Ñµo©ñ¤@¦¸§Q®§ */
#define BANK_RATE	1.06	/* wildcat:»È¦æ§Q²v 1.06  */
#define SONG_TIME	86400	/* hialan :¤@¤Ñµo¤@¦¸§ë²¼¦¸¼Æ */

void
update_data()
{
  int add = (time(0) - update_time)/30, change=0;
  static userec tuser;

  do_getuser(cuser.userid, &tuser);
  
  if((time(0) - tuser.update_songtime) >= SONG_TIME || tuser.songtimes > 5)
  {
    tuser.songtimes = 5;
    tuser.update_songtime = time(0);
    change = 1;
  }

  if((time(0) - tuser.dtime) >= INTEREST_TIME && tuser.silvermoney)
  {
    if(tuser.scoretimes < 100) tuser.scoretimes = 100; /*¤å³¹µû¤Àµo¦¸¼Æ*/
    tuser.silvermoney *= BANK_RATE;
    tuser.dtime = time(0);
    change = 1;
  }  
  
  if(add)
  {
    tuser.silvermoney += add*5;
    tuser.totaltime += (time(0)-update_time);
    update_time = time(0);
    change = 1;
  }
  cuser = tuser;
  if(change==1)
    substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
}

#if 0
/*---------------------------------------------------------------------*/
/* int chyiuan_ansi(buf,str,max)ªº¥Îªk:¦Ûµe¹³¥Î                        */
/* buf:chyiuan_ansi¹L«áªºstring                                        */
/* str:chyiuan_ansi¤§«eªºstring                                        */
/* count:¶Ç¦^move®ÉÀ³¸Óshiftªº¼Æ­È                                     */
/* ³Æµù:¦pªG¬O±m¦â¼Ò¦¡, ¶W¹L­­¨î­È®É, ·|¬å±¼¦r¦ê¶W¹L³¡¥÷, ¦ý«O¯dcolor  */
/*---------------------------------------------------------------------*/

int
chyiuan_ansi(buf,str,max)
  char *buf,*str;
  int max;
{
  int count = 0;
  int count0 = 0;
  int count1 = 0;
  char buf0[256];
  
  count0 = strip_ansi(buf0,str,0);
  if((cuser.uflag & COLOR_FLAG) && count0 <= max)
  {
    count1=strip_ansi(NULL,str,1);
    count=count1-count0;
    strcpy(buf, str);
  }
  else if((cuser.uflag & COLOR_FLAG) && count0 > max)
  {
    count0 = cut_ansistr(buf0,str,max);
    count1 = strip_ansi(NULL,buf0,1);
    count=count1-count0;
    strcpy(buf, buf0);
  }
  else
  {
    count=0;
    strcpy(buf, buf0);
  }
  return count;
}
#endif

int
answer(char *msg)
{
  outmsg(msg);

  return tolower(igetkey());
}

#if defined(SunOS) || defined(SOLARIS)

#include <syslog.h>

void
setenv(name, value, overwrite)
  const char *name;
  const char *value;
  int overwrite;
{
  if (overwrite || (getenv(name) == 0))
  {
    char *p;

    if ((p = malloc(strlen(name) + strlen(value) + 2)) == 0)
    {
      syslog(LOG_ERR, "out of memory\n");
      exit(1);
    }
    sprintf(p, "%s=%s", name, value);
    putenv(p);
  }
}

atexit(procp)
void (*procp)();
{
   on_exit(procp, 0);
}

#endif

void edit_note()
{
   char fname[PATHLEN];
   int mode0 = currutmp->mode;
   int stat0 = currstat;
   char c0 = *quote_file;

   *quote_file = 0;
   setutmpmode(NOTE);
   sethomefile(fname, cuser.userid, "note");
   vedit(fname, 0);
   currutmp->mode = mode0;
   currstat = stat0;
   *quote_file = c0;
}


char*
my_ctime(const time_t *t)
{
  struct tm *tp;
  static char ans[100];

  tp = localtime(t);
  sprintf(ans, "%d/%02d/%02d %02d:%02d:%02d", tp->tm_year % 100,
     tp->tm_mon + 1,tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
  return ans;
}
#if 0
/* ----------------------------------------------------- */
/* ¼È¦sÀÉ TBF (Temporary Buffer File) routines           */
/* ----------------------------------------------------- */

char *tbf_ask()
{
  static char fn_tbf[10] = "buf.0";
  getdata(b_lines, 0, "½Ð¿ï¾Ü¼È¦sÀÉ(0-9)¡G", fn_tbf + 4, 2, DOECHO,"0");
  return fn_tbf;
}
#endif

int
mail2user(char *muser, char *title, char *fname, int filemode)
{
  fileheader mhdr;
  char buf[256], buf1[80];

  sethomepath(buf1, muser);
  stampfile(buf1, &mhdr);

  if(filemode != 0)
    strcpy(mhdr.owner, "[³Æ.§Ñ.¿ý]");    
  else
    strcpy(mhdr.owner, cuser.userid);

  strcpy(mhdr.title, title);
  mhdr.savemode = 0;
  mhdr.filemode = filemode;
  sethomedir(buf, muser);
  rec_add(buf, &mhdr, sizeof(mhdr));
  f_cp(fname, buf1, O_TRUNC);
  return 0;
}

void
debug(mode)
  char *mode;
{
  time_t now = time(0);
  char buf[128];

  sprintf(buf, "%s %s %s\n", Etime(&now), mode, cuser.userid);      
  f_cat("debug",buf);
}

/*-------------------------------------------------------*/
/* register.c   ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : user register routines                       */
/*-------------------------------------------------------*/

/* origin: SOB & Ptt              */
/* modify: wildcat/980909         */
/* ½T»{user¬O§_³q¹Lµù¥U¡B¸ê®Æ¥¿½T */
check_register()
{
  char *ptr;
  char genbuf[200],buf[100];

  if(!HAS_PERM(PERM_POST) && (cuser.lastlogin - cuser.firstlogin >= 86400))
    cuser.userlevel |= PERM_POST;  

  stand_title("½Ð¸Ô²Ó¶ñ¼g­Ó¤H¸ê®Æ");

  while (strlen(cuser.username) < 2)
    getdata(2, 0, "ºï¸¹¼ÊºÙ¡G", cuser.username, 24, DOECHO,0);
  strcpy(currutmp->username, cuser.username);

  for (ptr = cuser.username; *ptr; ptr++)
    if (*ptr == 9)              /* TAB convert */
      strcpy(ptr, " ");

  while (strlen(cuser.feeling) < 2)
    getdata(3, 0, "¤ß±¡ª¬ºA¡G", cuser.feeling, 5, DOECHO,0);
  cuser.feeling[4] = '\0';
  strcpy(currutmp->feeling, cuser.feeling);

  while (strlen(cuser.realname) < 4)
    getdata(4, 0, "¯u¹ê©m¦W¡G", cuser.realname, 20, DOECHO,0);

  while (!cuser.month || !cuser.day || !cuser.year)
  {
    sprintf(genbuf, "%02i/%02i/%02i", cuser.year,cuser.month, cuser.day);
    getdata(6, 0, "¥X¥Í¦~¥÷¡A¦è¤¸ 19¡H", buf, 3, DOECHO,0);
    cuser.year = (buf[0] - '0') * 10 + (buf[1] - '0');
    getdata(7, 0, "¥X¥Í¤ë¥÷¡H", buf, 3, DOECHO,0);
    cuser.month = (buf[0] - '0') * 10 + (buf[1] - '0');
    getdata(8, 0, "¥X¥Í¤é´Á¡H", buf, 3, DOECHO,0);
    cuser.day = (buf[0] - '0') * 10 + (buf[1] - '0');
    if (cuser.month > 12 || cuser.month < 1 ||
      cuser.day > 31 || cuser.day < 1 || cuser.year > 90 || cuser.year < 40)
      continue;
    break;
  }

  while (cuser.sex > 7)
  {
    char buf;
    char *choose_sex[8]={"11)¸¯®æ","22)©j±µ","33)©³­}","44)¬ü¬Ü","55)Á¦¨û","66)ªü«¼","77)´Óª«","88)Äqª«"};

    buf = getans2(10, 0, "©Ê§O¡G", choose_sex, 8, cuser.sex + '1');    
    if (buf >= '1' && buf <= '8')
      cuser.sex = buf - '1';
  }

  if (belong_spam(BBSHOME"/etc/spam-list",cuser.email))
  {
    strcpy(cuser.email,"NULL");
    pressanykey("©êºp¡A¥»¯¸¤£±µ¨ü§Aªº E-Mail «H½c¦ì¸m");
  }
  
  if (!strchr(cuser.email, '@'))
  {
    bell();
    move(t_lines - 4, 0);
    prints("\
¡° ¬°¤F±zªºÅv¯q¡A½Ð¶ñ¼g¯u¹êªº E-mail address¡A ¥H¸ê½T»{»Õ¤U¨­¥÷¡A\n\
   ®æ¦¡¬° [44muser@domain_name[0m ©Î [44muser@\\[ip_number\\][0m¡C\n\n\
¡° ¦pªG±z¯uªº¨S¦³ E-mail¡A½Ðª½±µ«ö [return] §Y¥i¡C");

    do
    {
      getdata(12, 0, "¹q¤l«H½c¡G", cuser.email, 50, DOECHO,0);
      if (!cuser.email[0])
        sprintf(cuser.email, "%s%s", cuser.userid, str_mail_address);
      if(belong_spam(BBSHOME"/etc/spam-list",cuser.email))
      {
        strcpy(cuser.email, "NULL");
        pressanykey("©êºp¡A¥»¯¸¤£±µ¨ü§Aªº E-Mail «H½c¦ì¸m¡I");
      }
    } while (!strchr(cuser.email, '@'));

#ifdef  REG_MAGICKEY   
    mail_justify(cuser);
#endif

  }

  cuser.userlevel |= PERM_DEFAULT;
  if (!HAS_PERM(PERM_SYSOP) && !(cuser.userlevel & PERM_LOGINOK))
  {
    /* ¦^ÂÐ¹L¨­¥÷»{ÃÒ«H¨ç¡A©Î´¿¸g E-mail post ¹L */

    sethomefile(genbuf, cuser.userid, "email");
    if (dashf(genbuf))
      cuser.userlevel |= ( PERM_POST );

#ifdef  STRICT
    else
    {
      cuser.userlevel &= ~PERM_POST;
      more("etc/justify", YEA);
    }
#endif

  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
/* wildcat 981218 */
    clear();
    update_data(); 

#ifdef REG_FORM
    if (HAS_PERM(PERM_POST) && !HAS_PERM(PERM_LOGINOK)
      && answer("¬O§_­n¶ñ¼gµù¥U³æ  Y)½T©w  N)¨ú®ø¡H") == 'y')
    DL_func("SO/register.so:u_register");
#endif

  }
  if (HAS_PERM(PERM_DENYPOST) && !HAS_PERM(PERM_SYSOP))
    cuser.userlevel &= ~PERM_POST;
}

int 
game_log(char *game, char *fmt, ... )
{
  va_list ap;
  char desc[128];
  FILE *fp;
  time_t now=time(0);
  
  va_start(ap, fmt);
  vsprintf(desc, fmt, ap);
  va_end(ap);
  
  fp=fopen(FN_GAMELOG, "a+");
  fprintf(fp, "\033[1;37m%s \033[33m%s \033[32m%s \033[36m%s\033[m\n", 
              game, Etime(&now), cuser.userid, desc);
  fclose(fp);

  return 0;
}

/* check_personal_note() ªº¥\¥Î¸ò chkmail() ¤@¼Ë.. */
/* ©Ò¥H¥i¥H¥[¦b my_query() ¤¤:³o¼Ë§O¤Hquery®É´N¥i¥H¬Ý¨ì¦³¨S¦³·s¯d¨¥ */
/* ÁÙ¦³¥i¥H¥[¦b show_title() ¤¤:³o¼Ë¦³·s¯d¨¥®É´N·|¹³¦³·s«H¥ó¤@¼Ë¦btitle´£¥Ü³á! */

int check_personal_note(int newflag, char* userid)
{
 char fpath[PATHLEN];
 FILE *fp;
 int  total = 0;
 notedata myitem;
 char *fn_note_dat      = "pnote.dat";

 if (userid == NULL)
   sethomefile(fpath, cuser.userid, fn_note_dat);
 else
   sethomefile(fpath, userid, fn_note_dat);

 if ((fp = fopen(fpath, "r")) != NULL) {
   while (fread(&myitem, sizeof(myitem), 1, fp) == 1) {
     if (newflag)
       if (myitem.buf[0][0] == 0) total++;
     else
       total++;
   }
   fclose(fp);
   return total;
 }
 return 0;
}

void bell()
{
  char sound[3], *ptr;

  ptr = sound;
  memset(ptr, Ctrl('G'), sizeof(sound));
  if (HAS_HABIT(HABIT_BELL))
    write(1, ptr, sizeof(sound));
}

int
search_num(int ch, int max)
{
  int num;
  char genbuf[10];
  
  sprintf(genbuf, "%c", ch);  
  getdata(b_lines, 0, "¸õ¦Ü²Ä´X¶µ¡G", genbuf, 7, DOECHO, genbuf);
  if(genbuf[0] == '\0')
    return -1;
  
  num = atoi(genbuf);

  if(num <= 0)
    return 0;
  if(num > max)
    return max;
  return num-1;
}
