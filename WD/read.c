/*-------------------------------------------------------*/
/* read.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : board/mail interactive reading routines      */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"
#include <sys/mman.h>

#define FHSZ    sizeof(fileheader)
char *str_ttl(char *title);
static fileheader *headers = NULL;
/* static */ 
/* shakalaca.000117: unmarked for list.c */
int last_line;

/* ----------------------------------------------------- */
/* cursor & reading record position control              */
/* ----------------------------------------------------- */

keeploc *
getkeep(char *s, int def_topline, int def_cursline)
{
  static struct keeploc *keeplist = NULL;
  struct keeploc *p;
  void *malloc();

  if (def_cursline >= 0)
     for (p = keeplist; p; p = p->next)
     {
       if (!strcmp(s, p->key))
       {
         if (p->crs_ln < 1)
           p->crs_ln = 1;
         return p;
       }
     }
  else
     def_cursline = -def_cursline;

  p = (keeploc *) malloc(sizeof(keeploc));
  p->key = (char *) malloc(strlen(s) + 1);
  strcpy(p->key, s);
  p->top_ln = def_topline;
  p->crs_ln = def_cursline;
  p->next = keeplist;
  return (keeplist = p);
}

/* calc cursor pos and show cursor correctly */

/* hialan: cursor_pos()傳值解釋 */
/*	locmem  : 指標結構 */
/*	val     : 希望移到的位置 */
/*	from_top: 移動後, 希望離頂端多少?*/
static int
cursor_pos(struct keeploc *locmem, int val, int from_top)
{
  int top = locmem->top_ln;

  if(!last_line)
    return RC_NONE;
  
  if (val > last_line)
  {
    if(HAS_HABIT(HABIT_CYCLE))
      val = 1;
    else
      val = last_line;
  }
  if (val <= 0)
  {
    if(HAS_HABIT(HABIT_CYCLE))
      val = last_line;
    else
      val = 1;
  }

  if (val >= top && val < top + p_lines)
  {
    locmem->crs_ln = val;
    return RC_NONE;
  }
  locmem->top_ln = val - from_top;
  if (locmem->top_ln <= 0)
    locmem->top_ln = 1;
  locmem->crs_ln = val;
  return RC_BODY;
}

/* ----------------------------------------------------- */
/* 主題式閱讀						 */
/* ----------------------------------------------------- */

static int
thread(keeploc *locmem, int stype)
{
  fileheader fh;
  char *tag, *query;
  int now, pos;
  int step, near = -1;

  pos = locmem->crs_ln;
  step = (stype & RS_FORWARD) ? 1 : -1;

  if (stype & RS_RELATED)
  {
    tag = headers[pos - locmem->top_ln].title;
    if (stype & RS_CURRENT)
    {
      if (stype & RS_FIRST)
      {
        if (!strncmp(currtitle, tag, 40))
          return pos;
      }
      query = currtitle;
    }
    else
    {
      query = str_ttl(tag);
      if (stype & RS_FIRST)
      {
        if (query == tag)
          return pos;
      }
    }
  }

  for(now = locmem->crs_ln + step;
      now > 0 && now <= last_line;
      now += step)
  {
    rec_get(currdirect, &fh, sizeof(fileheader), now);

    if (stype & RS_THREAD)
    {
      if (strncmp(fh.title, str_reply, 3))
        break;
    }
    else if(stype & (RS_TITLE | RS_RELATED))
    {
      tag = str_ttl(fh.title);

      if(!strncmp(tag, query, 40))
      {
        if (stype & RS_FIRST)
        {
          if (tag == fh.title)
            break;
          near = now;          
        }
        else 
          break;
      }
    }
  }

  if(now <= 0 || now > last_line) /* 沒找到 */
    now = (near == -1) ? pos : near;
  else
    strncpy(currtitle, str_ttl(fh.title), 40);
    
  return now;
}

/* SiE: 系列式閱讀....使用mmap來加速搜尋 */

static int
select_read(keeploc *locmem, int sr_mode)
{
  register char *tag,*query;
  fileheader fh, *tail, *head;
  char fpath[80], *fimage, genbuf[50];
  static char ans[TTLEN+1]="";
  const int size = sizeof(fileheader);
  int fd, fr, fsize;
  struct stat st;

  if( currmode & MODE_SELECT)
    return -1;
    
  /* 建立關鍵字 */
  if(sr_mode == RS_TITLE)
    query = str_ttl(headers[locmem->crs_ln - locmem->top_ln].title);
  else if (sr_mode == RS_FIRST) 
    query = str_reply;
  else if (sr_mode == RS_CURRENT)
     query = ".";
  else if (sr_mode == RS_THREAD)
     query = "m";
  else if (sr_mode == RS_SCORE)  /* 文章評分 */
     query = "";
  else if (sr_mode == RS_DATE)   /* 搜尋日期 */
  {
    char month[3], day[3];
    if(getdata(b_lines, 0, "搜尋日期(月) ", month, 3, DOECHO, 0))
    {
      if(atoi(month) < 1 || atoi(month) > 12)
        strlcpy(month, query, 3);
    }

    if(getdata(b_lines, 0, "搜尋日期(日) ", day, 3, DOECHO, 0))
    {
      if(atoi(month) < 1 || atoi(month) > 31)
        strlcpy(day, query+3, 3);
    }

    query = ans;
    sprintf(query, "%2d/%02d", atoi(month), atoi(day));
  }
  else 
  {
    query = ans;
    sprintf(fpath, "搜尋%s [%s] ",
        (sr_mode == RS_RELATED) ? "標題" : "作者", query);

    if(getdata(b_lines, 0, fpath, fpath, 30, DOECHO,0))
      strlcpy(query, fpath, 64);

    if(!(*query))
     return RC_NONE;
  }
  
  /* 設定輸出檔案 */
  sprintf(genbuf, "SR.%s", cuser.userid);
  
  if(sr_mode == RS_FIRST)
    setbfile(fpath, currboard, "SR.thread"); 
  else if(currstat == RMAIL)
    sethomefile(fpath, cuser.userid, genbuf);
  else
    setbfile(fpath, currboard, genbuf);

  /* 開始輸出檔案 */
  outmsg("搜尋中，請稍後..");
  if ((fd = open(currdirect,  O_RDONLY)) >= 0)
  {
    if ( !fstat(fd, &st) && S_ISREG(st.st_mode) && (fsize = st.st_size) > 0)
      fimage = mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);

    close(fd);
  }
  else 
    fimage = (char *) -1;

  if (fimage == (char *) -1)
  {
    outmsg("索引檔開啟失敗！");
    return RC_NONE;
  }

  head = (fileheader *) fimage;
  tail = (fileheader *) (fimage + fsize);

  if(((fr = open(fpath,O_WRONLY | O_CREAT | O_TRUNC,0600)) != -1))
  {
    fileheader *hdr, *currhead = head;
    int count;

    do
    {
      fh = *head;
      switch(sr_mode)
      {
        case RS_FIRST:
          if (strncmp(fh.title, query, 3))
          {
            count = 0;
            for (hdr = currhead; hdr < tail; hdr++)
            {
              tag = str_ttl(hdr->title);
              if (!strncmp(fh.title, tag, 40))
              {
                if(hdr < head && tag == hdr->title) 
                  break;
                ++count;
              }
            }
          
            if(hdr > head)
            {
              sprintf(fh.date, "%5d", count);
              write(fr, &fh, size);
            }
          }
          break;
        case RS_TITLE:
          tag = str_ttl(fh.title);
          if(!strncmp(tag, query, 40))
            write(fr, &fh, size);
          break;
        case RS_RELATED:
          tag = fh.title;
          if(strcasestr(tag,query))
            write(fr, &fh, size);
          break;
        case RS_AUTHOR:
          tag = fh.owner;
          if(strcasestr(tag,query))
            write(fr, &fh, size);
          break;
        case RS_CURRENT:
          tag = fh.owner;
          if(!strchr(tag, '.'))
            write(fr, &fh, size);
          break;
        case RS_THREAD:
          if(fh.filemode & (FILE_MARKED |  FILE_DIGEST))
            write(fr, &fh, size);
          break;
        case RS_SCORE:
          if(fh.score != 0 || fh.filemode & FILE_SCORED)
            write(fr, &fh, size);
          break;
        case RS_DATE:
          tag = fh.date;
          if(!strncmp(tag, query, 6))
            write(fr, &fh, size);
          break;
        }
      } while (++head < tail);
      fstat(fr,&st);
      close(fr);
    }
    munmap(fimage, fsize);

    if(st.st_size)
    {
      currmode ^= MODE_SELECT;

    if(sr_mode == RS_FIRST)
        currmode ^= MODE_TINLIKE;    

      strcpy(currdirect, fpath);
    }
  return st.st_size;
}


/*-----------------------------------------*/
/* i_read_helper() : i_read() 線上求助    */
/*-----------------------------------------*/

/* 基本指令集 */
struct one_key basic[]={
KEY_LEFT, NULL, 0, "離開。        其他相同的按鍵: q, e",0,
KEY_UP,   NULL, 0, "游標向上。    其他相同的按鍵: p, k",0,
KEY_DOWN, NULL, 0, "游標向下。    其他相同的按鍵: n, j",0,
KEY_PGDN, NULL, 0, "下一頁。      其他相同的按鍵: 空白鍵, N, Ctrl+f",0,
KEY_PGUP, NULL, 0, "上一頁。      其他相同的按鍵: P, Ctrl+b",0,
KEY_END,  NULL, 0, "跳到最後一項。其他相同的按鍵: $",0,
KEY_RIGHT,NULL, 0, "執行, 閱\讀。  其他相同的按鍵: Enter",0,
'\0', NULL, 0, NULL,0};


/* 主題式閱讀指令集 */
struct one_key sub_key[]={
'/',      NULL, 0, "找尋標題。              其他相同的按鍵: ?",0,
'S',      NULL, 0, "循序閱\讀新文章。",0,
'L',      NULL, 0, "閱\讀非轉信文章。",0,
'u',      NULL, 0, "標題式閱\讀。",0,
'=',      NULL, 0, "找尋首篇文章。",0,
'\\',     NULL, 0, "找尋游標該處之首篇文章。",0,
'[',	  NULL, 0, "向前搜尋相同標題之文章。其他相同的按鍵: +",0,
']',	  NULL, 0, "向後搜尋相同標題之文章。其他相同的按鍵: -",0,
'<',	  NULL, 0, "向前搜尋其他標題。      其他相同的按鍵: ,",0,
'>',	  NULL, 0, "向後搜尋其他標題。      其他相同的按鍵: .",0,
'A', 	  NULL, 0, "搜尋作者。              其他相同的按鍵: a",0,
'#',	  NULL, 0, "所有被加分過的文章。",0,
'G',	  NULL, 0, "所有被 mark 過的文章。",0,
'~',	  NULL, 0, "搜尋日期。",0,
'\0', NULL, 0, NULL,0};

static char *
show_cmd_level(usint level)  /* 顯示權限 by spy */
{
  register int i = -1;
  static char *none = "無限制";

  if (!level) return none;

  while (level)
  {
    level >>= 1;
    i++;
  }
  return permstrings[i];
}
                      

static int 
show_helplist_line(int row, struct one_key *cmd, char *barcolor)
{
  char buf[128];
  char key[10];
  int i;

  for(i='A';Ctrl(i) <= Ctrl('Z');i++)
  {
    if(Ctrl(i) == cmd->key) 
    {
      sprintf(key, "Ctrl+%c", i);
      break;
    }
  }
  if(i == 'Z'+1)
    sprintf(key, "%c", cmd->key);
  
  switch(cmd->key)
  {
    case KEY_UP:
      sprintf(key, "↑");
      break;
    case KEY_DOWN:
      sprintf(key, "↓");
      break;    
    case KEY_LEFT:
      sprintf(key, "←");
      break;    
    case KEY_RIGHT:
      sprintf(key, "→");
      break;    
    case KEY_PGUP:
      sprintf(key, "Page UP");
      break;    
    case KEY_PGDN:
      sprintf(key, "Page DOWN");
      break;    
    case KEY_END:
      sprintf(key, "End");
      break;    
    case KEY_TAB:
      sprintf(key, "Tab");
      break;
    case KEY_HOME:
      sprintf(key, "Home");    
      break;
  }
    
  sprintf(buf, "    %s %9s \033[m %-12s %s",  barcolor ? barcolor : "" , 
	       key, show_cmd_level(cmd->level), cmd->desc);
  move(3 + row, 0);
  clrtoeol();
  outs(buf);
}

int 
i_read_helper(struct one_key *rcmdlist)
{
  char cursor=0, i, draw=1, pos;
  char max_cursor;
  char barcolor[50];
  static char re_enter=0;
  int ch;
  char top, bottom;	/*hialan: 一頁的最上面和最下面 , for 根據權限顯示 */

  get_lightbar_color(barcolor);
  top = bottom = cursor = pos = 0;
  for(max_cursor=0;rcmdlist[max_cursor].key;max_cursor++);

  while(1)
  {
    switch(draw)
    {
      case 1:
        clear();
  
        sprintf(tmpbuf,"%s [線上 %d 人]",BOARDNAME,count_ulist());
        showtitle("線上求助", tmpbuf);

        outs("\033[30;46m◤\033[37m水球＼名單◢\033[30;47m說明\033[m◣     ←)上一頁  →)執行該指令 ↑|↓)選擇  Tab)本站說明手冊\n");
        prints("%s    指令/按鍵   權    限     說          明%36s\033[0m\n", COLOR3, "");

      case 2:
        bottom = top-1;
        ch=0;	/* 借用來記數 */
        for(i=0;i<p_lines && bottom<max_cursor;i++)
        {
          for(bottom++;bottom<max_cursor;bottom++)
            if(HAS_PERM(rcmdlist[bottom].level))
            {
              show_helplist_line(i, rcmdlist+bottom, 0);
              ch++;
              break;
            }
        }
        if(!ch) return -1;

      case 3:
        move(b_lines, 0);
        clrtoeol();
        prints("%s  選擇功\能  %s  b)基本指令集 %-40.40s  \033[m", COLOR2, COLOR3, "u)主題式閱\讀指令集");
        draw = 0;
        break;
    }
    
    if(HAS_HABIT(HABIT_LIGHTBAR))
    {
      show_helplist_line(pos, rcmdlist+cursor, barcolor);
      cursor_show(3+pos, 0);
      ch = igetkey();
      show_helplist_line(pos, rcmdlist+cursor, 0);
    }
    else
      ch = cursor_key(3 + pos, 0);
  
    switch(ch)
    {
      case KEY_LEFT:
      case 'q':
        return RC_NONE;

      case KEY_PGUP:
        cursor=top;
      case KEY_UP:
        cursor--;
        pos--;
        
        draw=0;
        if(cursor<0)
        {
          top = max_cursor;
          cursor = top-1;
        }
        if(cursor<top)
        {
          for(pos=0;pos<p_lines && top>0;pos++)
            for(top--;top>0 && !HAS_PERM(rcmdlist[top].level);top--);

          pos--;
          draw=1;
        }
        for(;!HAS_PERM(rcmdlist[cursor].level) && cursor>=0;cursor--);        
        break;

      case ' ':
      case KEY_PGDN:
        cursor=bottom;
      case KEY_DOWN:
        cursor++;
        pos++;

        draw=0;
        for(;!HAS_PERM(rcmdlist[cursor].level) && cursor < max_cursor;cursor++);        

        if(cursor > bottom || cursor >= max_cursor)
        {
          for(top=bottom+1;!HAS_PERM(rcmdlist[top].level) && top < max_cursor;top++);

          if(top >= max_cursor)
          {
            top = 0;
            for(;!HAS_PERM(rcmdlist[top].level);top++);
          }
          cursor = top;
          pos=0;
          draw=1;
        }
        break;
      
      case '\r':
      case '\n':
      case KEY_RIGHT:
      {
        char buf[80];
        
        sprintf(buf,"請問你是否要執行 %s？", rcmdlist[cursor].desc);
        if(getans2(b_lines, 0, buf, 0, 2, 'y') == 'y')
          return (ch = rcmdlist[cursor].key);
        draw = 3;
        break;
      }

      case 'b':
        if(!re_enter)
        {
          re_enter = 1;
          i_read_helper(basic);
          re_enter = 0;
          draw = 1;
        }
        break;
      
     case 'u':
       if(!re_enter)
       {
         re_enter = 1;
         i_read_helper(sub_key);
         re_enter = 0;
         draw = 1;
       }
       break;
     
     case KEY_TAB:
       HELP();
       draw=1;
       break;
    }
  }
  
  return RC_FULL;
}



static int
i_read_key(struct one_key *rcmdlist, struct keeploc *locmem, 
           int ch, int def_key, int bottom_line, void (*doentry)())
{
  int i, mode = RC_NONE, row, lastmode = 0;
  int new_ln = locmem->crs_ln, new_top = 10;
  char bar_color[50];
  static int default_ch = 0;     /*主題式閱讀, 記憶是屬於何種按鍵*/
  static int thread_title = 0;
  
  get_lightbar_color(bar_color);
  
  do
  {
    mode = cursor_pos(locmem, new_ln, new_top);
    
    /* 如果閱讀中碰到最後一篇, 則跳出 */
    if(default_ch != 0 && new_ln != locmem->crs_ln)
    {
      cursor_pos(locmem, last_line, p_lines - 1);
      default_ch = 0;
      return RC_FULL;
    }
    
    /* 某些動作要重新畫 */
    if(lastmode == POS_NEXT)
    {
      default_ch = 0;
      return (mode == RC_NONE) ? RC_DRAW : RC_FULL;
    }
    
    /* 換頁 */
    if(mode != RC_NONE)
    {
      return (default_ch != 0) ? RC_RELOAD : mode;
    }

    /* 取得新的按鍵 */
    if(!default_ch)
    {
      row= (last_line) ? 3 + locmem->crs_ln - locmem->top_ln : 3;
    
      if(HAVE_HABIT(HABIT_LIGHTBAR) && last_line != 0)
      {
        (*doentry) (locmem->crs_ln, &headers[locmem->crs_ln - locmem->top_ln], row, bar_color);
        move(row, 0);
        ch = igetkey();
        (*doentry) (locmem->crs_ln, &headers[locmem->crs_ln - locmem->top_ln], row, 0);
      }
      else
        ch = cursor_key(row, 0);
    }
    else
    {
      ch = default_ch;
    }
    
    new_top = 10;            /*default 10*/
    
    if(ch == 'h')
    {
      screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));    

      vs_save(screen);    
      ch = i_read_helper(rcmdlist);
      vs_restore(screen);
      if(!ch)
        mode = RC_NONE;
    }

    if ((currstat == RMAIL) || (currstat == READING) || (currstat == ANNOUNCE))
    {
      switch (ch)
      {
      case Ctrl('E'):
        if (HAS_PERM(PERM_SYSOP))
        {
          DL_func("SO/admin.so:m_user");
          mode = RC_FULL;
        }
        break;
 
      case '/':
      case '?':
        mode =  (select_read(locmem,RS_RELATED)) ? RC_NEWDIR : RC_FOOT;
        break;

      case 'S':
        mode = (select_read(locmem,RS_TITLE)) ? RC_NEWDIR : RC_FOOT;
        break;

      case 'L':
      if (currstat != ANNOUNCE)
        mode = (select_read(locmem,RS_CURRENT)) ? RC_NEWDIR : RC_FOOT;  /* local articles */    
      break;

      case 'u':
      if (currstat != ANNOUNCE)
        if (!thread_title && select_read(locmem,RS_FIRST)) 
        {
          thread_title = 1;
          mode = RC_NEWDIR;
        }
        else 
        {
          bell();
          mode = RC_NONE;
        }    
      break;
    
      case '=':
        new_ln = thread(locmem, RELATE_FIRST);
        break;

      case '\\':
        new_ln = thread(locmem, CURSOR_FIRST);
        break;

      /* quick search title backword */
      case '[':
        new_ln = thread(locmem, RELATE_PREV);
        break;
 
      /* quick search title forword */
      case ']':
        new_ln = thread(locmem, RELATE_NEXT);
        break;
 
      case '+':
        new_ln = thread(locmem, CURSOR_NEXT);
        break;

      case '-':
        new_ln = thread(locmem, CURSOR_PREV);
        break;

      case '<':
      case ',':
        new_ln = thread(locmem, THREAD_PREV);
        break;

      case '.':
      case '>':
        new_ln = thread(locmem, THREAD_NEXT);
        break;

      case 'A':
      case 'a':
        mode = (select_read(locmem,RS_AUTHOR)) ? RC_NEWDIR : RC_FOOT;
        break;

      case '#':
        mode = (select_read(locmem,RS_SCORE)) ? RC_NEWDIR : RC_FOOT;
        break;

      case 'G':   /* marked articles */
        mode = (select_read(locmem,RS_THREAD)) ? RC_NEWDIR : RC_FOOT;
        break;
    
      case '~':
        mode = (select_read(locmem, RS_DATE)) ? RC_NEWDIR : RC_FOOT;
        break;
      }
    }

    switch (ch)
    {
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
      if( (i = search_num(ch, last_line)) != -1)
        new_ln = i+1;
      break;

    case KEY_LEFT:
    case 'q':
    case 'e':
    if ((currstat == RMAIL) || (currstat == READING) || (currstat == ANNOUNCE))
    {
      if (thread_title > 0)
      {
        --thread_title;
        if (thread_title > 0)
        {
          setbfile(currdirect, currboard, "SR.thread");
          currmode |= (MODE_TINLIKE | MODE_SELECT);
          mode = RC_NEWDIR;
          break;
        }
      }      
      
      if (currmode & MODE_SELECT)
      {
        char genbuf[256];
        fileheader *fhdr = &headers[locmem->crs_ln - locmem->top_ln];
        
        board_select();
        strlcpy(genbuf, currdirect, sizeof(genbuf));
        locmem = getkeep(genbuf, 0, 1);
        locmem->crs_ln = getindex(genbuf, fhdr->filename, sizeof(fileheader));
        i = locmem->crs_ln - p_lines + 1;
        locmem->top_ln =  i < 1 ? 1 : i;
        
        mode = RC_NEWDIR;
      }
      else if(currmode & MODE_DIGEST)
        mode = board_digest();
      else
        mode = QUIT;
    }
    else
      mode = QUIT;
    
    break;

    case 'p':
    case 'k':
    case KEY_UP:
        new_ln = locmem->crs_ln - 1;
        new_top = p_lines - 2;
        break;

    case 'n':
    case 'j':
    case KEY_DOWN:
      new_ln = locmem->crs_ln + 1;
      new_top = 1;
      break;

    case ' ':
    case KEY_PGDN:
    case 'N':
    case Ctrl('F'):
      new_top = 0;
      if(last_line > locmem->top_ln + p_lines)
        new_ln = locmem->top_ln + p_lines;
      else
        new_ln = last_line;
      break;

    case 'P':
    case KEY_PGUP:
    case Ctrl('B'):
      new_top = 0;
      if(locmem->top_ln - p_lines > 0)
        new_ln = locmem->top_ln - p_lines;
      else
        new_ln = 1;
      break;
    
    case KEY_END:
    case '$':
      new_ln = last_line;
      new_top = p_lines - 1;
      break;
    
    case KEY_HOME:
      new_ln = 1;
      new_top = 0;
      break;
    
    case '\n':
    case '\r':
    case KEY_RIGHT:
      if ((currmode & MODE_TINLIKE) && ((currstat == RMAIL) || (currstat == READING))) 
      {
        ++thread_title;
        currmode &= ~(MODE_SELECT | MODE_TINLIKE);
        setbdir(currdirect, currboard);
        select_read(locmem,RS_TITLE);
        mode = RC_NEWDIR;
        break;
      } 
      ch = 'r';

    default:
      if(!last_line)
        ch = def_key;
      
      if(!ch)
      {
        mode = RC_NONE;
        break;
      } 
    
      for (i = 0; rcmdlist[i].key; i++)
      {
        if (rcmdlist[i].key == ch && rcmdlist[i].fptr)
        {
          /* shakalaca.000215: currdirect 為目前所在 root directory,
           * 如能善加修改, mail, post, anno 就可合而為一了, 這三者所差
           * 只在於 root dir. 
           * 相同: 轉寄 (F), 串列 (S), 搜尋 (/,A), 瀏覽 (r), 
           *       收錄 (c,C,^C,P), 暫存 (T), 查詢 (^Q),  
           *       移動 (上下左右鍵), tin-like read (u), 
           * 限制(not in mail):編修 (E), 發表 (^p)
           */
          
          if(!rcmdlist[i].level || HAS_PERM(rcmdlist[i].level))
          {
            void *p;
            int num;
            char direct[64], *newdir = NULL;
  	  
            num = locmem->crs_ln - bottom_line;
            if(num > 0)
            {
              setdirpath(direct, currdirect, FN_BOTTOM);
	      newdir = direct;
	    }
            else
  	    {
	      newdir = currdirect;
	      num = locmem->crs_ln;
	    }
	  
            if(!rcmdlist[i].mode)
              p = rcmdlist[i].fptr;
            else
              p = (void *) DL_get(rcmdlist[i].fptr);
  
            mode = (*((int (*)())p)) (num,
                   &headers[locmem->crs_ln - locmem->top_ln], newdir);
                                      
            if (mode == RS_NEXT || mode == RS_PREV || mode == POS_NEXT ||
                mode == RELATE_FIRST || mode == RELATE_NEXT || 
                mode == RELATE_PREV || mode == THREAD_NEXT || mode == THREAD_PREV)
            {
              lastmode = mode;

              if (mode == RS_NEXT)
                new_ln = locmem->crs_ln + 1;
              else if(mode == RS_PREV)
                new_ln = locmem->crs_ln - 1;
              else if(mode == POS_NEXT)
              {
                new_ln = locmem->crs_ln + 1;
                new_top = 1;
              }
              else
              {
                new_ln = thread(locmem, mode);
                
                /*相同的要跳出*/
                if(new_ln == locmem->crs_ln)
                {
                  default_ch = 0;
                  return RC_FULL;
                }
              }

              mode = RC_NONE;
              default_ch = 'r';
            }
            else
            {
              default_ch = 0;
              lastmode = 0;
            }
            break;
          }
        }
      } /* for() in default:*/
    } /* switch() */
    
  } while (mode == RC_NONE);
  return mode;
}


int
get_records_and_bottom(char *direct,  fileheader* headers,
                     int recbase, int p_lines, int last_line, int bottom_line)
{
    int     n = bottom_line - recbase + 1, rv;
    char    directbottom[60];

    if( !last_line )
	return 0;
    if( n >= p_lines || (currmode & (MODE_SELECT | MODE_DIGEST)) )
	return get_records(direct, headers, FHSZ, recbase, 
			   p_lines);

    setdirpath(directbottom, direct, FN_BOTTOM);
    if( n <= 0 )
	return get_records(directbottom, headers, FHSZ, 1-n, 
			   last_line-recbase + 1);
      
    rv = get_records(direct, headers, FHSZ, recbase, n);

    if( bottom_line < last_line )
	rv += get_records(directbottom, headers+n, FHSZ, 1, 
			  p_lines - n );
    return rv;
}

void
i_read(int cmdmode, char *direct, void (*dotitle)(), 
       void (*doentry)(), struct one_key *rcmdlist, int def_key)
{
  keeploc *locmem;
  int recbase, mode, ch;
  int num = 0, entries = 0;
  int i, bid;
  char currdirect0[64];
  int bottom_line = 0;
  int last_line0 = last_line;
  fileheader* headers0 = headers;
  
  strlcpy(currdirect0 ,currdirect, sizeof(currdirect0));
  
  headers = (fileheader *) calloc(p_lines, FHSZ);
  strcpy(currdirect, direct);
  mode = RC_NEWDIR;

  do
  {
    /* -------------------------------------------------------- */
    /* 依據 mode 顯示 fileheader                                 */
    /* -------------------------------------------------------- */

    setutmpmode(cmdmode);

    switch (mode)
    {
    case RC_NEWDIR:             /* 第一次載入此目錄 */
    case RC_CHDIR:

      if(currstat == READING && !(currmode & (MODE_SELECT | MODE_DIGEST)))
      {
        bid = getbnum(currboard);
        if( (bottom_line = getbtotal(bid)) == 0)
        {
          setbtotal(bid);

          //setbottomtotal(bid);
          bottom_line = getbtotal(bid);
        }
        last_line = bottom_line + getbottomtotal(bid);
      }
      else
        bottom_line = last_line = rec_num(currdirect, FHSZ);
      
      if (mode == RC_NEWDIR)
      {
        num = last_line - p_lines + 1;
        locmem = getkeep(currdirect, num < 1 ? 1 : num, last_line);
      }
      recbase = -1;

    case RC_FULL:
      (*dotitle) (RC_FULL);

    case RC_BODY:
      if (last_line < locmem->top_ln + p_lines) /*當畫面有顯示最後一項*/
      {						/*才更新 last_line*/
        if(currstat == READING && !(currmode & (MODE_SELECT | MODE_DIGEST)))
        {
          if(getbtotal(bid) == 0)
            setbtotal(bid);

          bottom_line = getbtotal(bid);
          num = bottom_line + getbottomtotal(bid);
        }
        else
          bottom_line = num = rec_num(currdirect, FHSZ);

        if (last_line != num)
        {
          last_line = num;
          recbase = -1;
        }
      }

      if (recbase != locmem->top_ln) 
      {
        recbase = locmem->top_ln;
        if (recbase > last_line)
        {
          recbase = last_line - p_lines + 1;
          if (recbase < 1)
            recbase = 1;
          locmem->top_ln = recbase;
        }
        entries = get_records_and_bottom(currdirect, headers, recbase, p_lines, last_line, bottom_line);
      }
      if (locmem->crs_ln > last_line || locmem->crs_ln < 1)
        locmem->crs_ln = last_line;

      move(3, 0);
      clrtobot();

    case RC_DRAW:
      move(3, 0);
      
      if( last_line == 0)
        outs("      空無一物");
      else
        for (i = 0; i < entries; i++)
          (*doentry) (locmem->top_ln + i, &headers[i], i+3, 0);

    case RC_FOOT:
       (void) (*dotitle) (RC_FOOT);
      break;
    
    case RC_RELOAD:
      if (recbase != locmem->top_ln) 
      {
        recbase = locmem->top_ln;
        if (recbase > last_line)
        {
          recbase = last_line - p_lines + 1;
          if (recbase < 1)
            recbase = 1;
          locmem->top_ln = recbase;
        }
        entries = get_records_and_bottom(currdirect, headers, recbase, p_lines, last_line, bottom_line);
      }      
      break;
    }

    /* -------------------------------------------------------- */
    /* 讀取鍵盤，加以處理，設定 mode                            */
    /* -------------------------------------------------------- */
    
    mode = i_read_key(rcmdlist, locmem, ch, def_key, bottom_line, doentry);

  } while (mode != QUIT);

   free(headers);
   last_line = last_line0;
   headers = headers0;
   strlcpy(currdirect ,currdirect0, sizeof(currdirect));
   return;
} 
