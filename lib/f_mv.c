#include "dao.h"
#include <fcntl.h>

int f_cp(char *src,char * dst,int mode);
int f_mv(char *src, char *dst)
{
  int ret;

  if ((ret = rename(src, dst)) != 0)
  {
    ret = f_cp(src, dst, O_TRUNC);
    if (!ret)
      unlink(src);
  }
  return ret;
}
