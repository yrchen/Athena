#include <fcntl.h>
#include <stdio.h>

void
b_suckinfile(fp, fname)
  FILE *fp;
  char *fname;
{
  FILE *sfp;

  if ((sfp = fopen(fname, "r")) != NULL)
  {
    char inbuf[256];

    while (fgets(inbuf, sizeof(inbuf), sfp))
      fputs(inbuf, fp);
    fclose(sfp);
  }
}
