
/* 	�o�O ��WD(9806�H��)==>>WD990617 �� .PASSWDS ���{�� 
	�����s�� PASSWDS �|�X�{�b ~bbs/PASSWDS.NEW
 	�`�N :  �ϥΫe�аȥ��ƥ��{�� .PASSWDS
		�л\�ɮ׫e�вM�� SHM �ýT�w���W�L���� USER 	
						���a�a Ziyun   */
#include "bbs.h"
#include "userec.at"
#include "userec.new"

//#define USE_KEYPASS

int main()
{
  int fdr,fdw,i=0;
  new new;
  old tuser;
  
  fdr=open(BBSHOME"/.PASSWDS",O_RDONLY);
  fdw=open(BBSHOME"/PASSWDS.NEW",O_WRONLY | O_CREAT | O_TRUNC, 0644);
  printf("old struct size :%d\n",sizeof(old));
  printf("new struct size :%d\n",sizeof(new));

#ifdef USE_KEYPASS
  getchar();
#endif

  while(read(fdr,&tuser,sizeof(old))==sizeof(old))
  {
    if (strlen(tuser.userid) < 2)
    {
      printf("BAD USERID!! LEN DIDN'T ENOUGH!!\n");
#ifdef USE_KEYPASS
      getchar();
#endif
      continue;
    }
    if (not_alpha(*tuser.userid))
    {
      printf("BAD USERID!!\n");
#ifdef USE_KEYPASS
      getchar();
#endif
      continue;
    }
    printf("tran user: %s\n", tuser.userid);

  /*�򥻸�� 136 bytes*/
        memcpy(new.userid,tuser.userid,IDLEN+1);         
        memcpy(new.realname,tuser.realname,20);          
        memcpy(new.realname,tuser.realname,20); 
        memcpy(new.username,tuser.username,24); 
        memcpy(new.passwd,tuser.passwd,PASSLEN);  
      	memcpy(new.email,tuser.email,50);      
      	memcpy(new.countryid, "", 11);   	//�����Ҧr��
  	new.month=tuser.month;              
  	new.day=tuser.day;                  
  	new.year=tuser.year;                
  	new.sex=tuser.sex;                  

  /*�t���v�� 32 bytes*/
        new.uflag=tuser.uflag;                  
        new.userlevel=tuser.userlevel; 
  	new.invisible=tuser.invisible; 
  	new.state=tuser.state;              
  	new.pager=tuser.pager;  
  	new.habit=tuser.habit;         
  	new.exmailbox=tuser.exmailbox;
  	new.exmailboxk=tuser.exmailboxk;  	
  	new.dtime=tuser.dtime;  	
//  	new.update_songtime=tuser.update_songtime;	/*�I�q���Ƨ�s*/
  	new.update_songtime=0;				/*�I�q���Ƨ�s*/
  	new.scoretimes=tuser.scoretimes;		/*��������*/

  /*���U��� 44 bytes*/
    	new.firstlogin=tuser.firstlogin;    
	memcpy(new.justify,tuser.justify,REGLEN + 1);      	
	new.rtimes=tuser.rtimes;

  /*�ߦn�]�w 61 bytes*/
        memcpy(new.feeling,tuser.feeling,5);
        new.lightbar[0] = 4;
        new.lightbar[1] = 7;
        new.lightbar[2] = 1;
        new.lightbar[3] = 0;
        new.lightbar[4] = 0;
        memcpy(new.cursor, ">>", 51);


  /*�t�θ�� 130 bytes*/
        new.numlogins=tuser.numlogins;          
        new.numposts=tuser.numposts;            
  	new.lastlogin=tuser.lastlogin;      
  	memcpy(new.lasthost,tuser.lasthost,24);
  	memcpy(new.vhost,tuser.lasthost,24);   
  	memcpy(new.toqid,tuser.userid,IDLEN+1);     
  	memcpy(new.beqid,tuser.userid,IDLEN+1);     
  	new.toquery=tuser.toquery;      
  	new.bequery=tuser.bequery;      
  	new.totaltime=tuser.totaltime;  
  	new.sendmsg=tuser.sendmsg;      
  	new.receivemsg=tuser.receivemsg;
	new.silvermoney=tuser.silvermoney;
        new.goldmoney = tuser.goldmoney;
//        new.songtimes=tuser.songtimes;	
        new.songtimes=0;

        write(fdw,&new,sizeof(new));

        ++i;
   }
   close(fdr);
   close(fdw);
}     
