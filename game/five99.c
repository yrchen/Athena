//天地五子棋

#include "bbs.h"

#define gotoxy(x,y)	move(y,x)
#define put_box(x,y,z)	move(y+1,x*2+1)
#define legal(x, y)	((x>0 && x<21 && y>0 && y<21) ? -1 : 0)

char chess[21][21];
char repent[21*21][3];
char role[2]={1,2};
int sum=0,flag=0,n,nn,m,mm,fturn=0,attack[2]={1,1};

static void who(int x)
{
  gotoxy(50,5+x);
  prints("<%d> %s",x+1, role[x]==1?"玩家":"");
  if(role[x]==2)
    prints("%s",attack[x]?"湯姆":"瑪麗");
}

static int vv(int x,int y,int race,int style,int real)
{
  int k1,k2;
  if(chess[x-1][y-1]) 
    return 0;

  for(k1=1;k1<5;k1++)
  {
    int p=x+(style%3-1)*k1-1,q=y+(style/3-1)*k1-1;
    if(legal(p,q))
    {
      if(!chess[p][q]) 
        break;
      else if(chess[p][q]==abs(race-2)+1) 
      {
        k1+=real; 
        break;
      }
    }
    else
     break;
  }

  for(k2=1;k2<5;k2++)
  {
    int p=x-(style%3-1)*k2-1,q=y-(style/3-1)*k2-1;
    if(legal(p,q))
    {
      if(!chess[p][q]) 
        break;
      else if(chess[p][q]!=race) 
      {
        k2+=real; 
        break;
      }
    }
    else
      break;
  }
  return (k1+k2-2)>0?k1+k2-2:0;
}

static int put_chess(int x,int y,int z)
{
  int i,r=0;
  for(i=0;i<4;i++) 
    if(vv(x,y,z,i,0)>=4) r=z;
   
  move(y+1,x*2); 
  outs(z==1?"●":"○");

  chess[x-1][y-1]=z;
  repent[sum][0]=z;
  repent[sum][1]=x;
  repent[sum][2]=y;
  sum++;
  if(sum==441) r=-1;
  for(i=sum;i&&(i>=sum-5);i--)
  {
    gotoxy(48,i-sum+13);
    prints("第%d步: %s (%2d,%2d)  ",i,(repent[i-1][0]==1)?"●":"○",
      repent[i-1][1],repent[i-1][2]);
  }
  refresh();
  return r;
}

static int value(int x,int y,int race)
{
  char te_v[10],i;
  if(chess[x-1][y-1]) return -100;
  for(i=0;i<10;i++) te_v[i]=0;
  for(i=0;i<4;i++) te_v[vv(x,y,race,i,-1)]++;
  return
 (te_v[9]+te_v[8]+te_v[7]+te_v[6]+te_v[5]+te_v[4])*15+te_v[3]*9+te_v[2]*3+te_v[1]*2;
}

static int show_win(int hk)
{
  char buf[50];
//  gotoxy(48,16);

  if(hk>0) 
  {
    sprintf(buf ,"%s贏了！再來一場? ",!fturn?"●":"○");
    return getans2(16, 48, buf, 0, 2, 'y');
  }
  else 
    return getans2(16, 48, "平手！再來一場? ", 0, 2, 'y');

  return igetch();
}

static int think(int race)
{
  int i,j,k=0;
  int da_x=0,da_y,da_v=0,tmp;
  int haha[442][3];
  haha[0][0]=0;
  for(i=1;i<22;i++)
    for(j=1;j<22;j++)
    {
      /* enemy's situation */
      tmp=value(i,j,abs(race-2)+1);
      if(tmp>da_v) {da_x=i; da_y=j; da_v=tmp;}
      /* mine */
      tmp=value(i,j,race);
      if(tmp>haha[k][0])
      {
        k=0;
        haha[0][0]=tmp;
        haha[0][1]=i; haha[0][2]=j;
      }
      else if(tmp==haha[k][0])
      {
        k++;
        haha[k][0]=tmp;
        haha[k][1]=i; 
        haha[k][2]=j;
      }
    }

  i=random()%(k+1);
  gotoxy(63,15);
  if(haha[i][0]>7) bell();

  m=haha[i][1];
  n=haha[i][2];

  if((da_v>haha[i][0])||(da_v==haha[i][0]&&!attack[race]))
   {m=da_x; n=da_y;}

  put_box(mm,nn,0);
  put_box(m,n,race);
  mm=m; nn=n;
  gotoxy(48,15); prints("目前位置(%2d,%2d)",m,n);

  return put_chess(m,n,race);
}

static int fplayer(int race)
{
  int l;
    while(-1)
    {
      gotoxy(48,15); prints("目前位置(%2d,%2d)",m,n);
      put_box(mm,nn,0);
      put_box(m,n,race);
      mm=m; nn=n;
      l=igetkey();
      switch(l)
      {
//      case '1':
        case '2':
         if(role[l-'1']==1)
         {
           role[l-'1']=2; 
           attack[l-'1']=0;
         }
         else if(attack[l-'1']==0)
           attack[l-'1']=1;
         else
           role[l-'1']=1;

         who(l-'1');
         break;
        case 'q': return 2;
        case 'r': return 1;
        case 'b':
         if(sum>2)
         {
           int i;
           for(i=0;i<2;i++)
           {
             int x=repent[sum-1][1],y=repent[sum-1][2];
             put_chess(x,y,0);
             move(y+1,x*2); outs("┼");
             sum-=2;
           }
           for(i=sum;i>=sum-5;i--)
           {
             gotoxy(48,i-sum+13);
             if(i>0)
               prints("第%d步: %s (%2d,%2d)  ",i,(repent[i-1][0]==1)?"●":"○",
                 repent[i-1][1],repent[i-1][2]);
             else
               outs("                  ");
           }
         }
         break;
        case ' ':
         if(!chess[m-1][n-1])
         {
           int hk=put_chess(m,n,race);
           if(hk)
           {
             int dk = show_win(hk);
             if(dk=='n'||dk=='q') return 2;
             return 1;
           }
           return 0;
         }
         break;
        case KEY_DOWN:  //down
          if(legal(m,n+1)) n++; 
          break; 
        case KEY_UP:	//up
          if(legal(m,n-1)) n--; 
          break;
        case KEY_RIGHT:	//right
          if(legal(m+1,n)) m++; 
          break;
        case KEY_LEFT:	//left
          if(legal(m-1,n)) m--; 
          break;
      }
    }
}

int p_five()
{
   int i=0;
   srandom(time(0));

restart:
  setutmpmode(FIVE);
  showtitle("天地五子棋", BOARDNAME);

  outs("┌┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┬┐\n");
for(sum=0;sum<20;sum++)
  outs("├┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┼┤\n");
  outs("└┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┴┘\n");


   gotoxy(48,1); outs("=-=-=-=-=-=-=-=-=-=");
   gotoxy(48,2); outs("|   天地五子棋    |");
   gotoxy(48,3); outs("=-=-=-=-=-=-=-=-=-=");
   who(0); who(1);

   gotoxy(45,17); outs(" ↓↑→← >> 控制方向");
   gotoxy(45,18); outs("  空白鍵  >> 確定");
   gotoxy(45,19); outs("     2    >> 二號玩家換人");
   gotoxy(45,20); outs("     b    >> 悔棋");
   gotoxy(45,21); outs("     r    >> 重新開始");
   gotoxy(45,22); outs("     q    >> 離開");

   for(m=1;m<22;m++)
    for(n=1;n<22;n++)
      chess[m][n]=0;

   sum=fturn=0;
   m=n=mm=nn=11;
   for(;;)
   {
     if(role[fturn]==1)
     {
       int tmp=fplayer(fturn+1);
       if(tmp==1) goto restart;
       if(tmp==2) return;
     }
     else
     {
       int hk=think(fturn+1);
       if(hk)
       {
         i=show_win(hk);
         if(i=='q'||i=='n') return;
         goto restart;
       }
     }
     fturn=abs(fturn-1);
   }
}
