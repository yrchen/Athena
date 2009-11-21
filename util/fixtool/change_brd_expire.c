#include "bbs.h"

#define NEWBRD	"BOARDS.NEW"
#define CHLOG	"log/chexpire.log"

int main()
{
  int fdr, fdw, type;
  FILE *fplog;
  boardheader bhdr;
  char *msg[]={
  "normal",
  "personal",
  "group",
  };

  chdir(BBSHOME);
  setuid(BBSUID);
  setgid(BBSGID);
  
  fdr=open(FN_BOARD , O_RDONLY);
  
  if(fdr == 0)
  {
    puts("faild!");
    return -1;
  }
  else
  {
    printf(" %d item!\n", rec_num(FN_BOARD, sizeof(bhdr)));
    getchar();
  }
  
  fplog = fopen(CHLOG, "w");
  fdw=open(NEWBRD, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  
  while(read(fdr, &bhdr, sizeof(bhdr)) == sizeof(bhdr))
  {
    if(!(bhdr.brdattr & BRD_PERSONAL) && !(bhdr.brdattr & BRD_GROUP) && bhdr.brdname[0])
    {
    	type = 0;
    	bhdr.maxpost = 1000;
    }
    else if(bhdr.brdattr & BRD_PERSONAL) 
    {
    	type = 1;
    	if(bhdr.maxpost < 8000)
    	  bhdr.maxpost = 8000;
    }
    else if(bhdr.brdattr & BRD_GROUP)
    {
    	type = 2;
    	if(bhdr.maxpost < 8000)
          bhdr.maxpost = 8000;    	                 
    }
    	
    fprintf(stdout,"name: %-20s\tmaxpost: %d\ttype: %s\n", bhdr.brdname, bhdr.maxpost, msg[type]);
    fprintf(fplog, "name: %-20s\tmaxpost: %d\ttype: %s\n", bhdr.brdname, bhdr.maxpost, msg[type]);

    write(fdw, &bhdr, sizeof(bhdr));
  }
  
  close(fdr);
  close(fdw);
  fclose(fplog);

  if(!f_mv(FN_BOARD, "BOARDS.OLD"))
  {  
    f_mv(NEWBRD, FN_BOARD);
    puts("board moved!");
  }
  
  return 0;
}
