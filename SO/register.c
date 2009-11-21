/*-------------------------------------------------------*/
/* register.c   ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : user register routines                       */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"
#include "stdarg.h"

/* -------------------------------- */
/* New policy for allocate new user */
/* (a) is the worst user currently  */
/* (b) is the object to be compared */
/* -------------------------------- */

#undef VACATION     // �O�_���H�����O�d�b������
char *genpasswd(char *pw);
static int
compute_user_value(urec, clock)
  userec *urec;
  time_t clock;
{
  int value;

  /* if (urec) has XEMPT permission, don't kick it */
  if ((urec->userid[0] == '\0') || (urec->userlevel & PERM_XEMPT))
    return 9999;

  value = (clock - urec->lastlogin) / 60;       /* minutes */

  /* new user should register in 60 mins */
  if (strcmp(urec->userid, str_new) == 0)
    return (60 - value);

#ifdef  VACATION
  return 180 * 24 * 60 - value; /* �H�����O�s�b�� 180 �� */
#else
  if (!urec->numlogins)         /* �� login ���\�̡A���O�d */
    return -1;
  else if (urec->numlogins <= 3)     /* #login �֩�T�̡A�O�d 30 �� */
    return 30 * 24 * 60 - value;

  /* ���������U�̡A�O�d 30 �� */
  /* �@�뱡�p�A�O�d 180 �� */
  else
    return (urec->userlevel & PERM_LOGINOK ? 180 : 30) * 24 * 60 - value;
#endif
}


static int 
getnewuserid()
{
  static char *fn_fresh = ".fresh";
  extern struct UCACHE *uidshm;
  userec utmp, zerorec;
  time_t clock;
  struct stat st;
  int fd, val, i;
  char genbuf[200];
  char genbuf2[200];

  memset(&zerorec, 0, sizeof(zerorec));
  clock = time(NULL);

  /* -------------------------------------- */
  /* Lazy method : ����M�w�g�M�����L���b�� */
  /* -------------------------------------- */

  if ((i = searchnewuser(0)) == 0)
  {

    /* ------------------------------- */
    /* �C 1 �Ӥp�ɡA�M�z user �b���@�� */
    /* ------------------------------- */

    if ((stat(fn_fresh, &st) == -1) || (st.st_mtime < clock - 3600))
    {
      if ((fd = open(fn_fresh, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      write(fd, ctime(&clock), 25);
      close(fd);
      log_usies("CLEAN", "dated users");

      printf("�M��s�b����, �еy�ݤ���...\n\r");
      if ((fd = open(fn_passwd, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      i = 0;  /* Ptt�ѨM�Ĥ@�ӱb���ѬO�Q����D */
      while (i < MAXUSERS)
      {
        i++;
        if (read(fd, &utmp, sizeof(userec)) != sizeof(userec))
          break;
	if(i==1) continue;

        if ((val = compute_user_value(&utmp, clock)) < 0) 
        {
           sprintf(genbuf, "#%d %-12s %15.15s %d %d %d",
             i, utmp.userid, ctime(&(utmp.lastlogin)) + 4,
             utmp.numlogins, utmp.numposts, val);
           if (val > -1 * 60 * 24 * 365)
           {
             log_usies("CLEAN", genbuf);
             sprintf(genbuf, "home/%s", utmp.userid);
             sprintf(genbuf2, "tmp/%s", utmp.userid);
// wildcat : ���� mv , ���ζ] rm home/userid
             if (dashd(genbuf))
               f_mv(genbuf, genbuf2);
             lseek(fd, (off_t)((i - 1) * sizeof(userec)), SEEK_SET);
             write(fd, &zerorec, sizeof(utmp));
           }
           else
              log_usies("DATED", genbuf);
        }
      }
      close(fd);
      time(&(uidshm->touchtime));
    }
  }
  if ((fd = open(fn_passwd, O_RDWR | O_CREAT, 0600)) == -1)
    return -1;
  flock(fd, LOCK_EX);

  i = searchnewuser(1);
  if ((i <= 0) || (i > MAXUSERS))
  {
    flock(fd, LOCK_UN);
    close(fd);
    if (more("etc/user_full", NA) == -1)
      printf("��p�A�ϥΪ̱b���w�g���F�A�L�k���U�s���b��\n\r");
    val = (st.st_mtime - clock + 3660) / 60;
    printf("�е��� %d ������A�դ@���A���A�n�B\n\r", val);
    sleep(2);
    exit(1);
  }

  sprintf(genbuf, "uid %d", i);
  log_usies("APPLY", genbuf);

  strcpy(zerorec.userid, str_new);
  zerorec.lastlogin = clock;
  if (lseek(fd, (off_t)(sizeof(zerorec) * (i - 1)), SEEK_SET) == -1)
  {
    flock(fd, LOCK_UN);
    close(fd);
    return -1;
  }
  write(fd, &zerorec, sizeof(zerorec));
  setuserid(i, zerorec.userid);
  flock(fd, LOCK_UN);
  close(fd);
  return i;
}

#ifdef REG_FORM
/* --------------------------------------------- */
/* �ϥΪ̶�g���U���                            */
/* --------------------------------------------- */

static void
getfield(line, info, desc, buf, len)
  int line, len;
  char *info, *desc, *buf;
{
  char prompt[STRLEN];
  char genbuf[200];

  sprintf(genbuf, "����]�w�G%-30.30s (%s)", buf, info);
  move(line, 2);
  outs(genbuf);
  sprintf(prompt, "%s�G", desc);
  if (getdata(line + 1, 2, prompt, genbuf, len, DOECHO,0))
    strcpy(buf, genbuf);
  move(line, 2);
  prints("%s�G%s", desc, buf);
  clrtoeol();
}


int 
u_register()
{
  char rname[20], howto[50]="�нT���g";
  char phone[20], career[40], email[50],birthday[9],sex_is[2],year,mon,day;
  char ans[3], *ptr;
  FILE *fn;
  time_t now;
  char genbuf[200];
  
  if (cuser.userlevel & PERM_LOGINOK)
  {
    pressanykey("�z�������T�{�w�g�����A���ݶ�g�ӽЪ�");
    return XEASY;
  }
  if (fn = fopen(fn_register, "r"))
  {
    while (fgets(genbuf, STRLEN, fn))
    {
      if (ptr = strchr(genbuf, '\n'))
        *ptr = '\0';
      if (strncmp(genbuf, "uid: ", 5) == 0 &&
        strcmp(genbuf + 5, cuser.userid) == 0)
      {
        fclose(fn);
        pressanykey("�z�����U�ӽг�|�b�B�z���A�Э@�ߵ���");
        return XEASY;
      }
    }
    fclose(fn);
  }

  move(2, 0);
  clrtobot();
  strcpy(rname, cuser.realname);
  strcpy(email, cuser.email);
  sprintf(birthday, "%02i/%02i/%02i",
        cuser.year, cuser.month, cuser.day);
  sex_is[0]=(cuser.sex >= '0' && cuser.sex <= '7') ? cuser.sex+'1': '1';sex_is[1]=0;
  career[0] = phone[0] = '\0';
  while (1)
  {
    clear();
    move(3, 0);
    prints("%s[1;32m�i[m%s[1;32m�j[m �z�n�A�оڹ��g�H�U�����:(�L�ܧ�Ы�enter���L)",
      cuser.userid, cuser.username);
    getfield(6, "�нT���g����m�W", "�u��m�W", rname, 20);
    getfield(8, "�Ǯըt�ũγ��¾��", "�A�ȳ��", career, 40);
    getfield(10, "�]�A���~�����ϰ�X", "�s���q��", phone, 20);
    while (1)
    {
    int len;
    getfield(12, " 19xx/��/�� �p: 77/12/01","�ͤ�",birthday,9);
    len = strlen(birthday);
    if(!len)
       {
         sprintf(birthday, "%02i/%02i/%02i",
         cuser.year, cuser.month, cuser.day);
         year=cuser.year;
         mon=cuser.month;
         day=cuser.day;
       }
    else if (len==8)
       {
        year  = (birthday[0] - '0') * 10 + (birthday[1] - '0');
        mon = (birthday[3] - '0') * 10 + (birthday[4] - '0');
        day   = (birthday[6] - '0') * 10 + (birthday[7] - '0');
       }
    else
        continue;
    if (mon > 12 || mon < 1 || day > 31 || day < 1 || year > 90 || year < 40)
        continue;
    break;
    }
    getfield(14,"1.���� 2.�j�� 3.���} 4.����","�ʧO",sex_is,2);
    getfield(16, "�����{�ҥ�", "E-Mail Address", email, 50);
    getfield(18, "�q���䪾�D�o�ӯ���", "�q��o��", howto, 50);

    ans[0] = getans2(b_lines - 1, 0, "�H�W��ƬO�_���T�H ", 0, 3, 'n');
    if (ans[0] == 'q')
      return 0;
    if (ans[0] == 'y')
      break;
  }
  cuser.rtimes++;
  strcpy(cuser.realname, rname);
  strcpy(cuser.email, email);  
  cuser.sex= sex_is[0]-'1';
  cuser.month=mon;cuser.day=day;cuser.year=year;
#ifdef  REG_MAGICKEY
  mail_justify(cuser); //�{�ҽX
#endif      
  if (fn = fopen(fn_register, "a"))
  {
    now = time(NULL);
    str_trim(career);
    str_trim(phone);
    fprintf(fn, "num: %d, %s", usernum, ctime(&now));
    fprintf(fn, "uid: %s\n", cuser.userid);
    fprintf(fn, "name: %s\n", rname);
    fprintf(fn, "howto: %s\n", howto);
    fprintf(fn, "career: %s\n", career);
    fprintf(fn, "phone: %s\n", phone);
    fprintf(fn, "email: %s\n", email);
    fprintf(fn, "----\n");
    fclose(fn);
  }
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* �O�� */
  return 0;
}
#endif

/*--------------------------*/ 
/* mode==0 ���n�J���U(�w�]) */ 
/* mode==1 ���޲z�s�W�ϥΪ� */
/*--------------------------*/

void
new_register(int mode)
{
  userec newuser;
  char passbuf[STRLEN];
  char origname[IDLEN + 1];	/*hialan*/
  char genbuf[4];
  int allocid, try;

  strcpy(origname, cuser.userid);

  memset(&newuser, 0, sizeof(newuser));

  //�ϥ� guest ���ߦn�]�w
  strlcpy(cuser.userid, STR_GUEST, sizeof(cuser.userid));
  cuser.habit = HABIT_NEWUSER;  

#ifdef ATREGISTERMODE

  more("etc/register_announce", YEA);

  move(b_lines - 5, 0);
  outs(" \
�п�ܡG1) ���٦P�����[�����z���A�@�N�~��������U�{�ǡI\n \
        2) �ڤ��٦P�����z���A�{�b���W���}�I\n \
        3) �ڥ��� guest ���[�@�U�A���M�w�I");

  getdata(b_lines - 1, 0, "�ڪ��M�w�O�G[2]", genbuf, 3, DOECHO, 0);
  switch(genbuf[0])
    {
     case '1':
       break;
     case '3':
       pressanykey("�Э��s�� guest �n�J :)");
       oflush();
       exit(1);
     default:
       pressanykey("���°��[ :)");
       oflush();
       exit(1);
    }

  more("etc/register_visio", YEA);
  more("etc/copyright", YEA);

  if(getans2(b_lines, 0, "�аݬO�_�������������p�v�O�@�F�� ", 0, 2, 'n') != 'y')
  {
    pressanykey("���°��[ :)");
    oflush();
    exit(1);
  }
#endif

  more("etc/register", NA);
  try = 0;
  while (1)
  {
    if (++try >= 6)
    {
      refresh();

      pressanykey("�z���տ��~����J�Ӧh�A�ФU���A�ӧa");
      oflush();
      if(mode==1)
        return;
      exit(1);
    }
    getdata(16, 0, msg_uid, newuser.userid, IDLEN + 1, DOECHO,0);

    if (bad_user_id(newuser.userid))
      outs("�L�k�����o�ӥN���A�Шϥέ^��r���A�åB���n�]�t�Ů�\n");
    else if (searchuser(newuser.userid))
      outs("���N���w�g���H�ϥ�\n");
    else
      break;
  }

  try = 0;
  while (1)
  {
    if (++try >= 6)
    {
      pressanykey("�z���տ��~����J�Ӧh�A�ФU���A�ӧa");
      oflush();
      if(mode==1)
        return;
      exit(1);
    }
    if ((getdata(17, 0, "�г]�w�K�X�G", passbuf, PASSLEN, PASS,0) < 4) ||
      !strcmp(passbuf, newuser.userid))
    {
      pressanykey("�K�X��²��A���D�J�I�A�ܤ֭n 4 �Ӧr�A�Э��s��J");
      continue;
    }
    strncpy(newuser.passwd, passbuf, PASSLEN);
    getdata(18, 0, "���ˬd�K�X�G", passbuf, PASSLEN, PASS,0);
    if (strncmp(passbuf, newuser.passwd, PASSLEN))
    {
      outs("�K�X��J���~, �Э��s��J�K�X.\n");
      continue;
    }
    passbuf[8] = '\0';
    strncpy(newuser.passwd, genpasswd(passbuf), PASSLEN);
    break;
  }
  newuser.userlevel = PERM_DEFAULT;
  newuser.pager = 1;
  newuser.uflag = COLOR_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
  newuser.firstlogin = newuser.lastlogin = time(NULL);
  srandom(time(0));
  newuser.silvermoney = 10000;
  newuser.habit = HABIT_NEWUSER;	/* user.habit */

  newuser.lightbar[0] = 4;   /* bg */       /* lightbar */
  newuser.lightbar[1] = 7;   /* wd */
  newuser.lightbar[2] = 1;   /* light */
  newuser.lightbar[3] = 0;   /* blite*/
  newuser.lightbar[4] = 0;   /* underline */

  strcpy(newuser.cursor, STR_CURSOR);		    /* cursor */
  allocid = getnewuserid();
  if (allocid > MAXUSERS || allocid <= 0)
  {
    fprintf(stderr, "�����H�f�w�F���M�I\n");
    if(mode==1)
    	return;
    exit(1);
  }
  

  if (substitute_record(fn_passwd, &newuser, sizeof(userec), allocid) == -1)
  {
    fprintf(stderr, "�Ⱥ��F�A�A���I\n");
    if(mode==1)
    	return;
    exit(1);
  }

  setuserid(allocid, newuser.userid);
  if (!dosearchuser(newuser.userid))
  {
    fprintf(stderr, "�L�k�إ߱b��\n");
    if(mode==1)
    	return;
    exit(1);
  }
  
  //�o�̨S����k�ϥ�return;�|��
  if(mode==1)
  {
    dosearchuser(origname);
    return;
  }
}



int m_newuser()
{
  clear();
  new_register(1);
  clear();
  return 0;
}

void va_new_register(va_list pvar)
{
  int mode;  
  mode = va_arg(pvar, int);
  new_register(mode);
}

#ifdef REG_MAGICKEY
/* shakalaca.000712: new justify */
int u_verify()
{
  char keyfile[80], buf[80], inbuf[15], *key;
  FILE *fp;

  if (HAS_PERM(PERM_LOGINOK))
  {
    pressanykey("�z�w�g�q�L�{�ҡA�����n��J MagicKey�I");
    return XEASY;
  }

  sethomefile(keyfile, cuser.userid, fn_magickey);
  if (!dashf(keyfile))
  {
    if(win_select("�{�ҽX", "�z�٥��o�{�ҫH�A�n�o�X�ܡH", 0, 2, 'y') == 'y')
      mail_justify(cuser);
    
    return XEASY;
  }

  if (!(fp = fopen(keyfile, "r")))
  {
    pressanykey("�}���ɮצ����D�A�гq�������I");
    fclose(fp);
    return XEASY;
  }

  fgets(buf, 80, fp);
  fclose(fp);  

  for(key=buf;*key;key++)
    if(*key == '\n')
    {
      *key='\0';
      break;
    }
  key = buf;
  
  getdata(b_lines, 0, "�п�J MagicKey�G", inbuf, 14, DOECHO, 0);
  if (*inbuf)
  {
    if (strcmp(key, inbuf))
      pressanykey("���~�A�Э��s��J�I");
    else
    {
      int unum = getuser(cuser.userid);
      
      unlink(keyfile);
      pressanykey("���߱z�q�L�{�ҡA�w��[�J :)");
      cuser.userlevel |= (PERM_PAGE | PERM_POST | PERM_CHAT | PERM_LOGINOK);
      mail2user(cuser.userid, "[���U���\\�o]", "etc/registered", 0);
      substitute_record (fn_passwd, &cuser, sizeof (cuser), unum);
    }
  }

  return RC_FULL;
}
#endif
