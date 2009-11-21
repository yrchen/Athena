#include "dao.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

static char datemsg[32];

char *
Atime(clock)	/* Thor.990125: ���� ARPANET �ɶ��榡 */
  time_t *clock;
{
  /* ARPANET format: Thu, 11 Feb 1999 06:00:37 +0800 (CST) */
  /* strftime(datemsg, 40, "%a, %d %b %Y %T %Z", localtime(clock)); */
  /* Thor.990125: time zone���Ǧ^�Ȥ����MARPANET�榡�O�_�@��,���w��,�Psendmail*/
  strftime(datemsg, 40, "%a, %d %b %Y %T +0800 (CST)", localtime(clock));
  return (datemsg);
}

char *
Btime(clock)
  time_t *clock;
{
  struct tm *t = localtime(clock);

  sprintf(datemsg, "%02d/%02d/%02d%3d:%02d:%02d ",
    t->tm_year % 100, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec);
  return (datemsg);
}


char *
Ctime(clock)
  time_t *clock;
{
  struct tm *t = localtime(clock);
  static char week[] = "��@�G�T�|����";

  sprintf(datemsg, "%d�~%2d��%2d��%3d:%02d:%02d �P��%.2s",
    t->tm_year - 11, t->tm_mon + 1, t->tm_mday,
    t->tm_hour, t->tm_min, t->tm_sec, &week[t->tm_wday << 1]);
  return (datemsg);
}


char *
Etime(clock)
  time_t *clock;
{
  strftime(datemsg, 24, "%Y/%m/%d %T %a", localtime(clock));
  return (datemsg);
}

char *
Now()
{
  time_t now;

  time(&now);
  return Btime(&now);
}
