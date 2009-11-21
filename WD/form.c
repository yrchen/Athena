/*-------------------------------------------------------*/
/* form.c       ( WD_hialan BBS    Ver 0.01 )            */
/*-------------------------------------------------------*/
/* author : hialan.nation@infor.org                      */
/* target : ��m���/���ά������/����                   */
/* create : 2003/10/22                                   */
/*-------------------------------------------------------*/

/*-------------------------------------------------------*/
/* color.c      ( WD_hialan BBS    Ver 0.01 )            */
/*-------------------------------------------------------*/
#include "bbs.h"

/*Add For LightBAR by hialan*/

/*-------------------*/
/*���� �ˬd-��� ���*/
/*-------------------*/
#define LB_BG 0	/* �I�� */
#define LB_WD 1 /* ��r */
#define LB_LT 2 /* light �G�� */
#define LB_BL 3 /* blink �{�{ */
#define LB_UL 4 /* underline ���u */

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
  
  /*��r�C��*/
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
      pressanykey("�ק粒��!!");
    else
      pressanykey("�ק異��>_<");
  }
  else
    pressanykey("����!!");
  
  return 0;
}  

/*---------------------------*/
/* �C���ܾ�		     */
/*---------------------------*/
static int
selector_item(char *prompt, int line, int pos, char *lightbar, int def)
{
  int bottom, num, i;
  
  char **choose;
  char *color[8]={"0)�¦�","1)����","2)���","3)����","4)�Ŧ�","5)����","6)�C��","7)�զ�"};
  char *underline[2]={"0)�L���u","1)���u"};
  char *blink[2]={"0)���{�{","1)�{�{"};
  char *light[3]={"0)�C�G��","1)���G��","2)���]�w"};
  
  switch(line)
  {
    case 0:  /* �I�� */
      bottom=b_lines - 5;
      num=8;
      choose=color;
      break;
    case 1:  /* ��r */
      bottom=b_lines - 4;
      num=8;
      choose=color;    
      break;
    case 2:  /* �G�� */
      bottom=b_lines - 3;
      num=3;
      choose=light;
      break;
    case 3:  /* �{�{ */
      bottom=b_lines - 2;
      num=2;
      choose=blink;
      break;
    case 4:  /* ���u */
      bottom=b_lines - 1;
      num=2;
      choose=underline;
      break;
  }
  
  move(bottom, 0);
  clrtoeol();
  outs(prompt);

  if (pos < 0)		/* hialan.030704:�o�䰽�i, ��j���ܼg�b�o��XD */
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


/* hialan.030704:�C�@�檺�ȥ� color �O��, �ĴX�C�� pos �O�� */
int
color_selector(char *color)
{
  int i;
  int pos=0, ch=0;
  char *ques[5]={"  ���� ","  �r�� ","  �G�� ","  �{�{ ","  ���u "}; 
  int def[5]={7, 4, 0, 0, 1};
  char lightbar[40];
  char preview[64];

  get_color(lightbar, cuser.lightbar);

  clear();
  
  sprintf(preview, "%s [�u�W %d �H]", BOARDNAME, count_ulist());
  showtitle("�C��]�w", preview);  

  move(b_lines-6,0);
  outs(msg_seperator);
  
  for(i=0;i<5;i++)
    color[i] = selector_item(ques[i], i, color[i], lightbar, def[i]);

  move(b_lines, 0);
  prints("\033[0;44m  �� �� �L  %s%68.68s\033[m",  COLOR3,
    	 "��������)���ܭק�ﶵ  Enter)�T�w�ק�  Q)����  ");
  
  while(ch !='\n' && ch !='q')
  {
    color[pos]= selector_item(ques[pos], pos, color[pos], lightbar, def[pos]);
    get_color(preview, color);
    move(b_lines/2, 34);
    clrtoeol();
    prints("%s  �o�̬O�w����!!  \033[m", preview);
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
        return 1;	/* �T�w!! */
      case 'q':
        return 0;	/* ����!! */
    }
  }
}

/*-------------------------------------------------------*/
/* window.c     ( Athenaeum BBS    Ver 0.01 )            */
/*-------------------------------------------------------*/
#if 0
show_winline(x, y, ��������/2, �r��, �I���C��, �����C��);
show_winbox(��,�e,���D,���ܦr��,��ܼҦ�);
msgbox(��,�e,���D,���ܦr��,��ܼҦ�);
win_select(���D,���ܦr��,�ﶵ,�ﶵ��,�w�]�r��);

EX:  win_select("�[�K�峹", "�O�_�s��i�ݨ��W��? ", 0, 2, 'n')
#endif

typedef struct Win_form
{
  char title[5];	/* ���D�C�� */
  char body[5];		/* �����C�� */
  char border[8][3];	/* �~�� */
} Win_form;

static char win_load=1;		/* �O�_�wŪ��? 1:��Ū�� 0:�wŪ�� */
static Win_form winform;

/*------------------------------*/
/*  �ϥΪ̳]�w�ɮצs��          */
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
    
    strcpy(woutput->border[0], "�w");
    strcpy(woutput->border[1], "�~");
    strcpy(woutput->border[2], "��");
    strcpy(woutput->border[3], "�u");
    strcpy(woutput->border[4], "�t");    
    strcpy(woutput->border[5], "��");
    strcpy(woutput->border[6], "��");
    strcpy(woutput->border[7], "�x");
  }
  if(currutmp && (woutput == &winform))
    win_load=0;	/* �]���wŪ�� */
    
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
    win_load=1;	/* �]����Ū�� */
  }
  return 1;
}

/*------------------------------*/
/*  ��������                    */
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

/* �b (x, y) ����m��J msg�A���k���n�L�X��Ӫ��m���r */
static void 
draw_line(int x, int y, uschar *msg)
{
  uschar *str, *ptr;
  uschar data[ANSILINELEN];
  char color[4];
  char end_flag = 0;		/* �P�_�O�_������w�g�����F */
  int ch, i;
  int len;
  int ansi;			/* 1: �b ANSI �� */
  int wstate = 0;		/* 1: �b����r�� */
  int fg = 37, bg = 40, hl = 0;	/* �e��/�I��/���m */

  i = x + x_roll;
  if (i > b_lines)
    i -= b_lines + 1;

  memset(data, 0, sizeof(data));
  strncpy(data, slt[i].data, slt[i].len);
  str = data;

  move(x, 0);
  clrtoeol();

  /* �L�X (x, 0) �� (x, y - 1) */
  ansi = 0;
  len = 0;		/* �w�L�X�X�Ӧr (���t����X) */
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
	/* �̫�@�r�Y�O����r�����X�A�N���L */
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

  /* �L�X (x, y) �� (x, y + strip_ansi_len(msg) - 1) */
  ptr = msg;
  ansi = 0;
  len = 0;		/* msg ������(���t����X) */
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

  /* ���� str �����@��q�A�è��X�̫᪺�C�� */
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
      if (--len < 0)	/* ���L strip_ansi_len(msg) ������ */
	break;

      if (wstate)
	wstate = 0;
      else if (ch & 0x80)
	wstate = 1;
    }
  }
  
  if(!*(str-1))
    end_flag = 1;

  /* �L�X (x, y + strip_ansi_len(msg)) �o�Ӧr�Ϋ᭱������X */
  
  if(!end_flag)
  {
    prints("\033[%d;%d;%dm", hl, fg, bg);
    /* ���r�Y�O����r�����X�A�N���L */
    outc(wstate ? ' ' : ch);
  
    /* �L�X (x, y + strip_ansi_len(msg) + 1) �� ��� */
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
          (barcolor != 0) ? "��" : "  ", 
          2*(win_len-4), words,
          form->border[7]);
                                                                                
  draw_line(x, y, buf);
  move(b_lines, 0);
}

static int
show_winbox(int x, int y, int line, int width, char *title, char *prompt)
{
  int win_len;  /*win_len �O���X�ӤG�줸�r!!*/
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
  /*�W����*/  

  sprintf(buf, " %s" ,winform.border[1]);

  j = win_len-1;

  for(i = 1;i < j;i++)
    strcat(buf, winform.border[0]);
  strcat(buf, winform.border[2]);

  draw_line(x, y, buf);
  
  /*���D*/
  show_winline(x+1, y, win_len, title, title_color, 0, &winform);  

  /*���D�U��b*/
  sprintf(buf, " %s",winform.border[3]);
  j = win_len -1;                                                                                
  for(i = 1;i < j;i++)
    strcat(buf, winform.border[0]);
  strcat(buf, winform.border[4]);
                                                                                
  draw_line(x+2, y, buf);
  
  /*����*/
    show_winline(x+3, y, win_len, prompt, bgcolor, 0, &winform);

  /*�ڪ�����*/
  sprintf(buf," %s", winform.border[5]);
  j = win_len -1;
  for(i = 1;i < j;i++)
    strcat(buf, winform.border[0]);
  strcat(buf, winform.border[6]);
                                                                                
  draw_line(x+3+line, y, buf);

  return win_len;
}

/*------------------------------*/
/*  ��������                    */
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
  for(i = 0;i < many;i++) /*ch�Ȯɷ�@�Ȧs�ܼ�..:pp*/
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
/*  �ϥΪ̳]�w�����~��          */
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
  char *choose[2][5]={{"���D�C��", "����", "�W��", "����", "�U��"}, 
      		      {"�I���C��", "���", "�W�k", "���k", "�U�k"}};
  		 
  load_winform(&preview);
  get_lightbar_color(lightbar);

  clear();
  init_slt(&wslt, 0);
  
  redraw = 1;
  while(1)
  {
    if(redraw)/* ���e window preview */
    {
      clear();
      sprintf(buf, "%s [�u�W %d �H]", BOARDNAME, count_ulist());
      showtitle("�C��]�w", buf);  
      
      move(b_lines-4, 0);
      prints("\033[m%s\033[m", msg_seperator);
      
      move(b_lines, 0);
      clrtoeol();
      prints("\033[0;44m  �˦����  %s%68.68s\033[m", COLOR3, 
             "��������)��ܷQ�n�ק諸����  Q)���}  ");
      redraw = top;
      move(redraw++, 20);
      clrtoeol();
      outs(preview.border[1]);
      for(i=0;i<winlen-2;i++)
        outs(preview.border[0]);
      outs(preview.border[2]);
      
      get_color(buf, preview.title);
      show_winline(redraw++, 19, winlen, "�o�O���D", buf, 0, &preview);
      
      move(redraw++,20);
      clrtoeol();
      outs(preview.border[3]);
      for(i=0;i<winlen-2;i++)
        outs(preview.border[0]);
      outs(preview.border[4]);      

      get_color(buf, preview.body);
      show_winline(redraw++, 19, winlen, "�o�O����", buf, 0, &preview);
      
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
          getdata(b_lines-1, 0, "�п�J�s����:", buf, 3, DOECHO, tmp);
          if(strlen(buf) == 2)
            strcpy(tmp, buf);
        }
        redraw=1;
        break;
      }
      case 'Q':
      case 'q':
        i = getans2(b_lines-1, 0,"�аݬO�_�n�s�����}? ", 0, 3, 'y');
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
            pressanykey("�ק異�� >_<");
            return -1;
          }
        }
    }
  }
  restore_slt(&wslt, 0);
  return 1;
}
