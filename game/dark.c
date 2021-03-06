/*******************************************/
/*    天地暗棋 2000/05/09                  */
/*     by dsyan.bbs@forever.twbbs.org      */
/* 特別感謝 kuang.bbs@cicada.twbbs.org ^^Y */
/*******************************************/

//找尋程式中有 //* 字樣的..改成適合自己的值

#include "bbs.h"
//#include <varargs.h>
#include <stdarg.h>
//#include <sys/socket.h>

#define ttt                 (180) //* 每一回合的時間限制 
#define red(a)              (a>64)
#define rank(a)             ((a%64)/2)
#define nexto(a,b)          (abs(a)+abs(b)==1)
#define nexto2(a,b)	    ((abs(a) && !abs(b)) || (!abs(a) && abs(b)))
#define mychess(y,x,t)      (red(chess[y][x])==t)
#define nexturn(t)          (abs(t-1))
#define darked(a)           (a%2)
#define max(a,b)            (a>b?a:b)
#define min(a,b)            (a<b?a:b)

char chess[8][4];
char nn[]="　將士象車馬包卒帥仕相硨傌炮兵";

printtt(int y, int x, char *fmt, ...)
{
 extern screenline* big_picture;
 va_list ap;
 char buff[512];

 va_start(ap, fmt);
 vsprintf(buff, fmt, ap);
 va_end(ap);

 memcpy(&(big_picture[y].data[x]), buff, strlen(buff));
 big_picture[y].mode |= MODIFIED;
}

show_chess(int y, int x, char a, char b)
{
 char nn1[42][7]={
"★★★",
" ★★ ",
"  將  ",

"   �� ",
" 士�� ",
"   �� ",

"╭─╮",
"象 .│",
"  ╰ˋ",

"    車",
"╭Π╮",
"◎�龤�",

"馬  ▲",
"╭�灨�",
" ⊥⊥ ",

" △包 ",
" █   ",
"||||  ",

"  ●卒",
"╰�屣�",
" ╱╲ ",

"★★★",
" ★★ ",
"  帥  ",

"   �� ",
" 仕�� ",
"   �� ",

"╭─╮",
"相 .│",
"  ╰ˋ",

"    硨",
"╭Π╮",
"◎�龤�",

"傌  ▲",
"╭�灨�",
" ⊥⊥ ",

" △炮 ",
" █   ",
"||||  ",

"  ●兵",
"╰�屣�",
" ╱╲ "};

 y=y*24+20; 
 x=x*4+2;
 
 if(b) 
 {
   char p,i;
   p=(red(a)*7 + (a%64)/2-1)*3; 
   for(i=0; i<3; i++) printtt(x+i,y,"3%dm%s",red(a),nn1[i+p]);
 }
 else 
 {
   char *ptr;
   ptr = nn + red(a)*14 + a%64 - a%2;
   printtt(x  ,y,"3%dm%s", red(a), a?"╭─╮":"      ");
   printtt(x+1,y,"3%dm%s%c%c%s", 
      red(a), a?"│":"  ", *ptr, *(ptr+1), a?"│":"  ");
   printtt(x+2,y,"3%dm%s", red(a), a?"╰─╯":"      ");
 }
 refresh();
}

change_record(char c)
{
 int a[3];
 char genbuf[250];
 FILE *fs;
 
// sprintf(genbuf, BBSHOME "/home/%s/.dark", currutmp->userid);
 sethomefile(genbuf, currutmp->userid, ".dark");
 if(fs=fopen(genbuf, "r")) {
   fscanf(fs, "%d %d %d", &a[0], &a[1], &a[2]);
   fclose(fs); 
 } else a[0]=a[1]=a[2]=0;
 a[c]++;

 if(fs=fopen(genbuf, "w")) {
   fprintf(fs, "%d %d %d", a[0], a[1], a[2]);
   fclose(fs); 
 } else pressanykey("資料寫入有誤..請通知站長.. :)");
}

cancel_mark(char yy, char xx, char *mark, int *key)
{
 show_chess(yy, xx, chess[yy][xx], 0);
 *mark=0; *key=0;
}

int show_eaten(char a, char b, char c)
{
 char *ptr;
 ptr = nn + red(a)*14 + a%64 - a%2;
 a=b/8;
 if(c) {
   printtt(a+21, (b%8)*2+87-a*32,"%c%c", *ptr, *(ptr+1));
   printtt(21,78,"%2d",b+1);
 }
 else {
   printtt(a+21, (b%8)*2+26-a*11,"%c%c", *ptr, *(ptr+1));
   printtt(21,17,"%2d",b+1);
 }
 refresh();
 return b==15;
}

start(user_info *uin)
{
 char x,y,xx,yy,tmp,genbuf[255],n[8][4]=
       {{3,5,5,7},{7,9,9,11},{11,13,13,15},{15,15,15,15},
        {67,69,69,71},{71,73,73,75},{75,77,77,79},{79,79,79,79}};
 int i,j,k;
 FILE *fs;

 clear();
 setutmpmode(DARK);
 showtitle("天地暗棋", BOARDNAME);

 move(1,0);
 fs=fopen("game/dark.i","r"); //* 改成自己的路徑
 while (fgets(genbuf, 254, fs)) prints(genbuf);
 fclose(fs);

 sprintf(genbuf, BBSHOME "/home/%s/.dark", currutmp->userid);
  if(fs=fopen(genbuf, "r")) {
   fscanf(fs, "%d %d %d", &i, &j, &k);
   fclose(fs); 
 } else i=j=k=0;
 printtt(19, 26, "%-13s", currutmp->userid);
 sprintf(genbuf,"%d-%d-%d",i,j,k);
 strncat(genbuf,"        ",13-strlen(genbuf));
 printtt(20, 26, "%s", genbuf);

 sprintf(genbuf, BBSHOME "/home/%s/.dark", uin->userid);
 if(fs=fopen(genbuf, "r")) {
   fscanf(fs, "%d %d %d", &i, &j, &k);
   fclose(fs);
 } else i=j=k=0;
 printtt(19, 87, "%-13s", uin->userid);
 sprintf(genbuf,"%d-%d-%d",i,j,k);
 strncat(genbuf,"        ",13-strlen(genbuf));
 printtt(20, 87, "%s", genbuf);
 refresh();

 memcpy(chess,n,strlen((char *)n));
 for(i=0;i<30000;i++) {
  x=random()%4; xx=random()%4;
  y=random()%8; yy=random()%8;
  tmp=chess[yy][xx];
  chess[yy][xx]=chess[y][x];
  chess[y][x]=tmp;
 }
}

/* move chess 就等於吃棋子 */
move_chess(char y, char x, char yy, char xx, char *mark, char *turn)
{
 chess[y][x]=chess[yy][xx];
 chess[yy][xx]=0;
 show_chess(y, x, chess[y][x], 0);
 show_chess(yy, xx, 0, 0);
 *mark=0;
 *turn=nexturn(*turn);
}

show_cursor(int y, int x, char a)
{
 char i;

 y=y*24+18;
 x=x*4+2;
 
 for(i=0; i<3; i++) printtt(x+i, y, "%d", 7-a);
 refresh();
}

show_color(char color)
{
 char i;
 for(i=19; i<22; i++) {
   printtt(i,8,"47;3%d",color);
   printtt(i,69,"47;3%d",1-color);
 }
}

int
dark(int fd, user_info *uin, int t)
                                      {
 char x=0,y=0,turn=0,xx,yy,mark=0,color=-1;
 char data[100],shown[2]={0,0},result=0,peace=0;
 time_t rest,init=time(0);
 int key,datac;
//       screenline *screen; //for save the scre
 
 data[0]=0;
 start(uin);
 add_io(fd, 0);
 t=t?1:0;
 
 if(!t) { /* 兩邊棋盤資料同步 */
  for(yy=0; yy<8; yy++)
   strncat(data, chess[yy], 4);
  do {
   datac=send(fd, data, strlen(data), 0);
  } while(datac<0);
 }
 else {
  do {
   key=igetkey();
   if (key == I_OTHERDATA) {
     datac= recv(fd, data, sizeof(data), 0);
     if(datac<=0) {add_io(0, 0); return;}
     for(yy=0; yy<8; yy++)
      strncpy(chess[yy], data+yy*4, 4);
   }
  } while(key != I_OTHERDATA);
 }
 move(18,0); clrtoeol();
 
 do {
  printtt(19, 56, "%s",t==turn?"自己":"對方"); 
  refresh();
  rest=time(0);
  
  do {
    if(t==turn) {
      show_cursor(y, x, 1);
      move(18, 0);
    }
    key=time(0)-init;
    datac=ttt+rest-time(0);

    if(datac<1 && turn==t) {
      change_record(1);
      add_io(0,0); bell();
      pressanykey("時間到囉!!..這局輸囉.. :~(");
      return 0;
    }
    else if(datac<1) {
      change_record(0);
      add_io(0,0); bell();
      pressanykey("對方可能睡著啦~ so算你贏了!! ^^Y");
      return 0;
    }
    
    move(22,34); 
    prints("%2d:%2d:%2d",key/3600,(key%3600)/60,key%60);
    printtt(21,47,"4%d;1m  %d:%2d",datac<30?1:4,datac/60,datac%60);
    move(b_lines,0); clrtoeol();
    prints("[45;37m 說明 [47;31m [←↑↓→/jikl][30m移動 [31m\
[Enter/Space][30m確定 [31m[w][30m傳訊 [31m[p][30m和棋 [31m[q]\
[30m投降 [31m[h][30mHelp [m");
    refresh();
    key=igetkey();
    peace=0;

     if(key==I_OTHERDATA) {
       bell();
       datac= recv(fd, data, sizeof(data), 0);
       if(datac<=0) {add_io(0, 0); return;}
       
       if(*data=='P') {
         getdata(18, 0, "對方提出和棋的要求..答應和棋嗎?? (y/n) [n]", data, 4, LCECHO, 0);
         move(18,0); clrtoeol();
         do {
          datac=send(fd, data, 1, 0);
         } while(datac<0);
         if(*data=='y') {
           change_record(2);
           add_io(0, 0);
           pressanykey("雙方握手言和 :)");
           return 0;
         }
         key=0;
       }
       else if(*data=='Q') {
         change_record(0);
         add_io(0, 0);
         pressanykey("耶耶耶..對方投降了.. ^^Y");
         return 0;
       }
       else {
         yy=data[0]-1; xx=data[1]-1;
         y=data[2]-1; x=data[3]-1;

         if(darked(chess[y][x])) { /* 對方翻棋 */
           if(color<0) {
             color=nexturn(red(chess[y][x]));
             show_color(color);
           }
           show_chess(y, x, chess[y][x], 0);
           chess[y][x]--;
           turn=nexturn(turn);
         }
         else {
           if(chess[y][x]) 
             result=show_eaten(chess[y][x],shown[t]++,0);
           move_chess(y,x,yy,xx,&mark,&turn);
         }
       }
     }

   switch(key) {
     case 'h':
      {
       extern screenline* big_picture;
       screenline* screen0 = calloc(t_lines, sizeof(screenline));
       memcpy(screen0, big_picture, t_lines * sizeof(screenline));
       more("game/dark.help",YEA); //* 改成自己的路徑
       memcpy(big_picture, screen0, t_lines * sizeof(screenline));
       free(screen0);
       redoscr();
      }
       break;
     case 'w':
      {
       extern screenline* big_picture;
       screenline* screen0 = calloc(2, sizeof(screenline));
       memcpy(screen0, big_picture, 2 * sizeof(screenline));
       my_write(uin->pid, "傳訊息: ");
       memcpy(big_picture, screen0, 2 * sizeof(screenline));
       free(screen0);
       redoscr();
      }
       break;
     case 'p':
       if(t==turn) {
         int keyy;
         if(peace) break;
         bell();
         getdata(18, 0, "真的要和棋嗎?? (y/n) [n]", data, 4, LCECHO, 0);
         move(18, 0); clrtoeol(); 
         if(*data!='y') break;
         prints("[33;1m詢問對手是否同意和棋..[5m請稍後... :)[m"); refresh();
         peace=1;
         do {
          datac=send(fd, "P", 1, 0);
         } while(datac<0);

         do {
           keyy=igetkey();
           if(keyy==I_OTHERDATA) {
             datac= recv(fd, data, sizeof(data), 0);
             if(datac<=0) {add_io(0, 0); return 0;}
             move(18, 0); clrtoeol(); 
             if(*data=='y') {
               change_record(2);
               add_io(0, 0);
               pressanykey("雙方握手言和 :)");
               return 0;
             }
             else 
               pressanykey("對方不同意和棋.. :~");
           }
         } while (keyy!=I_OTHERDATA);
       }
       break;
     case 'q':
       if(t==turn) {
         bell();
         getdata(18, 0, "真的要投降嗎?? (y/n) [n]", data, 4, LCECHO, 0);
         move(18,0); clrtoeol();
         if(*data!='y') break;
         do {
          datac=send(fd, "Q", 1, 0);
         } while(datac<0);
         change_record(1);
         add_io(0, 0);
         pressanykey("啊啊啊..我不行了..投降~ ^^;");
         return 0;
       }
       break;
     default:
      if(t!=turn) key=0;
      break;
   }

   if(t!=turn) continue;
   switch(key) {
    case ' ':
    case 13:
     if(!mark && darked(chess[y][x])) { /* 翻棋 */
       if(color<0) {
        color=red(chess[y][x]);
        show_color(color);
       }
       show_chess(y, x, chess[y][x], 0);
       chess[y][x]--;
       turn=nexturn(turn);
     }
     
     else if(mark) { /* 已 mark 的處理 */
       if(!chess[y][x] && nexto(x-xx,y-yy)) /* 移動 */
        move_chess(y,x,yy,xx,&mark,&turn);
       else if(!mychess(y,x,color) && !darked(chess[y][x])) { /* 吃對方的棋子 */
         if(nexto2(x-xx,y-yy) && mark==6) { /* mark 起來的棋子不在隔壁(炮) */
          char i,tmp=0;
          if(x==xx)
           for(i=min(y,yy)+1; i<max(y,yy); i++)
            tmp+=(chess[i][x]!=0);  
          else
           for(i=min(x,xx)+1; i<max(x,xx); i++)
            tmp+=(chess[y][i]!=0);  
          if(tmp==1 && chess[y][x]) {
            result=show_eaten(chess[y][x],shown[nexturn(t)]++,1);
            move_chess(y,x,yy,xx,&mark,&turn);
          }
          else cancel_mark(yy,xx,&mark,&key);
         }
         else { /* mark 起來的棋子在隔壁 */
           if( nexto(x-xx,y-yy) &&
              ((mark==7 && rank(chess[y][x])==1) || /* 小兵立大功 */
               ( mark<=rank(chess[y][x]) &&
                 rank(chess[y][x])-mark!=6  ))  ) { /* 將軍怕小兵 */
             result=show_eaten(chess[y][x],shown[nexturn(t)]++,1);
             move_chess(y,x,yy,xx,&mark,&turn);
           }
           else cancel_mark(yy,xx,&mark,&key);
         }
       }
       else cancel_mark(yy,xx,&mark,&key); /* 不能移不能吃 */
     }
     
     /* mark 起來 */     
     else if(!mark && chess[y][x] && mychess(y,x,color)) { /* mark 起來 */
       show_chess(y, x, chess[y][x], 1);
       mark=rank(chess[y][x]);
       xx=x; yy=y;
       key=0;
     }
     else key=0;
     break;
     
    case 'u':
     if(mark) cancel_mark(yy, xx, &mark, &key);
     break;
    case KEY_LEFT:
    case 'j':
     show_cursor(y, x, 0);
     y=y==0?7:y-1;
     break;
    case KEY_RIGHT:
    case 'l':
     show_cursor(y, x, 0);
     y=y==7?0:y+1;
     break;
    case KEY_UP:
    case 'i':
     show_cursor(y, x, 0);
     x=x==0?3:x-1;
     break;
    case KEY_DOWN:
    case 'k':
     show_cursor(y, x, 0);
     x=x==3?0:x+1;
     break;
   } 
   
  } while(key!=' ' && key!=13 && key!=I_OTHERDATA);
  
  show_cursor(y, x, 0);
  
  if(t==turn) continue; /* turn已經動過..so是相反的值 */
  data[0]=yy+1; data[1]=xx+1; 
  data[2]=y+1; data[3]=x+1;
  do {
    datac=send(fd, data, 4, 0);
  } while(datac<0);
  
 } while(!result);
 add_io(0, 0);  
 if(!turn) 
 {
   FILE *fs;
   init=time(0)-init;
   if(fs=fopen("log/dark.log","a+")) //* 可改成自己想要的路徑
   {
     fprintf(fs,"%s win %s %d:%d\n",currutmp->userid,uin->userid,init/60,init%60);
     fclose(fs); 
   }
 }
 change_record(1);
 pressanykey("恭禧你%s了 ^^Y",t==turn?"輸":"贏");
}

int va_dark(va_list pvar)
{
 int fd;
 user_info *uin;
 int t;
 fd = va_arg(pvar, int);
 uin = va_arg(pvar, user_info *);
 t = va_arg(pvar, int);
 return dark( fd, uin,  t);
}
