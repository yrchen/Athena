/*-------------------------------------------------------*/
/* form.c       ( WD_hialan BBS    Ver 0.01 )            */
/*-------------------------------------------------------*/
/* author : hialan.nation@infor.org                      */
/* target : 色彩選擇/光棒相關函數/視窗                   */
/* create : 2003/10/22                                   */
/*-------------------------------------------------------*/

/*-------------------------------------------------------*/
/* color.c      ( WD_hialan BBS    Ver 0.01 )            */
/*-------------------------------------------------------*/
#include "bbs.h"

/*Add For LightBAR by hialan*/

/*-------------------*/
/*光棒 檢查-更動 函數*/
/*-------------------*/
#define LB_BG 0	/* 背景 */
#define LB_WD 1 /* 文字 */
#define LB_LT 2 /* light 亮度 */
#define LB_BL 3 /* blink 閃爍 */
#define LB_UL 4 /* underline 底線 */

int 
get_color(char *color, char cset[5])
{
  char buf[40];
  
  color[0] = '\0';
    
  if(cset[LB_LT] == 0)
    strcpy(color, "\033[m");
  else if(cset[LB_LT] == 1)
    strcpy(color, "\033[1m");

  if(cset[LB_BG])
  {
    sprintf(buf,"\033[4%dm",cset[LB_BG]);
    strcat(color, buf);
  }

  if(cset[LB_BL])
    strcat(color,"\033[5m");

  if(cset[LB_UL])
    strcat(color,"\033[4m");
  
  /*文字顏色*/
  sprintf(buf,"\033[3%dm",cset[LB_WD]);
  strcat(color,buf);

  return 0;
}

int 
save_lightbar_color(char *color)
{
  int i, unum = do_getuser(cuser.userid, &xuser);
  
  if(unum > 0)
  {
    for(i=0;i<5;i++)
      xuser.lightbar[i]=color[i];
    substitute_record(fn_passwd, &xuser, sizeof(userec), unum);
    update_data();
    return 1;
  }
  else
    return -1;
}

char * 
get_lightbar_color(char *color)
{
  if(strcmp(cuser.userid, STR_GUEST))
    get_color(color, cuser.lightbar);
  else
  {
    char def_color[] = DEFBARCOLOR;
    get_color(color, def_color);
  }

  return color;
}

int 
change_lightbar()
{
  char ch[5];
  int i;

  for(i=0;i<5;i++)
    ch[i]=cuser.lightbar[i];
  
  if(color_selector(ch) == 1)
  {
    if(save_lightbar_color(ch) == 1)
      pressanykey("修改完成!!");
    else
      pressanykey("修改失敗>_<");
  }
  else
    pressanykey("取消!!");
  
  return 0;
}  

/*---------------------------*/
/* 顏色選擇器		     */
/*---------------------------*/
static int
selector_item(char *prompt, int line, int pos, char *lightbar, int def)
{
  int bottom, num, i;
  
  char **choose;
  char *color[8]={"0)黑色","1)紅色","2)綠色","3)黃色","4)藍色","5)紫色","6)青色","7)白色"};
  char *underline[2]={"0)無底線","1)底線"};
  char *blink[2]={"0)不閃爍","1)閃爍"};
  char *light[3]={"0)低亮度","1)高亮度","2)不設定"};
  
  switch(line)
  {
    case 0:  /* 背景 */
      bottom=b_lines - 5;
      num=8;
      choose=color;
      break;
    case 1:  /* 文字 */
      bottom=b_lines - 4;
      num=8;
      choose=color;    
      break;
    case 2:  /* 亮度 */
      bottom=b_lines - 3;
      num=3;
      choose=light;
      break;
    case 3:  /* 閃爍 */
      bottom=b_lines - 2;
      num=2;
      choose=blink;
      break;
    case 4:  /* 底線 */
      bottom=b_lines - 1;
      num=2;
      choose=underline;
      break;
  }
  
  move(bottom, 0);
  clrtoeol();
  outs(prompt);

  if (pos < 0)		/* hialan.030704:這邊偷懶, 把迴圈選擇寫在這邊XD */
    pos = num-1;
  else if(pos >=num)
    pos = 0;
    
  for(i=0;i<num;i++)
  {
    if(i==pos)
      prints("[%s%s\033[m]", lightbar, choose[i]);
    else
      prints(" %s ", choose[i]);
  }
  return pos;
}


/* hialan.030704:每一行的值由 color 記憶, 第幾列由 pos 記憶 */
int
color_selector(char *color)
{
  int i;
  int pos=0, ch=0;
  char *ques[5]={"  底色 ","  字色 ","  亮度 ","  閃爍 ","  底線 "}; 
  int def[5]={7, 4, 0, 0, 1};
  char lightbar[40];
  char preview[64];

  get_color(lightbar, cuser.lightbar);

  clear();
  
  sprintf(preview, "%s [線上 %d 人]", BOARDNAME, count_ulist());
  showtitle("顏色設定", preview);  

  move(b_lines-6,0);
  outs(msg_seperator);
  
  for(i=0;i<5;i++)
    color[i] = selector_item(ques[i], i, color[i], lightbar, def[i]);

  move(b_lines, 0);
  prints("\033[0;44m  調 色 盤  %s%68.68s\033[m",  COLOR3,
    	 "↑↓←→)改變修改選項  Enter)確定修改  Q)取消  ");
  
  while(ch !='\n' && ch !='q')
  {
    color[pos]= selector_item(ques[pos], pos, color[pos], lightbar, def[pos]);
    get_color(preview, color);
    move(b_lines/2, 34);
    clrtoeol();
    prints("%s  這裡是預覽喔!!  \033[m", preview);
    ch = cursor_key(b_lines-5+pos, 0);

    switch(ch)
    {
      case KEY_UP:
        pos--;
        if(pos < 0) pos = 4;
        break;
      case KEY_DOWN:
        pos++;
        if(pos > 4) pos = 0;
        break;
      case KEY_LEFT:
        color[pos]--;
        break;
      case KEY_RIGHT:
        color[pos]++;
        break;
      case '\r':
      case '\n':
        return 1;	/* 確定!! */
      case 'q':
        return 0;	/* 取消!! */
    }
  }
}

/*-------------------------------------------------------*/
/* window.c     ( Athenaeum BBS    Ver 0.01 )            */
/*-------------------------------------------------------*/
#if 0
show_winline(x, y, 視窗長度/2, 字串, 背景顏色, 光棒顏色);
show_winbox(直,寬,標題,提示字串,顯示模式);
msgbox(直,寬,標題,提示字串,顯示模式);
win_select(標題,提示字串,選項,選項數,預設字元);

EX:  win_select("加密文章", "是否編輯可看見名單? ", 0, 2, 'n')
#endif

typedef struct Win_form
{
  char title[5];	/* 標題顏色 */
  char body[5];		/* 內文顏色 */
  char border[8][3];	/* 外框 */
} Win_form;

static char win_load=1;		/* 是否已讀取? 1:未讀取 0:已讀取 */
static Win_form winform;

/*------------------------------*/
/*  使用者設定檔案存取          */
/*------------------------------*/
static int 
load_winform(Win_form *woutput)
{
  char fpath[PATHLEN];

  sethomefile(fpath, cuser.userid, FN_WINFORM);
  
  if(!strncmp(cuser.userid, STR_GUEST, 5) || 
     rec_get(fpath, woutput, sizeof(Win_form), 1) == -1 || !currutmp)
  {
    woutput->title[0]=4;
    woutput->title[1]=7;
    woutput->title[2]=1;
    woutput->title[3]=0;
    woutput->title[4]=0;
    
    woutput->body[0]=7;
    woutput->body[1]=0;
    woutput->body[2]=0;
    woutput->body[3]=0;
    woutput->body[4]=0;
    
    strcpy(woutput->border[0], "─");
    strcpy(woutput->border[1], "╭");
    strcpy(woutput->border[2], "╮");
    strcpy(woutput->border[3], "├");
    strcpy(woutput->border[4], "┤");    
    strcpy(woutput->border[5], "╰");
    strcpy(woutput->border[6], "╯");
    strcpy(woutput->border[7], "│");
  }
  if(currutmp && (woutput == &winform))
    win_load=0;	/* 設成已讀取 */
    
  return 1;
}

static int 
save_winform(Win_form *winput)
{
  char fpath[PATHLEN];
  int fd;
  
  sethomefile(fpath, cuser.userid, FN_WINFORM);
  
  if((fd = open(fpath, O_WRONLY | O_CREAT, 0644)) == -1)
    return -1;
  else
  {
    write(fd, winput, sizeof(Win_form));
    close(fd);
    win_load=1;	/* 設成未讀取 */
  }
  return 1;
}

/*------------------------------*/
/*  視窗產生                    */
/*------------------------------*/
typedef struct winscreen 
{
  screenline *slt0;
  int x_roll0;
} winscreen;

static screenline *slt = NULL;
static int x_roll = 0;

static void
init_slt(winscreen *wslt, screenline *newslt)
{
  wslt->slt0 = slt;
  wslt->x_roll0 = x_roll;

  if(newslt)
    slt = newslt;
  else
    slt = (screenline *)calloc(t_lines, sizeof(screenline));
    
  x_roll = vs_save(slt);
}

static void
restore_slt(winscreen *wslt, screenline *newslt)
{
  if(!newslt)
    vs_restore(slt);

  slt = wslt->slt0;
  x_roll = wslt->x_roll0;
}

/* 在 (x, y) 的位置塞入 msg，左右仍要印出原來的彩色文字 */
static void 
draw_line(int x, int y, uschar *msg)
{
  uschar *str, *ptr;
  uschar data[ANSILINELEN];
  char color[4];
  char end_flag = 0;		/* 判斷是否原先的已經結束了 */
  int ch, i;
  int len;
  int ansi;			/* 1: 在 ANSI 中 */
  int wstate = 0;		/* 1: 在中文字中 */
  int fg = 37, bg = 40, hl = 0;	/* 前景/背景/高彩 */

  i = x + x_roll;
  if (i > b_lines)
    i -= b_lines + 1;

  memset(data, 0, sizeof(data));
  strncpy(data, slt[i].data, slt[i].len);
  str = data;

  move(x, 0);
  clrtoeol();

  /* 印出 (x, 0) 至 (x, y - 1) */
  ansi = 0;
  len = 0;		/* 已印出幾個字 (不含控制碼) */
  while (ch = *str++)
  {
    if (ch == KEY_ESC)
    {
      ansi = 1;
      i = 0;
    }
    else if (ansi)
    {
      if (ch == '[')
      {
      }
      else if (ch >= '0' && ch <= '9')
      {
	color[i] = ch;
	if (++i >= 4)
	  i = 0;
      }
      else
      {
	color[i] = 0;

	i = atoi(color);
	if (i == 0)
	{
	  hl = 0;
	  fg = 37;
	  bg = 40;
	}
	else if (i == 1)
	  hl = 1;
	else if (i >= 30 && i <= 37)
	  fg = i;
	else if (i >= 40 && i <= 47)
	  bg = i;

	i = 0;

	if (ch != ';')
	  ansi = 0;
      }
    }
    else
    {
      if (++len >= y)
      {
	/* 最後一字若是中文字的首碼，就不印 */
	if (!wstate && ch & 0x80)
	{
	  outc(' ');
	  wstate = 1;
	}
	else
	{
	  outc(ch);
	  wstate = 0;
	}
	outs("\033[m");
	break;
      }

      if (wstate)
	wstate = 0;
      else if (ch & 0x80)
	wstate = 1;
    }

    outc(ch);
  }
  
  if(len < y)
  {
    end_flag = 1;
    while (len++ < y)
      outc(' ');
  }

  /* 印出 (x, y) 至 (x, y + strip_ansi_len(msg) - 1) */
  ptr = msg;
  ansi = 0;
  len = 0;		/* msg 的長度(不含控制碼) */
  while (ch = *ptr++)
  {
    if (ch == KEY_ESC)
      ansi = 1;
    else if (ansi)
    {
      if ((ch < '0' || ch > '9') && ch != ';' && ch != '[')
	ansi = 0;
    }
    else
      len++;

    outc(ch);
  }

  /* 跳掉 str 中間一整段，並取出最後的顏色 */
  ansi = 0;
  while ((ch = *str++) && !end_flag) 
  {
    if (ch == KEY_ESC)
    {
      ansi = 1;
      i = 0;
    }
    else if (ansi)
    {
      if (ch == '[')
	continue;
      if (ch >= '0' && ch <= '9')
      {
	color[i] = ch;
	if (++i >= 4)
	  i = 0;
      }
      else
      {
	color[i] = 0;

	i = atoi(color);
	if (i == 0)
	{
	  hl = 0;
	  fg = 37;
	  bg = 40;
	}
	else if (i == 1)
	  hl = 1;
	else if (i >= 30 && i <= 37)
	  fg = i;
	else if (i >= 40 && i <= 47)
	  bg = i;

	i = 0;

	if (ch != ';')
	  ansi = 0;
      }
    }
    else
    {
      if (--len < 0)	/* 跳過 strip_ansi_len(msg) 的長度 */
	break;

      if (wstate)
	wstate = 0;
      else if (ch & 0x80)
	wstate = 1;
    }
  }
  
  if(!*(str-1))
    end_flag = 1;

  /* 印出 (x, y + strip_ansi_len(msg)) 這個字及後面的控制碼 */
  
  if(!end_flag)
  {
    prints("\033[%d;%d;%dm", hl, fg, bg);
    /* 此字若是中文字的尾碼，就不印 */
    outc(wstate ? ' ' : ch);
  
    /* 印出 (x, y + strip_ansi_len(msg) + 1) 至 行尾 */
    outs(str);
  }
  outs("\033[m");
}

static void 
show_winline(int x, int y, int win_len, char *words, 
             char *bgcolor, char *barcolor, Win_form *form)
{
  char buf[128];

  sprintf(buf, " %s%s %s %-*s\033[m%s",
  	  form->border[7],
          (barcolor != 0 && HAS_HABIT(HABIT_LIGHTBAR)) ? barcolor : bgcolor, 
          (barcolor != 0) ? "→" : "  ", 
          2*(win_len-4), words,
          form->border[7]);
                                                                                
  draw_line(x, y, buf);
  move(b_lines, 0);
}

static int
show_winbox(int x, int y, int line, int width, char *title, char *prompt)
{
  int win_len;  /*win_len 是有幾個二位元字!!*/
  int i,j;
  char buf[256], bgcolor[40], title_color[40];

  /* check */
  if(win_load) load_winform(&winform);
  if(!line || line < 0) line = 1;
  
  if(width%2)
    win_len = (width / 2) + 1;
  else
    win_len = width / 2;

  get_color(bgcolor, winform.body);
  get_color(title_color, winform.title);  

  for(i = 0;i <= line+3;i++)
  {
    move(x + i, 0);
    clrtoeol();
    prints("%80s","");
  }
  /*上部分*/  

  sprintf(buf, " %s" ,winform.border[1]);

  j = win_len-1;

  for(i = 1;i < j;i++)
    strcat(buf, winform.border[0]);
  strcat(buf, winform.border[2]);

  draw_line(x, y, buf);
  
  /*標題*/
  show_winline(x+1, y, win_len, title, title_color, 0, &winform);  

  /*標題下橫槓*/
  sprintf(buf, " %s",winform.border[3]);
  j = win_len -1;                                                                                
  for(i = 1;i < j;i++)
    strcat(buf, winform.border[0]);
  strcat(buf, winform.border[4]);
                                                                                
  draw_line(x+2, y, buf);
  
  /*提示*/
    show_winline(x+3, y, win_len, prompt, bgcolor, 0, &winform);

  /*我的屁股*/
  sprintf(buf," %s", winform.border[5]);
  j = win_len -1;
  for(i = 1;i < j;i++)
    strcat(buf, winform.border[0]);
  strcat(buf, winform.border[6]);
                                                                                
  draw_line(x+3+line, y, buf);

  return win_len;
}

/*------------------------------*/
/*  視窗應用                    */
/*------------------------------*/
void msgbox(screenline *newslt, char *title, char *msg)
{
  int x,y, win_len;
  int len;
  winscreen wslt;
  
  init_slt(&wslt, newslt);
  
  len = strlen(msg) + 12;
  if(len > 80) len = 80;
  if(len < 28) len = 28;  /*title*/

  /*init window*/
  x = (b_lines - 5) / 2;
  y = (80 - len) / 2;

  win_len = show_winbox(x, y, 0, len, title, msg);
  
  move(b_lines, 0); /* by Hubert */

  restore_slt(&wslt, newslt);
}

int
win_select(char *title, char *prompt, char **choose, int many, char def)
{
  int x, y, i;
  int win_len, ch;
  int width;
  char *p, leave=-1;
  char barcolor[50], bgcolor[40];
  winscreen wslt;
  
  init_slt(&wslt, 0);
  
  if(!choose)
    choose = msg_choose;

  /*init window*/
  width = strlen(title);
  i = strlen(prompt);
  if(i > width) width = i;
  for(i = 0;i < many;i++) /*ch暫時當作暫存變數..:pp*/
  {
    ch = strlen(choose[i]);
    if(ch > width) width = ch;
  }
    
  width = width + 12;
  x = (b_lines - many - 6) / 2;
  y = (80 - width) / 2;

  get_lightbar_color(barcolor);
  win_len = show_winbox(x, y, many+1, width, title, prompt);
  get_color(bgcolor, winform.body);

  for(i = 0;i < many;i++)
  {
    p = choose[i];
    if(def == *p) def = i;
    if(p == msg_choose_cancel) leave=i;
    show_winline(x + 4 + i, y, win_len, p+1, bgcolor, 0, &winform);
  }

  i = def;
  
  do
  {
    p = choose[i];
    show_winline(x + 4 + i, y, win_len, p+1, bgcolor, barcolor, &winform);
    ch = igetkey();
    show_winline(x + 4 + i, y, win_len, p+1, bgcolor, 0, &winform);
    
    switch(ch)
    {
      case KEY_UP:
        i--;
        if(i < 0) i = many -1;
        break;

      case KEY_DOWN:
        i++;
        if(i >= many) i = 0;
        break;
        
      case KEY_RIGHT:
        ch = '\r';
        break;
      
      case KEY_LEFT:
        if(leave<0) 
          i=def;
        else
          i=leave;
        break;

      case KEY_PGUP:
      case KEY_HOME:
        i=0;
        break;
        
      case KEY_PGDN:
      case KEY_END:
        i=many-1;
        break;
      
      default:
      {
        int j;

        ch = tolower(ch);
        for(j = 0;j < many;j++)
          if(ch == tolower(*(choose[j])))
          {
            i = j;
            break;
          }
        break;
      }
    }
  }while(ch != '\r');
  
  restore_slt(&wslt, 0);
  return *p;
}  

/*------------------------------*/
/*  使用者設定視窗外貌          */
/*------------------------------*/
int 
win_formchange()
{
  Win_form preview;
  int x=0,y=0, i, j,redraw, ch;
  const int winlen = 20;
  const int top=8;
  char buf[40], lightbar[40], *tmp;
  winscreen wslt;
  char *choose[2][5]={{"標題顏色", "直棒", "上左", "中左", "下左"}, 
      		      {"背景顏色", "橫棒", "上右", "中右", "下右"}};
  		 
  load_winform(&preview);
  get_lightbar_color(lightbar);

  clear();
  init_slt(&wslt, 0);
  
  redraw = 1;
  while(1)
  {
    if(redraw)/* 重畫 window preview */
    {
      clear();
      sprintf(buf, "%s [線上 %d 人]", BOARDNAME, count_ulist());
      showtitle("顏色設定", buf);  
      
      move(b_lines-4, 0);
      prints("\033[m%s\033[m", msg_seperator);
      
      move(b_lines, 0);
      clrtoeol();
      prints("\033[0;44m  樣式選擇  %s%68.68s\033[m", COLOR3, 
             "↑↓←→)選擇想要修改的項目  Q)離開  ");
      redraw = top;
      move(redraw++, 20);
      clrtoeol();
      outs(preview.border[1]);
      for(i=0;i<winlen-2;i++)
        outs(preview.border[0]);
      outs(preview.border[2]);
      
      get_color(buf, preview.title);
      show_winline(redraw++, 19, winlen, "這是標題", buf, 0, &preview);
      
      move(redraw++,20);
      clrtoeol();
      outs(preview.border[3]);
      for(i=0;i<winlen-2;i++)
        outs(preview.border[0]);
      outs(preview.border[4]);      

      get_color(buf, preview.body);
      show_winline(redraw++, 19, winlen, "這是內文", buf, 0, &preview);
      
      move(redraw++,20);
      clrtoeol();
      outs(preview.border[5]);
      for(i=0;i<winlen-2;i++)
        outs(preview.border[0]);
      outs(preview.border[6]);      

      redraw=0;
    }

    for(i=0;i<2;i++)
    {
      move(b_lines - 3 + i, 0);
      clrtoeol();
      for(j=0;j<5;j++)
      {
        if(x==j && y==i)
          prints("[%s%s\033[m]", lightbar, choose[i][j]);
        else
          prints(" %s\033[m ", choose[i][j]);
      }
    }
    
    ch = igetkey();
    switch(ch)
    {
      case KEY_UP:
        y--;
        if(y<0) y=1;
        break;
      case KEY_DOWN:
        y++;
        if(y>1) y=0;
        break;
      case KEY_LEFT:
        x--;
        if(x<0) x=4;
        break;
      case KEY_RIGHT:
        x++;
        if(x>4) x=0;
        break;
      case '\n':
      case '\r':
      {
        switch(x)
        {
          case 0:
            if(!y)
              tmp=preview.title;
            else
              tmp=preview.body;

            for(i=0;i<5;i++)
                buf[i]=tmp[i];
              
            if(color_selector(buf) == 1)
              for(i=0;i<5;i++)
                tmp[i]=buf[i];
            break;
          case 1:
            if(!y)
              tmp=preview.border[7];
            else
              tmp=preview.border[0];
            break;
          case 2:
          case 3:
          case 4:
            i=2*(x-1);          
            if(!y)
              tmp=preview.border[i-1];
            else
              tmp=preview.border[i];
            break;
        }
        
        if(x)
        {
          getdata(b_lines-1, 0, "請輸入新的框:", buf, 3, DOECHO, tmp);
          if(strlen(buf) == 2)
            strcpy(tmp, buf);
        }
        redraw=1;
        break;
      }
      case 'Q':
      case 'q':
        i = getans2(b_lines-1, 0,"請問是否要存檔離開? ", 0, 3, 'y');
        if(i=='q')
        {
          redraw=1;
          break;
        }
        else if(i=='n')
          return -1;
        else
        {
          if (save_winform(&preview) == 1)
            return 1;
          else
          {
            pressanykey("修改失敗 >_<");
            return -1;
          }
        }
    }
  }
  restore_slt(&wslt, 0);
  return 1;
}
