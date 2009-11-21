#include "dao.h"
#include <stdio.h>
#include <string.h>

void
setdirpath(char *fpath, char *direct, char *fname)
{
  int ch;
  char *target = NULL;
      
  while ((ch = *direct) != 0)
  {
    *fpath++ = ch;
    if (ch == '/')
      target = fpath;
    direct++;
  }
                              
  strcpy(target, fname);
}
