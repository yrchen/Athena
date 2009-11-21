# ------------------------------------------------------ #
#  Makefile	( WD-BBS Ver 2.3)			 #
# ------------------------------------------------------ #
#  target : Makefile for WD-BBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 98/12/21					 #
# ------------------------------------------------------ #
BBSHOME = ../..

# ------------------------------------------------------
# FreeBSD 3.x 4.x
# ------------------------------------------------------
OS_DEF	= -DBSD44 -DTWO_LONGJMP_ARGS 
CC	= gcc
CFLAGS	= -O1 -Wunused -I../include -pipe -fomit-frame-pointer -s -I/usr/local/include#-g -ggdb
LDFLAGS	= -lcrypt -lcompat -L../lib -ldao -export-dynamic -lm #-L/usr/local/lib -liconv#-ltermcap #-L/usr/local/lib/mysql -lmysqlclient
CPROTO	= cproto -E"gcc -pipe -E -I../include"

# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

SRC =	announce.c bbs.c board.c cache.c edit.c \
	mail.c main.c menu.c more.c name.c read.c record.c\
	stuff.c talk.c user.c\
	money.c list.c votebrd.c\
	window.c msg.c color.c loginplan.c visio.c favor.c\

OBJ =	announce.o bbs.o board.o cache.o edit.o \
	mail.o main.o menu.o more.o name.o read.o record.o\
	stuff.o talk.o user.o\
	money.o list.o votebrd.o\
	window.o msg.o color.o loginplan.o visio.o favor.o\

MOBJ =	announce.o bbs.o board.o cache.o edit.o \
	mail.o mbbsd.o menu.o more.o name.o read.o record.o\
	stuff.o talk.o user.o\
	money.o list.o votebrd.o\
	window.o msg.o color.o loginplan.o visio.o favor.o\

PRO =	announce.x bbs.x board.x cache.x edit.x \
	mail.x main.x menu.x more.x name.x read.x record.x\
	stuff.x talk.x user.x\
	money.x list.x votebrd.x\
	window.x msg.x color.x loginplan.x visio.x\

EXE =	bbs

ETC =	Makefile $(HDR)

       
.SUFFIXES: .o .c .ln .x

.c.o:   ;   $(CC) $(CFLAGS) $(OS_DEF) -c $*.c
.c.x:   ;   $(CPROTO) -o $*.x $*.c
.c.ln:  ;   lint -abhi $*.c

BIN =	$(EXE) mbbsd bbsrf xchatd

ALL =   $(BIN) in.zbbsd

all: $(ALL)

$(EXE): $(OBJ) $(ETC)
	@echo "====================="
	@echo "      Build bbs"
	@echo "====================="
	$(CC) -o $(EXE) $(CFLAGS) $(OBJ) $(LDFLAGS)


mbbsd:	$(MOBJ) $(ETC)
	@echo "====================="
	@echo "     Build mbbsd"
	@echo "====================="
	$(CC) -o mbbsd $(CFLAGS) $(MOBJ) $(LDFLAGS)
     
lint: $(LNFILES)
	lint -abh $(LNFILES)

xchatd: xchatd.c
	@echo "====================="
	@echo "     Build xchat"
	@echo "====================="
	$(CC) -o xchatd $(OS_DEF) $(CFLAGS) xchatd.c $(LDFLAGS)

bbsrf: bbsrf.c 
	@echo "====================="
	@echo "     Build bbsrf"
	@echo "====================="
	$(CC) $(CFLAGS) -o bbsrf $(OS_DEF) bbsrf.c

in.zbbsd: in.zbbsd.c
	@echo "====================="
	@echo "   Build in.zbbsd"
	@echo "====================="
	$(CC) $(CFLAGS) -o in.zbbsd $(OS_DEF) in.zbbsd.c $(LDFLAGS)

newbbs: $(EXE)
	if [ -f $(BBSHOME)/bin/bbs ]; then \
	  mv -f $(BBSHOME)/bin/bbs $(BBSHOME)/bin/bbs.old; \
	fi
	if [ -f $(BBSHOME)/bin/mbbsd ]; then \
	  mv -f $(BBSHOME)/bin/mbbsd $(BBSHOME)/bin/mbbsd.old; \
	fi
	cp bbs $(BBSHOME)/bin
	cp mbbsd $(BBSHOME)/bin

install: $(ALL)
	if [ -f $(BBSHOME)/bin/bbs ]; then\
	  mv -f $(BBSHOME)/bin/bbs $(BBSHOME)/bin/bbs.old; \
	fi
	if [ -f $(BBSHOME)/bin/mbbsd ]; then\
	  mv -f $(BBSHOME)/bin/mbbsd $(BBSHOME)/bin/mbbsd.old; \
	fi
	if [ -f $(BBSHOME)/bin/bbsrf ]; then\
	  mv -f $(BBSHOME)/bin/bbsrf $(BBSHOME)/bin/bbsrf.old; \
	fi
	if [ -f $(BBSHOME)/bin/xchatd ]; then\
	  mv -f $(BBSHOME)/bin/xchatd $(BBSHOME)/bin/xchatd.old; \
	fi
	if [ -f $(BBSHOME)/bin/in.zbbsd ]; then\
	  mv -f $(BBSHOME)/bin/in.zbbsd $(BBSHOME)/bin/in.zbbsd.old; \
	fi
#	chown bbs.bbs $(ALL) $(HDR) $(SRC) $(OBJ)
#
# shakalaca.990401: 前面 LIBS = -s 與 strip 效果相同
#	strip $(ALL)
#
	ls -l bbs
	ls -l mbbsd
	cp -p $(ALL) $(BBSHOME)/bin

WD.p: $(SRC)
	$(CPROTO) -o $@ $?

proto.x: $(PRO)
	/bin/rm	-f proto.x
	/bin/cat $(PRO) > proto.x

clean: /tmp
	rm -fr $(GARBAGE) $(OBJ) $(MOBJ) $(ALL) $(PRO) proto.x $(LNFILES) *~ DEADJOE

tags:
	rm -f tags
	ctags $(SRC)
