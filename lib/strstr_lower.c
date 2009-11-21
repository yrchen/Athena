#include "dao.h"
void str_lower(char *dst,char *src);
int strstr_lower(char *str, char *tag)	/* tag : lower-case string */
{
  char buf[80];

  str_lower(buf, str);
  return (int) strstr(buf, tag);
}
