/*-------------------------------------------------------*/
/* money.c       ( WD-BBS Version 1.54 )                 */
/*-------------------------------------------------------*/
/* target : money control function                       */
/* create : 98/08/16                                     */
/* update : 99/03/05                                     */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef HAVE_GAME
void
waste_money()
{
  while(cuser.silvermoney >= MAXMONEY(cuser)
    && cuser.numlogins > 2)
  {
    clear();
    move(10,0);
    prints("�A���ȹ��W���� %ld�I\n\n\n\n",MAXMONEY(cuser));
    outs("�зQ��k�ᱼ�@�� , �άO����ন�����a!\n�b�ӷ~���ߪ��Ȧ椤���������ﶵ .");    
    pressanykey("�A���Ӧh�o�I�Q��k�ᱼ�a�I");
    finance();
  }
}
#endif


int
inumoney(char *tuser,int money)
{
  int unum;
  if (unum = getuser(tuser))
    {
      xuser.silvermoney += money; 
      substitute_record(fn_passwd, &xuser,sizeof(userec), unum); 
      return xuser.silvermoney;
    }
  else
      return -1;
}

int
inugold(char *tuser,int money)
{
  int unum;
  if (unum = getuser(tuser))
    {
      xuser.goldmoney += money; 
      substitute_record(fn_passwd, &xuser,sizeof(userec), unum); 
      return xuser.goldmoney;
    }
  else
      return -1;
}

int
inmoney(int money)
{
      getuser(cuser.userid);
      cuser.silvermoney = xuser.silvermoney + money;
      substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
      return cuser.silvermoney;
}

int
ingold(unsigned long int money)
{
      getuser(cuser.userid);
      cuser.goldmoney = xuser.goldmoney + money;
      substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
      return cuser.goldmoney;
}

int
inmailbox(int m)
{
      getuser(cuser.userid);
      cuser.exmailbox = xuser.exmailbox + m;
      substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
      return cuser.exmailbox;
}

int
demailbox(int m)
{
      getuser(cuser.userid);
      cuser.exmailbox = xuser.exmailbox -m;
      substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
      return cuser.exmailbox;
}


int
deumoney(char *tuser, int money)
{
  int unum;
  if (unum = getuser(tuser))
    {
      if((unsigned long int)xuser.silvermoney <=
         (unsigned long int) money) xuser.silvermoney=0;
      else xuser.silvermoney -= money;
      substitute_record(fn_passwd, &xuser, sizeof(userec), unum);
      return xuser.silvermoney;
    }
  else
      return -1;
}

int
demoney(unsigned long int money)
{
  getuser(cuser.userid);
  if(xuser.silvermoney <= money) 
  {
    if(cuser.goldmoney > (money-xuser.silvermoney)/10000)
      cuser.goldmoney -= (money-xuser.silvermoney)/10000;
    else
      cuser.goldmoney=0;
    cuser.silvermoney=0;
    pressanykey("�ȹ����� , ��������!");
  }
  else
    cuser.silvermoney = xuser.silvermoney - money;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  return cuser.silvermoney;
}

int
degold(unsigned long int money)
{
  getuser(cuser.userid);
  if(xuser.goldmoney <= money) 
  {
    if(cuser.silvermoney > (money-cuser.goldmoney)*10000)
      cuser.silvermoney-=(money-cuser.goldmoney)*10000;
    else
      cuser.silvermoney=0;
    cuser.goldmoney=0;
    pressanykey("�������� , �����ȹ�!");
  }
  else
    cuser.goldmoney = xuser.goldmoney - money;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  return cuser.goldmoney;
}

/* ���� Multi play */
int
count_multiplay(int unmode)
{
  register int i, j;
  register user_info *uentp;
  extern struct UTMPFILE *utmpshm;

  resolve_utmp();
  for (i = j = 0; i < USHM_SIZE; i++)
  {
    uentp = &(utmpshm->uinfo[i]);
    if (uentp->uid == usernum)
     if(uentp->lockmode == unmode)
      j++;
  }
  return j;
}

/* wildcat : ��سf�� */
int
check_money(unsigned long int money,int mode)
{
  unsigned long int usermoney;

  usermoney = mode ? cuser.goldmoney : cuser.silvermoney;
  if(usermoney<money)
  {
    move(1,0);
    clrtobot();
    move(10,10);
    pressanykey("�z���W�u�� %d ���A������I",usermoney);
    return 1;
  }
  return 0;
}
