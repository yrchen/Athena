#include <stdio.h>

void setadir(char *buf, char *path)
{
  sprintf(buf, "%s/%s", path, ".DIR");
}

void setapath(char *buf, char *boardname)
{
  sprintf(buf, "man/boards/%s", boardname);
}

void setbfile(char *buf, char *boardname, char *fname)
{
  sprintf(buf, "boards/%s/%s", boardname, fname);
}

void setbgdir(char *buf, char *boardname)                                      
{                                                             
  sprintf(buf, "boards/%s/%s", boardname, ".Names");
}

void setbpath(char *buf, char *boardname)
{
  sprintf(buf, "boards/%s", boardname);
}

void sethomedir(char *buf, char *userid)
{
  sprintf(buf, "home/%s/%s", userid, ".DIR");
}

void sethomefile(char *buf, char *userid, char *fname)
{
  sprintf(buf, "home/%s/%s", userid, fname);
}

void sethomeman(char *buf, char *userid)
{
  sprintf(buf, "home/%s/%s", userid, "man");
}

void sethomepath(char *buf, char *userid)
{
  sprintf(buf, "home/%s", userid);
}

