/*-------------------------------------------------------*/
/* loginplan.c       ( WD_hialan BBS    Ver 0.01 )       */
/*-------------------------------------------------------*/
/* author : hialan.nation@infor.org                      */
/* target : �W�� plan                                    */
/* create : 2003/07/11                                   */
/* update : 2003/07/11                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

typedef struct planinfo{
  int num;		/* �s�� */
  int (*fptr)();	/* �{������ */
  int tag;		/* �O�_�w�gŪ�L? 0:�_ 1:�O */
  char desc[48];	/* �ԭz */
} planinfo;

/*----------------------------*/
/*�W���i�H��ܪ�    	      */
/*----------------------------*/
static int view_noteans()
{
  more("note.ans",YEA);
}

static int
habit_from()
{
  if(HAS_PERM(PERM_FROM))
  {
    char fbuf[50];
    sprintf(fbuf, "�G�m [%s]�G", currutmp->from);
    if(getdata(b_lines, 0, fbuf, currutmp->from, 17, DOECHO,0))
      currutmp->from_alias=0;
  }
}

static int
habit_feeling()
{
  getdata(b_lines ,0,"���Ѫ��߱��p��O�H", cuser.feeling, 5 ,DOECHO,cuser.feeling);
  cuser.feeling[4] = '\0';
  strcpy(currutmp->feeling, cuser.feeling);
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
}

int u_habit(), t_users(), Announce(), edit_loginplan();

#define MAXLOGINPLAN 7	 /* �`�@���X�� (�s���[�@) */
static planinfo loginplan[] = {
 0, Announce,		0, "�[�ݨt�ά���",
 1, view_noteans,	0, "�[�ݯd���O",
 2, u_habit, 		0, "�ק�ߦn�]�w",
 3, habit_from,		0, "�ק�G�m",
 4, habit_feeling,	0, "�ק�߱�",
 5, t_users, 		0, "�[�ݨϥΪ̦W��",
 6, edit_loginplan,	0, "�W���]�w"
};

/*----------------------------*/
/*�W���P�_�禡      	      */
/*----------------------------*/
#define PN_UNSORTED 9999	/*�٨S�ƧǪ���*/

static int 
resetloginplan(p1, p2)
  planinfo *p1, *p2;
{
  return (p1->num - p2->num);
}

static int
cmploginplan(p1, p2)
  planinfo *p1, *p2;
{
  if(p1->tag != p2->tag)
    return (p1->tag - p2->tag);
  else
    return (p1->num - p2->num);
}

static void 
loginplan_order()	/*���o����*/
{
  FILE *fp;
  char fpath[PATHLEN];
  char buf[512];
  int i;
  
  qsort(loginplan, MAXLOGINPLAN, sizeof(planinfo), resetloginplan);  /*�٭�*/
  for(i=0;i<MAXLOGINPLAN;i++)
    loginplan[i].tag=PN_UNSORTED;
  
  sethomefile(fpath, cuser.userid, FN_LOGINPLAN);
  if((fp=fopen(fpath, "r"))!=NULL)
  {
    int n=1;
    while(fgets(buf, 511, fp) != NULL)
    {
      if(*buf == '#') continue;
      i=atoi(buf);
      if(i<1 || i > MAXLOGINPLAN)
        continue;
      
      loginplan[i-1].tag=n++;
    }
    fclose(fp);
  }
  qsort(loginplan, MAXLOGINPLAN, sizeof(planinfo), cmploginplan);
}

static int 
save_loginplan()
{
  FILE *fp;
  int i;
  char fpath[PATHLEN];
  
  sethomefile(fpath, cuser.userid, FN_LOGINPLAN);
  
  fp=fopen(fpath, "w");
  fprintf(fp, "#�Цۦ����m�H�F��ק諸�ت�\n#�s��  #����\n");
  
  for(i=0;i<MAXLOGINPLAN;i++)
  {
    if(loginplan[i].tag!=PN_UNSORTED)
      fprintf(fp, "%-7d#%s\n", loginplan[i].num+1, loginplan[i].desc);
    else
      break;
  }
  fclose(fp);
  
  return 0;
}

int 
login_plan()
{
  FILE *fp;
  char fpath[PATHLEN];
  char buf[512];
  int i;
  char clean = 0; /*�O�_�ݭn���s�h�����~*/

  sethomefile(fpath, cuser.userid, FN_LOGINPLAN);  
  
  if((fp = fopen(fpath, "r")) == NULL)
  {
    for(i=0;i<1;i++)	/*�w�]���\�e��*/
      (*(loginplan[i].fptr))();

    return -1;
  }
  else
  {
    while(fgets(buf, 511, fp) != NULL)
    {
      if(*buf == '#') continue;
      i=atoi(buf);
      if(i<1 || i > MAXLOGINPLAN)
      {
        clean = 1;
        continue;
      }
      
      i--;
      if(!loginplan[i].tag)
      {
        (*(loginplan[i].fptr))();
        loginplan[i].tag=1;
      }
      else /*���u����@��, �ݭn�M��!!*/
        clean = 1;
    }
    fclose(fp);
    
    if(clean)  /*���s�g�J!!*/
    {
      loginplan_order();
      save_loginplan();
    }
  }
  return 0;
}

static void 
edit_planitem(int pos, char *bar)
{
  char buf[10];
  
  sprintf(buf, "%d", loginplan[pos].tag);
  move(pos+3, 2);
  clrtoeol();

  prints("%4d.%s%-48.48s\033[m %-8s", 
  	  loginplan[pos].num+1, 
  	  (bar) ? bar : "",
    	  loginplan[pos].desc, 
    	  (loginplan[pos].tag==PN_UNSORTED) ? "���Ƨ�" : buf);

}

int 
edit_loginplan()
{
  int pos, ch, i;
  char sort, buf[256], bar[40];
  
  clear();  
  loginplan_order();	/*���n����*/
  
  get_lightbar_color(bar);

  sprintf(buf, "%s [�u�W %d �H]", BOARDNAME, count_ulist());
  showtitle("�W���]�w", buf);    
  
  move(1, 0);
  clrtoeol();
  prints("%79.79s", "d)�����Ӷ��� e)��ʽs���� ");
  move(2, 0);
  clrtoeol();
  prints("%s%-79.79s\033[m", COLOR3,
  	 "  �s��        ��     �z                                 �Ƨ�");
  
  move(b_lines, 0);
  clrtoeol();
  prints("\033[0;44m  �W���]�w  %s%68.68s\033[m", COLOR3,
  	 "����)��ܷQ�n���ܶ��Ǫ�����  Enter|��)���ܶ���  ��|Q)���}  ");
  pos=0;  
  sort=1;
  while(ch != 'q')
  {
    if(sort)	/*���e�ù�*/
    {
      qsort(loginplan, MAXLOGINPLAN, sizeof(planinfo), cmploginplan);    
      for(i=0;i<MAXLOGINPLAN;i++)
	edit_planitem(i, 0);
      sort=0;
    }  
    edit_planitem(pos, bar);
    ch = cursor_key(pos+3, 0);
    edit_planitem(pos, 0);
    switch(ch)
    {
      case KEY_UP:
        pos--;
        if(pos<0) pos = MAXLOGINPLAN -1;
        break;
      case KEY_DOWN:
        pos++;
        if(pos>=MAXLOGINPLAN) pos = 0;
        break;
      case 'e':
        sethomefile(buf, cuser.userid, FN_LOGINPLAN);
        return vedit(buf, YEA);
      case 'd':
        loginplan[pos].tag = PN_UNSORTED;
        sort =1;
        break;
      case KEY_RIGHT:
      case '\r':
      case '\n':
        getdata(b_lines-1, 0, "�аݷs���ƧǡG", buf, 3, DOECHO, 0);
        i=atoi(buf);
        if(i>0 && i<MAXLOGINPLAN)
        {
          loginplan[pos].tag=i;
          sort = 1;
        }
        break;
      case KEY_LEFT:
      case 'q':
      case 'Q':
        save_loginplan();	/*�g�J�ɮ�*/
        return 0;
    }
    move(b_lines-1, 0);
    clrtoeol();
    prints("%80s", "");    
  }
  return 0;
}
