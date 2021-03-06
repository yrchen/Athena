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

#undef VACATION     // 是否為寒暑假保留帳號期間
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
  return 180 * 24 * 60 - value; /* 寒暑假保存帳號 180 天 */
#else
  if (!urec->numlogins)         /* 未 login 成功者，不保留 */
    return -1;
  else if (urec->numlogins <= 3)     /* #login 少於三者，保留 30 天 */
    return 30 * 24 * 60 - value;

  /* 未完成註冊者，保留 30 天 */
  /* 一般情況，保留 180 天 */
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
  /* Lazy method : 先找尋已經清除的過期帳號 */
  /* -------------------------------------- */

  if ((i = searchnewuser(0)) == 0)
  {

    /* ------------------------------- */
    /* 每 1 個小時，清理 user 帳號一次 */
    /* ------------------------------- */

    if ((stat(fn_fresh, &st) == -1) || (st.st_mtime < clock - 3600))
    {
      if ((fd = open(fn_fresh, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      write(fd, ctime(&clock), 25);
      close(fd);
      log_usies("CLEAN", "dated users");

      printf("尋找新帳號中, 請稍待片刻...\n\r");
      if ((fd = open(fn_passwd, O_RDWR | O_CREAT, 0600)) == -1)
        return -1;
      i = 0;  /* Ptt解決第一個帳號老是被砍問題 */
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
// wildcat : 直接 mv , 不用跑 rm home/userid
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
      printf("抱歉，使用者帳號已經滿了，無法註冊新的帳號\n\r");
    val = (st.st_mtime - clock + 3660) / 60;
    printf("請等待 %d 分鐘後再試一次，祝你好運\n\r", val);
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
/* 使用者填寫註冊表格                            */
/* --------------------------------------------- */

static void
getfield(line, info, desc, buf, len)
  int line, len;
  char *info, *desc, *buf;
{
  char prompt[STRLEN];
  char genbuf[200];

  sprintf(genbuf, "原先設定：%-30.30s (%s)", buf, info);
  move(line, 2);
  outs(genbuf);
  sprintf(prompt, "%s：", desc);
  if (getdata(line + 1, 2, prompt, genbuf, len, DOECHO,0))
    strcpy(buf, genbuf);
  move(line, 2);
  prints("%s：%s", desc, buf);
  clrtoeol();
}


int 
u_register()
{
  char rname[20], howto[50]="請確實填寫";
  char phone[20], career[40], email[50],birthday[9],sex_is[2],year,mon,day;
  char ans[3], *ptr;
  FILE *fn;
  time_t now;
  char genbuf[200];
  
  if (cuser.userlevel & PERM_LOGINOK)
  {
    pressanykey("您的身份確認已經完成，不需填寫申請表");
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
        pressanykey("您的註冊申請單尚在處理中，請耐心等候");
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
    prints("%s[1;32m【[m%s[1;32m】[m 您好，請據實填寫以下的資料:(無變更請按enter跳過)",
      cuser.userid, cuser.username);
    getfield(6, "請確實填寫中文姓名", "真實姓名", rname, 20);
    getfield(8, "學校系級或單位職稱", "服務單位", career, 40);
    getfield(10, "包括長途撥號區域碼", "連絡電話", phone, 20);
    while (1)
    {
    int len;
    getfield(12, " 19xx/月/日 如: 77/12/01","生日",birthday,9);
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
    getfield(14,"1.葛格 2.姐接 3.底迪 4.美眉","性別",sex_is,2);
    getfield(16, "身分認證用", "E-Mail Address", email, 50);
    getfield(18, "從哪邊知道這個站的", "從何得知", howto, 50);

    ans[0] = getans2(b_lines - 1, 0, "以上資料是否正確？ ", 0, 3, 'n');
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
  mail_justify(cuser); //認證碼
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
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* 記錄 */
  return 0;
}
#endif

/*--------------------------*/ 
/* mode==0 為登入註冊(預設) */ 
/* mode==1 為管理新增使用者 */
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

  //使用 guest 的喜好設定
  strlcpy(cuser.userid, STR_GUEST, sizeof(cuser.userid));
  cuser.habit = HABIT_NEWUSER;  

#ifdef ATREGISTERMODE

  more("etc/register_announce", YEA);

  move(b_lines - 5, 0);
  outs(" \
請選擇：1) 我贊同本站架站的理念，願意繼續相關註冊程序！\n \
        2) 我不贊同本站理念，現在馬上離開！\n \
        3) 我先用 guest 參觀一下再做決定！");

  getdata(b_lines - 1, 0, "我的決定是：[2]", genbuf, 3, DOECHO, 0);
  switch(genbuf[0])
    {
     case '1':
       break;
     case '3':
       pressanykey("請重新用 guest 登入 :)");
       oflush();
       exit(1);
     default:
       pressanykey("謝謝參觀 :)");
       oflush();
       exit(1);
    }

  more("etc/register_visio", YEA);
  more("etc/copyright", YEA);

  if(getans2(b_lines, 0, "請問是否接受本站的隱私權保護政策 ", 0, 2, 'n') != 'y')
  {
    pressanykey("謝謝參觀 :)");
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

      pressanykey("您嘗試錯誤的輸入太多，請下次再來吧");
      oflush();
      if(mode==1)
        return;
      exit(1);
    }
    getdata(16, 0, msg_uid, newuser.userid, IDLEN + 1, DOECHO,0);

    if (bad_user_id(newuser.userid))
      outs("無法接受這個代號，請使用英文字母，並且不要包含空格\n");
    else if (searchuser(newuser.userid))
      outs("此代號已經有人使用\n");
    else
      break;
  }

  try = 0;
  while (1)
  {
    if (++try >= 6)
    {
      pressanykey("您嘗試錯誤的輸入太多，請下次再來吧");
      oflush();
      if(mode==1)
        return;
      exit(1);
    }
    if ((getdata(17, 0, "請設定密碼：", passbuf, PASSLEN, PASS,0) < 4) ||
      !strcmp(passbuf, newuser.userid))
    {
      pressanykey("密碼太簡單，易遭入侵，至少要 4 個字，請重新輸入");
      continue;
    }
    strncpy(newuser.passwd, passbuf, PASSLEN);
    getdata(18, 0, "請檢查密碼：", passbuf, PASSLEN, PASS,0);
    if (strncmp(passbuf, newuser.passwd, PASSLEN))
    {
      outs("密碼輸入錯誤, 請重新輸入密碼.\n");
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
    fprintf(stderr, "本站人口已達飽和！\n");
    if(mode==1)
    	return;
    exit(1);
  }
  

  if (substitute_record(fn_passwd, &newuser, sizeof(userec), allocid) == -1)
  {
    fprintf(stderr, "客滿了，再見！\n");
    if(mode==1)
    	return;
    exit(1);
  }

  setuserid(allocid, newuser.userid);
  if (!dosearchuser(newuser.userid))
  {
    fprintf(stderr, "無法建立帳號\n");
    if(mode==1)
    	return;
    exit(1);
  }
  
  //這裡沒有辦法使用return;會當掉
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
    pressanykey("您已經通過認證，不須要輸入 MagicKey！");
    return XEASY;
  }

  sethomefile(keyfile, cuser.userid, fn_magickey);
  if (!dashf(keyfile))
  {
    if(win_select("認證碼", "您還未發認證信，要發出嗎？", 0, 2, 'y') == 'y')
      mail_justify(cuser);
    
    return XEASY;
  }

  if (!(fp = fopen(keyfile, "r")))
  {
    pressanykey("開啟檔案有問題，請通知站長！");
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
  
  getdata(b_lines, 0, "請輸入 MagicKey：", inbuf, 14, DOECHO, 0);
  if (*inbuf)
  {
    if (strcmp(key, inbuf))
      pressanykey("錯誤，請重新輸入！");
    else
    {
      int unum = getuser(cuser.userid);
      
      unlink(keyfile);
      pressanykey("恭喜您通過認證，歡迎加入 :)");
      cuser.userlevel |= (PERM_PAGE | PERM_POST | PERM_CHAT | PERM_LOGINOK);
      mail2user(cuser.userid, "[註冊成功\囉]", "etc/registered", 0);
      substitute_record (fn_passwd, &cuser, sizeof (cuser), unum);
    }
  }

  return RC_FULL;
}
#endif
