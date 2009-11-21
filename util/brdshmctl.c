/* $Id: brdshmctl.c,v 1.6 2003/11/30 15:16:41 sby Exp $ */

#include "bbs.h"
#include "cache.c"

struct UTMPFILE *utmpshm;

void
resolve_utmp()
{
  if (utmpshm == NULL)
  {
    utmpshm = shm_new(UTMPSHM_KEY, sizeof(*utmpshm));

    if (utmpshm->uptime == 0)
      utmpshm->uptime = utmpshm->number = 1;
  }
}

static int
bfriends_count(const int bid)
{
  register int bfriends_number = 0, i;
  user_info *uentp = NULL;

  for (i = 0; i < USHM_SIZE; i++) 
  {
    uentp = &utmpshm->uinfo[i];

    if (bid && uentp->brc_id == bid)
    {
      bfriends_number++;
    }
  }

  return bfriends_number;
}

int
main()
{
  register int bid;

  resolve_utmp();
  resolve_boards();

  for (bid = 0; bid < MAXBOARD; bid++)
  {
    if (brdshm->bcache[bid].brdname[0])
      brdshm->nusers[bid] = bfriends_count(bid + 1);
  }
  return 0;
}

