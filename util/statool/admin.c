/*-------------------------------------------------------*/
/* util/topusr2.c        ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : 使用者 上站記錄/文章篇數 排行榜              */
/* create : 97/03/27                                     */
/* update : 95/03/31                                     */
/*-------------------------------------------------------*/

#undef MONTH
#define REAL_INFO
#undef HIDE
#define ADMIN_REPORT

#include "bbs.h"
#include "record.c"
int mannum;

struct binfo
{
  char  boardname[18];
  char  expname[28];
  int times;
  int sum;
  int post;
  int num;
  int del;
  int pur;
  int tag;
  usint attr;
} st[MAXBOARD];

int numboards=0;
int numgroups=0;
int numhide=0;
int numhideg=0;

int
apply_record(char *fpath, int (*fptr)(), int size)
{
  char abuf[512];
  FILE* fp;

  if (!(fp = fopen(fpath, "r")))
    return -1;

  while (fread(abuf, 1, size, fp) == size)
     if ((*fptr) (abuf) == QUIT) 
     {
        fclose(fp);
        return QUIT;
     }
  fclose(fp);
  return 0;
}

int
brd_cmp(b, a)
struct binfo *a, *b;
{
    if(a->sum!=b->sum)
            return (a->sum - b->sum);
    return a->times - b->times;
}

int
personal_cmp(b, a)
struct binfo *a, *b;
{
    return (((a->post*500)+(a->times*100)+(a->sum/3)) - ((b->post*500)+(b->times*100)+(b->sum/3)));
}

int
record_data(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].times++;
                        st[i].sum+=sec;
                        return;
                }
        }
        return ;
}

int
record_data2(board)
char *board;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].post++;
                        return;
                }
        }
        return ;
}

int
record_data3(board)
char *board;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].del++;
                        st[i].num++;
                        return;
                }
        }
        return ;
}

int
record_data4(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].pur++;
                        st[i].num+=sec;
                        return;
                }
        }
        return ;
}

int
record_data5(board,sec)
char *board;
int sec;
{
        int i;
        for(i=0;i<numboards;i++)
        {
                if(!strcmp(st[i].boardname,board))
                {
                        st[i].tag++;
                        st[i].num+=sec;
                        return;
                }
        }
        return ;
}






int
fillbcache(boardheader *fptr)
{
    if( numboards >= MAXBOARD )
        return 0;

    /* Gene */
#ifdef HIDE
    if((fptr->level != 0) && !(fptr->level & PERM_POSTMASK))
        return;
#endif
    if((fptr->level != 0) && !(fptr->level & PERM_POSTMASK))
        numhide++;

    if ((strstr(fptr->title, "Ω"))||(strstr(fptr->title, "Σ"))||
         (strstr(fptr->title, "Π")))
    {
      numgroups++;
      if((fptr->level != 0) && !(fptr->level & PERM_POSTMASK))
        numhideg++;
      return;
    }

    strcpy(st[numboards].boardname,fptr->brdname);
    strcpy(st[numboards].expname,fptr->title);
/*    printf("%s %s\n",st[numboards].boardname,st[numboards].expname); */
    st[numboards].times=0;
    st[numboards].sum=0;
    st[numboards].post=0;
    st[numboards].del=0;
    st[numboards].pur=0;
    st[numboards].num=0;
    st[numboards].tag=0;
    st[numboards].attr=fptr->brdattr;

    numboards++;
    return 0 ;
}

int
fillboard()
{
  apply_record(BBSHOME"/.BOARDS", fillbcache, sizeof(boardheader));
}

/*
char *
timetostr(i)
int i;
{
        static char str[30];
        int minute,sec,hour;

        minute=(i/60);
        hour=minute/60;
        minute=minute%60;
        sec=i&60;
        sprintf(str,"%2d:%2d\'%2d\"",hour,minute,sec);
        return str;
}
*/

struct manrec
{
  char userid[IDLEN+1];
  char username[23];
  char userid2[IDLEN+1];
  usint userlevel;
  usint userlevel2;
  ushort numlogins;
  ushort numposts;
  ushort numloginsto;
  ushort numpoststo;
  ushort numloginsyes;
  ushort numpostsyes;
  ushort messto;
  ushort messfrom;
};
typedef struct manrec manrec;
struct manrec allman[MAXUSERS];

userec aman;
int num;
FILE *fp,*fp1,*fp2,*fp3;



int
record_mess(char *name, int type)
{
  int i;
  int n;

  for(i=0;i<mannum;i++)
  {
    if (!strcmp(name, allman[i].userid))
    {
      if (type == 0) allman[i].messfrom++;
      else allman[i].messto++;
      return;
    }
  }
  return ;
}


int 
main(int argc, char **argv)
{
  FILE *inf3,*inf2,*inf,*fpf;
  int i,n;
  int numlog=0, numlog2=0, numpo=0, numpo2=0;
  int maxlog=0, maxlog2=0, maxpo=0, maxpo2=0;
  char maxlogid[IDLEN+1], maxpoid[IDLEN+1], maxlogid2[IDLEN+1], maxpoid2[IDLEN+1], temp[20];
  int userlog=0, userlog2=0, userpo=0, userpo2=0;
  time_t now = time(0);
  struct tm *ptime;
  char *progmode;
/*  FILE *op; */
  char buf[256], *p,bname[20];
/*  char date[80];
  int mode;
  int c[3]; */
  int max[3];
  unsigned int ave[3];
  int sec;
  int j,k;
  char timesbname[20];
  char sumbname[20];
  char uname_from[20];
  char uname_to[20];
  int messnum;
  int max_from=0;
  int max_to=0;
  int user_from=0;
  char setby[13];
  char setto[13];
  int act[27];                  /* 次數/累計時間/pointer */
  int user_to=0;
  int hour;
  int newreg=0;
  int numtalk=0, numchat=0, numnewb=0, numnameb=0, numdelb=0,numattrb=0,numprefix=0,numboardlog=0;
  int numdated=0, numclean=0, numsetb=0, numkill=0, numsuci=0;
  int numsetu=0, numsetself=0, numcdict=0, numfortune=0, numrailway=0;
// wildcat add
  int nummsgmenu=0, numbet=0, numfive=0, numgamble=0, nummine=0, numbbcall=0;
  int nummn=0, numpedit=0, numpcall=0, numpread=0, numdragon=0;
  int numrpgchoose=0, numrpguild=0, numrpgtop=0, numrpgtrain=0, numrpgset=0;
  int numrpgpk=0, numosong=0, numcatv=0, numvote=0, numvotedit=0;
  int numvotemake=0, numvbreply=0, numvbmake=0, numhint=0, numtetris=0;
  int nummj=0, numbig2=0, numchess=0, numbbsnet=0, numsetbm=0, numsetbp=0;
  int numspam=0, numxaxb=0, numchicken=0, numbj=0, numstock=0;
  int numdice=0, numgp=0, nummarie=0, numrace=0, numbingo=0;
  int numnine=0, numnfight=0, numchessmj=0, numsevencard=0;
  int num;
  int alltime=0;
  int alltimes=0;
  int allpost=0;
  int allnum=0;
  int alldel=0;
  int allpur=0;
  int alltag=0;
  int maxtoday=0;
  int numsysop=0;
  int numboard=0;
  int numaccount=0;
  int numchatroom=0;
  int numbm=0;
  int numsee=0;
  int numcloak=0;
  int numloginok=0;
  int guestnum=0;

  setuid(BBSUID);
  setgid(BBSGID);
  chdir(BBSHOME);

#ifdef MONTH
  if ((fp1 = fopen(BBSHOME "/adm/board.read", "r")) == NULL)
#else
  if ((fp1 = fopen(BBSHOME"/usboard", "r")) == NULL)
#endif
  {
    printf("cann't open usboard\n");
    /* return 1 */;
  }

  fillboard();
  while (fgets(buf, 512, fp1))
  {
    if ( !strncmp(buf, "USE", 3))
    {
      p=strstr(buf,"USE");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);

      if ( p = (char *)strstr(buf+25, "with: "))
        sec=atoi( p + 6);
      else
        sec=0;
        
      record_data(bname,sec);
    }
    
    if ( !strncmp(buf, "DEL", 3))
    {
      p=strstr(buf,"DEL");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    record_data3(bname);
    }

    if ( !strncmp(buf, "PUR", 3))
    {
      p=strstr(buf,"PUR");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+25, "with: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data4(bname,sec);
    }
    if ( !strncmp(buf, "TAG", 3))
    {
      p=strstr(buf,"TAG");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
    if ( p = (char *)strstr(buf+25, "with: "))
    {
      sec=atoi( p + 6);
    }
    else
        sec=0;
    record_data5(bname,sec);
    }


    if ( !strncmp(buf, "POS", 3))
    {
      p=strstr(buf,"POS");
      p+=4;
      p=strtok(p," ");
      strcpy(bname,p);
      record_data2(bname);
    }


   }
   /* qsort */
   ave[0]=0;
   ave[1]=0;
   ave[2]=0;
   max[1]=0;
   max[0]=0;
   max[2]=0;
   for(i=0;i<numboards;i++)
   {
        ave[0]+=st[i].times;
        ave[1]+=st[i].sum;
        ave[2]+=st[i].times==0?0:st[i].sum/st[i].times;
        if(max[0]<st[i].times)
        {
                max[0]=st[i].times;
                strcpy(timesbname, st[i].boardname);
        }
        if(max[1]<st[i].sum)
        {
                max[1]=st[i].sum;
                strcpy(sumbname, st[i].boardname);
        }
        if(max[2]<(st[i].times==0?0:st[i].sum/st[i].times))
        {
                max[2]=(st[i].times==0?0:st[i].sum/st[i].times);
        }
        alltime+=st[i].sum;
        alltimes+=st[i].times;
        alldel+=st[i].del;
        allpur+=st[i].pur;
        alltag+=st[i].tag;
        allpost+=st[i].post;
        allnum+=st[i].num;
   }
   numboards++;
   st[numboards-1].times=ave[0]/numboards;
   st[numboards-1].sum=ave[1]/numboards;
   strcpy(st[numboards-1].boardname,"Total");
   strcpy(st[numboards-1].expname,"總合");
   qsort(st, numboards, sizeof( st[0] ), brd_cmp);


  now = time(NULL);
  ptime = localtime(&now);
  fclose(fp1);

#ifdef MONTH
  inf = fopen(BBSHOME "/.PASSWDS.yes", "rb");
#else
  inf = fopen(BBSHOME "/.PASSWDS", "rb");
#endif

  if (inf == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    printf("1\n");
    /* exit(0) */;
  }

  for (i = 0; fread(&aman, sizeof(userec), 1, inf); i++)
  {
    strcpy(allman[i].userid, aman.userid);
    strncpy(allman[i].username, aman.username,23);
    allman[i].numloginsto = aman.numlogins;
    allman[i].numpoststo = aman.numposts;
    allman[i].userlevel = aman.userlevel;

#ifdef  HAVE_TIN
    allman[i].numposts += post_in_tin(allman[i].userid);
#endif
  }
  fclose(inf);

#ifdef MONTH
  inf2 = fopen(BBSHOME "/.PASSWDS.month", "rb");
#else
  inf2 = fopen(BBSHOME "/.PASSWDS.yes", "rb");
#endif
  if (inf2 == NULL)
  {
    printf("Sorry, the data is not ready.\n");
    printf("2\n");
    /* exit(0) */;
  }

  for (i = 0; fread(&aman, sizeof(userec), 1, inf2); i++)
  {
    strcpy(allman[i].userid2, aman.userid);
    allman[i].numloginsyes = aman.numlogins;
    allman[i].numpostsyes = aman.numposts;
    allman[i].userlevel2 = aman.userlevel;

#ifdef  HAVE_TIN
    allman[i].numposts += post_in_tin(allman[i].userid);
#endif
  }

  n=i-1;
  mannum=n;

  for (i = 0; i<=n; i++)
  {
    if (!strcmp(allman[i].userid, allman[i].userid2))
    {
      allman[i].numlogins = allman[i].numloginsto - allman[i].numloginsyes;
      allman[i].numposts = allman[i].numpoststo - allman[i].numpostsyes;
    }
    else
    {
      allman[i].numlogins = allman[i].numloginsto;
      allman[i].numposts = allman[i].numpoststo;
    }
    if (allman[i].numpoststo < allman[i].numpostsyes)
      allman[i].numposts = 0;
    if (allman[i].numloginsto < allman[i].numloginsyes)
      allman[i].numlogins = 0;
  }
  fclose(inf2);

#ifdef MONTH
    if ((fpf = fopen(BBSHOME "/adm/usies", "r")) == NULL)
#else
    if ((fpf = fopen(BBSHOME "/usies", "r")) == NULL)
#endif
  {
    printf("cann't open usies\n");
    /* return 1 */;
  }

    while (fgets(buf, 512, fpf))
  {
    hour = atoi(buf + 9);
    if (hour < 0 || hour > 23)
    {
      continue;
    }
    if (!(strncmp(buf +28 , "guest ", 6)))
      {
      guestnum++;
      }

    if (!(strncmp(buf +22, "APPLY", 5)))
      {
      newreg++;
      continue;
      }
    else if (!(strncmp(buf +22, "DATED", 5)))
      {
      numdated++;
      continue;
      }
    else if (!(strncmp(buf +22, "CLEAN", 5)))
      {
      numclean++;
      continue;
      }
    else if (!(strncmp(buf +22, "SUCI", 4)))
      {
      numsuci++;
      continue;
      }
    else if (!(strncmp(buf +22, "KILL", 4)))
      {
      numkill++;
      continue;
      }
    else if (!(strncmp(buf +22, "NewBoard", 8)))
      {
      numnewb++;
      continue;
      }
    else if (!(strncmp(buf +22, "DelBoard", 8)))
      {
      numdelb++;
      continue;
      }
    else if (!(strncmp(buf +22, "SetBoard", 8)))
      {
      numsetb++;
      continue;
      }
    else if (!(strncmp(buf +22, "NameBoard", 9)))
      {
      numnameb++;
      continue;
      }
    else if (!(strncmp(buf +22, "ATTR_Board", 10)))
      {
      numattrb++;
      continue;
      }
    else if (!(strncmp(buf +22, "PREFIX", 6)))
      {
      numprefix++;
      continue;
      }
    else if (!(strncmp(buf +22, "BOARDLOG", 8)))
      {
      numboardlog++;
      continue;
      }

    else if (!(strncmp(buf +22, "CHAT ", 5)))
      {
      numchat++;
      continue;
      }
    else if (!(strncmp(buf +22, "TALK ", 5)))
      {
      numtalk++;
      continue;
      }
    else if (!(strncmp(buf +22, "FORTUNE", 7)))
      {
      numfortune++;
      continue;
      }
    else if (!(strncmp(buf +22, "RAILWAY", 7)))
      {
      numrailway++;
      continue;
      }
    else if (!(strncmp(buf +22, "CDICT", 5)))
      {
      numcdict++;
      continue;
      }
    else if (!(strncmp(buf +22, "BBCALL", 6)))
      {
      numbbcall++;
      continue;
      }
    else if (!(strncmp(buf +22, "MSGMENU", 7)))
      {
      nummsgmenu++;
      continue;
      }
    else if (!(strncmp(buf +22, "BET", 3)))
      {
      numbet++;
      continue;
      }
    else if (!(strncmp(buf +22, "FIVE", 4)))
      {
      numfive++;
      continue;
      }
    else if (!(strncmp(buf +22, "GAMBLE", 6)))
      {
      numgamble++;
      continue;
      }
    else if (!(strncmp(buf +22, "MINE", 4)))
      {
      nummine++;
      continue;
      }
    else if (!(strncmp(buf +22, "MN", 2)))
      {
      nummn++;
      continue;
      }
    else if (!(strncmp(buf +22, "PCALL", 5)))
      {
      numpcall++;
      continue;
      }
    else if (!(strncmp(buf +22, "PREAD", 5)))
      {
      numpread++;
      continue;
      }
    else if (!(strncmp(buf +22, "DRAGON", 6)))
      {
      numdragon++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_Choose", 10)))
      {
      numrpgchoose++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_Guild", 9)))
      {
      numrpguild++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_Toplist", 11)))
      {
      numrpgtop++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_Train", 9)))
      {
      numrpgtrain++;
      continue;
      }
    else if (!(strncmp(buf +22, "SetRPG", 6)))
      {
      numrpgset++;
      continue;
      }
    else if (!(strncmp(buf +22, "RPG_PK", 6)))
      {
      numrpgpk++;
      continue;
      }
    else if (!(strncmp(buf +22, "OSONG", 5)))
      {
      numosong++;
      continue;
      }
    else if (!(strncmp(buf +22, "CATV", 4)))
      {
      numcatv++;
      continue;
      }
    else if (!(strncmp(buf +22, "VOTE", 4)))
      {
      numvote++;
      continue;
      }
    else if (!(strncmp(buf +22, "VOTE_Edit", 9)))
      {
      numvotedit++;
      continue;
      }
    else if (!(strncmp(buf +22, "VOTE_Make", 9)))
      {
      numvotemake++;
      continue;
      }
    else if (!(strncmp(buf +22, "VB_Reply", 8)))
      {
      numvbreply++;
      continue;
      }
    else if (!(strncmp(buf +22, "VB_Make", 7)))
      {
      numvbmake++;
      continue;
      }
    else if (!(strncmp(buf +22, "HINT", 4)))
      {
      numhint++;
      continue;
      }
    else if (!(strncmp(buf +22, "TETRIS", 6)))
      {
      numtetris++;
      continue;
      }
    else if (!(strncmp(buf +22, "MJ", 2)))
      {
      nummj++;
      continue;
      }
    else if (!(strncmp(buf +22, "BIG2", 4)))
      {
      numbig2++;
      continue;
      }
    else if (!(strncmp(buf +22, "CHESS", 5)))
      {
      numchess++;
      continue;
      }
    else if (!(strncmp(buf +22, "BBSNET", 6)))
      {
      numbbsnet++;
      continue;
      }
    else if (!(strncmp(buf +22, "SetBoardBM", 10)))
      {
      numsetbm++;
      continue;
      }
    else if (!(strncmp(buf +22, "SetBrdPass", 10)))
      {
      numsetbp++;
      continue;
      }
    else if (!(strncmp(buf +22, "SPAM ", 5)))
      {
      numspam++;
      continue;
      }
    else if (!(strncmp(buf +22, "XAXB", 4)))
      {
      numxaxb++;
      continue;
      }
    else if (!(strncmp(buf +22, "CHICKEN", 7)))
      {
      numchicken++;
      continue;
      }
    else if (!(strncmp(buf +22, "BJ", 2)))
      {
      numbj++;
      continue;
      }
    else if (!(strncmp(buf +22, "STOCK", 5)))
      {
      numstock++;
      continue;
      }
    else if (!(strncmp(buf +22, "DICE", 4)))
      {
      numdice++;
      continue;
      }
    else if (!(strncmp(buf +22, "GP", 2)))
      {
      numgp++;
      continue;
      }
    else if (!(strncmp(buf +22, "MARIE", 5)))
      {
      nummarie++;
      continue;
      }
    else if (!(strncmp(buf +22, "RACE", 4)))
      {
      numrace++;
      continue;
      }
    else if (!(strncmp(buf +22, "BINGO", 5)))
      {
      numbingo++;
      continue;
      }
    else if (!(strncmp(buf +22, "NINE", 4)))
      {
      numnine++;
      continue;
      }
    else if (!(strncmp(buf +22, "NumFight", 8)))
      {
      numnfight++;
      continue;
      }
    else if (!(strncmp(buf +22, "CHESSMJ", 7)))
      {
      numchessmj++;
      continue;
      }
    else if (!(strncmp(buf +22, "SEVENCARD", 9)))
      {
      numsevencard++;
      continue;
      }
    else if (!strncmp(buf + 22, "ENTER", 5))
    {
      act[hour]++;
      continue;
    }
    else if (!strncmp(buf + 22, "SetUser", 7))
    {
      p=strstr(buf,"SetUser");
      p+=8;
      p=strtok(p," ");
      strcpy(setby,p);
      p=strstr(buf, setby);
      p+=13;
      p=strtok(p," ");
      if (strstr(p,"\n"))
          p=strtok(p,"\n");
      strcpy(setto,p);
      if (strcmp(setto, setby))
        numsetu++;
      else
        numsetself++;
    }
    if (p = (char *) strstr(buf + 40, "Stay:"))
    {
      if (hour = atoi(p + 5))
      {
        act[24] += hour;
        act[25]++;
      }
      continue;
    }
  }
  fclose(fpf);

  if(fpf = fopen(BBSHOME"/.maxtoday", "r"))
  {
    fscanf(fpf, "%d", &maxtoday);
    fclose(fpf);
  }
  


  if ((fp = fopen(BBSHOME "/log/admin.log", "w")) == NULL)
  {
    printf("cann't open admin.log\n");
    /* return 0*/;
  }

  if ((fp1 = fopen(BBSHOME "/log/func.log", "w")) == NULL)
  {
    printf("cann't open func.log\n");
    /* return 0*/;
  }

  if((fp2 = fopen(BBSHOME "/log/board.log", "w")) == NULL)
  {
    printf("cann't open board.log\n");
    /* return 0*/;
  }

  if((fp3 = fopen(BBSHOME "/log/personal.log", "w")) == NULL)
  {
    printf("cann't open personal.log\n");
    /* return 0*/;
  }


  for(i=0; i<=n; i++)
  {
    numlog+=allman[i].numloginsto;
    numpo+=allman[i].numpoststo;
    numlog2+=allman[i].numlogins;
    numpo2+=allman[i].numposts;
    if (allman[i].numloginsto>0) userlog++;
    if (allman[i].numlogins>0) userlog2++;
    if (allman[i].numpoststo>0) userpo++;
    if (allman[i].numposts>0) userpo2++;
    if (allman[i].numloginsto>maxlog) {
      maxlog=allman[i].numloginsto;
      strcpy(maxlogid, allman[i].userid2);
    }
    if (allman[i].numpoststo>maxpo) {
      maxpo=allman[i].numpoststo;
      strcpy(maxpoid, allman[i].userid2);
    }
    if (allman[i].numlogins>maxlog2) {
      maxlog2=allman[i].numlogins;
      strcpy(maxlogid2, allman[i].userid2);
    }
    if (allman[i].numposts>maxpo2) {
      maxpo2=allman[i].numposts;
      strcpy(maxpoid2, allman[i].userid2);
    }
    if (allman[i].messfrom>0) user_from++;
    if (allman[i].messto>0) user_to++;
    if (allman[i].messfrom>max_from) {
      max_from=allman[i].messfrom;
      strcpy(uname_from, allman[i].userid2);
    }
    if (allman[i].messto>max_to) {
      max_to=allman[i].messto;
      strcpy(uname_to, allman[i].userid2);
    }
    if (allman[i].userlevel & PERM_SYSOP)
      {
       numsysop++;
       strcpy(temp, allman[i].userid2);
       printf("%s\n", temp);	//LB.2004/09/09
      }
    if (allman[i].userlevel & PERM_ACCOUNTS) numaccount++;
    if (allman[i].userlevel & PERM_BOARD) numboard++;
    if (allman[i].userlevel & PERM_CHATROOM) numchatroom++;
    if (allman[i].userlevel & PERM_BM) numbm++;
    if (allman[i].userlevel & PERM_SEECLOAK) numsee++;
    if (allman[i].userlevel & PERM_CLOAK) numcloak++;
    if (allman[i].userlevel & PERM_LOGINOK) numloginok++;
  }

  fprintf(fp, "    [1;31m%s[m %s 報告\n",
     BOARDNAME,Ctime(&now));
  printf("hialan test\n");             
  fprintf(fp, "\n");
  fprintf(fp, "    迄今 已有 [1;33m%10d[m 人上站過 [1;33m%10d[m 次, 平均每人 [1;33m%5d[m 次\n",
     userlog, numlog, numlog/userlog);
  printf("hialan test\n");             
  fprintf(fp, "    迄今 已有 [1;33m%10d[m 人發表過 [1;33m%10d[m 篇, 平均每人 [1;33m%5d[m 篇\n\n",
     userpo, numpo, numpo/userlog);
  printf("hialan test userlog2=%d, numlog2=%d\n", userlog2, numlog2);             
  fprintf(fp, "    今天 已有 [1;33m%10d[m 人上站過 [1;33m%10d[m 次, 平均每人 [1;33m%5d[m 次\n",
     userlog2, numlog2,/* numlog2/userlog2*/ 1);
  printf("hialan test\n");             
  fprintf(fp, "    今天 已有 [1;33m%10d[m 人發表過 [1;33m%10d[m 篇, 平均每人 [1;33m%5d[m 篇\n\n",
     userpo2, numpo2, /*numpo2/userlog2*/1);
  printf("hialan test\n");             
  fprintf(fp, "    今天 上站 最多次的人是 [1;33m%13s[m 有 [1;33m%10d[m 次\n",
     maxlogid2, maxlog2);
  printf("hialan test\n");             
  fprintf(fp, "    今天 發表 最多次的人是 [1;33m%13s[m 有 [1;33m%10d[m 篇\n",
     maxpoid2, maxpo2);
  printf("hialan test\n");             
  fprintf(fp, "    今天 讀版 [1;33m%8d[m 次 共 [1;33m%8d[m 分"
             " 平均每版 [1;33m%5d[m 人次 共 [1;33m%5d[m 分 \n",
     ave[0], ave[1]/60, ave[0]/numboards, ave[1]/(numboards*60));
  printf("hialan test\n");             
  fprintf(fp, "    今天 讀版 次數最高是 [1;33m%-13s[m 版 共 [1;33m%5d[m 次 一般版個數為 [1;33m%5d[m 個 \n"
              "    今天 讀版 時間最高是 [1;33m%-13s[m 版 共 [1;33m%5d[m 分 一般群組數為 [1;33m%5d[m 個\n\n",
     timesbname, max[0], numboards-1, sumbname, max[1]/60, numgroups);
  printf("hialan test\n");             
/*
  fprintf(fp, "    今天 總共有 [1;33m%6d[m 個訊息 其中 有 [1;33m%5d[m 個人發 有 [1;33m%5d[m 個人收\n"
              "    發最多的是 [1;33m%13s[m 有 [1;33m%4d[m 次"
              " 收最多的是 [1;33m%13s[m 有 [1;33m%4d[m 次\n\n",
     messnum, user_from, user_to, uname_from, max_from, uname_to, max_to);
*/
  fprintf(fp, "    今天 有 [1;33m%5d[m 個人註冊  有 [1;33m%5d[m 個 guest 上來過"
              " 全部花了 [1;33m%8d[m 分鐘\n"
              "    今天 最高有 [1;33m%5d[m 同時上站 平均有 [1;33m%5d[m 人上站\n",
     newreg, /* act[25]-numlog2 */  guestnum , act[24], maxtoday, act[24]/1440);
  printf("hialan test\n");             
  fprintf(fp, "    今天 有 [1;33m%5d[m 個帳號過期 有 [1;33m%5d[m 被清\n",
     numdated, numclean);
  printf("hialan test\n");        
  fprintf(fp, "\n    有 [1;33m%5d[m 個 有限制的 版 及 [1;33m%5d[m 個 有限制的 群組",
      numhide-numhideg, numhideg);
  printf("hialan test\n");        
  fprintf(fp, "\n    站長有 [1;33m%3d[m 人, 帳號總管有 [1;33m%3d[m 人, "
              "看版總管有 [1;33m%3d[m 人, 聊天室總管有 [1;33m%3d[m 人\n",
    numsysop, numaccount, numboard, numchatroom);
  printf("hialan test\n");            
  fprintf(fp, "    版主有 [1;33m%3d[m 人, 看見忍者有 [1;33m%3d[m 人, "
              "有隱身術有 [1;33m%3d[m 人, 完成註冊有 [1;33m%5d[m 人\n",
    numbm, numsee, numcloak, numloginok);

  printf("hialan test\n");        
  fprintf(fp1,"\
[1;46m                               各項功\能使用統計                                [m");
  fprintf(fp1,"\n \
聊天        %4d 次    聊天室      %4d 次\n \
自己改資料  %4d 次    被站長改資料%4d 次    看看板記錄  %4d 次\n \
新增看板    %4d 個    刪除看板    %4d 個    更改看板類別%4d 次\n \
設定看板    %4d 個    更改看板名稱%4d 次    更改看板屬性%4d 次\n \
站長設定板主%4d 次    設定看板密碼%4d 次    扶花落楓斬  %4d 次\n \
站長砍 User %4d 次    User 自殺   %4d 次    BBSNET      %4d 次\n \
火車時刻    %4d 次    個人運勢    %4d 次    電子字典    %4d 次\n \
BBCALL      %4d 次    通訊錄      %4d 次    記帳本      %4d 次\n \
點歌        %4d 次    電視節目查詢%4d 次    教學精靈    %4d 次\n \
投票        %4d 次    修改/觀看投票%3d 次    舉辦投票    %4d 次\n \
申請看板/板主%3d 次    回應申請    %4d 次\n \
答錄機留言  %4d 次    答錄機聽留言%4d 次\n \
",
      numtalk, numchat,numsetself, numsetu, numboardlog,
      numnewb, numdelb, numprefix,numsetb, numnameb,numattrb,
      numsetbm,numsetbp,numspam,
      numkill, numsuci,numbbsnet,
      numrailway, numfortune,numcdict,numbbcall,nummsgmenu,nummn,numosong,
      numcatv,numhint,
      numvote,numvotedit,numvotemake,numvbmake,numvbreply,
      numpcall,numpread);
  fprintf(fp1," \
瘋狂賭盤    %4d 次    五子棋      %4d 次    對對樂      %4d 次\n \
踩地雷      %4d 次    接龍        %4d 次    俄羅斯方塊  %4d 次\n \
網路麻將    %4d 次    大老二      %4d 次    象棋        %4d 次\n \
猜數字      %4d 次    電子雞      %4d 次    黑傑克      %4d 次\n \
股市        %4d 次    西巴拉      %4d 次    金撲克      %4d 次\n \
小瑪麗      %4d 次    賭馬        %4d 次    賓果        %4d 次\n \
九九        %4d 次    對戰猜數字  %4d 次    象棋麻將    %4d 次\n \
賭城七張    %4d 次\n \
RPG挑職業   %4d 次    RPG工會     %4d 次    RPG排行榜   %4d 次\n \
RPG訓練場   %4d 次    RPG設定user %4d 次    RPG對戰     %4d 次\n \
",numbet,numfive,numgamble,nummine,numdragon,numtetris,nummj,numbig2,
numchess,numxaxb,numchicken,numbj,numstock,numdice,numgp,nummarie,numrace,
numbingo,numnine,numnfight,numchessmj,numsevencard
,numrpgchoose,numrpguild,numrpgtop,numrpgtrain,numrpgset,numrpgpk);

/*------- wildcat : 分兩個檔紀錄 -------*/

  fprintf(fp2, "==>[1;32m 看板狀況報告 [33m%s[m\n",Ctime(&now));
  fprintf(fp2,"說明:時間->停留時間(秒) 人次->進板人次 刪除->被刪除篇數");

  fprintf(fp2,"\n[1;37;42m名次 %-15.15s%-28.28s %6s %4s %4s %3s %3s %3s[m\n",
                "討論區名稱","中文敘述","  時間","人次","刪除","POST","TAG","DEL");

 for(i=0;i<MAXBOARD;i++)
 {
   if(st[i].sum)
     fprintf(fp2,"[1;33m%4d[m %-15.15s%-28.28s [1;32m%6d [31m%4d [m%4d [1;36m%3d[m %3d %3d\n",
     i+1,st[i].boardname,st[i].expname,st[i].sum,st[i].times,st[i].num
        , st[i].post,st[i].tag,st[i].del);
 }
     fprintf(fp2,"[1;37;42m     %-15.15s%-28.28s %6d %4d %4d %3d %3d %3d\n",
     "Total","總合",alltime,alltimes,allnum,allpost,alltag,alldel);

  fprintf(fp3, "==>[1;32m 個人板排行榜 [33m%s[m\n",Ctime(&now));
  fprintf(fp3,"說明:時間->停留時間(秒) 人次->進板人次 POST->發表次數 分數->公式算出來的分數");

  fprintf(fp3,"\n[1;37;42m名次 %-15.15s%-28.28s %6s %4s %4s - %4s -   [m\n",
                "討論區名稱","中文敘述","  時間","人次","POST","得分");

 qsort(st, numboards, sizeof( st[0] ), personal_cmp);
 { int j=0;
 for(i=0;i<MAXBOARD;i++)
 {
   if(st[i].sum && st[i].attr & BRD_PERSONAL)
     fprintf(fp3,"[1;33m%4d[m %-15.15s%-28.28s [1;32m%6d [31m%4d [1;36m%4d - [1;33m%.2f[m  \n",
     ++j,st[i].boardname,st[i].expname,st[i].sum,st[i].times,st[i].post
        , (float)((st[i].post*500) + (st[i].times*100) +(st[i].sum/3))/100);
 }
 }
//     fprintf(fp3,"[1;37;42m     %-15.15s%-28.28s %6d %4d %4d %3d %3d %3d\n",
//     "Total","總合",alltime,alltimes,allnum,allpost,alltag,alldel);

  printf("numlog = %d\n", numlog);
  printf("numlog2 = %d\n", numlog2);
  printf("numpo = %d\n", numpo);
  printf("numpo2 = %d\n", numpo2);
  printf("userlog = %d\n", userlog);
  printf("userlog2 = %d\n", userlog2);
  printf("userpo = %d\n", userpo);
  printf("userpo2 = %d\n", userpo2);
  printf("Maxpost %s = %d\n", maxpoid, maxpo);
  printf("Maxlogin %s = %d\n", maxlogid, maxlog);
  printf("Maxpost2 %s = %d\n", maxpoid2, maxpo2);
  printf("Maxlogin2 %s = %d\n", maxlogid2, maxlog2);
  fclose(fp);
  fclose(fp1);
  fclose(fp2);
  fclose(fp3);
}
