#include "dao.h"
#include <stdio.h>
#include <string.h>

int str_ncmp(char *s1,char * s2,int n);
int
str_has(list, tag)
  char *list;
  char *tag;
{
  int cc, len;

    len = strlen(tag);
    for (;;)
    {
      cc = list[len];
      if ((!cc || cc == '/') && !str_ncmp(list, tag, len))
	return 1;

      for (;;)
      {
	cc = *list;
	if (!cc)
	  return 0;
	list++;
	if (cc == '/')
	  break;
      }
    }
}
