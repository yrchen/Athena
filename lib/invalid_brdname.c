#include "dao.h"
int not_alnum(register char ch);
/* 定義錯誤看板名稱 */
int invalid_brdname(char *brd)
{
  register char ch;
                                                                                
  ch = *brd++;
  if (not_alnum (ch))
    return 1;
  while ((ch = *brd++) != '\0')
  {
    if (not_alnum (ch) && ch != '_' && ch != '-' && ch != '.')
      return 1;
  }
  return 0;
}
