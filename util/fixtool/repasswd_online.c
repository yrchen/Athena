/*
�N�Ҧ� bad_user_id �M���C

�i�b bbs �B�@�ɰ���C

�Y�n�^�_��ק�e�����A:
cp ~bbs/PASSWDS ~bbs/.PASSWDS

*/

#include <stdio.h>
#include "bbs.h"

#define DOTPASSWDS BBSHOME"/.PASSWDS"
#define PASSWDSBAK BBSHOME"/PASSWDS"
#define TMPFILE    BBSHOME"/tmp/tmpfile"


struct userec cuser;

int 
bad_user_id(userid)
  char *userid;
{
  register char ch;

  if (strlen(userid) < 2) return 1;
  if (!isalpha(*userid)) return 1;
  if (!strcasecmp(userid, "new")) return 1;

  while (ch = *(++userid))
  {
    if (!isalnum(ch))
      return 1;
  }
  return 0;
}


#define OVERTIME 2*31 /* ��Ӥ� */

int 
over_time(user_t)
  time_t user_t;
{
  time_t now = time(NULL);
  struct tm ptime, utime;
  int top, down;
  
  localtime_r(&user_t, &utime);
  localtime_r(&now, &ptime);
  
  if (utime.tm_year != ptime.tm_year)
  {
    if(utime.tm_yday + OVERTIME < 365)
      return 1;
    else
    {
      utime.tm_yday = utime.tm_yday + OVERTIME - 365;
      utime.tm_year++;
    }
  }
  if(ptime.tm_yday - utime.tm_yday > OVERTIME) return 1;
  
  return 0;
}

int
main()
{
    FILE *foo1, *foo2;
    int cnum,i,match;
    int tag;
    char *errmsg[4] = {"",
    		       "���~���Ϫ̦W��",
    		       "�������t��",
    		       "��Ӥ�S�W��"};

    setgid(BBSGID);
    setuid(BBSUID);
    if( ((foo1=fopen(DOTPASSWDS, "r")) == NULL) || ((foo2=fopen(TMPFILE,"w"))== NULL) )
    {
        if(foo1 != NULL) fclose(foo1);
        if(foo2 != NULL) fclose(foo2);    

        puts("file opening error");
        exit(1);
    }

    while( (cnum=fread( &cuser, sizeof(cuser), 1, foo1))>0 ) 
    {

      if(bad_user_id(cuser.userid))
        tag = 1;
      else if(cuser.goldmoney <0 || cuser.silvermoney <0)
        tag = 2;
      else if(!HAVE_PERM(PERM_SYSOP) && over_time(cuser.lastlogin))
	tag = 3;
      else
        tag = 0;      

      if(cuser.userid[0] == 0)
        tag = 0;
        
       if (tag != 0)
       {
         char src[80], dst[80];       

         printf("KILL %*s : %s\t�̫�W���ɶ�:%s",IDLEN, cuser.userid, errmsg[tag], ctime(&cuser.lastlogin));
         /* user home */
         sprintf(src, "%s/home/%s", BBSHOME, cuser.userid);
         sprintf(dst, "%s/tmp/%s", BBSHOME, cuser.userid);
         if(dashd(src))
         {
           printf("�ϥΪ̮a�ؿ�: %s �w�R�� *userid=%d\n\n", src, cuser.userid[0]);
           f_mv(src , dst);         
	 }
	 else
	   printf("%s �Ӹ�Ƨ����s�b!!\n\n", src);

         /* .PASSWDS */
         memset(&cuser, 0, sizeof(cuser));	   
       }
       fwrite( &cuser, sizeof(cuser), 1, foo2);
    }
    fclose(foo1);
    fclose(foo2);

    if(rename(DOTPASSWDS,PASSWDSBAK)==-1)
    {
        puts("replace fails");
        exit(1);
    }
    unlink(DOTPASSWDS);
    rename(TMPFILE,DOTPASSWDS);
    unlink("tmpfile");

    return 0;
}

