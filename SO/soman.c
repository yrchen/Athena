/*-------------------------------------------------------*/
/* soman.c        ( AT-BBS/WD_hialan BBS    Ver 1.00 )   */
/*-------------------------------------------------------*/
/* target : so �~���޲z�M�� (so manager)                 */
/* create : 2003/07/22                                   */
/* author : hialan					 */
/*-------------------------------------------------------*/
#include "bbs.h"

//extern *currdirect;
extern int cmpfilename();
extern int list_move();

#if 0
struct fileheader
{
  char filename[FNLEN-1];       /* �ɮצW��	 	33 bytes*/
  char score;			/* ����                  1 bytes*/
  char savemode;                /* file save mode 	 1 bytes*/
  char owner[IDLEN + 2];        /* �{���i�J�I 		14 bytes*/
  char date[6];                 /* [02/02] or space(5)   6 bytes*/
  char title[TTLEN + 1];	/* �{������		73 bytes*/
  uschar filemode;              /* must be last field @ boards.c 1 bytes*/
};
#endif

static void 
somantitle(int tag)
{
  switch(tag)
  {
    case RC_FULL:
    {
      char buf[80];
  
      sprintf(buf,"%s [�u�W %d �H]",BOARDNAME, count_ulist());
      showtitle("�~���{��", buf);

      prints("[��/��]�W�U [PgUp/PgDn]�W�U�� [Home/End]���� [��][q]���}\n");
      prints("%s �s��    �~  ��  �{  ��  ��  �z                                                \033[m", COLOR3);
      break;
    }

    case RC_FOOT:
      move(b_lines, 0);
      prints("%s  �~���{��  %s                       ��������|PgUp|PgDn|Home|End)����  r)����  \033[m", 
             COLOR2, COLOR3);
      break;
  }
}

static void
soman_doent(int num, fileheader *soman, int row, char *barcolor)
{
  char buf[31];
  
  sprintf(buf, "%s:%s", soman->filename, soman->owner);
  
  move(row, 0);
  clrtoeol();
  prints("%5d %s%s\033[m",
       num, (barcolor) ? barcolor : "", soman->title);
}

/*--------------------------------------*/
/*  SO Manager key function             */
/*--------------------------------------*/
static int
soman_desc(fileheader *fhdr)
{
  int i=1;
  clear();
  getdata(i++, 0, "�п�J�~���ɮ�: ", fhdr->filename, 20, DOECHO, fhdr->filename);
  if(!fhdr->filename[0])
    return -1;
    
  getdata(i++, 0, "�п�J�{���n�J�I: ", fhdr->owner, 13, DOECHO, fhdr->owner);
  if(!fhdr->owner[0])
    return -1;
  
  getdata(i++, 0, "�п�J���~�����ԭz: ", fhdr->title, 60, DOECHO, fhdr->title);
  return 0;
}

static int
soman_add()
{
  fileheader hdr;
  char fname[PATHLEN];
  
  if(!HAS_PERM(PERM_SYSOP))
    return RC_NONE;
  
  hdr.filename[0]=hdr.owner[0]=hdr.title[0]='\0';
  if(soman_desc(&hdr) < 0)
    return RC_FULL;
  
  sprintf(fname, "etc/%s", FN_SOMAN);

  if (rec_add(fname, &hdr, sizeof(fileheader)) == -1)
    pressanykey("�t�εo�Ϳ��~! �гq������!");
  return RC_FULL;
}

static int
soman_exec(int ent, fileheader *fhdr, char *direct)
{
  if(dashf(fhdr->filename))
  {
    char buf[PATHLEN];
    void (*p)();
    
    sprintf(buf, "%s:%s", fhdr->filename, fhdr->owner);

    if((p = (void *) DL_get(buf)) == 0)
      return RC_FULL;
    
    (void)(p)();
  }
  else
    pressanykey("�~���{�����s�b�A�и߰ݯ���!!");
  
  return RC_FULL;
}

static int
soman_del(int ent, fileheader *fhdr, char *direct)
{
  if(!HAS_PERM(PERM_SYSOP))
    return RC_NONE;
  
  if(getans2(b_lines, 0, msg_sure, 0, 2, 'n') == 'y')
  {
    strcpy(currfile, fhdr->filename);
    delete_file(direct, sizeof(fileheader), ent, cmpfilename);    
  }
  return RC_FULL;
}

static int soman_edit(int ent, fileheader *fhdr, char *direct)
{
  fileheader fhtmp = *fhdr;

  soman_desc(&fhtmp);

  if(getans2(b_lines, 0, msg_sure, 0, 2, 'n') == 'y')
  {
    substitute_record(direct, &fhtmp, sizeof(*fhdr), ent);
    return RC_NEWDIR;
  }
  
  return RC_FULL;
}

static int soman_detial(int ent, fileheader *fhdr, char *direct)
{
  int i=0;
  clear();
  move(i++, 0);
  outs("�ɮצ�m: ");
  outs(fhdr->filename);
  
  move(i++, 0);
  outs("�{���n�J�I: ");
  outs(fhdr->owner);
  
  move(i++, 0);
  outs("�{������: ");
  outs(fhdr->title);
  
  pressanykey_old(NULL);
  return RC_FULL;
}

static struct one_key soman_comm[]={
  'r', soman_exec,  	     0, "����~���{���C", 0,
  'a', soman_add,   PERM_SYSOP, "�W�[�~���{���C", 0,
  'd', soman_del,   PERM_SYSOP, "�R���~���{���C", 0, 
  'c', soman_edit,  PERM_SYSOP, "�s��~���]�w�C", 0,
  'Q', soman_detial,PERM_SYSOP, "�~���{���ԲӤ��e�C", 0,
  'm', list_move,   PERM_SYSOP, "���ʦ�m�C", 0,
 '\0', NULL, 0, NULL, 0};

int soman()
{
  char fname[PATHLEN];
  
  sprintf(fname, "etc/%s", FN_SOMAN);
  i_read(GAME, fname, somantitle, soman_doent, soman_comm, 'a');
  
  return 0;
}

