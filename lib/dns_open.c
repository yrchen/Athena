/* $Id: dns_open.c,v 1.3 2003/10/18 11:39:25 sby Exp $ */

/* target : included C file for DNS routines */


#include "dao.h"
#include "dns.h"

#include <unistd.h>
int dns_query(char *name,int qtype,querybuf *ans);
unsigned long dns_aton(char *name);
/* Thor.990811: find out A record */

unsigned long
dns_a(host)
  char *host;
{
  querybuf ans;
  int n, ancount, qdcount;
  unsigned char *cp, *eom;
  char buf[MAXDNAME];
  int type;
  ip_addr addr;
  
  n = dns_query(host, T_A, &ans);
  if (n < 0)
    return INADDR_NONE;

  /* find first satisfactory answer */

  cp = (u_char *) & ans + sizeof(HEADER);
  eom = (u_char *) & ans + n;

  for (qdcount = ntohs(ans.hdr.qdcount); qdcount--; cp += n + QFIXEDSZ)
  {
    if ((n = dn_skipname(cp, eom)) < 0)
      return INADDR_NONE;
  }

  ancount = ntohs(ans.hdr.ancount);

  while (--ancount >= 0 && cp < eom)
  {
    if ((n = dn_expand((void *) &ans, eom, cp, buf, MAXDNAME)) < 0)
      return INADDR_NONE;

    cp += n;

    type = getshort(cp);
    n = getshort(cp + 8);
    cp += 10;

    if (type == T_A)
    {
      addr.d[0] = cp[0];
      addr.d[1] = cp[1];
      addr.d[2] = cp[2];
      addr.d[3] = cp[3];
      return addr.addr;
    }

    cp += n;
  }

  return INADDR_NONE;
}

int
dns_open(host, port)
  char *host;
  int port;
{
  struct sockaddr_in sin;
  ip_addr addr;
  
  /* Thor.990811: check ¬O§_¤w¬°ip */
  if ((addr.addr = dns_aton(host)) != INADDR_NONE 
    || (addr.addr = dns_a(host)) != INADDR_NONE)
  {
    int sock;

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

    sin.sin_addr.s_addr = addr.addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
      return sock;

    if (!connect(sock, (struct sockaddr *) & sin, sizeof(sin)))
      return sock;

    close(sock);
  }
  return -1;
}

