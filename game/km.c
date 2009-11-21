/*-------------------------------------------------------*/
/* km.c         ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : KongMing Chess routines                      */
/* create : 01/02/08                                     */
/* update : 02/12/08 For WD -- sheng                      */
/* author : einstein@bbs.tnfsh.tn.edu.tw                 */
/*          itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/

#include "bbs.h"

enum{
	KM_XPOS = 5,
	KM_YPOS = 5,
	MAX_X = 7,            /* �_�ơA�ק� MAX_X MAX_Y �n�۹�ק� etc/km */
	MAX_Y = 7,            /* �_�� */

	TILE_NOUSE = 0,       /* ���ಾ�ʪ���l */
	TILE_BLANK = 1,       /* �Ů� */
	TILE_CHESS = 2        /* �Ѥl */
};

static int board[MAX_X][MAX_Y];
static int cx, cy;		/* �ثe���Ҧb */
static char piece[4][3] = {"�@", "��", "��", "��"};
static char title[20];		/* ���ЦW�� */

/* ���ѨB�Ƥ��i��W�L�ѽL�j�p */
static int route[MAX_X * MAX_Y][4]; /* �O�� (fx, fy) -> (tx, ty) */
static int step;

static void
out_song()
{
	/* �Цۤv�g���w���y�l�A�Ϊ̥κq���]�i�H :p */
	uschar *msg[8] = {
	"�A�ӱj�F�A�N�O�o�ˡI",
	"�A���i��Q��o�@�B�I",
	"�����I�����I �R���I�R���I",
	"�ڤ����D�ӻ��Ǥ���F�I",
	"�o�@�ۯu�O�ѤH�ⵧ�r�I",
	"�ӨتA�A�F�A�o�ˤ]��I",
	"���l�p���I ²���O���I",
	"�n�����Эn�i�D������I"
	};
  move(21, 0);
  clrtoeol();
  prints("\033[1;3%dm%s\033[m", time(0) % 7, msg[time(0) % 8]);
}

static void
show_board()
{
  int i, j;

  setutmpmode(KM);

  move(2, KM_YPOS + MAX_Y - 6);     /* �m����ܴ��ЦW�� */
  outs(title);

  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      move(KM_XPOS + i, KM_YPOS + j * 2);
      outs(piece[board[i][j]]);
    }
  }
  move(3, 40);
  outs("�Y��Ц����ʿ��~���{�H");
  move(4,40);
  outs("�ȮɱN������V����������Y�i");
  move(6, 40);
  outs("��������  ��V��");
  move(8, 40);
  outs("[Space]   ���/�Ͽ��");
  move(10, 40);
  outs("Q/q       ���}");
  move(12, 40);
  outs("h         Ū�����нd��");

  move(14, 40);
  outs("r         ����");


  move(16, 40);
  outs("��        �Ŧ�");
  move(17, 40);
  outs("��        �Ѥl");
  move(18, 40);
  outs("��        ���");

  out_song();
  move(KM_XPOS + MAX_X / 2, KM_YPOS + MAX_Y / 2 * 2);
}

static inline int
read_board()
{
  int i, j, count;
  int NUM_TABLE;
  FILE *fp;
  char buf[40], ans[4];

  if (!(fp = fopen("game/km", "r")))
    return 0;

  fgets(buf, 4, fp);

  NUM_TABLE = atoi(buf);    /* game/km �Ĥ@��O�����м� */

  clear();
  more("game/km.welcome", NULL);
  sprintf(buf, "�п�ܽs�� [1-%d]�A[0] �H���X�D�A�Ϋ� [Q] ���}�G", NUM_TABLE);
  getdata(b_lines, 0, buf, ans,3,1,0);
  clear();
  if (ans[0] == 'q' || ans[0] == 'Q')
  {
    fclose(fp);
    return 0;
  }

  i = atoi(ans) - 1;
  if (i < 0 || i >= NUM_TABLE)
    i = time(0) % NUM_TABLE;


  fseek(fp, 4 + i * (2 * MAX_X * MAX_Y + 14), SEEK_SET);
  /* 4: �Ĥ@�檺�T��ƴ��мƥ�\n  14: \n#999���ЦW��\n */

  fscanf(fp, "%s", &title);     /* ���ЦW�� */

  count = 0;
  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)

    {
      fscanf(fp, "%d", &board[i][j]);
      if (board[i][j] & TILE_CHESS)
      {
    count++;
      }
    }
  }
  fclose(fp);
  return count;
}

static inline int
valid_pos( x, y)
  int x, y;
{
  if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y ||
    board[x][y] == TILE_NOUSE)  /* TILE_NOUSE = 0 ����� & operation */
  {
    return 0;
  }
  return 1;
}

static void
get_pos( x, y)
  int *x, *y;
{
  int ch;
  while (1){
    ch = igetkey();
    if (ch == KEY_UP && valid_pos(cx - 1, cy))
    {
      move(KM_XPOS + (cx - 1), KM_YPOS + cy * 2);
      cx -= 1;
    }
    else if (ch == KEY_DOWN && valid_pos(cx + 1, cy))
    {
      move(KM_XPOS + (cx + 1), KM_YPOS + cy * 2);
      cx += 1;
    }
    else if (ch == KEY_LEFT && valid_pos(cx, cy - 1))
    {
      move(KM_XPOS + cx, KM_YPOS + (cy - 1) * 2);
      cy -= 1;
    }
    else if (ch == KEY_RIGHT && valid_pos(cx, cy + 1))
    {
      move(KM_XPOS + cx, KM_YPOS + (cy + 1) * 2);
      cy += 1;
    }

    else if (ch == 32)
    {
      *x = cx;
      *y = cy;
      break;
    }

    else if (ch == 'h')
    {
	clear();
      more("game/km.hlp", YEA);
	clear();
      show_board();
      move(KM_XPOS + cx, KM_YPOS + cy * 2);
    }
    else if (ch == 'q' || ch == 'Q')
    {
	pressanykey("�w��z�U���A��!!");
      *x = -1;
      break;
    }
    else if (ch == 'r')
    {
      *x = -2;
      break;
    }
    else if (ch == 13)
    {
      *x = cx;
      *y = cy;
      break;
    }
  }
}

static inline void
jump(fx, fy, tx, ty)
  int fx, fy, tx, ty;       /* From (fx, fy) To (tx, ty) */
{
  out_song();

  board[fx][fy] = TILE_BLANK;
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  outs(piece[1]);

  board[(fx + tx) / 2][(fy + ty) / 2] = TILE_BLANK;
  move(KM_XPOS + (fx + tx) / 2, KM_YPOS + (fy + ty));
  outs(piece[1]);

  board[tx][ty] = TILE_CHESS;
  move(KM_XPOS + tx, KM_YPOS + ty * 2);
  outs(piece[2]);
  move(KM_XPOS + tx, KM_YPOS + ty * 2);

  route[step][0] = fx;
  route[step][1] = fy;
  route[step][2] = tx;
  route[step][3] = ty;
  step++;
}

static inline void
retract()
{
  int fx, fy, tx, ty;

  out_song();

  step--;
  ty = route[step][3];
  tx = route[step][2];
  fy = route[step][1];
  fx = route[step][0];

  board[tx][ty] = TILE_BLANK;
  move(KM_XPOS + tx, KM_YPOS + ty * 2);
  outs(piece[1]);
  board[(fx + tx) / 2][(fy + ty) / 2] = TILE_CHESS;
  move(KM_XPOS + (fx + tx) / 2, KM_YPOS + (fy + ty));
  outs(piece[2]);

  board[fx][fy] = TILE_CHESS;
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  outs(piece[2]);
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  cx = fx;
  cy = fy;
}

static inline int
check(fx, fy, tx, ty)
  int fx, fy, tx, ty;
{
  if ((board[(fx + tx) / 2][(fy + ty) / 2] & TILE_CHESS) &&
    ((abs(fx - tx) == 2 && fy == ty) || (fx == tx && abs(fy - ty) == 2)))
  {
    return 1;
  }
  return 0;
}

static inline int
live()
{
  int dir[4][2] = {1, 0, -1, 0, 0, 1, 0, -1};
  int i, j, k, nx, ny, nx2, ny2;
  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      for (k = 0; k < 4; k++)
      {
    nx = i + dir[k][0];
    ny = j + dir[k][1];
    nx2 = nx + dir[k][0];
    ny2 = ny + dir[k][1];
    if (valid_pos(nx2, ny2) && (board[i][j] & TILE_CHESS) &&
      (board[nx][ny] & TILE_CHESS) && (board[nx2][ny2] & TILE_BLANK))
    {
      return 1;
    }
      }
    }
  }
  return 0;
}

static void
log_km(fp)
  FILE *fp;
{
  int i, j;

  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      fprintf(fp, "%s", piece[board[i][j]]);
    }
    fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
}

int
main_km(){
  int fx, fy, tx, ty, count;
  char fpath[80],buf[40];
  FILE *fp;
  time_t now;

  if (!(count = read_board()))
	return 0;

  time(&now);

  sprintf(fpath ,BBSHOME"/home/%s/km.log",cuser.userid);

  fp = fopen(fpath, "w");

  fprintf(fp, "%s %s (%s)\n", str_author1, cuser.userid,cuser.username);
  fprintf(fp, "���D: �թ����� %s �}�ѹL�{\n�ɶ�: %s\n", title,ctime(&now));
  fprintf(fp, "%s\n\n", title);

  log_km(fp);
  step = 0;

  show_board();

  cx = MAX_X / 2;
  cy = MAX_Y / 2;

  while (1)
  {
    if (count == 1 && board[MAX_X / 2][MAX_Y / 2] & TILE_CHESS)
    {           /* �̫�@�l�n�b������ */
      pressanykey("���߱z���\\�F");
      fprintf(fp, "\n--\n\033[1;32m�� Origin: \033[33m%s \033[37m%s\033[31m"
        "�� From: \033[36m%s\033[m\n",BBSNAME, MYHOSTNAME, fromhost);

      fclose(fp);
	outs("�z�O�_�n�⧹�������ЫO�s�b�H�c��(Y/N)�H[Y] ");
      if (igetch() != 'n')
      {
       sprintf(buf, "�թ����� %s �}�ѹL�{", title);
       mail2user(cuser, buf, fpath);
      }
      unlink(fpath);
      break;
    }

    if (!live())
    {
      pressanykey("�V�|...�S�ѤF...@@");
      break;
    }

    while (1)       /* �Ĥ@�� */
    {
      get_pos(&fx, &fy);
      if (fx < 0)
      {
	if (fx == -2)
	{
		if (step) /* �@�B���٨S���A���஬�� */
		{
	        retract();
	        count++;
		fprintf(fp, "���ѡA�^��W�@�B\n");
		log_km(fp);
	        }
	 continue;
       }
  goto abort_game;
      }
      if (!(board[fx][fy] & TILE_CHESS))
      {
	continue;
      }
      else      /* ��l */
      {
    outs(piece[3]);
    move(KM_XPOS + fx, KM_YPOS + fy * 2);
    break;
      }
    }

    while (1)       /* �ĤG�� */
    {
      get_pos(&tx, &ty);
      if (tx < 0)
      {
    if (tx == -2)
    {
      continue; /* �n������l�~�஬�� */
    }
    goto abort_game;
      }
      if (fx == tx && fy == ty) /* ����l */
      {
    outs(piece[2]);
    move(KM_XPOS + tx, KM_YPOS + ty * 2);
    break;
      }
      else if (!(board[tx][ty] & TILE_BLANK) || !check(fx, fy, tx, ty))
      {     /* ������a�褣��� */
    continue;
      }
      else      /* ����Ӧa�� */
      {
    jump(fx, fy, tx, ty);
    count--;
    log_km(fp);
    break;
      }
    }
 }
abort_game:
fclose(fp);
return;
}
