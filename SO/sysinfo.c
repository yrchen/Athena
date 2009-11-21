#include "bbs.h"

extern struct UTMPFILE *utmpshm;

void sysinfo()
{
  time_t tbuf;
  char buf[80];
  int  load;
  FILE *fp;
  
  resolve_utmp();
  setutmpmode(SYSINFO);
  showtitle("系統資訊", BOARDNAME);

  prints("\n您現在位於 %s\033[1m%s\033[m (%s)\n", COLOR1, BOARDNAME, MYIP);
  
  load = cpuload(buf);
  prints("系統負載情況: %s (%s)\n", (load < 2 ? "良好" : (load < 5 ? "尚可" : "過重")), buf);
  prints("使用的系統是: %s\n", MYVERSION);
  prints("線上服務人數: %d/%d\n", utmpshm->number, MAXACTIVE);

  tbuf = dasht("bin/mbbsd");
  prints("編譯時間:     %s", ctime(&tbuf));
  
  if(( fp = fopen("src/.last_mbbsd", "r") ) != NULL)
  {
    fgets(buf, sizeof(buf), fp);
    fclose(fp);
    prints("起始時間:     %s", buf);
  }
  
  if(HAS_PERM(PERM_SYSOP))
  {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    prints("記憶體用量: sbrk: %d KB, idrss: %d KB, isrss: %d KB\n", 
      ((int)sbrk(0) - 0x8048000) / 1024,
      (int)ru.ru_idrss, (int)ru.ru_isrss);
  }
  
  pressanykey_old(NULL);
}

