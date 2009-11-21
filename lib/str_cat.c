void
str_cat(char *dst, char *s1, char *s2)
{
  while ((*dst = *s1) != 0)
  {
    s1++;
    dst++;
  }

  while ((*dst++ = *s2++) != 0)
    ;
}
