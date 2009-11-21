/* ----------------------------------------------------- */
/* f_ln() : link() cross partition / disk		 */
/* ----------------------------------------------------- */

#include "dao.h"

#include <fcntl.h>
#include <errno.h>

int f_cp(char *src,char *dst,int mode);
int f_ln(char *src, char *dst)
{
  int ret;

  if ((ret = link(src, dst)) != 0)
  {
    if (errno != EEXIST)
      ret = f_cp(src, dst, O_EXCL);
  }
  return ret;
}
