/*-------------------------------------------------------*/
/* loginplan.c       ( WD_hialan BBS    Ver 0.01 )       */
/*-------------------------------------------------------*/
/* author : hialan.nation@infor.org                      */
/* target : 上站 plan                                    */
/* create : 2003/07/11                                   */
/* update : 2003/07/11                                   */
/*-------------------------------------------------------*/

#include "bbs.h"

typedef struct planinfo{
  int num;		/* 編號 */
  int (*fptr)();	/* 程式指標 */
  int tag;		/* 是否已經讀過? 0:否 1:是 */
  char desc[48];	/* 敘述 */
} planinfo;

/*----------------------------*/
/*上站可以選擇的    	      */
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
    sprintf(fbuf, "故鄉 [%s]：", currutmp->from);
    if(getdata(b_lines, 0, fbuf, currutmp->from, 17, DOECHO,0))
      currutmp->from_alias=0;
  }
}

static int
habit_feeling()
{
  getdata(b_lines ,0,"今天的心情如何呢？", cuser.feeling, 5 ,DOECHO,cuser.feeling);
  cuser.feeling[4] = '\0';
  strcpy(currutmp->feeling, cuser.feeling);
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
}

int u_habit(), t_users(), Announce(), edit_loginplan();

#define MAXLOGINPLAN 7	 /* 總共有幾項 (編號加一) */
static planinfo loginplan[] = {
 0, Announce,		0, "觀看系統紀錄",
 1, view_noteans,	0, "觀看留言板",
 2, u_habit, 		0, "修改喜好設定",
 3, habit_from,		0, "修改故鄉",
 4, habit_feeling,	0, "修改心情",
 5, t_users, 		0, "觀看使用者名單",
 6, edit_loginplan,	0, "上站設定"
};

/*----------------------------*/
/*上站判斷函式      	      */
/*----------------------------*/
#define PN_UNSORTED 9999	/*還沒排序的值*/

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
loginplan_order()	/*取得順序*/
{
  FILE *fp;
  char fpath[PATHLEN];
  char buf[512];
  int i;
  
  qsort(loginplan, MAXLOGINPLAN, sizeof(planinfo), resetloginplan);  /*還原*/
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
  fprintf(fp, "#請自行更改位置以達到修改的目的\n#編號  #說明\n");
  
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
  char clean = 0; /*是否需要重新去除錯誤*/

  sethomefile(fpath, cuser.userid, FN_LOGINPLAN);  
  
  if((fp = fopen(fpath, "r")) == NULL)
  {
    for(i=0;i<1;i++)	/*預設的擺前面*/
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
      else /*不只執行一次, 需要清空!!*/
        clean = 1;
    }
    fclose(fp);
    
    if(clean)  /*重新寫入!!*/
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
    	  (loginplan[pos].tag==PN_UNSORTED) ? "未排序" : buf);

}

int 
edit_loginplan()
{
  int pos, ch, i;
  char sort, buf[256], bar[40];
  
  clear();  
  loginplan_order();	/*做好順序*/
  
  get_lightbar_color(bar);

  sprintf(buf, "%s [線上 %d 人]", BOARDNAME, count_ulist());
  showtitle("上站設定", buf);    
  
  move(1, 0);
  clrtoeol();
  prints("%79.79s", "d)取消該項目 e)手動編輯文件 ");
  move(2, 0);
  clrtoeol();
  prints("%s%-79.79s\033[m", COLOR3,
  	 "  編號        敘     述                                 排序");
  
  move(b_lines, 0);
  clrtoeol();
  prints("\033[0;44m  上站設定  %s%68.68s\033[m", COLOR3,
  	 "↑↓)選擇想要改變順序的項目  Enter|→)改變順序  ←|Q)離開  ");
  pos=0;  
  sort=1;
  while(ch != 'q')
  {
    if(sort)	/*重畫螢幕*/
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
        getdata(b_lines-1, 0, "請問新的排序：", buf, 3, DOECHO, 0);
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
        save_loginplan();	/*寫入檔案*/
        return 0;
    }
    move(b_lines-1, 0);
    clrtoeol();
    prints("%80s", "");    
  }
  return 0;
}
