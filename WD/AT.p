/* announce.c */
FILE *go_cmd(ITEM *node, int *sock);
char *nextfield(register char *data, register char *field);
FILE *my_open(char *path);
void gem(char *maintitle, ITEM *path, int update);
int load_paste(void);
int a_copyitem(char *fpath, char *title, char *owner);
int a_menu(char *maintitle, char *path, int lastlevel, int mode);
int AnnounceSelect(void);
int Announce(void);
int XFile(void);
int HELP(void);
int user_gem(char *uid);
void my_gem(void);
