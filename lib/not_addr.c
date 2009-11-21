#include "dao.h"

#define	STRICT_FQDN_EMAIL
int is_alnum(int ch);
int not_addr(char *addr)
{
  int ch, mode;

  mode = -1;

  while ((ch = *addr) != 0)
  {
    if (ch == '@')
    {
      if (++mode)
	break;
    }

#ifdef	STRICT_FQDN_EMAIL
    else if ((ch != '.') && (ch != '-') && (ch != '_') && !is_alnum(ch))
#else
    else if (!is_alnum(ch) && !strchr(".-_[]%!:", ch))
#endif

      return 1;

    addr++;
  }

  return mode;
}
