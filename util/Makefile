# ------------------------------------------------------- #
#  util/Makefile	( NTHU CS MapleBBS Ver 2.36 )	  #
# ------------------------------------------------------- #
#  target : Makefile for 寄信、統計、備份、系統維護工具	  #
#  create : 95/03/29				 	  #
#  update : 95/12/15				 	  #
# ------------------------------------------------------- #
BBSHOME=../..

# For FreeBSD
CC	= gcc34
CFLAGS	= -O -DBSD44 -pipe -fomit-frame-pointer -I../include -I../WD -D_BBS_UTIL_C_
LDFLAGS	= -s -L../lib -ldao -lcrypt

# For SunOS 5.6
#CC	= gcc
#CFLAGS	= -O2 -pipe -fomit-frame-pointer -I../include -I../WD -DSYSV -DSOLARIS
#LDFLAGS	= -ltermcap -lnsl -lsocket -ldl -L../lib -ldao

# For Linux
#CC	= gcc
#CFLAGS	= -O2 -pipe -fomit-frame-pointer -I../include -I../WD -DLINUX -D_BBS_UTIL_C_
#LDFLAGS	= -s -fforce-mem -funroll-loops -fstrict-prototype -Wunused -L../lib -ldao -lcrypt

.SUFFIXES: .o .c

.c:   ;   $(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)

TOOL =	bbsmail bfinger clrdir expire showb rmtusr mailpost\
	showboard showuser showdir setuser bbsadm delspam query BM_money\
	mandex setboard openticket cna query\
	bbspost emailist hfinger hint byte\
	rmtrash allgoldexpire camera brdshmctl initbbs enews-open
#	bpop3d bhttpd bgopherd blookd man2web brdpop3d
#	allboardfix bet_open camera

all: 	$(TOOL)
	@cd fixtool;make all
	@cd statool;make all


install:$(TOOL)
	strip $(TOOL)
#	chown bbs2.bbs2 $(TOOL)
	cp -fp $(TOOL) $(BBSHOME)/bin
	@cd fixtool;make install
	@cd statool;make install

clean:
	rm -fr $(TOOL) *.o *.BAK *.log *~
	@cd fixtool;make clean
	@cd statool;make clean
