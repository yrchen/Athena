int
str_hash(char *str, int seed)
{
  int cc;

  while ((cc = *str++) != 0)
  {
    seed = (seed << 5) - seed + cc;	/* 31 * seed + cc */
  }
  return (seed & 0x7fffffff);
}
