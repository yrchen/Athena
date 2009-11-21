/*-------------------------------------------------------*/
/* lib/url_encode.c     ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* author : thor.bbs@bbs.cs.nthu.edu.tw                  */
/* target : included C for URL encoding                  */
/* create : 99/03/30                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/

//extern char radix32[32];
#include "dao.h"
int is_alnum(int ch);
void
url_encode(dst, src)
  unsigned char *dst; /* Thor.990331: n srcªº¤T¿ªÅ¶¡ */
  unsigned char *src;
{
  for(; *src; src++)
  {
    if(*src == ' ')
      *dst++ = '+';
    else if(is_alnum(*src))
      *dst++ = *src;
    else
    {
      register unsigned char cc = *src;
      *dst++ = '%';
      *dst++ = radix32[cc >> 4];
      *dst++ = radix32[cc & 0xf];
    }
  }
  *dst = '\0';
}
