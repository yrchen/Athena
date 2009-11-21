# ------------------------------------------------------ #
#  lib/Makefile		( NTHU CS MapleBBS Ver 3.00 )	 #
# ------------------------------------------------------ #
#  author : opus.bbs@bbs.cs.nthu.edu.tw		 	 #
#  target : Makefile for MapleBBS library routines	 #
#  create : 95/03/29				 	 #
#  update : 95/12/15					 #
# ------------------------------------------------------ #

CC	= gcc34
RANLIB	= ranlib
CPROTO	= cproto -E"gcc -pipe -E"
CFLAGS	= -O2 -s -pipe -fomit-frame-pointer -Werror -Wall -I../include -I/usr/include

# ------------------------------------------------------ #
# 下列的 make rules 不需修改				 #
# ------------------------------------------------------ #

HDR =	dao.h dao.p

DNS =	\
	dns.c dns_addr.c dns_name.c dns_smtp.c dns_ident.c dns_open.c dns_aton.c\

SRC =	\
	is_alnum.c is_alpha.c is_fpath.c not_addr.c \
	\
	str_ansi.c str_cat.c str_cmp.c str_decode.c str_fpath.c\
	str_from.c str_has.c str_hash.c str_lower.c \
	str_ncmp.c str_ncpy.c str_passwd.c str_stamp.c str_time.c \
	str_trim.c str_ttl.c str_trim2.c\
	\
	archiv32.c radix32.c\
	\
	f_cat.c f_cp.c f_img.c f_ln.c f_lock.c f_map.c f_mode.c f_mv.c \
	f_path.c f_rm.c f_suck.c mak_dirs.c\
	\
	rec_add.c rec_num.c rec_get.c rec_ins.c rec_put.c rec_apply.c\
	\
	\
	shm.c sem.c\
	\
	dl_lib.c\
	\
	xsort.c\
	\
	setpf.c strip_ansi.c cut_ansistr.c isprint2.c \
	not_alnum.c not_alpha.c strstr_lower.c nextfield.c counter.c\
	b_suckinfile.c invalid_brdname.c url_encode.c\
	\
	rfc2047.c\

OBJ =	$(SRC:.c=.o) $(DNS:.c=.o)


.c.o:   ;   $(CC) $(CFLAGS) -c $*.c


all:	libdao.a


install:	libdao.a


dao.p:	$(SRC)
	$(CPROTO) -o $@ $?

dns.p:	$(DNS)
	$(CPROTO) -o $@ $?

libdao.a:	dao.p dns.p $(OBJ)
	ar rv $@ $?
	$(RANLIB) $@

clean:
	rm -fr $(GARBAGE) $(OBJ) libdao.a *.bak *.BAK *~


tags:
	ctags $(SRC) $(DNS) ../include/*.h
