/*-------------------------------------------------------*/
/* allgoldexpire.c                (Athena BBS Ver 0.0.9) */
/*-------------------------------------------------------*/
/* 功   能：自動扣稅                                     */
/* 作   者：[AT]-23-robert                               */
/* 修   改：[AT]-1-LinuxBoy                              */
/* 網   址：telnet://athena.twbbs.org                    */
/*-------------------------------------------------------*/

#include "record.c"
#include "cache.c"

#define fn_passwd BBSHOME"/.PASSWDS"
#define pwdfile fn_passwd
#define taxfile BBSHOME"/log/tax_income.log"

#define CHECK_LEVEL(x,level)     ((x)?level&(x):1)
#define SUPERHIGHTAX

userec auser;
userec xuser;

int getuser(userid)
  char *userid;
{
  int uid;
  if (uid = searchuser(userid))
    rec_get(pwdfile, &xuser, sizeof(xuser), uid);
  return uid;
}

int taxtax(char *tuser,int money, int exp)
{
  int unum;
  if (unum = getuser(tuser))
    {
      xuser.goldmoney -= money;
//      xuser.exp -= exp;
      substitute_record(pwdfile, &xuser, sizeof(userec), unum);
      return xuser.goldmoney;
    }
  return -1;
}


int
bad_user_id(userid)
  char *userid;
{
  register char ch;
  if (strlen(userid) < 2)    return 1;
  if (not_alpha(*userid))    return 1;
  while (ch = *(++userid))
    if (not_alnum(ch))      return 1;
  return 0;
}

int main(int argc, char *argv[])
{
  FILE *inf;
  time_t now;
  int i, j=0, t=0, own_money=0, expire, expire1, expiretotal, p_money,
      exp, total=0;
  char u='s', l='u', mode[10], name[20];

  inf = fopen(pwdfile,"rb");

  if (inf == NULL)
  {
    printf("Sorry, user data is not ready.\n");
    exit(0);
  }

  if (argc != 2)
  {
     printf("\nusage: allgoldexpire mode \n\n");
     printf(" -l: list all users who have more than 100 golds\n");
     printf(" -e: list users who need to degold\n");
     printf(" -t: get tax from all users (only tax)\n");
     printf(" -g: get tax and get more gold from all users\n\n");
     exit(0);
  }
   else
     strcpy(mode,argv[1]);

  time(&now);
  for (i = 0; fread(&auser, sizeof(userec), 1, inf); i++)
  {
    if (bad_user_id(auser.userid) || strchr(auser.userid,'.') )
        i--;
    else
    {
       exp=0; p_money=0; expire=0; expire1=0;
       own_money=auser.goldmoney;
       t=now-auser.lastlogin;

       if(own_money > 500)    expire=1;
       if(own_money > 1000)   expire=2;
       if(own_money > 5000)   expire=3;
       if(own_money > 10000)  expire=4;
       if(own_money > 50000)  expire=5;

       u='s';
       if(t>60) {          t=t/60;          u='m';       }
       if(t>60) {          t=t/60;          u='h';       }
       if(t>24)
       {
          t=t/24;
          u='d';
          if (t>=7)   expire1=1;         // 超過7天加倍
          if (t>=28)  expire1=2;         // 超過28天再加倍
          if (t>=56)  expire1=3;         // 依此類推
          if (t>=112) expire1=4;
          if (t>=224) expire1=5;
          if (t>=336) expire1=6;
       }

       l='u';
       if CHECK_LEVEL(PERM_BM, auser.userlevel) l='b';
       if CHECK_LEVEL(PERM_BBSADM, auser.userlevel) l='a';
       if CHECK_LEVEL(PERM_SYSOP, auser.userlevel)
//       if CHECK_LEVEL(PERM_NOTAX, auser.userlevel)
       {
          expire=0; expire1=0; l='s';
       }

#ifdef SUPERHIGHTAX
                                                                                  
        if(own_money > 500 && auser.totaltime <= 345600)  expire=20;  // 有些人玩遊戲玩到錢爆多

#endif

       expiretotal=expire+expire1;

       strcpy(name, auser.userid);
       if (expiretotal==0 || (strcmp(mode, "-t")==0))
       {
        p_money=1 + (auser.goldmoney/tax_rate_a);
       }

       if (expiretotal>=4 || (strcmp(mode, "-g")==0))
       {
        p_money=(expire * (auser.goldmoney/tax_rate_b));
       }

       else
       {
          p_money=1 + (auser.goldmoney/tax_rate_a);
          p_money=p_money + (expiretotal * (auser.goldmoney/tax_rate_b));
       }
       if (auser.goldmoney < gold_lower_bound) p_money=0;

       if (strcmp(mode, "-l")==0)
          printf("%04d. %12s (%c) 金幣：%6d 扣除倍率：%2d，扣除：%5dg\n",
             ++j, name, l, auser.goldmoney, expiretotal, p_money);
       else
       {
          if((expiretotal>0) && (p_money>0))
          {
             printf("%04d. %12s (%c)擁有 %6d 金幣，徵收稅金：%5d枚\n",
                ++j, name, l, own_money, p_money);
          }
          if((p_money>0) || (exp>0))
             if((strcmp(mode, "-g")==0) || (strcmp(mode, "-t")==0))
             {
                total = total + p_money;
                taxtax(name, p_money, exp);
             }
       }
    }

  }
  fclose(inf);

  own_money=0;
  inf = fopen(taxfile, "r");  /* 稅收記錄檔  累積稅收金幣 -> 國庫 */

  if (inf != NULL)
  {
     fscanf(inf,"%d",&own_money);
     fclose(inf);
  }
  own_money = own_money + total;

  inf = fopen(taxfile,"w+");
  fprintf(inf,"%d",own_money);
  fclose(inf);

  return 1;
}
