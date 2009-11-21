/*-------------------------------------------------------*/
/* edit.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : simple ANSI/Chinese editor                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

#define KEEP_EDITING    -2
#define SCR_WIDTH       80

static void insert_string(char *);

enum
{
  NOBODY, MANAGER, SYSOP
};

static textline *firstline = NULL;
static textline *lastline = NULL;
static textline *currline = NULL;
static textline *blockline = NULL;
static textline *top_of_win = NULL;
static textline *deleted_lines = NULL;

extern int local_article;
extern char real_name[20];
char line[WRAPMARGIN  + 2];
int currpnt, currln, totaln;
int curr_window_line;
int redraw_everything;
int insert_character;
int my_ansimode;
int raw_mode;
int edit_margin;
int blockln  = -1;
int blockpnt;
int prevln = -1;
int prevpnt;
int line_dirty;
int indent_mode;
int insert_c = ' ';

char fp_bak[] = "bak";

char save_title[STRLEN];

/* ¯¸Ã± */
static void
site_logo(FILE *fp)
{
  char temp[25];
  strncpy (temp,fromhost,24);
  temp[25]='\0';
  
  fprintf(fp, "\n--\n"
  	      "\033[1;36mùÝùùùùùßùÝùçùùùß\033[m  \033[1;37m¶®¨å®R¹q¤l§G§iÄæ¨t²Î\033[m \033[1;33m(Athena.ATCity.org)\033[m\n"
  	      "\033[1;37mùø    ùø¡@ùø    \033[m  §@ªÌ¡G\033[1;37m%-12s\033[m  ¨Ó¦Û¡G%-24.24s\n"
  	      "\033[1;46;37mùàùùùùùâ  ùø\033[0;46m                                                                  \033[m\n"
  	      "\033[1;36mùã    ùå  ùãùù¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w¢w\033[m Powered by [\033[1;44;37mAT\033[m]Clan\n", 
  	      cuser.userid, temp);
}

int 
beginsig(char* s)
{
   char buf[10];

   strncpy(buf, s, 9);
   buf[9] = 0;
   strtok(buf, "\n\r");
   trim2(buf);
   return !strcmp(buf, "--");
}

/* ----------------------------------------------------- */
/* °O¾ÐÅéºÞ²z»P½s¿è³B²z                                  */
/* ----------------------------------------------------- */

static void
indigestion(i)
{
  fprintf(stderr, "ÄY­«¤º¶Ë %d\n", i);
}

/* ----------------------------------------------------- */
/* Thor: ansi ®y¼ÐÂà´«  for color ½s¿è¼Ò¦¡               */
/* ----------------------------------------------------- */
static int
ansi2n(int ansix, textline * line)
{
  register char *data, *tmp;
  register char ch;

  data = tmp = line->data;

  while(*tmp) 
  {
    if (*tmp == KEY_ESC)
    {
      while((ch = *tmp) && !isalpha(ch))
        tmp++;
      if (ch)
        tmp++;
      continue;
    }
    if (ansix <= 0) break;
    tmp++;
    ansix--;
  }
  return tmp - data;
}

static int
n2ansi(int nx, textline * line)
{
  register ansix = 0;
  register char *tmp,*nxp;
  register char ch;

  tmp = nxp = line->data;
  nxp += nx;

  while(*tmp){
    if (*tmp == KEY_ESC)
    {
      while((ch = *tmp) && !isalpha(ch))
        tmp++;
      if (ch)
        tmp++;
      continue;
    }
    if (tmp >= nxp) break;
    tmp++;
    ansix++;
  }
  return ansix;
}


/* ----------------------------------------------------- */
/* ¿Ã¹õ³B²z¡G»²§U°T®§¡BÅã¥Ü½s¿è¤º®e                      */
/* ----------------------------------------------------- */

static void 
edit_msg(void)
{
  static char *edit_mode[2] = {"¨ú¥N", "´¡¤J"};
  register int n = currpnt;

  if (my_ansimode)                      /* Thor: §@ ansi ½s¿è */
    n = n2ansi(n, currline);
  n++;
  move(b_lines, 0);
  clrtoeol();
  prints("%s  ½s¿è¤å³¹  %s  ùø%s¢x%c%c%c%cùø %3d:%3d     ^G)´¡¤J¹Ï¤å®w  ^X|^Q)Â÷¶}  Ctrl-Z)»¡©ú \033[m",
    COLOR2, COLOR3,
    edit_mode[insert_character],
    my_ansimode ? 'A' : 'a', indent_mode ? 'I' : 'i',
    'p', 
    raw_mode ? 'R' : 'r',
    currln + 1, n);
}


static textline *
back_line(pos, num)
  register textline *pos;
  int num;
{
  while (num-- > 0)
  {
    register textline *item;

    if (pos && (item = pos->prev))
    {
      pos = item;
      currln--;
    }
  }
  return pos;
}


static textline *
forward_line(pos, num)
  register textline *pos;
  int num;
{
  while (num-- > 0)
  {
    register textline *item;

    if (pos && (item = pos->next))
    {
      pos = item;
      currln++;
    }
  }
  return pos;
}


static int
getlineno()
{
  int cnt = 0;
  textline *p = currline;

  while (p && (p != top_of_win))
  {
    cnt++;
    p = p->prev;
  }
  return cnt;
}


static char *
killsp(char *s)
{
  while (*s == ' ')
    s++;
  return s;
}


static textline *
alloc_line()
{
  extern void *malloc();
  register textline *p;

  if (p = (textline *) malloc(sizeof(textline)))
  {
    memset(p, 0, sizeof(textline));
    return p;
  }

  indigestion(13);
  abort_bbs();
}


/* ----------------------------------------------------- */
/* append p after line in list. keeps up with last line  */
/* ----------------------------------------------------- */

static void
append(p, line)
  register textline *p, *line;
{
  register textline *n;

  if (p->next = n = line->next)
    n->prev = p;
  else
    lastline = p;
  line->next = p;
  p->prev = line;
}


/* ----------------------------------------------------- */
/* delete_line deletes 'line' from the list,             */
/* and maintains the lastline, and firstline pointers.   */
/* ----------------------------------------------------- */

static void
delete_line(line)
  register textline *line;
{
  register textline *p = line->prev;
  register textline *n = line->next;

  if (!p && !n)
  {
    line->data[0] = line->len = 0;
    return;
  }
  if (n)
    n->prev = p;
  else
    lastline = p;
  if (p)
    p->next = n;
  else
    firstline = n;
  strcat(line->data, "\n");
  line->prev = deleted_lines;
  deleted_lines = line;
  totaln--;
}

/*
woju
*/

static int
undelete_line()
{
   textline* p = deleted_lines;

   textline* currline0 = currline;
   textline* top_of_win0 = top_of_win;
   int currpnt0 = currpnt;
   int currln0 = currln;
   int curr_window_line0 = curr_window_line;
   int indent_mode0 = indent_mode;

   if (!deleted_lines)
      return 0;

   indent_mode = 0;
   insert_string(deleted_lines->data);
   indent_mode = indent_mode0;
   deleted_lines = deleted_lines->prev;
   free(p);

   currline = currline0;
   top_of_win = top_of_win0;
   currpnt = currpnt0;
   currln = currln0;
   curr_window_line = curr_window_line0;
}


static int
indent_spcs()
{
   textline* p;
   int spcs;

   if (!indent_mode)
      return 0;

   for (p = currline; p; p = p->prev) {
      for (spcs = 0; p->data[spcs] == ' '; ++spcs)
         ;
      if (p->data[spcs])
         return spcs;
   }
   return 0;
}

/* ----------------------------------------------------- */
/* split 'line' right before the character pos           */
/* ----------------------------------------------------- */

static void
split(line, pos)
  register textline *line;
  register int pos;
{
  if (pos <= line->len)
  {
    register textline *p = alloc_line();
    register char *ptr;
    int spcs = indent_spcs();

    totaln++;

    p->len = line->len - pos + spcs;
    line->len = pos;

    memset(p->data, ' ', spcs);
    p->data[spcs] = 0;
    strcat(p->data, (ptr = line->data + pos));
    ptr[0] = '\0';
    append(p, line);
    if (line == currline && pos <= currpnt)
    {
      currline = p;
      if (pos == currpnt)
         currpnt = spcs;
      else
         currpnt -= pos;
      curr_window_line++;
      currln++;
    }
    redraw_everything = YEA;
  }
}

/* ----------------------------------------------------- */
/* 1) lines were joined and one was deleted              */
/* 2) lines could not be joined                          */
/* 3) next line is empty                                 */
/* returns false if:                                     */
/* 1) Some of the joined line wrapped                    */
/* ----------------------------------------------------- */

static int
join(line)
  register textline *line;
{
  register textline *n;
  register int ovfl;

  if (!(n = line->next))
    return YEA;
  if (!*killsp(n->data))
    return YEA;

  ovfl = line->len + n->len - WRAPMARGIN;
  if (ovfl < 0)
  {
    strcat(line->data, n->data);
    line->len += n->len;
    delete_line(n);
    return YEA;
  }
  else
  {
    register char *s;

    s = n->data + n->len - ovfl - 1;
    while (s != n->data && *s == ' ')
      s--;
    while (s != n->data && *s != ' ')
      s--;
    if (s == n->data)
      return YEA;
    split(n, (s - n->data) + 1);
    if (line->len + n->len >= WRAPMARGIN)
    {
      indigestion(0);
      return YEA;
    }
    join(line);
    n = line->next;
    ovfl = n->len - 1;
    if (ovfl >= 0 && ovfl < WRAPMARGIN - 2)
    {
      s = &(n->data[ovfl]);
      if (*s != ' ')
      {
        strcpy(s, " ");
        n->len++;
      }
    }
    return NA;
  }
}


static void
insert_char(register int ch)
{
  register textline *p = currline;
  register int i = p->len;
  register char *s;
  int wordwrap = YEA;

  if (currpnt > i)
  {
    indigestion(1);
    return;
  }
  if (currpnt < i && !insert_character)
  {
    p->data[currpnt++] = ch;
    if (my_ansimode) /* Thor: ansi ½s¿è, ¥i¥Hoverwrite, ¤£»\¨ì ansi code */
       currpnt = ansi2n(n2ansi(currpnt, p),p);
  }
  else
  {
    while (i >= currpnt)
    {
      p->data[i + 1] = p->data[i];
      i--;
    }
    p->data[currpnt++] = ch;
    i = ++(p->len);
  }
  if (i < WRAPMARGIN)
    return;
  s = p->data + (i - 1);
  while (s != p->data && *s == ' ')
    s--;
  while (s != p->data && *s != ' ')
    s--;
  if (s == p->data)
  {
    wordwrap = NA;
    s = p->data + (i - 2);
  }
  split(p, (s - p->data) + 1);
  p = p->next;
  i = p->len;
  if (wordwrap && i >= 1)
  {
    if (p->data[i - 1] != ' ')
    {
      p->data[i] = ' ';
      p->data[i + 1] = '\0';
      p->len++;
    }
  }
}

static void
insert_string(char *str)
{
  int ch;

  while (ch = *str++)
  {

#ifdef BIT8
    if (isprint2(ch) || ch == '')
#else
    if (isprint(ch))
#endif

    {
      insert_char(ch);
    }
    else if (ch == '\t')
    {
      do
      {
        insert_char(' ');
      } while (currpnt & 0x3);
    }
    else if (ch == '\n')
      split(currline, currpnt);
  }
}


static void
delete_char()
{
  register int len;

  if (len = currline->len)
  {
    register int i;
    register char *s;

    if (currpnt >= len)
    {
      indigestion(1);
      return;
    }
    for (i = currpnt, s = currline->data + i; i != len; i++, s++)
      s[0] = s[1];
    currline->len--;
  }
}


static void
load_file(FILE *fp)
{
  int indent_mode0 = indent_mode;

  indent_mode = 0;
  while (fgets(line, WRAPMARGIN + 2, fp))
    insert_string(line);
  fclose(fp);
  indent_mode = indent_mode0;
}


/* ----------------------------------------------------- */
/* ¼È¦sÀÉ                                                */
/* ----------------------------------------------------- */

char *
ask_tmpbuf(int y)
{
  static char fp_buf[10] = "buf.0";
  static char msg[] = "½Ð¿ï¾Ü¼È¦sÀÉ[0] ";  
  char *choose_tmp_num[11]={"00","11","22","33","44","55","66","77","88","99","lL)³Ì«á¥Î¹Lªº"};
  
  msg[13] = fp_buf[4];

  do
  {
    fp_buf[4] = getans2(y, 0, msg, choose_tmp_num, 11,fp_buf[4]);

    if(fp_buf[4] == 'l')
      fp_buf[4] = msg[13];
  } while (fp_buf[4] < '0' || fp_buf[4] > '9');
  return fp_buf;
}

static void
read_tmpbuf(int n)
{
  FILE *fp;
  char fp_tmpbuf[80];
  char tmpfname[] = "buf.0";
  char *tmpf;
  char ans[4] = "y";

  if (0 <= n && n <= 9) {
     tmpfname[4] = '0' + n;
     tmpf = tmpfname;
  }
  else {
     tmpf = ask_tmpbuf(3);
     n = tmpf[4] - '0';
  }

  sethomefile(fp_tmpbuf, cuser.userid, tmpf);

  if (n != 0 && n != 5 && more(fp_tmpbuf, NA) != -1)
     ans[0] = getans2(b_lines - 1, 0, "½T©wÅª¤J¶Ü? ",0,2,'y');
     
  if (*ans != 'n' && (fp = fopen(fp_tmpbuf, "r")))
  {
    prevln = currln;
    prevpnt = currpnt;
    load_file(fp);
    while (curr_window_line >= b_lines)
    {
      curr_window_line--;
      top_of_win = top_of_win->next;
    }
  }
}


static void
write_tmpbuf()
{
  FILE *fp;
  char fp_tmpbuf[80], ans[4];
  textline *p;

  sethomefile(fp_tmpbuf, cuser.userid, ask_tmpbuf(3));
  if (dashf(fp_tmpbuf))
  {
    char *choose_tmp[3]={"aA)ªþ¥[","wW)ÂÐ¼g", msg_choose_cancel};
    
    more(fp_tmpbuf, NA);

    ans[0] = getans2(b_lines -1, 0,"¼È¦sÀÉ¤w¦³¸ê®Æ ",choose_tmp,3,'a');

    if (ans[0] == 'q')
      return;
  }

  fp = fopen(fp_tmpbuf, (ans[0] == 'w' ? "w" : "a+"));
  for (p = firstline; p; p = p->next)
  {
    if (p->next || p->data[0])
      fprintf(fp, "%s\n", p->data);
  }
  fclose(fp);
}


static void
erase_tmpbuf()
{
  char fp_tmpbuf[80];

  sethomefile(fp_tmpbuf, cuser.userid, ask_tmpbuf(3));

  if (more(fp_tmpbuf, NA) != -1)
  {
    if (getans2(b_lines - 1, 0, "½T©w§R°£¶Ü? ",0,2,'n') == 'y')
      unlink(fp_tmpbuf);
  }
}


/* ----------------------------------------------------- */
/* ½s¿è¾¹¦Û°Ê³Æ¥÷                                        */
/* ----------------------------------------------------- */


void
auto_backup()
{
  if (currline)
  {
    FILE *fp;
    textline *p, *v;
    char bakfile[64];

    sethomefile(bakfile, cuser.userid, fp_bak);
    if (fp = fopen(bakfile, "w"))
    {
      for (p = firstline; p; p = v)
      {
        v = p->next;
        fprintf(fp, "%s\n", p->data);
        free(p);
      }
      fclose(fp);
    }
    currline = NULL;
  }
}

void
restore_backup()
{
  char bakfile[80], buf[80];

  sethomefile(bakfile, cuser.userid, fp_bak);
  if (dashf(bakfile))
  {
    char *choose_save[2] = {"sS)¼g¤J¼È¦sÀÉ", msg_choose_cancel};
    
    stand_title("½s¿è¾¹¦Û°Ê´_­ì");

    buf[0] = getans2(1, 0, "±z¦³¤@½g¤å³¹©|¥¼§¹¦¨¡A", choose_save, 2, 's');
    
    if (buf[0] != 'q')
    {
      sethomefile(buf, cuser.userid, ask_tmpbuf(3));
      f_mv(bakfile, buf);
    }
    else
      unlink(bakfile);
  }
}


/* ----------------------------------------------------- */
/* ¤Þ¥Î¤å³¹                                              */
/* ----------------------------------------------------- */

static inline int
quote_line(char *str, int qlimit)	/* qlimit : ¤¹³\´X¼h¤Þ¨¥¡H */
{
  int qlevel = 0;
  int ch;

  while ((ch = *str) == '>' || ch == ':')
  {
    if (*(++str) == ' ')
      str++;
    if (qlevel++ >= qlimit)
      return 0;
  }
  while ((ch = *str) == ' ' || ch == '\t')
    str++;
  if (qlevel >= qlimit)
  {
    if (!strncmp(str, "¡° ", 3) || !strncmp(str, "==>", 3) ||
      strstr(str, ") ´£¨ì:\n"))
      return 0;
  }
  return (*str != '\n');
}

static void
do_quote()
{
  int op;
  char *ptr, buf[512];
  char *choose[5]={msg_choose[0], msg_choose[1], "aA)¥þ³¡", "rR)­«·s½s¿è¤å³¹", "lL)¤Þ¥Î¼h¼Æ"};
  
  op = getans2(b_lines - 1, 0,"½Ð°Ý­n¤Þ¥Î­ì¤å¶Ü¡H",choose,5,'y');

  if(op == 'l')
  {
    getdata(b_lines - 1, 0, "½Ð¿é¤J¼h¼Æ(1-9): ", buf, 3, LCECHO, 0);
    op = *buf;
  }
  
  if ((op >= '1') && (op <= '9'))
    op -= '1';
  else if ((op != 'a') && (op != 'r') && (op != 'n'))
    op = 1;		/* default : 2 level */  
  
  if (op != 'n')
  {
    FILE *inf;

    if (inf = fopen(quote_file, "r"))
    {
      int indent_mode0 = indent_mode;

      if (op != 'a')	/* ¥[¤J­ì§@ªÌ ¤Î ¥h±¼ header */
      {
        /* Åª¨ú²Ä¤@¦æ§ä¥X­ì§@ªÌ */
        fgets(buf, 512, inf);
      
        if (ptr = strrchr(buf, ')'))
          ptr[1] = '\0';
        else if (ptr = strrchr(buf, '\n'))
          ptr[0] = '\0';

        if (ptr = strchr(buf, ':'))
        {
          char *str;

          while (*(++ptr) == ' ');

          /* ¶¶¤â²o¦Ï¡A¨ú±o author's address */
          if ((curredit & EDIT_BOTH) && (str = strchr(quote_user, '.')))
          {
            strcpy(++str, ptr);
            str = strchr(str, ' ');
            str[0] = '\0';
          }
        }
        else
          ptr = quote_user;

        indent_mode = 0;
        insert_string("¡° ¤Þ­z¡m");
        insert_string(ptr);
        insert_string("¡n¤§»Ê¨¥¡G\n");

        /* ¥h±¼ header */
        while (fgets(buf, 512, inf) && buf[0] != '\n');
        
        if (curredit & EDIT_LIST)       /* ¥h±¼ mail list ¤§ header */
        {
          while (fgets(buf, 512, inf) && (!strncmp(buf, "¡° ", 3)));
        }
      }
            
      ptr = buf;
      if (op == 'r')
      {
        op = 'a';
      }
      else
      {
        *ptr++ = '>';
        *ptr++ = ' ';
      }

      if (op == 'a')
      {
        while (fgets(ptr, 512 - 2, inf))	/* ¯dªÅ¶¡µ¹ "> " */
          insert_string((char *) Ptt_prints(buf,ONLY_COLOR));
      }
      else
      {
        while (fgets(ptr, 512 - 2, inf))	/* ¯dªÅ¶¡µ¹ "> " */
        {
          if (!strcmp(ptr, "--\n"))
            break;

          if(quote_line(ptr, op))
            insert_string((char *) Ptt_prints(buf,ONLY_COLOR));
        }
      }
      indent_mode = indent_mode0;
      fclose(inf);
    }
  }
}

/* ----------------------------------------------------- */
/* ¼f¬d user µoªí¤å³¹¦r¼Æ                                */
/* ----------------------------------------------------- */

static long
check_words()
{
  register textline *p = firstline;
  register char *str;
  int  i;
  long words = 0;

  while (p)
  {
/*  by shakalaca 990816 */
/*  ¦b FreeBSD ¨S°ÝÃDªº , ·h¥h linux ·|±¾ , ©Ò¥H·h¥X¨Ó */
/*  if (beginsig(str = p->data)) */

    str = p->data;
    if(!str) break;
    if (beginsig(str))
      break;

    if (!(!strncmp(str, "¡° ", 3) || (str[1] == ' ' && 
       ((str[0] == ':') || (str[0] == '>')))))
    {
       int len = strlen(str);
       
       for(i = 0; i <= len; i++)
         if(str[i] != ' ' && str[i] != '\0')
           words ++;
    }
    if (words > 10000000)
    {
      words = 10000000;
      return words;
    }
    p = p->next;
  }
  return words;
}


/* ----------------------------------------------------- */
/* ¼f¬d user ¤Þ¨¥ªº¨Ï¥Î                                  */
/* ----------------------------------------------------- */

static int
check_quote()
{
  register textline *p = firstline;
  register char *str;
  int post_line;
  int included_line;

  post_line = included_line = 0;
  while (p)
  {
    if (!strcmp(str = p->data, "--"))
      break;
    if (str[1] == ' ' && ((str[0] == ':') || (str[0] == '>')))
    {
      included_line++;
    }
    else
    {
      while (*str == ' ' || *str == '\t')
        str++;
      if (*str)
        post_line++;
    }
    p = p->next;
  }

  if ((included_line >> 2) > post_line)
  {
    char *choose_edit[2]={"eE)Ä~Äò½s¿è","wW)±j¨î¼g¤J"};

    move(4, 0);
    outs("\
          ¥»½g¤å³¹ªº¤Þ¨¥¤ñ¨Ò¶W¹L 80%¡A½Ð±z°µ¨Ç·Lªº­×¥¿¡G\n\n\
           [1;33m1.¼W¥[¤@¨Ç¤å³¹[0m\n\n\
           [1;33m2.§R°£¤£¥²­n¤§¤Þ¨¥[0m");


    if (getans2(12, 12, "½Ð°Ý±z­n¡G", choose_edit, 2, 'e') == 'w')
      return 0;
    else
      return 1;
  }
  return 0;
}

/* ----------------------------------------------------- */
/* ÀÉ®×³B²z¡GÅªÀÉ¡B¦sÀÉ¡B¼ÐÃD¡BÃ±¦WÀÉ                    */
/* ----------------------------------------------------- */

static void
read_file(char *fpath)
{
  FILE *fp;

  if ((fp = fopen(fpath, "r")) == NULL)
  {
    if (fp = fopen(fpath, "w+"))
    {
      fclose(fp);
      return;
    }
    indigestion(4);
    abort_bbs();
  }
  load_file(fp);
}

void
write_header(FILE *fp)
{
  time_t now = time(0);

  if (curredit & EDIT_MAIL || curredit & EDIT_LIST)
  {
    fprintf(fp, "%s %s (%s)\n", str_author1, cuser.userid,

#if defined(REALINFO) && defined(MAIL_REALNAMES)
      cuser.realname);
#else
      cuser.username);
#endif
  }
  else
  {
    char *ptr;

    struct
    {
      char author[IDLEN + 1];
      char board[IDLEN + 1];
      char title[66];
      time_t date;              /* last post's date */
      int number;               /* post number */
    }postlog;

    strcpy(postlog.author, cuser.userid);
#ifdef HAVE_ANONYMOUS
    if (currbrdattr& BRD_ANONYMOUS)
    {
      getdata(3, 0, "½Ð¿é¤J§A·Q¥ÎªºID¡A¤]¥iª½±µ«ö[Enter]¡A©Î¬O«ö[r]¥Î¯u¦W¡G",real_name, 12, DOECHO,0);
      if(!real_name[0])
      {
        strlcpy(real_name, STR_ANONYMOUS, sizeof(real_name));
        strlcpy(postlog.author,real_name, sizeof(postlog.author));
      }
      else
      {
        if (!strcmp("r",real_name))
           sprintf(postlog.author,"%s",cuser.userid);
        else
           sprintf(postlog.author,"%s.",real_name);
      }
    }
#endif
    strcpy(postlog.board, currboard);
    ptr = save_title;
    if (!strncmp(ptr, str_reply, 4))
      ptr += 4;
    strncpy(postlog.title, ptr, 65);
    postlog.date = now;
    postlog.number = 1;
    rec_add(".post", &postlog, sizeof(postlog));

#ifdef HAVE_ANONYMOUS
    if (currbrdattr & BRD_ANONYMOUS)
      fprintf(fp, "%s %s (%s) %s %s\n", str_author1, postlog.author ,
        (strcmp(real_name,"r")?"¦ó¶·°Ý§Ú¦W":cuser.username), local_article ? str_post2 : str_post1, currboard);
    else
    {
      fprintf(fp, "%s %s (%s) %s %s\n", str_author1, cuser.userid,

#if defined(REALINFO) && defined(POSTS_REALNAMES)
        cuser.realname
#else
        cuser.username
#endif

        ,local_article ? str_post2 : str_post1, currboard);
    }
#else   /*HAVE_ANONYMOUS else*/

    fprintf(fp, "%s %s (%s) %s %s\n", str_author1, cuser.userid,

#if defined(REALINFO) && defined(POSTS_REALNAMES)
      cuser.realname
#else
      cuser.username
#endif

      ,local_article ? str_post2 : str_post1, currboard);
#endif  /*HAVE_ANONYMOUS endif*/
  }
  save_title[72] = '\0';
  fprintf(fp, "¼ÐÃD: %s\n®É¶¡: %s\n", save_title, ctime(&now));
}


int
showsignature(char *fname)
{
  FILE *fp;
  char buf[512];
  int i, j;
  char ch;

  clear();
  move(2, 0);
  sethomefile(fname, cuser.userid, "sig.0");
  j = strlen(fname) - 1;

  for (ch = '1'; ch <= '9'; ch++)
  {
    fname[j] = ch;
    if (fp = fopen(fname, "r"))
    {
      prints("[36m¡i Ã±¦WÀÉ.%c ¡j[m\n", ch);
      for (i = 0; i++ < MAXSIGLINES && fgets(buf, 512, fp); outs(buf))
        ;
      fclose(fp);
    }
  }
  return j;
}


void
addsignature(FILE *fp)
{
  FILE *fs;
  int i;
  char buf[WRAPMARGIN + 1];
  char fpath[STRLEN];

  static char msg[] = "½Ð¿ï¾ÜÃ±¦WÀÉ (1-9,0=¤£¥[,r=¶Ã¼Æ)[0]: ";
  char ch=0,tmp=0;   

  static char currsig = '0';
  
/*¥»¯¸Ã±¦WÀÉ*/
  if (!strcmp(cuser.userid, STR_GUEST) || bad_user_id(cuser.userid)) 
  {
     fprintf(fp, "\n--\n[1;36m¡°Post by [37m%s [36mfrom [33m%-24.24s[m",cuser.userid, fromhost /*getenv("RFC931")*/);
     return;
  }
 
  i = showsignature(fpath);

  msg[33] = ch = currsig;

  getdata(0, 0, msg, buf, 3, LCECHO,0);
  if (buf[0]==' ') 
    buf[0] = currsig;	

  if (buf[0]=='r')
  {  
    do 
    {
      buf[0]='1'+ random()%9;
      fpath[i] = buf[0];
    }while(!dashf(fpath) && ++tmp<20);
    
    tmp = 'r';
  }

  if (ch != buf[0] && buf[0] >= '0' && buf[0] <= '9')
  {
    currsig = ch = buf[0];	/*Ageless*/
/* shakalaca.990426: original */
    cuser.uflag = (cuser.uflag & ~SIG_FLAG) | (ch & SIG_FLAG); 
  }

  if (ch != '0') 
  { 
    fpath[i] = ch;
    if (fs = fopen(fpath, "r"))
    {
      fputs("\n--\n", fp);
      for (i = 0; i < MAXSIGLINES && fgets(buf, sizeof(buf), fs); i++)
        fputs(buf, fp);
      fclose(fs);
    }
  } 
#ifdef  HAVE_ORIGIN
#ifdef  HAVE_ANONYMOUS
  if (currbrdattr & BRD_ANONYMOUS && strcmp(real_name,"r"))
    fprintf(fp, "\n--\n[1;36m ²q²q§Ú¬O½Ö¡I\n");
  else 
    site_logo(fp);
#else
  site_logo(fp);
#endif
#endif
}

long wordsnum;

static int
write_file(char *fpath, int saveheader)
{
  FILE *fp;
  textline *p, *v;
  char ans[TTLEN], *msg;
  int aborted = 0;
  char *choose[8]={"sS)Àx¦s","aA)©ñ±ó","tT)§ï¼ÐÃD","eE)Ä~Äò","rR)Åª","wW)¼g","dD)§R¼È¦sÀÉ","lL)¯¸¤º«H¥ó"};

  stand_title("ÀÉ®×³B²z");

  switch (getans2(1, 0, "", choose, (currstat == SMAIL || local_article == 1) ? 7 : 8, 's'))
  {
    case 'a':
      move(2,0);
      outs("¤å³¹[1m ¨S¦³ [0m¦s¤J");
      sleep(1);
      aborted = -1;
      break;

    case 'r':
      read_tmpbuf(-1);

    case 'e':
      return KEEP_EDITING;

    case 'w':
      write_tmpbuf();
      return KEEP_EDITING;

    case 'd':
      erase_tmpbuf();
      return KEEP_EDITING;

    case 't':
      move(3, 0);
      prints("ÂÂ¼ÐÃD¡G%s", save_title);
      if (getdata(4, 0, "·s¼ÐÃD¡G", ans, TTLEN, DOECHO,save_title))
        strcpy(save_title, ans);
      return KEEP_EDITING;

    case 'l':
      local_article = 1;
      break; 

    /* wildcat patch 000102 : default ´N¬O 's' */
    default :
      if (!HAS_PERM(PERM_LOGINOK))
      {
        local_article = 1;
        pressanykey("±z©|¥¼³q¹L¨­¥÷½T»{¡A¥u¯à Local Save¡C");
      }

      break;
  }

  if (saveheader == 2 && !aborted) 
  {
     stand_title("¼g¤J¥¢±Ñ: §ï¼g¨ì¼È¦sÀÉ");
     write_tmpbuf();
     return KEEP_EDITING;
  }

  if (!aborted)
  {
 
    if (saveheader && !(curredit & EDIT_MAIL) && check_quote())
      return KEEP_EDITING;

    if (!*fpath)
      sethomefile(fpath, cuser.userid, "ve_tmp");

    if ((fp = fopen(fpath, "w")) == NULL)
    {
      indigestion(5);
      abort_bbs();
    }

    if (saveheader)
      write_header(fp);
  }

  wordsnum = check_words();

  for (p = firstline; p; p = v)
  {
    v = p->next;
    if (!aborted)
    {
      msg = p->data;
      if (v || msg[0])
      {
        str_trim(msg);
        fprintf(fp, "%s\n", msg);
      }
    }
/* shakalaca patch            */
/* undef it for LINUX 990816  */
#ifndef LINUX
    free(p); 
#endif
  }
  currline = NULL;

  if (!aborted)
  {
    if (currstat == POSTING || currstat == SMAIL)
      addsignature(fp); 
    fclose(fp);

    if (local_article && (currstat == POSTING))
      return 1;

    return 0;
  }

  return aborted;
}

/*
void
edit_outs(char *text)
{
  register int column = 0;
  uschar ch;

  while ((ch = *text++) && (++column < SCR_WIDTH))
  {
    outc(ch == KEY_ESC ? '*' : ch);
  }
}
*/

static void
display_buffer()
{
  register textline *p;
  register int i;
  int inblock;
  char buf[WRAPMARGIN + 2];
  int min, max;

  if (currpnt > blockpnt) 
  {
     min = blockpnt;
     max = currpnt;
  }
  else 
  {
     min = currpnt;
     max = blockpnt;
  }

  for (p = top_of_win, i = 0; i < b_lines; i++)
  {
    move(i, 0);
    clrtoeol();

    if (blockln >= 0
        && (blockln <= currln
              && blockln <= (currln - curr_window_line + i)
              &&            (currln - curr_window_line + i) <= currln
           ||
                 currln  <= (currln - curr_window_line + i)
              &&            (currln - curr_window_line + i) <= blockln)) 
    {
       outs("[7m");
       inblock = 1;
    }
    else
       inblock = 0;
    if (p)
    {
      if (my_ansimode)
      {
         if (currln == blockln && p == currline && max > min) 
         {
             outs("[0m");
             strncpy(buf, p->data, min);
             buf[min] = 0;
             outs(buf);
             outs("[7m");
             strncpy(buf, p->data + min, max - min);
             buf[max - min] = 0;
             outs(buf);
             outs("[0m");
             outs(p->data + max);
         }
         else
            outs(p->data);
      }
      else
      {
         if (currln == blockln && p == currline && max > min) 
         {
             outs("[0m");
             strncpy(buf, p->data, min);
             buf[min] = 0;
             edit_outs(buf);
             outs("[7m");
             strncpy(buf, p->data + min, max - min);
             buf[max - min] = 0;
             edit_outs(buf);
             outs("[0m");
             edit_outs(p->data + max);
         }
         else
            edit_outs(&p->data[edit_margin]);
      }
            
      p = p->next;
      if (inblock)
         outs("[0m");
    }
    else
      outc('~');
  }
  edit_msg();
}


/*
woju
lino == 0 for prompt
*/

void
goto_line(int lino)
{
   char buf[10];

   if (lino > 0 || getdata(b_lines - 1, 0, "¸õ¦Ü²Ä´X¦æ:", buf, 10, DOECHO,0)
       && sscanf(buf, "%d", &lino) && lino > 0) {
      textline* p;

      prevln = currln;
      prevpnt = currpnt;
      p = firstline;
      currln = lino - 1;

      while (--lino && p->next)
         p = p->next;

      if (p)
         currline = p;
      else {
         currln = totaln;
         currline = lastline;
      }
      currpnt = 0;
      if (currln < 11) {
         top_of_win = firstline;
         curr_window_line = currln;
      }
      else {
         int i;
         curr_window_line = 11;

         for (i = curr_window_line; i; i--)
            p = p->prev;
            top_of_win = p;
      }
   }
   redraw_everything = YEA;
}

/*
woju
mode:
    0: prompt
    1: forward
    -1: backward
*/
void
search_str(int mode)
{
   static char str[80];
   typedef char* (*FPTR)();
   static FPTR fptr;
   char ans[4] = "n";

   if (!mode) {
      if (getdata(b_lines - 1, 0,"[·j´M]ÃöÁä¦r:",str, 65, DOECHO,0))
      if (*str)
         if (getans2(b_lines - 1, 0, "°Ï¤À¤j¤p¼g? ", 0, 3,'n')
             && *ans == 'y')             
            fptr = strstr;
         else
            fptr = strcasestr;
   }

   if (*str && *ans != 'q') {
      textline* p;
      char* pos;
      int lino;

      if (mode >= 0) {
         for (lino = currln, p = currline; p; p = p->next, lino++)
            if ((pos = fptr(p->data + (lino == currln ? currpnt + 1 : 0), str))
                && (lino != currln || pos - p->data != currpnt))
               break;
      }
      else {
         for (lino = currln, p = currline; p; p = p->prev, lino--)
            if ((pos = fptr(p->data, str))
                && (lino != currln || pos - p->data != currpnt))
               break;
      }
      if (pos) {
         prevln = currln;
         prevpnt = currpnt;
         currline = p;
         currln = lino;
         currpnt = pos - p->data;
         if (lino < 11) {
            top_of_win = firstline;
            curr_window_line = currln;
         }
         else {
            int i;
            curr_window_line = 11;

            for (i = curr_window_line; i; i--)
               p = p->prev;
            top_of_win = p;
         }
         redraw_everything = YEA;
      }
   }
   if (!mode)
      redraw_everything = YEA;
}


static void
match_paren()
{
   static char parens[] = "()[]{}";
   int type;
   int parenum = 0;
   char *ptype;
   textline* p;
   int lino;
   int c, i;

   if (!(ptype = strchr(parens, currline->data[currpnt])))
      return;

   type = (ptype - parens) / 2;
   parenum += ((ptype - parens) % 2) ? -1 : 1;


   if (parenum > 0) {
     for (lino = currln, p = currline; p; p = p->next, lino++) {
         lino = lino;
         for (i = (lino == currln) ? currpnt + 1 : 0; i < strlen(p->data); i++)
            if (p->data[i] == '/' && p->data[++i] == '*') {
               ++i;
               while (1) {
                  while(i < strlen(p->data) - 1
                      && !(p->data[i] == '*' && p->data[i + 1] == '/'))
                     i++;
                  if (i >= strlen(p->data) - 1 && p->next) {
                     p = p->next;
                     ++lino;
                     i = 0;
                  }
                  else
                     break;
               }
            }
            else if ((c = p->data[i]) == '\'' || c == '"') {
               while (1) {
                  while (i < (int)(strlen(p->data) - 1))
                     if (p->data[++i] == '\\' && i < strlen(p->data) - 2)
                        ++i;
                     else if (p->data[i] == c)
                        goto end_quote;
                  if (i >= strlen(p->data) - 1 && p->next) {
                     p = p->next;
                     ++lino;
                     i = -1;
                  }
                  else
                     break;
               }
end_quote:
               ;
            }
            else if ((ptype = strchr(parens, p->data[i]))
                     && (ptype - parens) / 2 == type)
               if (!(parenum += ((ptype - parens) % 2) ? -1 : 1))
                  goto p_outscan;
     }
   }
   else {
      for (lino = currln, p = currline; p; p = p->prev, lino--)
         for (i = (lino == currln) ? currpnt - 1 : strlen(p->data) - 1;
              i >= 0; i--)
            if (p->data[i] == '/' && p->data[--i] == '*' && i > 0) {
               --i;
               while (1) {
                  while(i > 0
                      && !(p->data[i] == '*' && p->data[i - 1] == '/'))
                     i--;
                  if (i <= 0 && p->prev) {
                     p = p->prev;
                     --lino;
                     i = strlen(p->data) - 1;
                  }
                  else
                     break;
               }
            }
            else if ((c = p->data[i]) == '\'' || c == '"') {
               while (1) {
                  while (i > 0)
                     if (i > 1 && p->data[i - 2] == '\\')
                        i -= 2;
                     else if ((p->data[--i]) == c)
                        goto begin_quote;
                  if (i <= 0 && p->prev) {
                     p = p->prev;
                     --lino;
                     i = strlen(p->data);
                  }
                  else
                     break;
               }
begin_quote:
               ;
            }
            else if ((ptype = strchr(parens, p->data[i]))
                     && (ptype - parens) / 2 == type)
               if (!(parenum += ((ptype - parens) % 2) ? -1 : 1))
                  goto p_outscan;
   }

p_outscan:

   if (!parenum) {
      int top = currln - curr_window_line;
      int bottom = currln - curr_window_line + b_lines - 1;

      currpnt = i;
      currline = p;
      curr_window_line += lino - currln;
      currln = lino;

      if (lino < top || lino > bottom) {
         if (lino < 11) {
            top_of_win = firstline;
            curr_window_line = currln;
         }
         else {
            int i;
            curr_window_line = 11;

            for (i = curr_window_line; i; i--)
               p = p->prev;
            top_of_win = p;
         }
         redraw_everything = YEA;
      }
   }
}


block_del(int hide)
{
   if (blockln < 0) {
      blockln = currln;
      blockpnt = currpnt;
      blockline = currline;
   }
   else {
      char fp_tmpbuf[80];
      FILE* fp;
      textline *begin, *end, *p;
      char tmpfname[10] = "buf.0";
      char ans[6] = "w+n";

      move(b_lines - 1, 0);
      clrtoeol();
      if (hide == 1)
         tmpfname[4] = 'q';
      else if (!hide && !getdata(b_lines - 1, 0, "§â°Ï¶ô²¾¦Ü¼È¦sÀÉ (0:Cut, 5:Copy, 6-9, q: Cancel)[0] ",  tmpfname + 4, 4, LCECHO,0))
         tmpfname[4] = '0';
      if (tmpfname[4] < '0' || tmpfname[4] > '9')
         tmpfname[4] = 'q';
      if ('1' <= tmpfname[4] && tmpfname[4] <= '9') {
         sethomefile(fp_tmpbuf, cuser.userid, tmpfname);
         if (tmpfname[4] != '5' && dashf(fp_tmpbuf)) 
          {
            char *choose_tmp[3] = {"aA)ªþ¥[","wW)ÂÐ¼g", msg_choose_cancel};
            
            more(fp_tmpbuf, NA);
            ans[0] = getans2(b_lines - 1, 0, "¼È¦sÀÉ¤w¦³¸ê®Æ ",choose_tmp,3,'w');

            if (*ans == 'q')
               tmpfname[4] = 'q';
            else if (*ans != 'a')
               *ans = 'w';
          }
          if (tmpfname[4] != '5') {
             ans[2] = getans2(b_lines - 1, 0, "§R°£°Ï¶ô(Y/N)? ",0,2,'n');
             if (ans[2] != 'y')
                ans[2] = 'n';
          }
      }
      else if (hide != 3)
         ans[2] = 'y';

      tmpfname[5] = ans[1] = ans[3] = 0;
      if (tmpfname[4] != 'q') {
         if (currln >= blockln) {
            begin = blockline;
            end = currline;
            if (ans[2] == 'y' && !(begin == end && currpnt != blockpnt)) {
               curr_window_line -= (currln - blockln);
               if (curr_window_line < 0) {
                  curr_window_line = 0;
               if (end->next)
                   (top_of_win = end->next)->prev = begin->prev;
                else
                   top_of_win = (lastline = begin->prev);
                }
                 currln -= (currln - blockln);
            }
         }
         else {
            begin = currline;
            end = blockline;
         }


         if (ans[2] == 'y' && !(begin == end && currpnt != blockpnt)) {
            if (begin->prev)
               begin->prev->next = end->next;
            else if (end->next)
               top_of_win = firstline = end->next;
            else {
               currline = top_of_win = firstline = lastline = alloc_line();
               currln = curr_window_line = edit_margin = 0;
            }

            if (end->next)
               (currline = end->next)->prev = begin->prev;
            else if (begin->prev) {
               currline = (lastline = begin->prev);
               currln--;
               if (curr_window_line > 0)
                  curr_window_line--;
            }
         }

         sethomefile(fp_tmpbuf, cuser.userid, tmpfname);
         if (fp = fopen(fp_tmpbuf, ans)) {
            if (begin == end && currpnt != blockpnt) {
               char buf[WRAPMARGIN + 2];

               if (currpnt > blockpnt) {
                  strcpy(buf, begin->data + blockpnt);
                  buf[currpnt - blockpnt] = 0;
               }
               else {
                  strcpy(buf, begin->data + currpnt);
                  buf[blockpnt - currpnt] = 0;
               }
               fputs(buf, fp);
            }
            else {
               for (p = begin; p != end; p = p->next)
                  fprintf(fp, "%s\n", p->data);
               fprintf(fp, "%s\n", end->data);
            }
            fclose(fp);
         }

         if (ans[2] == 'y') {
            if (begin == end && currpnt != blockpnt) {
               int min, max;

               if (currpnt > blockpnt) {
                  min = blockpnt;
                  max = currpnt;
               }
               else {
                  min = currpnt;
                  max = blockpnt;
               }
               strcpy(begin->data + min, begin->data + max);
               begin->len -= max - min;
               currpnt = min;
            }
            else {
               for (p = begin; p != end; totaln--)
                  free((p = p->next)->prev);
               free(end);
               totaln--;
               currpnt = 0;
            }
         }
      }
      blockln = -1;
      redraw_everything = YEA;
   }
}

block_shift_left()
{
   textline *begin, *end, *p;

   if (currln >= blockln) {
      begin = blockline;
      end = currline;
   }
   else {
      begin = currline;
      end = blockline;
   }
   p = begin;
   while (1) {
      if (p->len) {
         strcpy(p->data, p->data + 1);
         --p->len;
      }
      if (p == end)
         break;
      else
         p = p->next;
   }
   if (currpnt > currline->len)
      currpnt = currline->len;
   redraw_everything = YEA;
}

block_shift_right()
{
   textline *begin, *end, *p;

   if (currln >= blockln) {
      begin = blockline;
      end = currline;
   }
   else {
      begin = currline;
      end = blockline;
   }
   p = begin;
   while (1) {
      if (p->len < WRAPMARGIN) {
         int i = p->len + 1;

         while (i--)
            p->data[i + 1] = p->data[i];
         p->data[0] = insert_character ? ' ' : insert_c;
         ++p->len;
      }
      if (p == end)
         break;
      else
         p = p->next;
   }
   if (currpnt > currline->len)
      currpnt = currline->len;
   redraw_everything = YEA;
}

static void
transform_to_color(char* line)
{
   while (line[0] && line[1])
   {
      if (line[0] == '*' && line[1] == '[') 
      {
         line[0] = KEY_ESC;
         line += 2;
      }
      else
         ++line;
   }
}

static void
block_color()
{
   textline *begin, *end, *p;

   if (currln >= blockln) 
   {
      begin = blockline;
      end = currline;
   }
   else 
   {
      begin = currline;
      end = blockline;
   }
   p = begin;
   while (1) 
   {
      transform_to_color(p->data);
      if (p == end)
         break;
      else
         p = p->next;
   }
   block_del(1);
}


/* ----------------------------------------------------- */
/* ½s¿è³B²z¡G¥Dµ{¦¡¡BÁä½L³B²z                            */
/* ----------------------------------------------------- */
extern a_menu(char *maintitle, char *path, int lastlevel);

int
vedit(char *fpath, int saveheader)
{
  FILE *fp1;  /* Ptt */
  char buf[80];
  char last;
  int ch, foo;
  int lastindent = -1;
  int last_margin;
  int mode0 = currutmp->mode;
  int destuid0 = currutmp->destuid;
  unsigned int money=0;
  unsigned short int interval=0;
  unsigned short int count=0;
  time_t now=0,th;

  textline* firstline0 = firstline;
  textline* lastline0 = lastline;
  textline* currline0 = currline;
  textline* blockline0 = blockline;
  textline* top_of_win0 = top_of_win;
  int local_article0 = local_article;
  int currpnt0 = currpnt;
  int currln0 = currln;
  int totaln0 = totaln;
  int curr_window_line0 = curr_window_line;
  int insert_character0 = insert_character;
  int my_ansimode0 = my_ansimode;
  int edit_margin0 = edit_margin;
  int blockln0 = blockln;
  currutmp->mode = EDITING;
  currutmp->destuid = currstat;
  insert_character = redraw_everything = 1;
  prevln = blockln = -1;

  line_dirty = currpnt = totaln = my_ansimode = 0;
  currline = top_of_win = firstline = lastline = alloc_line();

  if (*fpath)
  {
    read_file(fpath);
  }

  if (*quote_file)
  {
    do_quote();
    *quote_file = '\0';
    if (quote_file[79] == 'L')
      local_article = 1;
  }

  currline = firstline;
  currpnt = currln = curr_window_line = edit_margin = last_margin = 0;

  while (1)
  {
    if (redraw_everything || blockln >=0)
    {
      display_buffer();
      redraw_everything = NA;
    }
    if (my_ansimode)
       ch = n2ansi(currpnt, currline);
    else
       ch = currpnt - edit_margin;
    move(curr_window_line, ch);
    if (!line_dirty && strcmp(line, currline->data))
       strcpy(line, currline->data);
    ch = igetkey();

    if(interval=(unsigned short int)((th=currutmp->lastact) - now))
    {
      if (interval==1)
        count++;
      else
        count=0;
      now = th;
      if ((char)ch!=last)
      {
        money++;
        last=(char)ch;
      }
    }
        
/* Jaky */
    if (count >= 200)
    {
       sprintf (buf,"[1;33;46m%s[37m¦b[37;45m%s[37mªO¹HªkÁÈ¿ú , %s[m",cuser.userid,currboard,ctime(&now));
       f_cat (BBSHOME"/log/illegal_money",buf);
       degold(100);
       pressanykey("³sÄò¨â¦Ê¬í¤£ÃP¤â,¤À©ú¬O¦bÀÄ°]");
       pressanykey("«DªkÁÈ¿ú , ¦©ª÷¹ô 100 ¤¸ , ½ð¥X¯¸!!");
       abort_bbs();              /* ³sÄò¨â¦Ê¬í¤£ÃP¤â,¤À©ú¬O¦bÀÄ°]*/
    }

    if (raw_mode)
       switch (ch) 
       {
         case Ctrl('S'):
         case Ctrl('Q'):
         case Ctrl('T'):
           continue;
           break;
       }

    if (ch == Ctrl('J') && !raw_mode)
       goto_line(0);

    if(ch < 0x100 && isprint2(ch))
    {
      insert_char(ch);
      lastindent = -1;
      line_dirty = 1;
    }
    else
    {
      if (ch == Ctrl('P') || ch == KEY_UP || ch == KEY_DOWN || ch == Ctrl('N'))
      {
        if (lastindent == -1)
          lastindent = currpnt;
      }
      else
        lastindent = -1;

/*
woju
*/
      if (ch == KEY_ESC)
         switch (KEY_ESC_arg) 
         {
         case ',':
            ch = Ctrl(']');
            break;
         case '.':
            ch = Ctrl('T');
            break;
         case 'v':
            ch = KEY_PGUP;
            break;
         case 'a':
         case 'A':
            ch = Ctrl('V');
            break;
         case 'X':
            ch = Ctrl('X');
            break;
         case 'q':
            ch = Ctrl('Q');
            break;
         case 'o':
            ch = Ctrl('O');
            break;
         case '-':
            ch = Ctrl('_');
            break;
         case 's':
            ch = Ctrl('S');
            break;
         }

      switch (ch)
      {
      case Ctrl('X'):           /* Save and exit */
        foo = write_file(fpath, saveheader);
        if (foo != KEEP_EDITING)
        {
          currutmp->mode = mode0;
          currutmp->destuid = destuid0;
          firstline = firstline0;
          lastline = lastline0;
          currline = currline0;
          blockline = blockline0;
          top_of_win = top_of_win0;
          local_article = local_article0;
          currpnt = currpnt0;
          currln = currln0;
          totaln = totaln0;
          curr_window_line = curr_window_line0;
          insert_character = insert_character0;
          my_ansimode = my_ansimode0;
          edit_margin = edit_margin0;
          blockln = blockln0;
          if(!foo) return money;
          else return foo;
        }
        line_dirty = 1;
        redraw_everything = YEA;
        break;

      case Ctrl('W'):
         if (blockln >= 0)
            block_del(2);
         line_dirty = 1;
         break;

      case Ctrl('Q'):           /* Quit without saving */
        if(getans2(0, 0, "µ²§ô¦ý¤£Àx¦s? ", 0, 2, 'n') == 'y')
        {
          currutmp->mode = mode0;
          currutmp->destuid = destuid0;
          firstline = firstline0;
          lastline = lastline0;
          currline = currline0;
          blockline = blockline0;
          top_of_win = top_of_win0;
          local_article = local_article0;
          currpnt = currpnt0;
          currln = currln0;
          totaln = totaln0;
          curr_window_line = curr_window_line0;
          insert_character = insert_character0;
          my_ansimode = my_ansimode0;
          edit_margin = edit_margin0;
          blockln = blockln0;
          return -1;
        }
        line_dirty = 1;
        redraw_everything = YEA;
        break;

      case Ctrl('C'):
        ch = insert_character;
        insert_character = redraw_everything = YEA;

        if (!my_ansimode)
        {
          insert_string(reset_color);
        }
        else
        {
          char ans[4];
          move(b_lines - 2, 55);
          outs("\033[1;33;40mB\033[41mR\033[42mG\033[43mY\033[44mL\033[45mP\033[46mC\033[47mW\033[m");
          if (getdata(b_lines - 1, 0, "½Ð¿é¤J  «G«×/«e´º/­I´º[¥¿±`¥Õ¦r¶Â©³][0wb]¡G", ans, 4, LCECHO,0))
          {
            char t[] = "BRGYLPCW";
            char color[15];
            char *tmp, *apos = ans;
            int fg, bg;
            strcpy(color, "\033[");
            if (isdigit(*apos))
            {
              sprintf(color, "%s%c", color, *(apos++));
              if (*apos)
                sprintf(color, "%s;", color);
            }
            if (*apos)
            {
              if (tmp = strchr(t, toupper(*(apos++))))
                fg = tmp - t + 30;
              else
                fg = 37;
              sprintf(color, "%s%d", color, fg);
            }
            if (*apos)
            {
              if (tmp = strchr(t, toupper(*(apos++))))
                bg = tmp - t + 40;
              else
                bg = 40;
              sprintf(color, "%s;%d", color, bg);
            }
            sprintf(color, "%sm", color);
            insert_string(color);
          }
          else
            insert_string(reset_color);
        }
        insert_character = ch;
        line_dirty = 1;
        break;

      case KEY_ESC:
         line_dirty = 0;
         switch (KEY_ESC_arg) 
         {
/*
hialan: ¶i¤J¨Ï¥ÎªÌ¦W³æ --> §Ö³t¿ï³æ --> ¿ï¾Ü¨ä¥L¬ÝªO --> «h·|µoªí¦b¤£¦Pªº¬ÝªO¤W
         case 'U':
         {
            int mode0 = currutmp->mode;
            int stat0 = currstat;
            screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
            vs_save(screen);
            t_users();
            vs_restore(screen);
            redraw_everything = YEA;
            line_dirty = 1;
            currutmp->mode = mode0;
            currstat = stat0;
            break;
         }
*/
         case 'i':
            t_idle();
            redraw_everything = YEA;
            line_dirty = 1;
            break;
         case 'n':
            search_str(1);
            break;
         case 'p':
            search_str(-1);
            break;
         case 'L':
         case 'J':
            goto_line(0);
            break;
         case ']':
            match_paren();
            break;
         case '0':
         case '1':
         case '2':
         case '3':
         case '4':
         case '5':
         case '6':
         case '7':
         case '8':
         case '9':
            read_tmpbuf(KEY_ESC_arg - '0');
            redraw_everything = YEA;
            break;
         case 'l':                       /* block delete */
         case ' ':
            block_del(0);
            line_dirty = 1;
            break;
         case 'u':
            if (blockln >= 0)
               block_del(1);
            line_dirty = 1;
            break;
         case 'c':
            if (blockln >= 0)
               block_del(3);
            line_dirty = 1;
            break;
         case 'y':
            undelete_line();
            break;
         case 'R':
            raw_mode ^= 1;
            line_dirty = 1;
            break;
         case 'I':
            indent_mode ^= 1;
            line_dirty = 1;
            break;
         case 'j':
            if (blockln >= 0)
               block_shift_left();
            else if (currline->len) 
            {
               int currpnt0 = currpnt;
               currpnt = 0;
               delete_char();
               currpnt = (currpnt0 <= currline->len) ? currpnt0 : currpnt0 - 1;
               if (my_ansimode)
                   currpnt = ansi2n(n2ansi(currpnt, currline),currline);
            }
            line_dirty = 1;
            break;
         case 'k':
            if (blockln >= 0)
               block_shift_right();
            else {
               int currpnt0 = currpnt;
               currpnt = 0;
               insert_char(' ');
               currpnt = currpnt0;
            }
            line_dirty = 1;
            break;
         case 'f':
            while (currpnt < currline->len && isalnum(currline->data[++currpnt]))
               ;
            while (currpnt < currline->len && isspace(currline->data[++currpnt]))
               ;
            line_dirty = 1;
            break;
         case 'b':
            while (currpnt && isalnum(currline->data[--currpnt]))
               ;
            while (currpnt && isspace(currline->data[--currpnt]))
               ;
            line_dirty = 1;
            break;
         case 'd':
            while (currpnt < currline->len) {
               delete_char();
               if (!isalnum(currline->data[currpnt]))
                  break;
            }
            while (currpnt < currline->len) {
               delete_char();
               if (!isspace(currline->data[currpnt]))
                  break;
            }
            line_dirty = 1;
            break;
         default:
            line_dirty = 1;
         }
         break;
      case Ctrl('_'):
         if (strcmp(line, currline->data)) {
            char buf[WRAPMARGIN];

            strcpy(buf, currline->data);
            strcpy(currline->data, line);
            strcpy(line, buf);
            currline->len = strlen(currline->data);
            currpnt = 0;
            line_dirty = 1;
         }
         break;
      case Ctrl('S'):
         search_str(0);
         break;

      case Ctrl('U'):
        insert_char('');
        line_dirty = 1;
        break;

      case Ctrl('V'):                   /* Toggle ANSI color */
         my_ansimode ^= 1;
         if (my_ansimode && blockln >= 0)
            block_color();
         clear();
         redraw_everything = YEA;
         line_dirty = 1;
         break;

      case Ctrl('I'):
        do
        {
          insert_char(' ');
        }
        while (currpnt & 0x3);
        line_dirty = 1;
        break;

      case '\r':
      case '\n':
        split(currline, currpnt);
        line_dirty = 0;
        break;

/* Ptt */
      case Ctrl('G'):  /* ±Ò°Ê½d¥»ºëÆF */
        {
          int mode0 = currutmp->mode;
          int currstat0 = currstat;
          setutmpmode(EDITEXP);
          a_menu("½sÄy»²§U¾¹", "etc/editexp", (HAS_PERM(PERM_SYSOP) ? SYSOP : NOBODY));
          currutmp->mode = mode0;
          currstat = currstat0;
        }
        if (trans_buffer[0])
        {
          if (fp1 = fopen(trans_buffer, "r"))
          {
            int indent_mode0 = indent_mode;
            indent_mode = 0;
            prevln = currln;
            prevpnt = currpnt;

            while (fgets(line, WRAPMARGIN + 2, fp1))
            {
              if (!strncmp(line,"§@ªÌ:",5) ||
                  !strncmp(line,"¼ÐÃD:",5) ||
                  !strncmp(line,"®É¶¡:",5)) 
                continue;
              insert_string(line);
            }
            fclose(fp1);
            indent_mode = indent_mode0;

            while (curr_window_line >= b_lines)
            {
              curr_window_line--;
              top_of_win = top_of_win->next;
            }
          }
        }
        redraw_everything = YEA;
        line_dirty = 1;
        break;

      case KEY_LEFT:
        if (currpnt) {
          if (my_ansimode)
             currpnt = n2ansi(currpnt, currline);
          currpnt--;
          if (my_ansimode)
             currpnt = ansi2n(currpnt, currline);
          line_dirty = 1;
        }
        else if (currline->prev)
        {
          curr_window_line--;
          currln--;
          currline = currline->prev;
          currpnt = currline->len;
          line_dirty = 0;
        }
        break;

      case KEY_RIGHT:
        if (currline->len != currpnt) {
          if (my_ansimode)
             currpnt = n2ansi(currpnt, currline);
          currpnt++;
          if (my_ansimode)
             currpnt = ansi2n(currpnt, currline);
          line_dirty = 1;
        }
        else if (currline->next)
        {
          currpnt = 0;
          curr_window_line++;
          currln++;
          currline = currline->next;
          line_dirty = 0;
        }
        break;

      case KEY_UP:
      case Ctrl('P'):
        if (currline->prev)
        {
          if (my_ansimode)
             ch = n2ansi(currpnt,currline);
          curr_window_line--;
          currln--;
          currline = currline->prev;
          if (my_ansimode)
             currpnt = ansi2n(ch , currline);
          else
             currpnt = (currline->len > lastindent) ? lastindent : currline->len;
          line_dirty = 0;
        }
        break;

      case KEY_DOWN:
      case Ctrl('N'):
        if (currline->next)
        {
          if (my_ansimode)
             ch = n2ansi(currpnt,currline);
          currline = currline->next;
          curr_window_line++;
          currln++;
          if (my_ansimode)
             currpnt = ansi2n(ch , currline);
          else
             currpnt = (currline->len > lastindent) ? lastindent : currline->len;
          line_dirty = 0;
        }
        break;
      case Ctrl('B'):
      case KEY_PGUP:
        redraw_everything = currln;
        top_of_win = back_line(top_of_win, 22);
        currln = redraw_everything;
        currline = back_line(currline, 22);
        curr_window_line = getlineno();
        if (currpnt > currline->len)
          currpnt = currline->len;
        redraw_everything = YEA;
        line_dirty = 0;
        break;

      case KEY_PGDN:
      case Ctrl('F'):
        redraw_everything = currln;
        top_of_win = forward_line(top_of_win, 22);
        currln = redraw_everything;
        currline = forward_line(currline, 22);
        curr_window_line = getlineno();
        if (currpnt > currline->len)
          currpnt = currline->len;
        redraw_everything = YEA;
        line_dirty = 0;
        break;

      case KEY_END:
      case Ctrl('E'):
        currpnt = currline->len;
        line_dirty = 1;
        break;

      case Ctrl(']'):   /* start of file */
        prevln = currln;
        prevpnt = currpnt;
        currline = top_of_win = firstline;
        currpnt = currln = curr_window_line = 0;
        redraw_everything = YEA;
        line_dirty = 0;
        break;

      case Ctrl('T'):           /* tail of file */
        prevln = currln;
        prevpnt = currpnt;
        top_of_win = back_line(lastline, 23);
        currline = lastline;
        curr_window_line = getlineno();
        currln = totaln;
        redraw_everything = YEA;
        currpnt = 0;
        line_dirty = 0;
        break;

      case KEY_HOME:
      case Ctrl('A'):
        currpnt = 0;
        line_dirty = 1;
        break;

      case KEY_INS:             /* Toggle insert/overwrite */
      case Ctrl('O'):
        if (blockln >= 0 && insert_character) {
           char ans[4];

           getdata(b_lines - 1, 0, "°Ï¶ô·L½Õ¥k²¾´¡¤J¦r¤¸(¹w³]¬°ªÅ¥Õ¦r¤¸)", ans, 4,0);
           insert_c = (*ans) ? *ans : ' ';
        }
        insert_character ^= 1;
        line_dirty = 1;
        break;

      case Ctrl('H'):
      case '\177':              /* backspace */
        line_dirty = 1;
        if (my_ansimode) {
           my_ansimode = 0;
           clear();
           redraw_everything = YEA;
        }
        else {
           if (currpnt == 0)
           {
             textline *p;

             if (!currline->prev)
             {
               break;
             }
             line_dirty = 0;
             curr_window_line--;
             currln--;
             currline = currline->prev;
             currpnt = currline->len;
             redraw_everything = YEA;
             if (*killsp(currline->next->data) == '\0')
             {
               delete_line(currline->next);
               break;
             }
             p = currline;
             while (!join(p))
             {
               p = p->next;
               if (p == NULL)
               {
                 indigestion(2);
                 abort_bbs();
               }
             }
             break;
           }
           currpnt--;
           delete_char();
        }
        break;

      case Ctrl('D'):
      case KEY_DEL:             /* delete current character */
         line_dirty = 1;
        if (currline->len == currpnt)
        {
          textline *p = currline;

          while (!join(p))
          {
            p = p->next;
            if (p == NULL)
            {
              indigestion(2);
              abort_bbs();
            }
          }
          line_dirty = 0;
          redraw_everything = YEA;
        }
        else {
           delete_char();
           if (my_ansimode)
              currpnt = ansi2n(n2ansi(currpnt, currline),currline);
        }
        break;

      case Ctrl('Y'):           /* delete current line */
        currline->len = currpnt = 0;

      case Ctrl('K'):           /* delete to end of line */
        if (currline->len == 0)
        {
          textline *p = currline->next;

          if (!p)
          {
            p = currline->prev;
            if (!p)
            {
              break;
            }
            if (curr_window_line > 0)
            {
              curr_window_line--;
              currln--;
            }
          }
          if (currline == top_of_win)
            top_of_win = p;
          delete_line(currline);
          currline = p;
          redraw_everything = YEA;
          line_dirty = 0;
          break;

	case Ctrl('Z'):
          more(BBSHOME"/etc/help/EDIT.help",YEA);
          redraw_everything = YEA;
          line_dirty = 1;
          break;
        }

        if (currline->len == currpnt)
        {
          textline *p = currline;

          while (!join(p))
          {
            p = p->next;
            if (p == NULL)
            {
              indigestion(2);
              abort_bbs();
            }
          }
          redraw_everything = YEA;
          line_dirty = 0;
          break;
        }
        currline->len = currpnt;
        currline->data[currpnt] = '\0';
        line_dirty = 1;
        break;
      }

      if (currln < 0)
        currln = 0;
      if (curr_window_line < 0)
      {
        curr_window_line = 0;
        if (!top_of_win->prev)
        {
          indigestion(6);
        }
        else
        {
          top_of_win = top_of_win->prev;
          rscroll();
        }
      }
      if (curr_window_line == b_lines )
      {
        curr_window_line = t_lines - 2;

        if (!top_of_win->next)
          indigestion(7);
        else
        {
          top_of_win = top_of_win->next;
          move(b_lines, 0);
          clrtoeol();
          scroll();
        }
      }
    }
    edit_margin = currpnt < SCR_WIDTH - 1 ? 0 : currpnt / 72 * 72;

    if (!redraw_everything)
    {
      if (edit_margin != last_margin)
      {
        last_margin = edit_margin;
        redraw_everything = YEA;
      }
      else
      {
         move(curr_window_line, 0);
         clrtoeol();
         if (my_ansimode)
           outs(currline->data);
         else
            edit_outs(&currline->data[edit_margin]);
         edit_msg();
      }
    }
  }
}
