/*-------------------------------------------------------*/
/* mbbsd.c       ( NTHU CS MapleBBS Ver 2.36 )           */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : BBS main/			                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"
#include <sys/wait.h>
#include <arpa/telnet.h>
#include <syslog.h>


#define SOCKET_QLEN 4
#define TH_LOW 100
#define TH_HIGH 120
#define PID_FILE "run/mbbsd.pid"


#define MBBSD_LAST
#ifdef MBBSD_LAST
#define MBBSD_LASTLOG "src/.last_mbbsd"
#endif

char		fromhost  [STRLEN] = "\0";
char		remoteusername[40];

static uschar	enter_uflag;
void		check_register();

/* ----------------------------------------------------- */
/* Â÷¶} BBS µ{¦¡                                         */
/* ----------------------------------------------------- */

void
log_usies(char *mode, char *msg)
{
	char		buf       [256], data[256];
	time_t		now;

	time(&now);
	if (!msg) {
		sprintf(data, "Stay: %d (%s)",
			(now - login_start_time) / 60, cuser.username);
		msg = data;
	}
	sprintf(buf, "%s %s %-13s%s", Etime(&now), mode, cuser.userid, msg);
	f_cat(FN_USIES, buf);
}


static void
setflags(int mask, int value)
{
	if (value)
		cuser.uflag |= mask;
	else
		cuser.uflag &= ~mask;
}


void 
u_exit(char *mode)
{
	extern void	auto_backup();	/* ½s¿è¾¹¦Û°Ê³Æ¥÷ */
	userec		xuser;
	int		diff = (time(0) - login_start_time) / 60;

	rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
	auto_backup();
	setflags(PAGER_FLAG, currutmp->pager != 1);
	setflags(CLOAK_FLAG, currutmp->invisible);
	xuser.pager = currutmp->pager;	/* °O¿ýpagerª¬ºA, add by wisely */
	xuser.invisible = currutmp->invisible;	/* ¬ö¿ýÁô§Îª¬ºA by wildcat */
	xuser.totaltime += time(0) - update_time;
	xuser.numposts = cuser.numposts;
	xuser.feeling[4] = '\0';

	if (!HAS_PERM(PERM_DENYPOST) && !currutmp->invisible) {
		char		buf       [256];
		time_t		now;

		time(&now);
		sprintf(buf, "<<¤U¯¸³qª¾>> -- §Ú¨«Åo¡I - %s", Etime(&now));
		do_aloha(buf);
	}
	purge_utmp(currutmp);
	if (!diff && cuser.numlogins > 1 && strcmp(cuser.userid, STR_GUEST))
		xuser.numlogins = --cuser.numlogins;	/* Leeym ¤W¯¸°±¯d®É¶¡­­¨î¦¡ */
	substitute_record(fn_passwd, &xuser, sizeof(userec), usernum);
	log_usies(mode, NULL);
}


void 
system_abort()
{
	if (currmode)
		u_exit("ABORT");

	clear();
	refresh();
	printf("ÁÂÁÂ¥úÁ{, °O±o±`¨Ó³á !\n");
	sleep(1);
	exit(0);
}


void 
abort_bbs()
{
	if (currmode)
		u_exit("AXXED");
	exit(0);
}

/* ----------------------------------------------------- */
/* µn¿ý BBS µ{¦¡                                         */
/* ----------------------------------------------------- */

int
dosearchuser(char *userid)
{
	if (usernum = getuser(userid))
		memcpy(&cuser, &xuser, sizeof(cuser));
	else
		memset(&cuser, 0, sizeof(cuser));
	return usernum;
}


static void 
talk_request()
{
#ifdef  LINUX
	/*
	 * Linux ¤U³sÄò page ¹ï¤è¨â¦¸´N¥i¥H§â¹ï¤è½ð¥X¥h¡G ³o¬O¥Ñ©ó¬Y¨Ç¨t²Î¤@
	 * nal ¶i¨Ó´N·|±N signal handler ³]©w¬°¤º©wªº handler, ¤£©¯ªº¬O
	 * default ¬O±Nµ{ erminate. ¸Ñ¨M¤èªk¬O¨C¦¸ signal ¶i¨Ó´N­«³] signal
	 * handler
	 */

	signal(SIGUSR1, talk_request);
#endif
	bell();
	if (currutmp->msgcount) {
		char		buf       [200];
		time_t		now = time(0);

		sprintf(buf, "[1m[33;41m[%s][34;47m [%s] %s [0m",
			(currutmp->destuip)->userid, my_ctime(&now),
			(currutmp->sig == 2) ? "­«­n®ø®§¼s¼½¡I(½ÐCtrl-U,l¬d¬Ý¼ö°T°O¿ý)" : "©I¥s¡B©I¥s¡AÅ¥¨ì½Ð¦^µª");
		move(0, 0);
		clrtoeol();
		outs(buf);
		refresh();
	} else {
		uschar		mode0 = currutmp->mode;
		char		c0 = currutmp->chatid[0];
		screenline     *screen = (screenline *) calloc(t_lines, sizeof(screenline));
		currutmp->mode = 0;
		currutmp->chatid[0] = 1;
		vs_save(screen);
		talkreply();
		vs_restore(screen);
		currutmp->mode = mode0;
		currutmp->chatid[0] = c0;
	}
}

extern msgque	oldmsg[MAX_REVIEW];	/* ¥á¹L¥hªº¤ô²y */
extern char	no_oldmsg, oldmsg_count;	/* pointer */

static void 
write_request()
{
	time_t		now;
	/* Half-hour remind  */
	if (*currmsg) {
		outmsg(currmsg);
		bell();
		*currmsg = 0;
		return;
	}
	time(&now);

#ifdef  LINUX
	signal(SIGUSR2, write_request);
#endif

	update_data();
	++cuser.receivemsg;
	substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
	bell();
	show_last_call_in();
	/* wildcat patch : ¬Ý¤£¨ì¤ô²y??!! */
	currutmp->msgcount--;
	memcpy(&oldmsg[no_oldmsg], &currutmp->msgs[0], sizeof(msgque));
	no_oldmsg++;
	no_oldmsg %= MAX_REVIEW;
	if (oldmsg_count < MAX_REVIEW)
		oldmsg_count++;
	if (watermode) {
		if (watermode < oldmsg_count)
			watermode++;
		t_display_new(0);
	}
	refresh();
	currutmp->msgcount = 0;
}

static void 
multi_user_check()
{
	register user_info *ui;
	register pid_t	pid;
	int		cmpuids    ();

	if (HAS_PERM(PERM_SYSOP))
		return;		/* wildcat:¯¸ªø¤£­­¨î */

	if (cuser.userlevel) {
		if (!(ui = (user_info *) search_ulist(cmpuids, usernum)))
			return;	/* user isn't logged in */

		pid = ui->pid;
		if (!pid || (kill(pid, 0) == -1))
			return;	/* stale entry in utmp file */

		if (getans2(b_lines, 0, "±z·Q§R°£¨ä¥L­«½Æªº login ¶Ü¡H", 0, 2, 'y') != 'n') {
			kill(pid, SIGHUP);
			log_usies("KICK ", cuser.username);
		} else {
			int		nums = MULTI_NUMS;
			if (HAS_PERM(PERM_BM))
				nums += 2;
			if (count_multi() >= nums)
				system_abort();
		}
	} else {		/* guestªº¸Ü */
		if (count_multi() > 512) {
			pressanykey("©êºp¡A¥Ø«e¤w¦³¤Ó¦h guest, ½Ðµy«á¦A¸Õ¡C");
			oflush();
			exit(1);
		}
	}
}

/* --------- */
/* bad login */
/* --------- */

static char	str_badlogin[] = "log/logins.bad";

static void
logattempt(char *uid, char type)
{				/* '-' login failure ' ' success '!' not
				 * exist '*' spamd */
	char		fname     [PATHLEN];
	char		genbuf    [200], datemsg[20];

	sprintf(genbuf, "%c%-12s[%s] %s@%s", type, uid,
		Etime(&login_start_time), remoteusername, fromhost);
	f_cat(str_badlogin, genbuf);

	if (type == '-') {
		sprintf(genbuf, "[%s] %s", Etime(&login_start_time), fromhost);
		sethomefile(fname, uid, str_badlogin);

		f_cat(fname, genbuf);
	}
	/* ­Ó¤H¤W¯¸°O¿ý from myth */
	if (type != '!') {
		strftime(datemsg, 20, "%Y/%m/%d %T", localtime(&login_start_time));
		sprintf(genbuf, "%s %c%-8s %s@%s",
			datemsg, type, "BBS", remoteusername, fromhost);
		sethomefile(fname, uid, FN_LOGLOGIN);

		f_cat(fname, genbuf);
	}
}

static void
login_query()
{
	char		uid       [IDLEN + 1], passbuf[PASSLEN];
	int		attempts;
	char		genbuf    [200];

	resolve_utmp();
	attempts = utmpshm->number;
	clear();

#ifdef CAMERA
	film_out(time(0) % 5, 0);
#else
	show_file("etc/Welcome0", 0, 20, ONLY_COLOR);
#endif

	if (attempts >= MAXACTIVE) {
		pressanykey("¥Ø«e¯¸¤W¤H¼Æ¤w¹F¤W­­¡A½Ð±zµy«á¦A¨Ó¡C");
		oflush();
		sleep(1);
		exit(1);
	}
	attempts = 0;
	while (1) {
		if (attempts++ >= LOGINATTEMPTS) {
			more("etc/goodbye", NA);
			pressanykey_old("¿ù»~¤Ó¦h¦¸,ÙTÙT~~~~~");
			exit(1);
		}
		uid[0] = '\0';
		getdata(22, 2, "±zªº¥N¸¹¡G", uid, IDLEN + 1, DOECHO, 0);
		if (strcasecmp(uid, str_new) == 0) {

#ifdef LOGINASNEW
			DL_func("SO/register.so:va_new_register", 0);
			break;
#else
			pressanykey("¥»¨t²Î¥Ø«eµLªk¥H new µù¥U, ½Ð¥Î guest ¶i¤J");
			continue;
#endif
		} else if (uid[0] == '\0' /* || !dosearchuser(uid) */ )
			pressanykey(err_uid);
		else if (belong(FN_DISABLED, uid)) {
			pressanykey("¸Ó ID ¬°¥»¯¸¸T¤î¤W¯¸¤§ ID");
			logattempt(uid, '*');
		} else if (strcmp(uid, STR_GUEST)) {
			getdata(22, 30, "±zªº±K½X¡G", passbuf, PASSLEN, PASS, 0);
			passbuf[8] = '\0';

			if (!dosearchuser(uid)) {
				logattempt(uid, '!');
				pressanykey(ERR_PASSWD);
			} else if (!chkpasswd(cuser.passwd, passbuf)) {
				logattempt(cuser.userid, '-');
				pressanykey(ERR_PASSWD);
			} else {
				/* SYSOP gets all permission bits */

				if (!strcasecmp(cuser.userid, str_sysop))
					cuser.userlevel = ~0;

				logattempt(cuser.userid, ' ');
				break;
			}
		} else {
			/* guest ªº¸Ü */
#ifdef LOGINASGUEST
			cuser.userlevel = 0;
			cuser.uflag = COLOR_FLAG | PAGER_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
			break;
#else
			pressanykey("¥»¯¸¤£´£¨Ñ guest ¤W¯¸");
			continue;
#endif
		}
	}

	multi_user_check();
	sethomepath(genbuf, cuser.userid);
	mkdir(genbuf, 0755);
	srand(time(0) ^ getpid() ^ (getpid() << 10));
	srandom(time(0) ^ getpid() ^ (getpid() << 10));
}

#ifdef WHERE
/* Jaky and Ptt */
int 
where(char *from)
{
	extern struct FROMCACHE *fcache;
	register int	i , count, j;

	resolve_fcache();
	for (j = 0; j < fcache->top; j++) {
		char           *token = strtok(fcache->domain[j], "&");
		i = 0;
		count = 0;
		while (token) {
			if (strstr(from, token))
				count++;
			token = strtok(NULL, "&");
			i++;
		}
		if (i == count)
			break;
	}
	if (i != count)
		return 0;
	return j;
}
#endif

static void 
check_BM()
{				/* Ptt ¦Û°Ê¨ú¤UÂ÷Â¾ªO¥DÅv¤O */
	int		i;
	boardheader    *bhdr;
	extern boardheader *bcache;
	extern int	numboards;

	resolve_boards();
	cuser.userlevel &= ~PERM_BM;
	for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
		userid_is_BM(cuser.userid, bhdr->BM);
}

void 
setup_utmp(int mode)
{
	user_info	uinfo;
	char		buf       [80];

	memset(&uinfo, 0, sizeof(uinfo));
	uinfo.pid = currpid = getpid();
	uinfo.uid = searchuser(cuser.userid);
	uinfo.mode = currstat = mode;
	uinfo.msgcount = 0;
	if (cuser.userlevel & PERM_BM)
		check_BM();	/* Ptt ¦Û°Ê¨ú¤UÂ÷Â¾ªO¥DÅv¤O */
	uinfo.userlevel = cuser.userlevel;
	uinfo.lastact = time(NULL);

	strcpy(uinfo.userid, cuser.userid);
	strcpy(uinfo.realname, cuser.realname);
	strcpy(uinfo.username, cuser.username);
	strcpy(uinfo.feeling, cuser.feeling);
	strncpy(uinfo.from, fromhost, 23);
	uinfo.invisible = cuser.invisible % 2;
	uinfo.pager = cuser.pager % 5;
	uinfo.brc_id = 0;
	uinfo.sex = cuser.sex;

	/* Ptt WHERE */

#ifdef WHERE
	uinfo.from_alias = where(fromhost);
#else
	uinfo.from_alias = 0;
#endif
	sethomefile(buf, cuser.userid, "remoteuser");

	if (enter_uflag & CLOAK_FLAG)
		uinfo.invisible = YEA;

	getnewutmpent(&uinfo);
	friend_load();
}

static void
user_login()
{
	char		genbuf    [200];
	struct tm      *ptime, *tmp;
	time_t		now = time(0);
	int		a;

	extern struct FROMCACHE *fcache;
	extern int	fcache_semid;

	log_usies("ENTER", fromhost);
	setproctitle("%s: %s", cuser.userid, fromhost);

	/* ------------------------ */
	/* ªì©l¤Æ uinfo¡Bflag¡Bmode */
	/* ------------------------ */

	setup_utmp(LOGIN);
	currmode = MODE_STARTED;
	enter_uflag = cuser.uflag;

	/* get local time */
	tmp = localtime(&cuser.lastlogin);

	update_data();		/* wildcat: update user data */
	/* Ptt check ¦P®É¤W½u¤H¼Æ */
	resolve_fcache();
	resolve_utmp();

	if ((a = utmpshm->number) > fcache->max_user) {
		sem_init(FROMSEM_KEY, &fcache_semid);
		sem_lock(SEM_ENTER, fcache_semid);
		fcache->max_user = a;
		fcache->max_time = now;
		sem_lock(SEM_LEAVE, fcache_semid);
	}
#ifdef  INITIAL_SETUP
	if (getbnum(DEFAULT_BOARD) == 0) {
		strcpy(currboard, "©|¥¼¿ï©w");
	} else
#endif

	{
		brc_initial(DEFAULT_BOARD);
		set_board();
	}

	/* ------------ */
	/* µe­±³B²z¶}©l */
	/* ------------ */

	if (!(HAS_PERM(PERM_SYSOP) && HAS_PERM(PERM_DENYPOST))) {
		char		buf       [256];
		time_t		now;

		time(&now);
		sprintf(buf, "<<¤W¯¸³qª¾>> -- §Ú¨ÓÅo¡I - %s", Etime(&now));
		do_aloha(buf);
	}
	time(&now);
	ptime = localtime(&now);

#ifdef CAMERA
	film_out(FILM_LOGIN, 0);
#else
	more("etc/Welcome_login", NA);
#endif

#if 0
	/* wildcat : ·h®a³qª¾¥Î */
	if (belong(BBSHOME "/etc/oldip", fromhost)) {
		more(BBSHOME "/etc/removal");
		abort_bbs();
	}
#endif

	if ((cuser.day == ptime->tm_mday) && (cuser.month == (ptime->tm_mon + 1)))
		currutmp->birth = 1;
	else
		currutmp->birth = 0;

	if (cuser.userlevel) {	/* not guest */
		move(t_lines - 3, 0);
		prints("      Åwªï±z²Ä [1;33m%d[0;37m «×«ô³X¥»¯¸¡A\
¤W¦¸±z¬O±q [1;33m%s[0;37m ³s©¹¥»¯¸¡A\n\
     §Ú°O±o¨º¤Ñ¬O [1;33m%s[0;37m¡C\n",
		       ++cuser.numlogins, cuser.lasthost,
		       Etime(&cuser.lastlogin));
		pressanykey(NULL);


		/* Ptt */
		if (currutmp->birth == 1) {
#ifdef CAMERA
			film_out(FILM_WEL_BIRTH, 0);
#else
			more("etc/Welcome_birth", YEA);
#endif
			brc_initial("Greeting");
			set_board();
			do_post();
		}
		sethomefile(genbuf, cuser.userid, str_badlogin);
		if (more(genbuf, NA) != -1) {
			if (getans2(b_lines, 0, "±z­n§R°£¥H¤W¿ù»~¹Á¸Õªº°O¿ý¶Ü¡H", 0, 2, 'y') != 'n')
				unlink(genbuf);
		}
		check_register();
		strncpy(cuser.lasthost, fromhost, 24);
		substitute_record(fn_passwd, &cuser, sizeof(cuser), usernum);
		cuser.lasthost[23] = '\0';
		restore_backup();
	}
	/* ªO¥D´£°ª«H½c¤W­­ */
	if (HAS_PERM(PERM_BM) && cuser.exmailbox < 100)
		cuser.exmailbox = 100;
	else if (!strcmp(cuser.userid, STR_GUEST)) {
		char           *nick[10] = {"ÄÑ¥]", "¬ü¤k¹Ï", "FreeBSD", "DBT¥Ø¿ý", "mp3",
			"¤k¤ýÀY", "¯f¬r", "µ£¦~", "¥Û¹³", "386 CPU"
		};
		char           *name[10] = {"¥d±þ¦Ì¨È", "¥Õ³¾¿AÄR¤l", "­·»P¹Ð®J¤p¯¸", "¨gx©Î«Ox±¶",
			"Wu Bai & China Blue", "¥ì¤O¨F¥Õ", "C-brain",
			"®É¶¡", "§Ú", "Intel inside"
		};
		int		sex        [10] = {6, 4, 7, 7, 2, 6, 0, 7, 7, 0};

		int		i = rand() % 10;
		sprintf(cuser.username, "³Q­·¤Æªº%s", nick[i]);
		sprintf(currutmp->username, cuser.username);
		sprintf(cuser.realname, name[i]);
		sprintf(currutmp->realname, cuser.realname);
		cuser.sex = sex[i];
		cuser.silvermoney = 300;
		cuser.habit = HABIT_GUEST;	/* guest's habit */
		currutmp->pager = 2;
		pressanykey(NULL);
	}
	more("etc/Welcome_announce", YEA);

	if (bad_user_id(cuser.userid)) {
		sprintf(currutmp->username, "BAD_USER");
		cuser.userlevel = 0;
		cuser.uflag = COLOR_FLAG | PAGER_FLAG | BRDSORT_FLAG | MOVIE_FLAG;
	}
	if (!PERM_HIDE(currutmp))
		cuser.lastlogin = login_start_time;
	substitute_record(fn_passwd, &cuser, sizeof(cuser), usernum);

	if (cuser.numlogins < 2) {
		more("etc/newuser", YEA);
		HELP();
	}
	login_plan();
}


static void
check_max_online()
{
	FILE           *fp;
	int		maxonline = 0;
	time_t		now = time(NULL);
	struct tm      *ptime;

	ptime = localtime(&now);

	if (fp = fopen(".maxonline", "r")) {
		fscanf(fp, "%d", &maxonline);
		fclose(fp);
	}
	if ((count_ulist() > maxonline) && (fp = fopen(".maxonline", "w"))) {
		fprintf(fp, "%d", count_ulist());
		fclose(fp);
	}
	if (fp = fopen(".maxtoday", "r")) {
		fscanf(fp, "%d", &maxonline);
		if (count_ulist() > maxonline) {
			fclose(fp);
			fp = fopen(".maxtoday", "w");
			fprintf(fp, "%d", count_ulist());
		}
		fclose(fp);
	}
}

inline static void
start_client()
{
	extern struct commands cmdlist[];
	extern char	currmaildir[32];

	/* ----------- */
	/* system init */
	/* ----------- */
	nice(2);		/* Ptt lower priority */
	login_start_time = time(NULL);
	currmode = 0;
	update_time = login_start_time;

#ifdef CAMERA
	fshm_init();
	feast_init();
#endif

	signal(SIGHUP, abort_bbs);
	signal(SIGBUS, abort_bbs);
	signal(SIGSEGV, abort_bbs);
	signal(SIGXCPU, abort_bbs);
#ifdef SIGSYS
	signal(SIGSYS, abort_bbs);
#endif
	signal(SIGTERM, SIG_IGN);

	signal(SIGUSR1, talk_request);	/* ¦¬¨ì°T®§ */
	signal(SIGUSR2, write_request);	/* °e¥X°T®§ */

	dup2(0, 1);

	initscr();
	login_query();
	user_login();
	check_max_online();
#if 0
	/* wildcat : ­×´_¤¤ */
	if (!HAVE_PERM(PERM_SYSOP)) {
		pressanykey("¾ã­×¤¤ , ¸T¤î¯¸ªø¥H¥~ user µn¤J");
		abort_bbs();
	}
#endif

	sethomedir(currmaildir, cuser.userid);
#ifdef DOTIMEOUT
	init_alarm();
#else
	signal(SIGALRM, SIG_IGN);
#endif
	if (HAVE_PERM(PERM_SYSOP | PERM_BM))
		DL_func("SO/vote.so:b_closepolls");
	if (!HAVE_HABIT(HABIT_COLOR))
		showansi = 0;
	while (chkmailbox())
		m_read();

#ifdef HAVE_GAME
	waste_money();
#endif

	force_board("Announce");
	strcpy(currboard, "©|¥¼¿ï©w");

	if (HAS_PERM(PERM_SYSOP)) {
		force_board("AdminChat");
		force_board("ATFBI");
	}
	if (HAS_PERM(PERM_ACCOUNTS) && dashf(fn_register))
		DL_func("SO/admin.so:m_register");

	domenu(MMENU, "¥D¥\\¯àªí", (chkmail(0) ? 'M' : 'B'), cmdlist);
}

/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol       */
/* ----------------------------------------------------- */

static void
telnet_init()
{
	static char	svr [] =
	{
		IAC, DO, TELOPT_TTYPE,
		IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
		IAC, WILL, TELOPT_ECHO,
		IAC, WILL, TELOPT_SGA
	};

	int		n         , len;
	char           *cmd;
	int		rset;
	struct timeval	to;
	char		buf       [64];

	/* --------------------------------------------------- */
	/* init telnet protocol                                */
	/* --------------------------------------------------- */

	cmd = svr;

	for (n = 0; n < 4; n++) {
		len = (n == 1 ? 6 : 3);
		send(0, cmd, len, 0);
		cmd += len;

		rset = 1;
		/* Thor.981221: for future reservation bug */
		to.tv_sec = 1;
		to.tv_usec = 1;
		if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
			recv(0, buf, sizeof(buf), 0);
	}
}

/* ----------------------------------------------------- */
/* ¤ä´©¶W¹L 24 ¦Cªºµe­±					 */
/* ----------------------------------------------------- */

static void
term_init()
{
#if 0				/* fuse.030518: µù¸Ñ */
	/* server°Ý¡G§A·|§ïÅÜ¦æ¦C¼Æ¶Ü¡H(TN_NAWS) */
	/* clientµª¡GYes, I do. (TNCH_DO) */

	/* ¨º»ò¦b³s½u®É¡A·íTERMÅÜ¤Æ¦æ¦C¼Æ®É´N·|µo¥X¡G */
	/* TNCH_IAC + TNCH_SB + TN_NAWS + ¦æ¼Æ¦C¼Æ + TNCH_IAC + TNCH_SE; */
#endif

	/* ask client to report it's term size */
	static char	svr [] =/* server */
	{
		IAC, DO, TELOPT_NAWS
	};

	static char	rvr [] =/* receiver */
	{
		IAC, WILL, TELOPT_NAWS,
		IAC, SB, TELOPT_NAWS,
		/* IAC, WONT, TELOPT_NAWS  */
	};

	int		rset;
	char		buf       [64];
	struct timeval	to;

	/* °Ý¹ï¤è (telnet client) ¦³¨S¦³¤ä´©¤£¦Pªº¿Ã¹õ¼e°ª */
	send(0, svr, 3, 0);

	rset = to.tv_sec = to.tv_usec = 1;
	if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
		recv(0, buf, sizeof(buf), 0);

	if (!memcmp(buf, rvr, 3)) {
		/* got IAC WILL NAWS : ¹ï¤è»¡¦³ */

		rset = to.tv_sec = to.tv_usec = 1;
		if (select(1, (fd_set *) & rset, NULL, NULL, &to) > 0)
			recv(0, buf, sizeof(buf), 0);
	}
	if (!memcmp(buf, rvr + 3, 3)) {
		/* got IAC SB NAWS ¡G±µµÛ¦^¶Ç¸ê®Æ */
		t_columns = /* buf[3] * 256 + */ buf[4];
		t_lines = /* buf[5] * 256 + */ buf[6];
	} else {
		/* got IAC WONT NAWS; default to 80x24 ¡G¹ï¤è»¡¨S¦³ */
		/* what ever else; default to 80x24 ¡G¨S¦³¦^ÂÐ */
		t_columns = 80;
		t_lines = 24;
	}

	/* b_lines ¦Ü¤Ö­n 23¡A³Ì¦h¤£¯à¶W¹L t_lines - 1 */
	if (t_lines < 24)
		t_lines = 24;

	b_lines = t_lines - 1;
	p_lines = t_lines - 4;

	return;
}

/* ----------------------------------------------------- */
/* stand-alone daemon                                    */
/* ----------------------------------------------------- */

static int	mainset;	/* read file descriptor set */
static struct sockaddr_in xsin;

static void
reapchild()
{
	int		state     , pid;

	while ((pid = waitpid(-1, &state, WNOHANG | WUNTRACED)) > 0);
}

static void
start_daemon(char *genbuf)
{
	int		n;

	/*
	 * More idiot speed-hacking --- the first time conversion makes the C
	 * library open the files containing the locale definition and time
	 * zone. If this hasn't happened in the parent process, it happens in
	 * the children, once per connection --- and it does add up.
	 */

	/* --------------------------------------------------- */
	/* «Ø¥ß PID ÀÉ®×                                       */
	/* --------------------------------------------------- */
	char		buf       [80];

	time_t		dummy = time(NULL);
	struct tm      *dummy_time = localtime(&dummy);
	(void)strftime(buf, 80, "%d/%b/%Y:%H:%M:%S", dummy_time);

	/* --------------------------------------------------- */
	/* speed-hacking DNS resolve                           */
	/* --------------------------------------------------- */

	dns_init();

	/* --------------------------------------------------- */

	if ((n = fork()) != 0) {
		printf("PID[%d]\n", n);
		exit(0);
	}
	n = getdtablesize();

	sprintf(genbuf, "%d\t%s", getpid(), buf);
}

static void 
close_daemon()
{
	exit(0);
}


static int
bind_port(port)
	int		port;
{
	int		sock      , on;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	on = 1;
	(void)setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	(void)setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));

	on = 0;
	(void)setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&on, sizeof(on));

	xsin.sin_port = htons(port);
	if (bind(sock, (struct sockaddr *)&xsin, sizeof xsin) < 0) {
		syslog(LOG_INFO, "bbsd bind_port can't bind to %d", port);
		exit(1);
	}
	if (listen(sock, SOCKET_QLEN) < 0) {
		syslog(LOG_INFO, "bbsd bind_port can't listen to %d", port);
		exit(1);
	}
	 /* (void) */ FD_SET(sock, (fd_set *) & mainset);
	return sock;
}

int 
bad_host(char *name)
{
	FILE           *list;
	char		buf       [40];

	if (list = fopen(BBSHOME "/etc/bad_host", "r")) {
		while (fgets(buf, 40, list)) {
			buf[strlen(buf) - 1] = '\0';
			if (!strcmp(buf, name))
				return 1;
			if (buf[strlen(buf) - 1] == '.' && !strncmp(buf, name, strlen(buf)))
				return 1;
			if (*buf == '.' && strlen(buf) < strlen(name) &&
			    !strcmp(buf, name + strlen(name) - strlen(buf)))
				return 1;
		}
		fclose(list);
	}
	return 0;
}

/**************************************************************/
static int	check_ban_and_load(int fd);

int
main(int argc, char *argv[], char *envp[])
{
	extern int	errno;
	register int	msock, csock;	/* socket for Master and Child */
	int		listen_port = 23;
	int		len_of_sock_addr, overloading = 0;
	char		genbuf    [80];
	char		margs     [64] = "\0";	/* main argv list */

	/* setup standalone                                    */
	start_daemon(genbuf);

	(void)signal(SIGCHLD, reapchild);
	(void)signal(SIGTERM, close_daemon);

	/* choose port */
	if (argc == 1)
		listen_port = 3006;
	else if (argc >= 2)
		listen_port = atoi(argv[1]);

	snprintf(margs, sizeof(margs), "%s %d ", argv[0], listen_port);

	/* port binding                                        */
	xsin.sin_family = AF_INET;

	msock = bind_port(listen_port);
	if (msock < 0)
		exit(1);

	setproctitle("%s: listening ", margs);

	/* Give up root privileges: no way back from here      */

	setgid(BBSGID);
	setuid(BBSUID);
	chdir(BBSHOME);

	snprintf(genbuf, sizeof(genbuf), "%d", (int)getpid());
	f_cat(PID_FILE, genbuf);


#ifdef MBBSD_LAST
	{
		time_t		now = time(NULL);
		if (dashf(MBBSD_LASTLOG))
			f_rm(MBBSD_LASTLOG);
		f_cat(MBBSD_LASTLOG, ctime(&now));
	}
#endif

	/* main loop                                           */
	while (1) {
		len_of_sock_addr = sizeof(xsin);

		if ((csock = accept(msock, (struct sockaddr *)&xsin, &len_of_sock_addr)) < 0) {
			if (errno != EINTR)
				sleep(1);
			continue;
		}
		overloading = check_ban_and_load(csock);

		if (overloading) {
			close(csock);
			continue;
		}
		if (fork() == 0)
			break;
		else
			close(csock);
	}


	/* here is only child running */
	setproctitle("%s: ...login wait... ", margs);
	close(msock);
	dup2(csock, 0);
	close(csock);

#if 0
	/* ident remote host / user name via RFC931          */
	dns_ident(0, &xsin, fromhost, remoteusername);
#else
	/* From Ptt FAST_LOGIN */
	strcpy(fromhost, (char *)inet_ntoa(xsin.sin_addr));
	strcpy(remoteusername, "unknown");
#endif

	telnet_init();
	term_init();
	start_client();

	return 1;
}

/*
 * check if we're banning login and if the load is too high. if login is
 * permitted, return 0; else return -1; approriate message is output to fd.
 */
static int
check_ban_and_load(int fd)
{
	static int	overload = 0;

#define BANNER "¡i"BOARDNAME"¡j¡· ¹q¤l§G§iÄæ¨t²Î ¡·("MYHOSTNAME")\r\n\
 IP ("MYIP") \r\nPowered by FreeBSD\r\n"
	write(fd, BANNER, sizeof(BANNER));

	overload = 0;

	if (cpuload(NULL) > MAX_CPULOAD)
		overload = 1;

	if (overload == 1)
		write(fd, "¨t²Î¹L¸ü, ½Ðµy«á¦A¨Ó\r\n", 22);

	if (overload)
		return -1;

	return 0;
}
