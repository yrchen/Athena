/* clear aloha empty mesgs */
#include "bbs.h"
#include "cache.c"
#include <stdarg.h>

#define USERHOME	BBSHOME"/home"

#define LOG_FILE	BBSHOME"/log/clear_aloha.log"

static void
ca_log(char *fmt, ...)
{
  va_list ap;
  char msg[128];

  va_start(ap, fmt);
  vsprintf(msg, fmt, ap); 
  va_end(ap);
  
  f_cat(LOG_FILE, msg);
  fputs(msg, stdout);
}

void 
clear_aloha(char *direct)
{
	char		fname    [PATHLEN];
	char 		newfname  [PATHLEN];
	FILE           *fp, *newfp;
	PAL		pal;
	int count_r = 0, count_w = 0;
	
	sprintf(fname, "%s/%s", direct, FN_ALOHA);
	sprintf(newfname, "%s/%s.new", direct, FN_ALOHA);

	if(!(fp = fopen(fname, "r")))
	{
		ca_log("%s: cannot open", fname);
		return;
	}
	
	if(!(newfp = fopen(newfname, "w")))
	{
		fclose(fp);
		ca_log("%s: cannot open", newfname);
		return;
	}

	while(fread(&pal, sizeof(PAL), 1, fp) != 0)
	{
		count_r++;
		if(!pal.userid[0] || !searchuser(pal.userid))
			continue;
		fwrite(&pal, sizeof(PAL), 1, newfp);
		count_w++;
	}
	
	ca_log("%s: delete %d bad records.", fname, count_r - count_w);

	fclose(fp);
	fclose(newfp);
	
	if(count_r - count_w != 0)
	{
		f_mv(fname, newfname);
	}
	else
		unlink(newfname);
	return;
}

int 
main()
{
	DIR            *dirp;
	struct dirent  *dp;

	chdir(USERHOME);
	setuid(BBSUID);
	setgid(BBSGID);

	if (!(dirp = opendir(USERHOME)))
	{
		printf("## unable enter to %s", USERHOME);
		return -1;
	}
	while ((dp = readdir(dirp)) != NULL)
	{
		if(dp->d_name[0] == '.')
			continue;

		clear_aloha(dp->d_name);
	}

	closedir(dirp);
	return 0;
}
