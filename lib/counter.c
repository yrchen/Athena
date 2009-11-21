#include "dao.h"

#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
void sem_init(int semkey,int *semid);
void sem_lock(int op,int semid);
void prints(const char *fmt, ... );

#define SEM_ENTER      -1      /* enter semaphore */
#define SEM_LEAVE      1       /* leave semaphore */
#define SEM_RESET      0       /* reset semaphore */
#define C_SEMKEY	2222

/* �p�ƾ� wildcat */

int counter_semid;

void
counter(filename,modes,n)
  char *filename;
  char *modes;
  int n;
{
  FILE *fp;
  unsigned long visited=0;

  sem_init(C_SEMKEY,&counter_semid);
  sem_lock(SEM_ENTER,counter_semid);

  if((fp = fopen(filename,"r")) != NULL)
  {
    fscanf(fp,"%lu",&visited);
    fclose(fp);
  }

  prints(" [1;44m [33m  �z�O%s��[36m %-10ld [33m�� %s ���ϥΪ�                            [m",
  n ? "����" : "���v�H��",++visited,modes);
  unlink(filename);

  if((fp = fopen(filename,"w")) != NULL)
  {
    fprintf(fp,"%ld",visited);
    fclose(fp);
  }
  sem_lock(SEM_LEAVE,counter_semid);
}
