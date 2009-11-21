#include "dao.h"

int isprint2(char ch)
{
  return ((ch & 0x80) ? 1 : isprint(ch));
}
