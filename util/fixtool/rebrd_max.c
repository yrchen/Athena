//Âà´«¬ÝªO¤W­­
#include "bbs.h"

int reboard()
{
  int fdr, fdw;
  boardheader bh;
  
  fdr=open(BBSHOME"/.BOARDS", O_RDONLY);
  fdw=open(BBSHOME"/BOARDS.NEW", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  
  while(read(fdr,&bh,sizeof(boardheader))==sizeof(boardheader))
  {
    if((bh.brdattr & BRD_CLASS) || (bh.brdattr & BRD_GROUP))
      printf("Class Name:%s\t\tNo need!\n", bh.brdname);
    else
    {
      printf("Board Name:%s\t\tMaxpost:%d\n", bh.brdname, bh.maxpost);
    
      bh.maxpost = 8000;
    }

    write(fdw, &bh, sizeof(boardheader));
  }

  close(fdr);  
  close(fdw);
}

int main(int argc, char **argv)
{
  reboard();
}

