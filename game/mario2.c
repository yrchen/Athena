#include "bbs.h"
//#define MARIOLOG "mario_log" /* LOG檔 */

int marioII()
{
  char bet[3],m1[7],bet2[3]; /* 賭金 */
  char *pic[10]={"  ","Ω","♀","♂","㊣","△","☆","77","◇","Χ"}; /* 項目名 */
  char buffer[200]; /* message */
  int money[10]={0,0,0,0,0,0,0,0,0,0};
  int save_pager;
  int price[10] = {0,50,40,30,25,20,15,10,5,2}; /* 倍率 */
#if 0
  int bar[49] = {0,3,9,8,4,6,7,2,3,5,8,
                 9,1,8,7,6,3,2,8,4,5,
                 1,2,6,9,7,5,3,9,7,8,
                 1,4,9,2,6,3,5,8,9,7,
                 2,4,8,7,9,1,5,9}; /* 板面位址 */
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
     for(i=1;i<10;i++) /* money 歸零 */
      money[i]=0;
     print_total(); /* 印板面 */
     do
     {
      do
      {
//       get_record (FN_PASSWD, &cuser, sizeof(cuser),usernum);
       getdata(b_lines, 0, "你要押那項(1~9)[S]開始[Q]離開：",bet,2,LCECHO,NULL,YEA);
       i=atoi(bet);
       if(i>0&&i<10)
       {
        sprintf(buffer,"(現金: %d)要押多少賭金(1~10000)? 按 Enter取消:", cuser.silvermoney);
        getdata(b_lines, 0,buffer,m1,6,DOECHO,NULL,YEA);
        if(atoi(m1)>cuser.silvermoney)
         pressanykey("你的現金不夠啦!!~~:)");
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
     p=get_m(price[0]);  /* 轉換成中的項目 */
     t=(s+301)%48-1;  /* 會停的位置 */
     if(t==0) t=48; /* 修正 */
     if(t==(-1)) t=47;

     j=correct(t,p); /* 正確位置 */
     if(j>=t)
      z=301+(j-t);
     else
      z=301-(t-j); /* 最後要走幾步 */
     t=(z+s)%48-1; /* 真的要停的位置 */
     if(t==0) t=48;
     if(t==(-1)) t=47;

     for(i=s; i<49; i++) /* 第一圈 */
     {
      refresh();
       usleep(300000*((49-i)^2)/(49-s));        /* 起始減速 */
      run(i,0);
     }
     z=z-(48-s)-1;
     do
     {
      for(i=1; i<49; i++)  /* 中間的圈 */
      {
       refresh();
       usleep(30000);
       run(i,0);
      }
      z=z-48;
     }while(z>47);

     for(i=1; i<z; i++) /* 最後一圈 */
     {
      if(i>z-5)    /* 最後減速 */
       sleep(1);
      else
       usleep(150000);
      refresh();
      run(i,0);
     }

     for(i=0; i<5; i++) /* 閃五次 */
     {
      run(t,0);
      refresh();
      usleep(500000);
      run(t,1);
      refresh();
      usleep(500000);
     }

     move(5,8);
     prints("[1;35m中獎的是[1;37m%s    [37m",pic[p]);
     if(money[p]>0)
     {
      money[0]=money[p]*price[p];
      move(6,8);
      prints("恭喜你押中了，獲得獎金 [32m%d[37m [m",money[0]);
      while(-1)
      {
       cwindows_s(7,10);
       sprintf(buffer,"目前獎金: %d 你還要比大小嗎??[Y/n]:", money[0]);
       getdata(7,8,buffer,bet,2,DOECHO,NULL,YEA);
       if(bet[0]!='N'&&bet[0]!='n')
       {
        getdata(8,8,"你要押什麼??[1.大 2.小]",bet2,2,DOECHO,NULL,YEA);
        move(9,8);
        if(bet2[0]=='2')
         outs("你押的是小!!");
        else
         outs("你押的是大!!");
        for(i=0; i<30; i++)
        {
         move(12,8);
         outs("                      ��大           □小 ");
         refresh();
         if(i<20)
          usleep(8000*((30-i)^2));
         else
          usleep(8000*(i^2));
         move(12,8);
         outs("                      □大           ��小 ");
         refresh();
         if(i<20)
          usleep(8000*((30-i)^2));
         else
          usleep(8000*(i^2));
        }
        price[0]=(rand()%2)+1;
        move(12,8);
        if(price[0]==1)
         outs("                      ��大           □小 ");
        else
         outs("                      □大           ��小 ");
        if(price[0]==atoi(bet2))
        {
         money[0]=money[0]*2;
         move(10,8);
         prints("啊!! 押中了!! 獎金變成%d張餐\券", money[0]);
        }
        else
        {
         money[0]=0;
         move(10,8);
         outs("嗚....押錯了....");
         break;
        }
       }
       else if(bet[0]=='N'||bet[0]=='n')
       {
        move(11,8);
        prints("得獎金 %d                       ", money[0]);
        break;
       }
      }

      if(money[0]>0)
      {
       inmoney(money[0]);
       sprintf(buffer,"壓中了[44;1;37m%s[m 賺了[1;33m%7d[m 張餐\卷", pic[p], money[0]);
       game_log("MARIOII", buffer);
      }
      else
      {
       sprintf(buffer,"壓中了[44;1;37m%s[m 但是又輸了", pic[p]);
       game_log("MARIOII", buffer);
      }
     }
     else
     {
      move(8,8);
      outs("抱歉...你沒押中ㄛ~~~[m");
      strcpy(buffer,"一隻都沒有壓中啦!!");
      game_log("MARIOII", buffer);
     }
//     pressreturn();
      pressanykey(NULL);
     s=t; /* 回傳起點 */
    }
   }

int correct(int t, int p)
{
 int j;
 int bar[49] = {0,3,9,8,4,6,7,2,3,5,8,
                9,1,8,7,6,3,2,8,4,5,
                1,2,6,9,7,5,3,9,7,8,
                1,4,9,2,6,3,5,8,9,7,
                2,4,8,7,9,1,5,9}; /* 板面位址 */

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
 char *pic[10]={"  ","Ω","♀","♂","㊣","△","☆","77","◇","Χ"}; /* 項目名 */
 int bar[49] = {0,3,9,8,4,6,7,2,3,5,8,
                9,1,8,7,6,3,2,8,4,5,
                1,2,6,9,7,5,3,9,7,8,
                1,4,9,2,6,3,5,8,9,7,
                2,4,8,7,9,1,5,9}; /* 板面位址 */

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
  outs("��");
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
 outs("  ╭─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─╮\n");
 outs("  │♂│Χ│◇│㊣│☆│77│♀│♂│△│◇│Χ│Ω│Χ│77│☆│♂│♀│◇│\n");
 outs("  ├─┼─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┼─┤\n");
 outs("  │Χ│                                                              │㊣│\n");
 outs("  ├─┤                                                              ├─┤\n");
 outs("  │△│                                                              │△│\n");
 outs("  ├─┤                                                              ├─┤\n");
 outs("  │Ω│                                                              │Ω│\n");
 outs("  ├─┤                                                              ├─┤\n");
 outs("  │Χ│                                                              │♀│\n");
 outs("  ├─┤                                                              ├─┤\n");
 outs("  │77│                                                              │☆│\n");
 outs("  ├─┤                      ��大           □小                     ├─┤\n");
 outs("  │◇│                                                              │Χ│\n");
 outs("  ├─┼─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┬─┼─┤\n");
 outs("  │㊣│♀│77│Χ│◇│△│♂│☆│♀│Χ│㊣│Ω│◇│77│Χ│♂│△│77│\n");
 outs("  ╰─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─┴─╯\n");
 outs("    [1;31m╭─╮  [1;32m╭─╮  [1;33m╭─╮  [1;34m╭─╮  [1;35m╭─╮  [1;36m╭─╮  [1;37m╭─╮  [1;38m╭─╮  [1;39m╭─╮ [m \n");
 outs("    [1;31m│Ω│  [1;32m│♀│  [1;33m│♂│  [1;34m│㊣│  [1;35m│△│  [1;36m│☆│  [1;37m│77│  [1;38m│◇│  [1;39m│Χ│ [m \n");
 outs("    [1;31m│50│  [1;32m│40│  [1;33m│30│  [1;34m│25│  [1;35m│20│  [1;36m│15│  [1;37m│10│  [1;38m│ 5│  [1;39m│ 2│ [m \n");
 outs("    [1;31m╰─╯  [1;32m╰─╯  [1;33m╰─╯  [1;34m╰─╯  [1;35m╰─╯  [1;36m╰─╯  [1;37m╰─╯  [1;38m╰─╯  [1;39m╰─╯ [m \n");

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

