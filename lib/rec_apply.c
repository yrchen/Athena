#include "dao.h"
#include <fcntl.h>
#include <unistd.h>

int
rec_apply(fpath, fptr, size)
  char *fpath;
  int (*fptr) ();
  int size;
{
  //char buf[REC_SIZ];
  void *buf;
  int fd;

  if ((fd = open(fpath, O_RDONLY)) == -1)
    return -1;

  buf = malloc(size);
  while (read(fd, buf, size) == size)
  {
    if ((*fptr) (buf))
    {
      close(fd);
      free(buf);
      return -2;
    }
  }
  close(fd);
  free(buf);
  return 0;
}
