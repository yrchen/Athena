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
  showtitle("�t�θ�T", BOARDNAME);

  prints("\n�z�{�b��� %s\033[1m%s\033[m (%s)\n", COLOR1, BOARDNAME, MYIP);
  
  load = cpuload(buf);
  prints("�t�έt�����p: %s (%s)\n", (load < 2 ? "�}�n" : (load < 5 ? "�|�i" : "�L��")), buf);
  prints("�ϥΪ��t�άO: %s\n", MYVERSION);
  prints("�u�W�A�ȤH��: %d/%d\n", utmpshm->number, MAXACTIVE);

  tbuf = dasht("bin/mbbsd");
  prints("�sĶ�ɶ�:     %s", ctime(&tbuf));
  
  if(( fp = fopen("src/.last_mbbsd", "r") ) != NULL)
  {
    fgets(buf, sizeof(buf), fp);
    fclose(fp);
    prints("�_�l�ɶ�:     %s", buf);
  }
  
  if(HAS_PERM(PERM_SYSOP))
  {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    prints("�O����ζq: sbrk: %d KB, idrss: %d KB, isrss: %d KB\n", 
      ((int)sbrk(0) - 0x8048000) / 1024,
      (int)ru.ru_idrss, (int)ru.ru_isrss);
  }
  
  pressanykey_old(NULL);
}

