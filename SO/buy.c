/* �ϥο������ */

#include "bbs.h"
void prints(const char *fmt, ... );
/* ������ */
/* �R�v���� */
unsigned 
setuperm(unsigned pbits, char nb, int money)  
{
  register int i;
  
  i = nb - 'a';
  if (!((pbits >> i) & 1))
  {
    pbits ^= (1 << i);
    degold(money);
  }
  return(pbits);
}


/*�R��O��*/
void 
tradelog(userid, i)
  char *userid;  
  int i;
{
  time_t now = time(0);
  char genbuf[200];
  char *item[4] = {"�H�c�W��","�ä[����","�ä[�ק�G�m","�H��L�W��"};

  now = time(NULL) - 6 * 60;
  sprintf(genbuf, "�b %s �ϥΪ� [1;32m%s[m �ʶR�F[1;36m%s[m���v��",
    Cdate(&now),cuser.userid,item[i]);
  f_cat("log/trade.log",genbuf);
}

void
p_cloak()
{
  if (answer(currutmp->invisible ? "�T�w�n�{��?[y/N]" : "�T�w�n����?[y/N]") != 'y')
    return;

  if(!currutmp->invisible)
  {
    if (check_money(2,GOLD)) 
      return;
    degold(2);
  }
  currutmp->invisible %= 2;
  pressanykey((currutmp->invisible ^= 1) ? MSG_CLOAKED : MSG_UNCLOAK);
  return;
}

void
p_fcloak()
{
  register int i;

  if (HAS_PERM(PERM_CLOAK))
    pressanykey("�A�w�g�i�H���ΤF�٨ӶR�A�����Ӧh�ڡH");

  if (HAS_PERM(PERM_CLOAK) || check_money(500,GOLD))
  {
    return;
  }

  if (answer("�T�w�n�� $500 �ǲ߲׷������j�k�H[y/N]") != 'y')
    return;
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  i = setuperm(cuser.userlevel,'g',500);
  update_data();
  cuser.userlevel = i;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  tradelog(cuser.userid,1);
  pressanykey("���߱z�w�g�Ƿ|�F�׷������j�k!!");
  return;
}

void
p_from()
{
  
  if (check_money(5, GOLD)) 
    return;

  if (answer("�T�w�n��G�m?[y/N]") != 'y')
    return;

  if (getdata(b_lines, 0, "�п�J�s�G�m:", currutmp->from, 17, DOECHO,0))
  {
    degold(5);
    currutmp->from_alias=0;
  }
  return;
}

void
p_ffrom()
{
  register int i;

  if(check_money(1000,GOLD) || HAS_PERM(PERM_FROM) || HAS_PERM(PERM_SYSOP))
  {
    if(HAS_PERM(PERM_FROM) || HAS_PERM(PERM_SYSOP))
      pressanykey("�A�w�g�i�H�ק�G�m�F�٨ӶR�A�����Ӧh�ڡH");
    return;
  }
  
  if (answer("�T�w�n�� $1000 �ʶR�ק�G�m�_��H[y/N]") != 'y')
     return;
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  i=setuperm(cuser.userlevel,'t',1000);
  update_data();
  cuser.userlevel=i;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  tradelog(cuser.userid,2);
  pressanykey("���ߧA�o��F�ק�G�m�_��");
  return;
}


void
p_exmail()
{
  char ans[4],buf[100];
  int  n;
  if(cuser.exmailbox >= MAXEXKEEPMAIL )
  {
    pressanykey("�e�q�̦h�W�[ %d �ʡA����A�R�F�C", MAXEXKEEPMAIL);
    return;
  }
  sprintf(buf,"�z���W�� %d �ʮe�q�A�٭n�A�R�h��?",cuser.exmailbox);
  getdata(b_lines, 0, buf,ans,3,LCECHO,"10");
  n = atoi(ans);
  if(!ans[0] || !n )
    return;
  if(n+cuser.exmailbox > MAXEXKEEPMAIL ) 
     n = MAXEXKEEPMAIL - cuser.exmailbox;
  if(check_money(n*100,GOLD))
    return;
  degold(n*100);
  inmailbox(n);
  return;
}

void
p_spmail()
{
  char ans[4],buf[128];
  int n;
  if (HAVE_PERM(PERM_SYSOP) || HAVE_PERM(PERM_MAILLIMIT)) 
  {
    if(HAVE_PERM(PERM_MAILLIMIT))
      pressanykey("�w�g�H�c�L�W���F, ������!!");
    return;
  }
  if(HAS_PERM(PERM_LOGINOK))  /* �O�_�q�L�����{�� */
  {
    if(cuser.exmailbox <=0)
    {
      pressanykey("Sorry,�z�S���R�L�H�c�ҥH����� !!");
      return;
    }
    else  /* �p�G�� check �L�F�A�D�{��  */
    {
      sprintf(buf,"�A�ثe�� %d �ʫH�c�e�q�i�H��X�A�A�n��X�X�� ? ",cuser.exmailbox);
      getdata(b_lines, 0, buf, ans, 3, LCECHO, "10");
      n = atoi(ans);
      if(!ans[0] || !n )
        return;
      if (n > cuser.exmailbox)  
      {
        n = cuser.exmailbox;
        pressanykey("�W�L�i�H��X���ƶq!! ��X�ƶq�אּ %d", cuser.exmailbox);
      }

      ingold(n*80);
      demailbox(n);
    }
  }
  sprintf(buf,"�z�ثe��X�F %d �ʫH�A�٦��ѤU %d �ʼW�[�H�c�e�q",n, cuser.exmailbox);  
  pressanykey(buf);
  chkmailbox();
}


void
p_ulmail()
{
  register int i;
  if(check_money(100000,GOLD) || HAS_PERM(PERM_MAILLIMIT))
  {
    if(HAS_PERM(PERM_MAILLIMIT))
      pressanykey("�A���H�c�w�g�S������F�٨ӶR�A�����Ӧh�ڡH");
    return;
  }
  if (answer("�T�w�n�� $100000 �ʶR�L�W���H�c?[y/N]") != 'y')
    return;
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  i=setuperm(cuser.userlevel,'f',100000);
  update_data();
  cuser.userlevel=i;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  tradelog(cuser.userid,3);
  pressanykey("���߱z�w�g�o��F�L�W�����H�c!!");
  return;
}

void
p_give()
{
   int money;
   char id[IDLEN+1],buf[256],reason[60];
   FILE *fp=fopen("tmp/givemoney","w");
   fileheader mymail;
   time_t now;
   time(&now);
   move(12,0);
   update_data();
   usercomplete("��J��誺ID�G", id);
   if (!id[0] || !getdata(14, 0, "�n��h�ֿ��L�h�H", buf, 9, LCECHO,0)) return;
   money = atoi(buf);
   if(check_money(money,SILVER)) return;
   if(money > 0)
   {
     demoney(money);
     money *= 0.9;
     inumoney(id, money);
     sprintf(buf,"�@��: %s \n"
                 "���D:[��b�q��] �e�A %d ����I\n"
                 "�ɶ�: %s\n",cuser.userid,money,ctime(&now));
     fputs(buf,fp);
     while(!getdata(15,0,"�п�J�z�ѡG",reason,60,DOECHO ,"���Ӧh"));
     sprintf(buf,"[1;32m%s[37m �e�A [33m%d [37m���C\n"
                 "�L���z�ѬO�G[33m %s [m",cuser.userid,money,reason);
     fputs(buf,fp);
     fclose(fp);
     sprintf(buf,"home/%s", id);
     stampfile(buf, &mymail);
     strcpy(mymail.owner, cuser.userid);
     f_mv ("tmp/givemoney",buf);
     sprintf(mymail.title,"[��b�q��] �e�A %d ����I",money);
     sprintf(buf,"home/%s/.DIR",id);
     rec_add(buf, &mymail, sizeof(mymail));
     sprintf(buf,"[1;33m%s %s [37m��{�� [33m%d �� [37m��b��[33m %s[37m",
     Cdate(&now),cuser.userid,money,id);
     f_cat("log/bank.log",buf);
   }
   return;
}


void
exchange()
{
  char buf[100], ans[10];
  int i, Money = 0;
  time_t now = time(0);
  
  move(12, 0);
  clrtobot();
  prints("�A���W������ %d ��,�ȹ� %d ��\n", cuser.goldmoney, cuser.silvermoney);
  outs("\n���� �G �ȹ�  =  1 �G 10000\n");
  if (!getdata(17, 0, "(1)�ȹ�������  (2)�������ȹ� ", ans, 3, LCECHO, 0)) 
    return;

  if (ans[0] < '1' || ans[0] > '2') 
    return;

  i = atoi(ans);
  while (Money <= 0 || 
    (i == 1 ? (Money > cuser.silvermoney) : (Money > cuser.goldmoney)))
  {
    if (i == 1)
      getdata(18,0,"�n���h�ֻȹ��Ӵ��H ",ans,10,LCECHO,0);
    else
      getdata(18,0,"�n���h�֪����Ӵ��H ",ans,10,LCECHO,0);
    if(!ans[0]) return;
    Money = atol(ans);
  }
  if(i == 1)
    sprintf(buf,"�O�_�n�ഫ�ȹ� %d �� ������ %d ? [y/N]",Money,Money/10000);
  else
    sprintf(buf,"�O�_�n�ഫ���� %d �� ���ȹ� %d ? [y/N]",Money,Money*10000);
  getdata(19,0,buf,ans,3,LCECHO,0);
  if(ans[0] == 'y')
  {
    if(i == 1)
    {       
      Money *= 1.05;
      demoney(Money);
      ingold(Money/10500);
      sprintf(buf,"[1;36m%s %s [37m��ȹ� [33m%d �� [37m�ഫ������ %d ��",
        Cdate(&now),cuser.userid,Money, Money/10500);
    }
    else
    {
      degold(Money);
      inmoney(Money*9500);
      sprintf(buf,"[1;32m%s %s [37m����� [33m%d �� [37m�ഫ���ȹ� %d ��",
        Cdate(&now),cuser.userid,Money, Money*9500);
    }
    f_cat("log/bank.log",buf);
    pressanykey("�A���W������ %d ��,�ȹ� %d ��",cuser.goldmoney,cuser.silvermoney);
  }
  else
    pressanykey("����.....");
}


/* ���w */
void
bank()
{
  char buf[10];
 
  if (lockutmpmode(BANK)) 
    return;

  setutmpmode(BANK);
  stand_title("�����Ȧ�");
  
  if (count_multi() > 1)
  {
    pressanykey("�z���ଣ�������i�J�Ȧ��� !");
    unlockutmpmode();    
    return;
  }
  
  counter(BBSHOME"/log/counter/�Ȧ�","�ϥλȦ�",0);
  move(2, 0);
  update_data();
  prints("\033[1;36m%12s\033[0;1m �z�n�r�I�w����{���Ȧ�C\n \
[1;36m��������������������������������������������������������������\n \
��[32m�z�{�b���ȹ�[33m %12d [32m���A���� [33m%12d[32m ��[36m        ��\n \
��������������������������������������������������������������\n \
�� �ثe�Ȧ洣�ѤU�C�T���A�ȡG                               ��",
    cuser.userid, cuser.silvermoney, cuser.goldmoney);
    move(6, 0);
      outs(" \
��[33m1.[37m ��b -- ���������`�B�� 10% �@������O (���ȹ�)[36m         ��\n \
��[33m2.[37m �קI -- �ȹ�/���� �I�� (��� 5% ����O) [36m               ��\n \
��[33mQ.[37m ���}�Ȧ�[36m                                               ��\n \
��������������������������������������������������������������[m");
  getdata(12, 0, "  �п�J�z�ݭn���A�ȡG", buf, 3, DOECHO, 0);
  if (buf[0] == '1')
    p_give();
  else if (buf[0] == '2')
    exchange();

  update_data();  
  pressanykey("���¥��{�A�U���A�ӡI");
  unlockutmpmode();
}

