#include "bbs.h"
//#define MARIOLOG "mario_log" /* LOGÀÉ */

int marioII()
{
  char bet[3],m1[7],bet2[3]; /* ½äª÷ */
  char *pic[10]={"  ","£[","¡ð","¡ñ","¡À","¡µ","¡¸","77","¡º","£Y"}; /* ¶µ¥Ø¦W */
  char buffer[200]; /* message */
  int money[10]={0,0,0,0,0,0,0,0,0,0};
  int save_pager;
  int price[10] = {0,50,40,30,25,20,15,10,5,2}; /* ­¿²v */
#if 0
  int bar[49] = {0,3,9,8,4,6,7,2,3,5,8,
                 9,1,8,7,6,3,2,8,4,5,
                 1,2,6,9,7,5,3,9,7,8,
                 1,4,9,2,6,3,5,8,9,7,
                 2,4,8,7,9,1,5,9}; /* ªO­±¦ì§} */
#endif
  int i,j,p,s=1,t,z;
//  time_t nowtime = time(0);
//  extern 
  int getddd;

//  setutmpmode(MARIOII);

  srandom(time(0));
  save_pager=currutmp->pager;
  clear();
  more("game/mario/mario.welcome");
  currutmp->pager = NA;
  getddd=1;

    while(-1)
    {
     clear();
     for(i=1;i<10;i++) /* money Âk¹s */
      money[i]=0;
     print_total(); /* ¦LªO­± */
     do
     {
      do
      {
//       get_record (FN_PASSWD, &cuser, sizeof(cuser),usernum);
       getdata(b_lines, 0, "§A­n©ã¨º¶µ(1~9)[S]¶}©l[Q]Â÷¶}¡G",bet,2,LCECHO,NULL,YEA);
       i=atoi(bet);
       if(i>0&&i<10)
       {
        sprintf(buffer,"(²{ª÷: %d)­n©ã¦h¤Ö½äª÷(1~10000)? «ö Enter¨ú®ø:", cuser.silvermoney);
        getdata(b_lines, 0,buffer,m1,6,DOECHO,NULL,YEA);
        if(atoi(m1)>cuser.silvermoney)
         pressanykey("§Aªº²{ª÷¤£°÷°Õ!!~~:)");
        else if(money[i]+atoi(m1)>=0&&money[i]+atoi(m1)<=10000)
        {
         money[i]+=atoi(m1);
         demoney(atoi(m1), 1);
        }
        cwindows_s(3,4);
       }
       move(21,3);clrtoeol();
       for(i=1;i<10;i++)
        prints("[1;3%dm%6d[m  ",i,money[i]);
      } while( !( (bet[0]=='s'&&money[1]+money[2]+money[3]+money[4]+money[5]+money[6]+money[7]+money[8]+money[9]>0)||bet[0]=='q'));

      if(bet[0]=='q')
      {
       if(money[1]+money[2]+money[3]+money[4]+money[5]+money[6]+money[7]+money[8]+money[9]>0)
       {
        for(i=1;i<10;i++)
         inmoney(money[i]);
       }
       currutmp->pager = save_pager;
       clear();
       getddd=0;
       return 0;
      }
     }while(bet[0]!='s');

     price[0]=rand()%250+1;
     p=get_m(price[0]);  /* Âà´«¦¨¤¤ªº¶µ¥Ø */
     t=(s+301)%48-1;  /* ·|°±ªº¦ì¸m */
     if(t==0) t=48; /* ­×¥¿ */
     if(t==(-1)) t=47;

     j=correct(t,p); /* ¥¿½T¦ì¸m */
     if(j>=t)
      z=301+(j-t);
     else
      z=301-(t-j); /* ³Ì«á­n¨«´X¨B */
     t=(z+s)%48-1; /* ¯uªº­n°±ªº¦ì¸m */
     if(t==0) t=48;
     if(t==(-1)) t=47;

     for(i=s; i<49; i++) /* ²Ä¤@°é */
     {
      refresh();
       usleep(300000*((49-i)^2)/(49-s));        /* °_©l´î³t */
      run(i,0);
     }
     z=z-(48-s)-1;
     do
     {
      for(i=1; i<49; i++)  /* ¤¤¶¡ªº°é */
      {
       refresh();
       usleep(30000);
       run(i,0);
      }
      z=z-48;
     }while(z>47);

     for(i=1; i<z; i++) /* ³Ì«á¤@°é */
     {
      if(i>z-5)    /* ³Ì«á´î³t */
       sleep(1);
      else
       usleep(150000);
      refresh();
      run(i,0);
     }

     for(i=0; i<5; i++) /* °{¤­¦¸ */
     {
      run(t,0);
      refresh();
      usleep(500000);
      run(t,1);
      refresh();
      usleep(500000);
     }

     move(5,8);
     prints("[1;35m¤¤¼úªº¬O[1;37m%s    [37m",pic[p]);
     if(money[p]>0)
     {
      money[0]=money[p]*price[p];
      move(6,8);
      prints("®¥³ß§A©ã¤¤¤F¡AÀò±o¼úª÷ [32m%d[37m [m",money[0]);
      while(-1)
      {
       cwindows_s(7,10);
       sprintf(buffer,"¥Ø«e¼úª÷: %d §AÁÙ­n¤ñ¤j¤p¶Ü??[Y/n]:", money[0]);
       getdata(7,8,buffer,bet,2,DOECHO,NULL,YEA);
       if(bet[0]!='N'&&bet[0]!='n')
       {
        getdata(8,8,"§A­n©ã¤°»ò??[1.¤j 2.¤p]",bet2,2,DOECHO,NULL,YEA);
        move(9,8);
        if(bet2[0]=='2')
         outs("§A©ãªº¬O¤p!!");
        else
         outs("§A©ãªº¬O¤j!!");
        for(i=0; i<30; i++)
        {
         move(12,8);
         outs("                      ùþ¤j           ¡¼¤p ");
         refresh();
         if(i<20)
          usleep(8000*((30-i)^2));
         else
          usleep(8000*(i^2));
         move(12,8);
         outs("                      ¡¼¤j           ùþ¤p ");
         refresh();
         if(i<20)
          usleep(8000*((30-i)^2));
         else
          usleep(8000*(i^2));
        }
        price[0]=(rand()%2)+1;
        move(12,8);
        if(price[0]==1)
         outs("                      ùþ¤j           ¡¼¤p ");
        else
         outs("                      ¡¼¤j           ùþ¤p ");
        if(price[0]==atoi(bet2))
        {
         money[0]=money[0]*2;
         move(10,8);
         prints("°Ú!! ©ã¤¤¤F!! ¼úª÷ÅÜ¦¨%d±iÀ\\¨é", money[0]);
        }
        else
        {
         money[0]=0;
         move(10,8);
         outs("¶ã....©ã¿ù¤F....");
         break;
        }
       }
       else if(bet[0]=='N'||bet[0]=='n')
       {
        move(11,8);
        prints("±o¼úª÷ %d                       ", money[0]);
        break;
       }
      }

      if(money[0]>0)
      {
       inmoney(money[0]);
       sprintf(buffer,"À£¤¤¤F[44;1;37m%s[m ÁÈ¤F[1;33m%7d[m ±iÀ\\¨÷", pic[p], money[0]);
       game_log("MARIOII", buffer);
      }
      else
      {
       sprintf(buffer,"À£¤¤¤F[44;1;37m%s[m ¦ý¬O¤S¿é¤F", pic[p]);
       game_log("MARIOII", buffer);
      }
     }
     else
     {
      move(8,8);
      outs("©êºp...§A¨S©ã¤¤£¬~~~[m");
      strcpy(buffer,"¤@°¦³£¨S¦³À£¤¤°Õ!!");
      game_log("MARIOII", buffer);
     }
//     pressreturn();
      pressanykey(NULL);
     s=t; /* ¦^¶Ç°_ÂI */
    }
   }

int correct(int t, int p)
{
 int j;
 int bar[49] = {0,3,9,8,4,6,7,2,3,5,8,
                9,1,8,7,6,3,2,8,4,5,
                1,2,6,9,7,5,3,9,7,8,
                1,4,9,2,6,3,5,8,9,7,
                2,4,8,7,9,1,5,9}; /* ªO­±¦ì§} */

   for (j=t;j<49;j++)
   {
    if(bar[j]==p)
       return j;
   }
   for (j=t;j>0;j--)
   {
    if(bar[j]==p)
     return j;
   }
}

int run(int q,int p)
{
 int x1,y1,x2,y2,z,k;
 char *pic[10]={"  ","£[","¡ð","¡ñ","¡À","¡µ","¡¸","77","¡º","£Y"}; /* ¶µ¥Ø¦W */
 int bar[49] = {0,3,9,8,4,6,7,2,3,5,8,
                9,1,8,7,6,3,2,8,4,5,
                1,2,6,9,7,5,3,9,7,8,
                1,4,9,2,6,3,5,8,9,7,
                2,4,8,7,9,1,5,9}; /* ªO­±¦ì§} */

 z=bar[q-1];
 k=bar[q];
 if(q==1)
 {
  x1=3;  y1=4;
  x2=1;  y2=4;
 }
 if(q>1&&q<19)
 {
  x1=1; y1=4*q-4;
  x2=1; y2=4*q;
 }
 if(q>18&&q<26)
 {
  x1=2*q-37; y1=72;
  x2=2*q-35; y2=72;
 }
 if(q>25&&q<43)
 {
  x1=15; y1=176-4*q;
  x2=15; y2=172-4*q;
 }
 if(q>42&&q<49)
 {
  x1=101-2*q;    y1=4;
  x2=99-2*q; y2=4;
 }
 move(x1,y1);
 if(q==1)
  outs(pic[9]);
 else
  outs(pic[z]);
 move(x2,y2);
 if(p==1)
  outs(pic[k]);
 else
  outs("ùþ");
}

int get_m(int x)
{
 int p;

 if(x>0&&x<2) p=0;
 if(x>1&&x<4) p=1;
 if(x>3&&x<7) p=2;
 if(x>6&&x<13) p=3;
 if(x>12&&x<24) p=4;
 if(x>23&&x<52) p=5;
 if(x>51&&x<101) p=6;
 if(x>100&&x<171) p=7;
 if(x>170) p=8;
 return p+1;
}

int print_total()
{
 outs("  ¢~¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢¡\n");
 outs("  ¢x¡ñ¢x£Y¢x¡º¢x¡À¢x¡¸¢x77¢x¡ð¢x¡ñ¢x¡µ¢x¡º¢x£Y¢x£[¢x£Y¢x77¢x¡¸¢x¡ñ¢x¡ð¢x¡º¢x\n");
 outs("  ¢u¢w¢q¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢q¢w¢t\n");
 outs("  ¢x£Y¢x                                                              ¢x¡À¢x\n");
 outs("  ¢u¢w¢t                                                              ¢u¢w¢t\n");
 outs("  ¢x¡µ¢x                                                              ¢x¡µ¢x\n");
 outs("  ¢u¢w¢t                                                              ¢u¢w¢t\n");
 outs("  ¢x£[¢x                                                              ¢x£[¢x\n");
 outs("  ¢u¢w¢t                                                              ¢u¢w¢t\n");
 outs("  ¢x£Y¢x                                                              ¢x¡ð¢x\n");
 outs("  ¢u¢w¢t                                                              ¢u¢w¢t\n");
 outs("  ¢x77¢x                                                              ¢x¡¸¢x\n");
 outs("  ¢u¢w¢t                      ùþ¤j           ¡¼¤p                     ¢u¢w¢t\n");
 outs("  ¢x¡º¢x                                                              ¢x£Y¢x\n");
 outs("  ¢u¢w¢q¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢s¢w¢q¢w¢t\n");
 outs("  ¢x¡À¢x¡ð¢x77¢x£Y¢x¡º¢x¡µ¢x¡ñ¢x¡¸¢x¡ð¢x£Y¢x¡À¢x£[¢x¡º¢x77¢x£Y¢x¡ñ¢x¡µ¢x77¢x\n");
 outs("  ¢¢¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢r¢w¢£\n");
 outs("    [1;31m¢~¢w¢¡  [1;32m¢~¢w¢¡  [1;33m¢~¢w¢¡  [1;34m¢~¢w¢¡  [1;35m¢~¢w¢¡  [1;36m¢~¢w¢¡  [1;37m¢~¢w¢¡  [1;38m¢~¢w¢¡  [1;39m¢~¢w¢¡ [m \n");
 outs("    [1;31m¢x£[¢x  [1;32m¢x¡ð¢x  [1;33m¢x¡ñ¢x  [1;34m¢x¡À¢x  [1;35m¢x¡µ¢x  [1;36m¢x¡¸¢x  [1;37m¢x77¢x  [1;38m¢x¡º¢x  [1;39m¢x£Y¢x [m \n");
 outs("    [1;31m¢x50¢x  [1;32m¢x40¢x  [1;33m¢x30¢x  [1;34m¢x25¢x  [1;35m¢x20¢x  [1;36m¢x15¢x  [1;37m¢x10¢x  [1;38m¢x 5¢x  [1;39m¢x 2¢x [m \n");
 outs("    [1;31m¢¢¢w¢£  [1;32m¢¢¢w¢£  [1;33m¢¢¢w¢£  [1;34m¢¢¢w¢£  [1;35m¢¢¢w¢£  [1;36m¢¢¢w¢£  [1;37m¢¢¢w¢£  [1;38m¢¢¢w¢£  [1;39m¢¢¢w¢£ [m \n");

}

int cwindows_s(int x1,int x2)
{
 char buffer[] = "                                                              ";
 int i;

 for (i = x1; i < (x2+1); i++)
 {
  move (i, 8);
  prints ("%s", buffer);
 }
}

