#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/* ----------------------------------------------------- */
/* semaphore : for critical section                      */
/* ----------------------------------------------------- */

#define SEM_FLG        0600    /* semaphore mode */


void              /*  sem_init(BSEM_KEY,&ap_semid)  */
sem_init(int semkey,int *semid)
{
  union semun
  {
    int val;
    struct semid_ds *buf;
    ushort *array;
  }     arg =
  {
    1
  };
  
  *semid = semget(semkey, 1, 0);
  if (*semid == -1)
  {
    *semid = semget(semkey, 1, IPC_CREAT | SEM_FLG);
//    if (*semid == -1)
//      attach_err(semkey, "semget");
 
    semctl(*semid, 0, SETVAL, arg);
  }
}

void
sem_lock(int op,int semid)   /*  sem_lock(SEM_LEAVE,ap_semid)  */
{
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_flg = SEM_UNDO;
  sops.sem_op = op;
  semop(semid, &sops, 1);
}
