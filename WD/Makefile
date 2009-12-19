# ------------------------------------------------------ #
#  Makefile	( WD-BBS Ver 2.3)			 #
# ------------------------------------------------------ #
#  target : Makefile for WD-BBS main programs		 #
#  create : 95/03/29				 	 #
#  update : 98/12/21					 #
# ------------------------------------------------------ #
BBSHOME = /home/bbs

# ------------------------------------------------------
# FreeBSD 3.x 4.x
# ------------------------------------------------------
OS_DEF	= -DBSD44 #-DTWO_LONGJMP_ARGS 
CC	= gcc
CFLAGS	= -O2 -Wunused -I../include -pipe -fomit-frame-pointer -s #-I/usr/local/include#-g -ggdb
LDFLAGS	= -lcrypt -lcompat -lkvm -lm -L../lib -ldao -export-dynamic #-L/usr/local/lib -liconv
CPROTO	= cproto -E"gcc -pipe -E -I../include"

# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

SRC =	announce.c bbs.c board.c cache.c edit.c var.c\
	mail.c menu.c more.c name.c read.c record.c\
	stuff.c talk.c user.c money.c list.c\
	msg.c loginplan.c visio.c favor.c form.c osdep.c\
	mbbsd.c pmore.c

OBJ =	announce.o bbs.o board.o cache.o edit.o var.o\
	mail.o menu.o more.o name.o read.o record.o\
	stuff.o talk.o user.o money.o list.o\
	msg.o loginplan.o visio.o favor.o form.o osdep.o pmore.o\

PRO =	announce.x bbs.x board.x cache.x edit.x var.x\
	mail.x menu.x more.x name.x read.x record.x\
	stuff.x talk.x user.x money.x list.x\
	msg.x loginplan.x visio.x favor.x form.x osdep.x pmore.x\

EXE =	bbs

ETC =	Makefile $(HDR)

       
.SUFFIXES: .o .c .ln .x

.c.o:   ;   $(CC) $(CFLAGS) $(OS_DEF) -c $*.c
.c.x:   ;   $(CPROTO) -o $*.x $*.c
.c.ln:  ;   lint -abhi $*.c

BIN =	$(EXE) mbbsd xchatd

ALL =   $(BIN)

all: $(ALL)

$(EXE): $(OBJ) $(ETC) main.o
	@echo "====================="
	@echo "      Build bbs"
	@echo "====================="
	$(CC) -o $(EXE) $(CFLAGS) $(OBJ) main.o $(LDFLAGS)


mbbsd:	$(OBJ) $(ETC) mbbsd.o
	@echo "====================="
	@echo "     Build mbbsd"
	@echo "====================="
	$(CC) -o mbbsd $(CFLAGS) $(OBJ) mbbsd.o $(LDFLAGS)
     
lint: $(LNFILES)
	lint -abh $(LNFILES)

xchatd: xchatd.c
	@echo "====================="
	@echo "     Build xchat"
	@echo "====================="
	$(CC) -o xchatd $(OS_DEF) $(CFLAGS) xchatd.c $(LDFLAGS)

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
	@echo "====================="
	@echo " Copy Files to Update"
	@echo "====================="
	if [ -f $(BBSHOME)/bin/bbs ]; then\
	  mv -f $(BBSHOME)/bin/bbs $(BBSHOME)/bin/bbs.old; \
	fi
	if [ -f $(BBSHOME)/bin/mbbsd ]; then\
	  mv -f $(BBSHOME)/bin/mbbsd $(BBSHOME)/bin/mbbsd.old; \
	fi
	if [ -f $(BBSHOME)/bin/xchatd ]; then\
	  mv -f $(BBSHOME)/bin/xchatd $(BBSHOME)/bin/xchatd.old; \
	fi
#	chown bbs.bbs $(ALL) $(HDR) $(SRC) $(OBJ)
#
# shakalaca.990401: 前面 LIBS = -s 與 strip 效果相同
#	strip $(ALL)
#
	ls -l bbs
	ls -l mbbsd
	cp -p $(ALL) $(BBSHOME)/bin
	tail -n 1 ../../run/mbbsd.pid

AT.p: $(SRC)
	if [ -f AT.p ]; then\
	  mv -f AT.p AT_old.p; \
	fi
	$(CPROTO) -o $@ $?

proto.x: $(PRO)
	/bin/rm	-f proto.x
	/bin/cat $(PRO) > proto.x

clean: /tmp
	rm -fr $(GARBAGE) $(OBJ) $(MOBJ) $(ALL) $(PRO) proto.x $(LNFILES) *~ DEADJOE main.o mbbsd.o

tags: $(SRC)
	ctags $(SRC)
