/*--------------------------------------------------------------*/
/* ¿}ªG«ÎÁÉ½Þ³õ                  98'12/17   */
/*--------------------------------------------------------------*/
/* ¥»µ{¦¡¥Ñ¤p§Ì§Ú(SugarII)                  */
/* idea¨ú¦Û©ó "¬Õ¤ë»PÁc¬P" ¨Ã°Ñ¦Ò badboy ªº source ­«·s§ï¼g¥X¨Ó */
/* ªº¡A¼g±o¦³ÂIÄê¡AÅwªï¤j®a´ú¸Õ¨Ã­×§ï¡A¦p¦³¥ô¦ó°ÝÃD¡A©Î¥ô¦ó·Qªk */
/* ¡B«ØÄ³³£§Æ±æ¯à³qª¾§Ú¤@Án¡C   EMail:u861838@Oz.nthu.edu.tw    */
/*--------------------------------------------------------------*/

#include "bbs.h"

int p[5],i;

int race_path(int run,int j,int step)
{
  if(step==0) return -1;
  else if(step<0)
  {
    if(j+step<0) j=-step;
    move(run+9,(j+step)*2+8);
    clrtoeol();
    p[run]+=step*100;
    if(p[run]<1) p[run]=1;
    return -1;
  }
  move(run+9,j*2+8);
  prints("[1;3%dm",run+1);
  for(i=0;i<step;i++) prints("¡½");
  prints("[0m");
  if(p[run]+step*100>3000) return run;
  else return -1;
}

int race()
{
  char bet[1],m1[8],ch,*racename[5]={"ª°¯W","®üÀt","ªQ¹«","¤p½Þ","¥øÃZ"};
  int rate[5],flag,stop[5],run,win,ball,money[6],save_pager;
  time_t now;

  setutmpmode(RACE);
  save_pager=currutmp->pager;
  clear();
  more("game/race.welcome");
  currutmp->pager =2;
    while(-1)
    {
        ball=win=-1;flag=0;for(i=0;i<5;i++){p[i]=1;stop[i]=money[i+1]=0;rate[i]=100;}
    clear();
    move(2,0);
    outs("  [1m¤H¦W¡G[m    ");
    for(i=0;i<5;i++)
        prints(" [200m[%dm[%d;613m[1;%dm%d. %s[201m ",i+1442,i+649,i+31,i+1,racename[i]);
    outs("\n [m [1m³t«×¡G[m    \n  [1m½äª÷¡G[m    \n");
    outs("¡]½Ð«ö [1;36mk[m ¬°§Aªº°Êª«¥[ªo¡A«ö [1;36mz[m ¥i¥á¥XºhåÑ(¥u¦³¤@¦¸¾÷·|)¡^");
    move(9,0);
    for(i=0;i<5;i++)
        prints("%d.[1;3%dm%s[mùø\n",i+1,i+1,racename[i]);
    outs("¢w¢w¢wùö¢w¢w¢r¢w¢w¢r¢w¢w¢r¢w¢w¢r¢w¢w¢r¢w¢w¢r¢w¢w¢r¢w¢w¢r¢w¢w¢r¢w¢wù÷");
    do
    {
        do{
        getdata(0,0,"§A­n©ã­þ­Ó(1~5)[S]¶}©l[Q]Â÷¶}¡G",bet,2,DOECHO,0);
        i=atoi(bet);
        if(i>0&&i<6)
        {
            getdata(1,0,"­n©ã¦h¤Ö½äª÷(1~10000)? «ö Enter ¨ú®ø¡G",m1,6,1,0);
            if(atoi(m1)>cuser.silvermoney)
            pressanykey("§Aªº²{ª÷¤£°÷°Õ!!~~:)");
            else if(money[i]+atoi(m1)>=0&&money[i]+atoi(m1)<=10000)
            {
             money[i]+=atoi(m1);
             demoney(atoi(m1));
            }
            move(1,0);clrtoeol();
        }
        move(4,12);clrtoeol();
        for(i=1;i<6;i++)
            prints("   [1;3%dm%5d[m ",i,money[i]);
        } while( !( ((bet[0]=='s'||bet[0]=='S')&&money[1]+money[2]+money[3]+money[4]+money[5]>0)||bet[0]=='q'||bet[0]=='Q') );
        if(bet[0]=='q'||bet[0]=='Q')
        {
        if(money[1]+money[2]+money[3]+money[4]+money[5]>0)
            for(i=1;i<6;i++)
            inmoney(money[i]);
            currutmp->pager = save_pager;
        return 0;
        }
    } while(bet[0]!='s'&&bet[0]!='S');
    while(win==-1)
    {
        move(3,12);clrtoeol();
        for(i=0;i<5;i++)
        {
        now=random();
        if(stop[i]<1)
            rate[i]+=now%20-(rate[i]+170)/30;
        if(rate[i]<0) rate[i]=0;
        prints("    [1;3%dm%4d[m ",i+1,rate[i]);
        }
        do ch=igetch();
        while(!((ch=='k'||ch=='K')||((ch=='z'||ch=='Z')&&ball==-1)));
            if(ch=='z'||ch=='Z')
            {
                now=random();
                run=now%5;
                stop[run]=3;
                if(flag<6)      move(15+flag,0);
                else            move(9+flag,42);
                prints("[1m§A¥á¥XºhåÑ[3%dm%s[37m°±¤î«e¶i¤T¦¸¡A³t«×0[m",run+1,racename[run]);
                rate[run]=0;flag++;ball=-2;
            }
            else
            {
        now=random();
        run=now%5;
        now=random();
        if( now%12==0 && (flag<11||(flag<12&&ball==-2)) )
        {
            if(flag<6)  move(15+flag,0);
            else        move(9+flag,42);
            prints("[1;3%dm%s[36m",run+1,racename[run]);
            now=random();
            switch(now%14)
            {
            case 0:
                prints("±Ð©x¦b«á­±°l¡A³t«×x1.5[m");
                rate[run]*=1.5;
                break;
            case 1:
                prints("¨Ï¥XÀþ¶¡²¾°Ê¡A«e¶i¤­®æ[m");
                win=race_path(run,p[run]/100,5);
                p[run]+=500;
                break;
            case 2:
                prints("½ò¨ì¦a¹p¡A³t«×´î¥b[m");
                rate[run]/=2;
                break;
            case 3:
                prints("³Q²³¤H³ò¼Þ¡A¼È°±¤G¦¸[m");
                stop[run]+=2;
                break;
            case 4:
                prints("»E®ð¡A¼È°±¥|¦¸¡A³t«×¥[­¿[m");
                stop[run]+=4;rate[run]*=2;
                break;
            case 5:
                if(p[0]+p[1]+p[2]+p[3]+p[4]>8000)
                {
                prints("¥Ñ©ó¨t²Î²V¶Ã¡Aª½¹F²×ÂI[m");
                win=race_path(run,p[run]/100,(3100-p[run])/100);
                p[run]=3001;
                }
                else
                {
                prints("¨Ï¥Î¤Ñµ¾Às°{¡A¨Ï¨ä¥L¤H¼È°±¤T¦¸[m");
                for(i=0;i<5&&i!=run;i++)
                stop[i]+=3;
                }
                break;
            case 6:
                prints("¨Ï¥Î¤õ½b¼Q®g¾¹¡A³t«×+100[m");
                rate[run]+=100;
                break;
            case 7:
                prints("¨Ï¥Xªü­×Ã¹°{ªÅ¡A«e¶i¤T®æ¡A³t«×+30[m");
                win=race_path(run,p[run]/100,3);
                rate[run]+=30;
                break;
            case 8:
                prints("¦º¯«¤W¨­³t«×´î¥b¡A®ÇÃä¼È°±¤G¦¸[m");
                rate[run]/=2;
                if(run>0) stop[run-1]+=2;
                if(run<4) stop[run+1]+=2;
                break;
            case 9:
                prints("³Q ¯ó¤H´¡°w ¶A©G¡A¦^¨ì°_ÂI[m");
                win=race_path(run,p[run]/100,-30);
                break;
            case 10:
                if(p[0]+p[1]+p[2]+p[3]+p[4]>6000)
                {
                prints("[5m³Q±Ð©x§ì¥]¡A©Ò¦³¤H¦^¨ì°_ÂI[m");
                for(i=0;i<5;i++)
                win=race_path(i,p[i]/100,-30);
                }
                else
                {
                prints("¨Ï¥X¤j®¿²¾¡A³t«×x1.3¡A¨ä¥L¤H´î¥b[m");
                for(i=0;i<5&&i!=run;i++) rate[i]/=2;
                rate[run]*=1.3;
                }
                break;
            case 11:
                if(money[run+1]>0)
                {
                prints("³Q¹p¥´¨ì¡A¼È°±¤@¦¸[m");
                inmoney(10);
                stop[run]+=1;
                }
                else
                {
                prints("¸}©³©Ùªo¡A³t«×+50[m");
                rate[run]+=50;
                }
                break;
            case 12:
                while(run==now%5)
                now=random();
                prints("¥f¤W¤F[3%dm%s[36m¡A³t«×¸ò¥L¤@¼Ë",now%5+1,racename[now%5]);
                rate[run]=rate[now%5];
                break;
            case 13:
                if(money[run+1]>0)
                {
                prints("ªº½äª÷x1.5¡AÁÈ°Õ!!~[m");
                money[run+1]*=1.5;
                move(4,12);clrtoeol();
                for(i=1;i<6;i++)
                    prints("   [1;3%dm%5d[m ",i,money[i]);
                }
                else
                {
                prints("¾c¤l±¼¤F¡A°h«á¤T®æ[m");
                race_path(run,p[run]/100,-3);
                }
                break;
            }
            flag++;
        }
        else
        {
            move(7,2);
            clrtoeol();
            if(stop[run]>0)
            {
            prints("[1;3%dm%s[37m ª¦¤£°_¨Ó[m",run+1,racename[run]);
            stop[run]--;
            }
            else
            {
            prints("[1;3%dm%s[37m ©é©R©b¶][m",run+1,racename[run]);
            i=p[run]/100;
            win=race_path(run,i,(p[run]+rate[run])/100-i);
            p[run]+=rate[run];
            }
        }
        }
    }
    move(22,0);
    prints("  [1;35m¡¹[37m¹CÀ¸µ²§ô[35m¡¹[37m  Àò³Óªº¬O[3%dm%s    [37m",win+1,racename[win]);
    if(money[win+1]>0)
    {
        money[win+1]+=money[win+1]*(p[win]-(p[0]+p[1]+p[2]+p[3]+p[4])/5)/500;
        prints("®¥³ß§A©ã¤¤¤F¡AÀò±o¼úª÷ [32m%d[37m ¤¸[m",money[win+1]);
        inmoney(money[win+1]);
        game_log("RACE","[1;33mÀ£¤¤¤F %s ÁÈ¤F %d¤¸",racename[win],money[win+1]);
    }
    else
    {
        outs("©êºp...§A¨S©ã¤¤£¬~~~[m");
        game_log("RACE","[1;31m¤@­Ó³£¨S¦³À£¤¤°Õ!!");
    }
    pressanykey(NULL);
    }
}
