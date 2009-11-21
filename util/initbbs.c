/* $Id: initbbs.c 1096 2003-08-15 06:13:29Z victor $ */
#include "bbs.h"

static void initDir() 
{
    mkdir("adm",	0755);
    mkdir("home",	0755);
    mkdir("boards", 	0755);
    mkdir("etc", 	0755);
    mkdir("man", 	0755);
    mkdir("man/boards", 0755);
    mkdir("man/log",	0755);
//    mkdir("out", 	0755);
    mkdir("tmp", 	0755);
    mkdir("run", 	0755);
    mkdir("log", 	0755);
    mkdir("bin",	0755);
    mkdir("SO",		0755);
    mkdir("innd", 	0755);
    mkdir("game",	0755);
    mkdir("m2",		0755);

    mkdir("backup",	0755);
    mkdir("backup/bbs",	0755);
    mkdir("backup/bbs/brd", 0755);
    mkdir("backup/bbs/man", 0755);
    mkdir("backup/bbs/pwd", 0755);
    mkdir("backup/bbs/src", 0755);
    
}

static void initPasswds() 
{
    int i;
    userec u;
    FILE *fp;
    
    fp = fopen(FN_PASSWD, "w");
    memset(&u, 0, sizeof(u));
    if(fp) 
    {
	for(i = 0; i < MAXUSERS; i++)
	  fwrite(&u, sizeof(u), 1, fp);
	fclose(fp);
    }
}

static void initBoards() 
{
    int i;
    userec u;
    FILE *fp;
    
    fp = fopen(FN_BOARD, "w");
    
    memset(&u, 0, sizeof(u));
    if(fp) 
    {
	for(i = 0; i < MAXBOARD; i++)
	  fwrite(&u, sizeof(u), 1, fp);
	fclose(fp);
    }
}


int main(int argc, char **argv)
{
    if( argc != 2 || strcmp(argv[1], "-DoIt") != 0 )
    {
	fprintf(stderr,
		"警告!  initbbs只用在「第一次安裝」的時候.\n"
		"若您的站台已經上線,  initbbs將會破壞掉原有資料!\n\n"
		"確定要執行, 請使用 initbbs -DoIt\n");
	return 1;
    }

    if(chdir(BBSHOME)) 
    {
      perror(BBSHOME);
      exit(1);
    }

    initDir();
    initPasswds();
    initBoards();

    return 0;
}
