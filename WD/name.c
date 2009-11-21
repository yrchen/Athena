/*-------------------------------------------------------*/
/* name.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : name-complete routines                       */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include "bbs.h"

struct word *toplev = NULL;
static struct word *current = NULL;
static char *msg_more = "[7m-- More --[0m";

static void
printdash(char *mesg)
{
  int head = 0, tail;

  if (mesg)
    head = (strlen(mesg) + 1) >> 1;

  tail = head;

  while (head++ < 38)
    outc('-');

  if (tail)
  {
    outc(' ');
    outs(mesg);
    outc(' ');
  }

  while (tail++ < 38)
    outc('-');
  outc('\n');
}

static void 
FreeNameList()
{
  struct word *p, *temp;

  for (p = toplev; p; p = temp)
  {
    temp = p->next;
    free(p->word);
    free(p);
  }
}

static void
BackUpNameList(struct word *backup[2], int mode)
{
  if(mode == 0)
  {
    /* backup */
    backup[0] = toplev;
    backup[1] = current;
    toplev = current = NULL;
  }
  else
  {
    /* ©ñ¦^¥h */
    if(toplev)
      FreeNameList();
    
    toplev = backup[0];
    current = backup[1];
  }  
}

void 
CreateNameList()
{
  if (toplev)
    FreeNameList();
  toplev = current = NULL;
}

void
AddNameList(char *name)
{
  struct word *node;

  node = (struct word *) malloc(sizeof(struct word));
  node->next = NULL;
  node->word = (char *) malloc(strlen(name) + 1);
  strcpy(node->word, name);

  if (toplev)
    current = current->next = node;
  else
    current = toplev = node;
}

int
RemoveNameList(char *name)
{
  struct word *curr, *prev = NULL;

  for (curr = toplev; curr; curr = curr->next)
  {
    if (!strcmp(curr->word, name))
    {
      if (prev == NULL)
        toplev = curr->next;
      else
        prev->next = curr->next;

      if (curr == current)
        current = prev;
      free(curr->word);
      free(curr);
      return 1;
    }
    prev = curr;
  }
  return 0;
}

int
InNameList(char *name)
{
  struct word *p;

  for (p = toplev; p; p = p->next)
  {
    if (!strcmp(p->word, name))
      return 1;
  }
  return 0;
}

void
ShowNameList(int row, int column, char *prompt)
{
  struct word *p;

  move(row, column);
  clrtobot();
  outs(prompt);

  column = 80;
  for (p = toplev; p; p = p->next)
  {
    row = strlen(p->word) + 1;
    if (column + row > 76)
    {
      column = row;
      outc('\n');
    }
    else
    {
      column += row;
      outc(' ');
    }
    outs(p->word);
  }
}

void
LoadNameList(int *reciper, char *listfile, char *msg)
{
  int fd;
  fileheader fhdr;

  if ((fd = open(listfile, O_RDONLY)) > 0)
  {
    while (read(fd, &fhdr, sizeof(fhdr)) == sizeof(fhdr))
    {
      if (!InNameList(fhdr.filename))
      {
        AddNameList(fhdr.filename);
        (*reciper)++;
      }
    }
    close(fd);
    ShowNameList(3, 0, msg);
  }
}

void
ToggleNameList(int *reciper, char *listfile, char *msg)
{
  int fd;
  fileheader fhdr;

  if ((fd = open(listfile, O_RDONLY)) > 0)
  {
    while (read(fd, &fhdr, sizeof(fhdr)) == sizeof(fhdr))
    {
      if (!InNameList(fhdr.filename))
      {
        AddNameList(fhdr.filename);
        (*reciper)++;
      }
      else {
        RemoveNameList(fhdr.filename);
        (*reciper)--;
      }
    }
    close(fd);
    ShowNameList(3, 0, msg);
  }
}

int
NumInList(struct word *list)
{
  register int i;

  for (i = 0; list; i++)
    list = list->next;
  return i;
}

static int
chkstr(char *otag, char *tag, char *name)
{
  char ch, *oname = name;

  while (*tag)
  {
    ch = *name++;
    if (*tag != toupper(ch))
      return 0;
    tag++;
  }
  if (*tag && *name == '\0')
    strcpy(otag, oname);
  return 1;
}


static struct word *
GetSubList(char *tag, struct word *list)
{
  struct word *wlist, *wcurr;
  char tagbuf[STRLEN];
  int n;

  wlist = wcurr = NULL;
  for (n = 0; tag[n]; n++)
  {
    tagbuf[n] = toupper(tag[n]);
  }
  tagbuf[n] = '\0';

  while (list)
  {
    if (chkstr(tag, tagbuf, list->word))
    {
      register struct word *node;

      node = (struct word *) malloc(sizeof(struct word));
      node->word = list->word;
      node->next = NULL;
      if (wlist)
        wcurr->next = node;
      else
        wlist = node;
      wcurr = node;
    }
    list = list->next;
  }
  return wlist;
}


static void
ClearSubList(struct word *list)
{
  struct word *tmp_list;

  while (list)
  {
    tmp_list = list->next;
    free(list);
    list = tmp_list;
  }
}


static int
MaxLen(struct word *list, int count)
{
  int len = strlen(list->word);
  int t;

  while (list && count)
  {
    if ((t = strlen(list->word)) > len)
      len = t;
    list = list->next;
    count--;
  }
  return len;
}

struct word *
AppearInList(struct word *cwlist, char *data)
{
  while (cwlist) 
  {
    if (!strcasecmp(cwlist->word, data))
      return cwlist;
    cwlist = cwlist->next;
  }

  return NULL;
}

void
namecomplete(char *prompt, char *data)
{
  char *temp;
  struct word *cwlist, *morelist;
  int x, y, origx, origy;
  int ch;
  int count = 0;
  int clearbot = NA;
  if (toplev == NULL)
    AddNameList("");
  cwlist = GetSubList("", toplev);
  morelist = NULL;
  temp = data;

  outs(prompt);
  clrtoeol();
  getyx(&y, &x);
  getyx(&origy, &origx);
  standout();
  prints("%*s", IDLEN + 1, "");
  standend();
  move(y, x);
  refresh();

  while ((ch = igetkey()) != EOF)
  {
    if (ch == '\n' || ch == '\r')
    {
      struct word *tempptr=NULL;
      *temp = '\0';
      outc('\n');
      if (tempptr=AppearInList(cwlist, data))
        strcpy(data, tempptr->word);
      else if(NumInList(cwlist) == 1)
        strcpy(data, cwlist->word);
      else
        *data = '\0';

      ClearSubList(cwlist);
      break;
    }
    if (ch == ' ')
    {
      int col, len, i;
      
      /* ¦Û°Ê¸É§¹ */
      len = NumInList(cwlist);
      while( (i = cwlist->word[count]) != '\0')
      {
        struct word *node;

        *temp++ = i;
        *temp = '\0';
        node = GetSubList(data, cwlist);
        col = NumInList(node);

        if (col != len)
        {
          temp--;
          *temp = '\0';
          if(node != NULL)
            ClearSubList(node);
          break;
        }
        ClearSubList(cwlist);
        cwlist = node;
        count++;
        move(y, x);
        outc(i);
        x++;
      }

      if (NumInList(cwlist) == 1)
        continue;

      clearbot = YEA;
      col = 0;
      if (!morelist)
        morelist = cwlist;
      len = MaxLen(morelist, p_lines);
      move(2, 0);
      clrtobot();
      printdash("¬ÛÃö¸ê°T¤@Äýªí");
      while (len + col < 80)
      {
        for (i = p_lines; (morelist) && (i > 0); i--)
        {
          move(3 + (p_lines - i), col);
          outs(morelist->word);
          morelist = morelist->next;
        }
        col += len + 2;
        if (!morelist)
          break;
        len = MaxLen(morelist, p_lines);
      }
      if (morelist)
      {
        move(b_lines, 0);
        outs(msg_more);
      }
      move(y, x);
      continue;
    }
    else if (ch == '\177' || ch == '\010')
    {
      if (temp == data)
        continue;
      temp--;
      count--;
      *temp = '\0';
      ClearSubList(cwlist);
      cwlist = GetSubList(data, toplev);
      morelist = NULL;
      x--;
      move(y, x);
      outc(' ');
      move(y, x);
      continue;
    }
    else if (count < STRLEN && isprint(ch))
    {
      struct word *node;

      *temp++ = ch;
      count++;
      *temp = '\0';
      node = GetSubList(data, cwlist);
      if (node == NULL)
      {
        temp--;
        *temp = '\0';
        count--;
        continue;
      }
      ClearSubList(cwlist);
      cwlist = node;
      morelist = NULL;
      move(y, x);
      outc(ch);
      x++;
    }
  }
  if (ch == EOF)
    raise(SIGHUP);          /* jochang: don't know if this is
                             * necessary... */
  outc('\n');
  refresh();
  if (clearbot)
  {
    move(2, 0);
    clrtobot();
  }
  if (*data)
  {
    move(origy, origx);
    outs(data);
    outc('\n');
  }
}


/* ---------------------------------------------------- */
/* name complete for user ID                             */
/* ---------------------------------------------------- */
void
usercomplete(char *prompt, char *data)
{
  int total, i;
  struct word *backup[2];
  
  resolve_ucache();
  total = uidshm->number;

  BackUpNameList(backup, 0);
  for(i=0;i<total;i++)
  {
    if(uidshm->userid[i][0] != '\0')
      AddNameList(uidshm->userid[i]);
  }
  
  namecomplete(prompt, data);
  BackUpNameList(backup, 1);
}

void
brdcomplete(char *prompt, char *data)
{
  int i, total;
  struct word *backup[2];

  resolve_boards();
  total = brdshm->number;

  BackUpNameList(backup, 0);
  for(i=0;i<total;i++)
  {
    if(!Ben_Perm(&brdshm->bcache[i]))
      continue;
    AddNameList(brdshm->bcache[i].brdname);
  }
  
  namecomplete(prompt, data);
  BackUpNameList(backup, 1);
}
