/*--------------------------------------------------------------*/
/* �}�G���ɽ޳�                  98'12/17   */
/*--------------------------------------------------------------*/
/* ���{���Ѥp�̧�(SugarII)                  */
/* idea���۩� "�դ�P�c�P" �ðѦ� badboy �� source ���s��g�X�� */
/* ���A�g�o���I��A�w��j�a���ըíק�A�p��������D�A�Υ���Q�k */
/* �B��ĳ���Ʊ��q���ڤ@�n�C   EMail:u861838@Oz.nthu.edu.tw    */
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
  for(i=0;i<step;i++) prints("��");
  prints("[0m");
  if(p[run]+step*100>3000) return run;
  else return -1;
}

int race()
{
  char bet[1],m1[8],ch,*racename[5]={"���W","���t","�Q��","�p��","���Z"};
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
    outs("  [1m�H�W�G[m    ");
    for(i=0;i<5;i++)
        prints(" [200m[%dm[%d;613m[1;%dm%d. %s[201m ",i+1442,i+649,i+31,i+1,racename[i]);
    outs("\n [m [1m�t�סG[m    \n  [1m����G[m    \n");
    outs("�]�Ы� [1;36mk[m ���A���ʪ��[�o�A�� [1;36mz[m �i��X�h��(�u���@�����|)�^");
    move(9,0);
    for(i=0;i<5;i++)
        prints("%d.[1;3%dm%s[m��\n",i+1,i+1,racename[i]);
    outs("�w�w�w���w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w�r�w�w��");
    do
    {
        do{
        getdata(0,0,"�A�n�����(1~5)[S]�}�l[Q]���}�G",bet,2,DOECHO,0);
        i=atoi(bet);
        if(i>0&&i<6)
        {
            getdata(1,0,"�n��h�ֽ��(1~10000)? �� Enter �����G",m1,6,1,0);
            if(atoi(m1)>cuser.silvermoney)
            pressanykey("�A���{��������!!~~:)");
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
                prints("[1m�A��X�h��[3%dm%s[37m����e�i�T���A�t��0[m",run+1,racename[run]);
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
                prints("�Щx�b�᭱�l�A�t��x1.5[m");
                rate[run]*=1.5;
                break;
            case 1:
                prints("�ϥX�������ʡA�e�i����[m");
                win=race_path(run,p[run]/100,5);
                p[run]+=500;
                break;
            case 2:
                prints("���a�p�A�t�״�b[m");
                rate[run]/=2;
                break;
            case 3:
                prints("�Q���H��ޡA�Ȱ��G��[m");
                stop[run]+=2;
                break;
            case 4:
                prints("�E��A�Ȱ��|���A�t�ץ[��[m");
                stop[run]+=4;rate[run]*=2;
                break;
            case 5:
                if(p[0]+p[1]+p[2]+p[3]+p[4]>8000)
                {
                prints("�ѩ�t�βV�áA���F���I[m");
                win=race_path(run,p[run]/100,(3100-p[run])/100);
                p[run]=3001;
                }
                else
                {
                prints("�ϥΤѵ��s�{�A�Ϩ�L�H�Ȱ��T��[m");
                for(i=0;i<5&&i!=run;i++)
                stop[i]+=3;
                }
                break;
            case 6:
                prints("�ϥΤ��b�Q�g���A�t��+100[m");
                rate[run]+=100;
                break;
            case 7:
                prints("�ϥX����ù�{�šA�e�i�T��A�t��+30[m");
                win=race_path(run,p[run]/100,3);
                rate[run]+=30;
                break;
            case 8:
                prints("�����W���t�״�b�A����Ȱ��G��[m");
                rate[run]/=2;
                if(run>0) stop[run-1]+=2;
                if(run<4) stop[run+1]+=2;
                break;
            case 9:
                prints("�Q ��H���w �A�G�A�^��_�I[m");
                win=race_path(run,p[run]/100,-30);
                break;
            case 10:
                if(p[0]+p[1]+p[2]+p[3]+p[4]>6000)
                {
                prints("[5m�Q�Щx��]�A�Ҧ��H�^��_�I[m");
                for(i=0;i<5;i++)
                win=race_path(i,p[i]/100,-30);
                }
                else
                {
                prints("�ϥX�j�����A�t��x1.3�A��L�H��b[m");
                for(i=0;i<5&&i!=run;i++) rate[i]/=2;
                rate[run]*=1.3;
                }
                break;
            case 11:
                if(money[run+1]>0)
                {
                prints("�Q�p����A�Ȱ��@��[m");
                inmoney(10);
                stop[run]+=1;
                }
                else
                {
                prints("�}���٪o�A�t��+50[m");
                rate[run]+=50;
                }
                break;
            case 12:
                while(run==now%5)
                now=random();
                prints("�f�W�F[3%dm%s[36m�A�t�׸�L�@��",now%5+1,racename[now%5]);
                rate[run]=rate[now%5];
                break;
            case 13:
                if(money[run+1]>0)
                {
                prints("�����x1.5�A�Ȱ�!!~[m");
                money[run+1]*=1.5;
                move(4,12);clrtoeol();
                for(i=1;i<6;i++)
                    prints("   [1;3%dm%5d[m ",i,money[i]);
                }
                else
                {
                prints("�c�l���F�A�h��T��[m");
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
            prints("[1;3%dm%s[37m �����_��[m",run+1,racename[run]);
            stop[run]--;
            }
            else
            {
            prints("[1;3%dm%s[37m ��R�b�][m",run+1,racename[run]);
            i=p[run]/100;
            win=race_path(run,i,(p[run]+rate[run])/100-i);
            p[run]+=rate[run];
            }
        }
        }
    }
    move(22,0);
    prints("  [1;35m��[37m�C������[35m��[37m  ��Ӫ��O[3%dm%s    [37m",win+1,racename[win]);
    if(money[win+1]>0)
    {
        money[win+1]+=money[win+1]*(p[win]-(p[0]+p[1]+p[2]+p[3]+p[4])/5)/500;
        prints("���ߧA�㤤�F�A��o���� [32m%d[37m ��[m",money[win+1]);
        inmoney(money[win+1]);
        game_log("RACE","[1;33m�����F %s �ȤF %d��",racename[win],money[win+1]);
    }
    else
    {
        outs("��p...�A�S�㤤��~~~[m");
        game_log("RACE","[1;31m�@�ӳ��S��������!!");
    }
    pressanykey(NULL);
    }
}
