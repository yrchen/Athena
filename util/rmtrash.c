/*-------------------------------------------------------*/
/* util/rmtrash.c                                        */
/*-------------------------------------------------------*/
/* target : �M���ӤH�ؿ������U���ɮ�                     */
/* create : 01/09/25                                     */
/* update :   /  /                                       */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/

#define RM_ALLPOST  //�R�� allpost/ �ؿ� by hialan

#if 0

  [�ت�] �M���ϥΪ̥ؿ��U���U���ɮ�

  [����] �{���h�R�� �ͤѰO�� talklog talk_xxxx
                    ��ѰO�� chat_xxxx
                    �ª��n�ͦW�� overrides
                    ��L�O�� bet_xxxx
                    �S���b .DIR �����H��
                    size = 0 ���ɮ�

  [��k] ��o�{����b src/util/rmtrash.c �U�A�å[�J src/util/Makefile �sĶ
         ���� rmtrash SYSOP �i�M�� SYSOP �ؿ����U���ɮ�
         ���� rmtrash       �i�M���Ҧ��ϥΪ̥ؿ����U���ɮ�

  [�Ƶ�] �q�`�n�]�ܤ[�A�p�G�U���ɮ׫ܦh�ΨϥΪ̤H�ƫܦh�ɡA�ýХ��ƥ� home
         �i�H�� find_dir() �өw�q�����ɮ׬O�U��
         �i�H�������]�S���Y

#endif


#include "bbs.h"

static int
_rec_get(fpath, data, size, pos)
  char *fpath;
  void *data;
  int size, pos;
{
  int fd;
  int ret;

  ret = -1;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    if (lseek(fd, (off_t) (size * pos), SEEK_SET) >= 0)
    {
      if (read(fd, data, size) == size)
       ret = 0;
    }
    close(fd);
  }
  return ret;
}


static int                      /* 1: �b .DIR ��  0: ���b .DIR �����U���H�� */
find_dir(userid, filename)
  char *userid;
  char *filename;
{
  char fpath[64];
  int pos;
  fileheader hdr;

  if (filename[0] == 't' ||                             /* talk_xxxx talklog */
    (filename[0] == 'c' && filename[2] == 'a') ||       /* chat_xxxx */
    (filename[0] == 'b' && filename[1] == 'e') ||       /* betxxx */
    filename[0] == 'o')                                 /* overrides */
  {
    return 0;                   /* �U�������R�� */
  }

  if (filename[0] != 'M')       /* ���O�H���ˬd */
    return 1;

  sprintf(fpath, BBSHOME "/home/%s/.DIR", userid);

  pos = 0;
  while (!_rec_get(fpath, &hdr, sizeof(fileheader), pos))
  {
    if (!strcmp(hdr.filename, filename))
      return 1;

    pos++;
  }

  return 0;
}


static void
reaper(userid, pos, fp)
  char *userid;
  int pos;
  FILE *fp;
{
  char fpath[64];
  int num;
  struct stat st;
  struct dirent *de;
  DIR *dirp;

  sprintf(fpath, BBSHOME "/home/%s", userid);

  if (!(dirp = opendir(fpath)))
  {
    printf("No such ID: %s\n", userid);
    return;
  }

  chdir(fpath);

  num = 0;

  while (de = readdir(dirp))
  {
    if (de->d_name[0] <= ' ' || de->d_name[0] == '.')
      continue;

    if ((!stat(de->d_name, &st) && (st.st_size == 0)) ||
      !find_dir(userid, de->d_name)) /* size == 0 �� ���b .DIR �� */
    {
      printf("%s/%s is deleted.\n", fpath, de->d_name);
      unlink(de->d_name);
      num++;
    }
  }

#ifdef RM_ALLPOST
  strcat(fpath, "/allpost");
  if (dirp = opendir(fpath))
  {
    char cmd[80];
    
    printf("Size of %s:\n", fpath);
    system("ls -l | grep \"allpost\"");    
    printf("\n");
    sprintf(cmd, "rm -rf %s", fpath);
    system(cmd);
    
    printf("[ID %d] %s %s deleted.\n", pos, userid, fpath);
  }
#endif

  printf("[ID %d] %s  -- Total deleted : %d files\n", pos, userid, num);
  if(fp)
    fprintf(fp, "[ID %d] %s  -- Total deleted : %d files\n", pos, userid, num);
}


int
main(argc, argv)
  int argc;
  char **argv;
{
  int pos;
  userec cuser;

  if (argc > 2)
  {
    printf("Usage: %s [ID]\n", argv[0]);
  }
  else if (argc == 2)
  {
    reaper(argv[1], 0, 0);
  }
  else
  {
    /* �p�G�]��@�b�Ȱ��F�άO���_�F�A�N�ݤW���]����ӼƦr
       �M��u�n��{�� pos = xxx ���s�sĶ�Y�i */

    FILE *fp;	//log �ɮ� by hialan
    fp = fopen(BBSHOME"/log/rmtrash.log", "w");
    
    pos = 0;        /* �Ѥ��~���_�ϥ� */
    while (!_rec_get(BBSHOME "/" FN_PASSWD, &cuser, sizeof(userec), pos))
    {
      reaper(cuser.userid, pos, fp);
      pos++;
    }
    printf("Total User: %d\n", pos);    /* �Ѥ��~���_�ϥ� */
    fprintf(fp, "Total User: %d\n", pos);
    fclose(fp);
  }
  exit(0);
}
