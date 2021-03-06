/* $Id$ */

/*
 * pmore: piaip's more, a new replacement for traditional pager
 *
 * piaip's new implementation of pager(more) with mmap,
 * designed for unlimilited length(lines).
 *
 * "pmore" is "piaip's more", NOT "PTT's more"!!!
 * pmore is designed for general BBS systems, not 
 * specific to any branch.
 *
 * Author: Hung-Te Lin (piaip), June 2005.
 * <piaip@csie.ntu.edu.tw>
 * All Rights Reserved.
 *
 * MAJOR IMPROVEMENTS:
 *  - Clean source code, and more readble to mortal
 *  - Correct navigation
 *  - Excellent search ability (for correctness and user behavior)
 *  - Less memory consumption (mmap is not considered anyway)
 *  - Better support for large terminals
 *  - Unlimited file length and line numbers
 *
 * TODO ANE DONE:
 *  - Optimized speed up with Scroll supporting [done]
 *  - Support PTT_PRINTS [done]
 *  - Wrap long lines [done]
 *  - DBCS friendly wrap [done]
 *  - ASCII Art movie support [done]
 *  - Left-right wide navigation [done]
 *  - Reenrtance for main procedure [done with little hack]
 *  - 
 *  - A new optimized terminal base system (piterm)
 *  - ASCII Art movie navigation keys
 */

// --------------------------------------------------------------- <FEATURES>
/* These are default values.
 * You may override them in your bbs.h or config.h etc etc.
 */
#define PMORE_PRELOAD_SIZE (64*1024L)	// on busy system set smaller or undef

#define PMORE_USE_PTT_PRINTS		// support PTT or special printing
#define PMORE_USE_OPT_SCROLL		// optimized scroll
#define PMORE_USE_DBCS_WRAP		// safe wrap for DBCS.
#define PMORE_USE_ASCII_MOVIE		// support ascii movie
#define PMORE_WORKAROUND_POORTERM	// try to work with poor terminal sys
#define PMORE_ACCURATE_WRAPEND		// try more harder to find file end in wrap mode

#define PMORE_TRADITIONAL_PROMPTEND	// when prompt=NA, show only page 1
#define PMORE_TRADITIONAL_FULLCOL	// to work with traditional ascii arts
// -------------------------------------------------------------- </FEATURES>

#include "bbs.h"

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h>
#include <string.h>

// Platform Related. NoSync is faster but if we don't have it...
#ifdef MAP_NOSYNC
#define MF_MMAP_OPTION (MAP_NOSYNC)
#else
#define MF_MMAP_OPTION (MAP_SHARED)
#endif

/* Developer's Guide
 * 
 * OVERVIEW
 *  - pmore is designed as a line-oriented pager. After you load (mf_attach)
 *    a file, you can move current display window by lines (mf_forward and
 *    mf_backward) and then display a page(mf_display).
 *    And please remember to delete allocated resources (mf_detach)
 *    when you exit.
 *  - Functions are designed to work with global variables.
 *    However you can overcome re-entrance problem by backuping up variables
 *    or replace all "." to "->" with little modification and add pointer as
 *    argument passed to each function. 
 *    (This is really tested and it works, however then using global variables
 *    is considered to be faster and easier to maintain, at lease shorter in
 *    time to key-in and for filelength).
 *  - Basically this file should only export one function, "pmore".
 *    Using any other functions here may be dangerous because they are not
 *    coded for external reentrance rightnow.
 *  - mf_* are operation functions to work with file buffer.
 *    Usually these function assumes "mf" can be accessed.
 *  - pmore_* are utility functions
 *
 * DETAILS
 *  - The most tricky part of pmore is the design of "maxdisps" and "maxlinenoS".
 *    What do they mean? "The pointer and its line number of last page".
 *    - Because pmore is designed to work with very large files, it's costly to
 *      calculate the total line numbers (and not necessary).  But if we don't
 *      know about how many lines left can we display then when navigating by
 *      pages may result in a page with single line conent (if you set display
 *      starting pointer to the real last line).
 *    - To overcome this issue, maxdisps is introduced. It tries to go backward
 *      one page from end of file (this operation is lighter than visiting 
 *      entire file content for line number calculation). Then we can set this
 *      as boundary of forward navigation.
 *    - maxlinenoS is the line number of maxdisps. It's NOT the real number of
 *      total line in current file (You have to add the last page). That's why
 *      it has a strange name of trailing "S", to hint you that it's not 
 *      "maxlineno" which is easily considered as "max(total) line number".
 *
 * HINTS:
 *  - Remember mmap pointers are NOT null terminated strings.
 *    You have to use strn* APIs and make sure not exceeding mmap buffer.
 *    DO NOT USE strcmp, strstr, strchr, ...
 *  - Scroll handling is painful. If you displayed anything on screen,
 *    remember to MFDISP_DIRTY();
 *  - To be portable between most BBS systems, pmore is designed to
 *    workaround most BBS bugs inside itself.
 *  - Basically pmore considered the 'outc' output system as unlimited buffer.
 *    However on most BBS implementation, outc used a buffer with ANSILINELEN
 *    in length. And for some branches they even used unsigned byte for index.
 *    So if user complained about output truncated or blanked, increase buffer.
 */

#ifdef DEBUG
int debug = 0;
# define MFPROTO
#else
# define MFPROTO inline static
#endif

// --------------------------------------------- <Defines and constants>

// --------------------------- <Display>

/* ANSI COMMAND SYSTEM */
/* On some systems with pmore style ANSI system applied,
 * we don't have to define these again.
 */
#ifndef PMORE_STYLE_ANSI
#define PMORE_STYLE_ANSI

// Escapes. I don't like \033 everywhere.
#define ESC_NUM (0x1b)
#define ESC_STR "\x1b"
#define ESC_CHR '\x1b'

// Common ANSI commands.
#define ANSI_RESET  ESC_STR "[m"
#define ANSI_COLOR(x) ESC_STR "[" #x "m"
#define ANSI_MOVETO(y,x) ESC_STR "[" #y ";" #x "H"
#define ANSI_CLRTOEND ESC_STR "[K"

#define ANSI_IN_ESCAPE(x) (((x) >= '0' && (x) <= '9') || \
	(x) == ';' || (x) == ',' || (x) == '[')

#endif /* PMORE_STYLE_ANSI */

// Poor BBS terminal system Workarounds
// - Most BBS implements clrtoeol() as fake command
//   and usually messed up when output ANSI quoted string.
// - A workaround is suggested by kcwu:
//   https://opensvn.csie.org/traccgi/pttbbs/trac.cgi/changeset/519
#define FORCE_CLRTOEOL() outs(ANSI_CLRTOEND)

/* Again, if you have a BBS system which optimized out* without recognizing
 * ANSI escapes, scrolling with ANSI text may result in melformed text (because
 * ANSI escapes were "optimized" ). So here we provide a method to overcome
 * with this situation. However your should increase your I/O buffer to prevent
 * flickers.
 */
MFPROTO void 
pmore_clrtoeol(int y, int x)
{
#ifdef PMORE_WORKAROUND_POORTERM
    int i; 
    move(y, x); 
    for (i = x; i < t_columns; i++) 
	outc(' '); 
    move(y, x);
#else
    move(y, x);
    clrtoeol();
#endif
}

// --------------------------- </Display>

// --------------------------- <Main Navigation>
typedef struct
{
    unsigned char 
	*start, *end,	// file buffer
	*disps, *dispe,	// displayed content start/end
	*maxdisps;	// a very special pointer, 
    			//   consider as "disps of last page"
    off_t len; 		// file total length
    long  lineno,	// lineno of disps
	  oldlineno,	// last drawn lineno, < 0 means full update
	  xpos,		// starting x position
	  		//
	  wraplines,	// wrapped lines in last display
	  trunclines,	// truncated lines in last display
	  dispedlines,	// how many different lines displayed
	  		//  usually dispedlines = PAGE-wraplines,
			//  but if last line is incomplete(wrapped),
			//  dispedlines = PAGE-wraplines + 1
	  lastpagelines,// lines of last page to show
	                //  this indicates how many lines can
			//  maxdisps(maxlinenoS) display.
	  maxlinenoS;	// lineno of maxdisps, "S"! 
			//  What does the magic "S" mean?
			//  Just trying to notify you that it's 
    			//  NOT REAL MAX LINENO NOR FILELENGTH!!!
			//  You may consider "S" of "Start" (disps).
} MmappedFile;

MmappedFile mf = { 
    0, 0, 0, 0, 0, 0L,
    0, -1L, 0, 0, -1L, -1L, -1L,-1L
};	// current file

/* mf_* navigation commands return value meanings */
enum {
    MFNAV_OK,		// navigation ok
    MFNAV_EXCEED,	// request exceeds buffer
} MF_NAV_COMMANDS;

/* Navigation units (dynamic, so not in enum const) */
#define MFNAV_PAGE  (t_lines-2)	// when navigation, how many lines in a page to move

/* Display system */
enum {
    /* newline method (because of poor BBS implementation) */
    MFDISP_NEWLINE_CLEAR = 0, // \n and cleartoeol
    MFDISP_NEWLINE_SKIP,
    MFDISP_NEWLINE_MOVE,  // use move to simulate newline.

    MFDISP_WRAP_TRUNCATE = 0,
    MFDISP_WRAP_WRAP,

    MFDISP_OPT_CLEAR = 0,
    MFDISP_OPT_OPTIMIZED,
    MFDISP_OPT_FORCEDIRTY,

    MFDISP_SEP_NONE = 0x00,
    MFDISP_SEP_LINE = 0x01,
    MFDISP_SEP_WRAP = 0x02,
    MFDISP_SEP_OLD  = MFDISP_SEP_LINE | MFDISP_SEP_WRAP,

    MFDISP_RAW_NA   = 0x00,
    MFDISP_RAW_NOANSI,
    MFDISP_RAW_PLAIN,
    MFDISP_RAW_MODES,
    // MFDISP_RAW_NOFMT, // this is rarely used sinde we have ansi and plain

} MF_DISP_CONST;

#define MFDISP_PAGE (t_lines-1) // the real number of lines to be shown.
#define MFDISP_DIRTY() { mf.oldlineno = -1; }

/* Indicators */
#define MFDISP_TRUNC_INDICATOR	ANSI_COLOR(0;1;37) ">" ANSI_RESET
#define MFDISP_WRAP_INDICATOR	ANSI_COLOR(0;1;37) "\\" ANSI_RESET
#define MFDISP_WNAV_INDICATOR	ANSI_COLOR(0;1;37) "<" ANSI_RESET
// --------------------------- </Main Navigation>

// --------------------------- <Aux. Structures>
/* browsing preference */
typedef struct
{
    /* mode flags */
    unsigned short int
	wrapmode,	// wrap?
	seperator,	// seperator style
    	indicator,	// show wrap indicators

	oldwrapmode,	// traditional wrap
        oldstatusbar,	// traditional statusbar
	rawmode;	// show file as-is.
} MF_BrowsingPrefrence;

MF_BrowsingPrefrence bpref =
{ MFDISP_WRAP_WRAP, MFDISP_SEP_OLD, 1, 
    0, 0, 0, };

/* pretty format header */
#define FH_HEADERS    (4)  // how many headers do we know?
#define FH_HEADER_LEN (4)  // strlen of each heads
static const char *_fh_disp_heads[FH_HEADERS] = 
    {"作者", "標題", "時間", "轉信"};

typedef struct
{
    int lines;	// header lines
    unsigned char *headers[FH_HEADERS];
    unsigned char *floats[2];	// right floating, name and val
} MF_PrettyFormattedHeader;

MF_PrettyFormattedHeader fh = { 0, {0,0,0,0}, {0, 0}};

/* search records */
typedef struct
{
    int  len;
    int (*cmpfunc) (const char *, const char *, size_t);
    char *search_str;	// maybe we can change to dynamic allocation
} MF_SearchRecord;

MF_SearchRecord sr = { 0, strncmp, NULL};

enum {
    MFSEARCH_FORWARD,
    MFSEARCH_BACKWARD,
} MFSEARCH_DIRECTION;

// Reset structures
#define RESETMF() { memset(&mf, 0, sizeof(mf)); \
    mf.lastpagelines = mf.maxlinenoS = mf.oldlineno = -1; }
#define RESETFH() { memset(&fh, 0, sizeof(fh)); \
    fh.lines = -1; }

// --------------------------- </Aux. Structures>

// --------------------------------------------- </Defines and constants>

// --------------------------------------------- <Optional Modules>
#ifdef PMORE_USE_ASCII_MOVIE
enum {
    MFDISP_MOVIE_UNKNOWN= 0,
    MFDISP_MOVIE_DETECTED,
    MFDISP_MOVIE_YES,
    MFDISP_MOVIE_NO,
    MFDISP_MOVIE_PLAYING,
    MFDISP_MOVIE_PLAYING_OLD,
}  _MFDISP_MOVIE_MODES;

typedef struct {
    struct timeval frameclk;
    struct timeval synctime;
    unsigned char  mode,
		   compat24;
} MF_Movie;

MF_Movie mfmovie;

#define RESET_MOVIE() { mfmovie.mode = MFDISP_MOVIE_UNKNOWN; \
    mfmovie.compat24 = 1; \
    mfmovie.synctime.tv_sec = mfmovie.synctime.tv_usec = 0; \
    mfmovie.frameclk.tv_sec = 1; mfmovie.frameclk.tv_usec = 0; }

MFPROTO unsigned char * mf_movieFrameHeader(unsigned char *p);
int pmore_wait_input(struct timeval *ptv);
int mf_movieNextFrame();
int mf_movieSyncFrame();

void float2tv(float f, struct timeval *ptv);

#define MOVIE_MIN_FRAMECLK (0.1f)
#define MOVIE_SECOND_U (1000000L)

#endif
// --------------------------------------------- </Optional Modules>

// used by mf_attach
void mf_parseHeaders();
void mf_freeHeaders();
void mf_determinemaxdisps(int, int);

/* 
 * mmap basic operations 
 */
int 
mf_attach(unsigned char *fn)
{
    struct stat st;
    int fd = open(fn, O_RDONLY, 0600);

    if(fd < 0)
	return 0;

    if (fstat(fd, &st) || ((mf.len = st.st_size) <= 0) || S_ISDIR(st.st_mode))
    {
	mf.len = 0;
	close(fd);
	return 0;
    }

    /*
    mf.len = lseek(fd, 0L, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    */

    mf.start = mmap(NULL, mf.len, PROT_READ, 
	    MF_MMAP_OPTION, fd, 0);
    close(fd);

    if(mf.start == MAP_FAILED)
    {
	RESETMF();
	return 0;
    }

    mf.end = mf.start + mf.len;
    mf.disps = mf.dispe = mf.start;
    mf.lineno = 0;

    mf_determinemaxdisps(MFNAV_PAGE, 0);

    mf.disps = mf.dispe = mf.start;
    mf.lineno = 0;

    /* reset and parse article header */
    mf_parseHeaders();

    /* a workaround for wrapped seperators */
    if(mf.maxlinenoS > 0 &&
	    fh.lines >= mf.maxlinenoS &&
	    bpref.seperator & MFDISP_SEP_WRAP)
    {
	mf_determinemaxdisps(+1, 1);
    }

    return  1;
}

void 
mf_detach()
{
    if(mf.start) {
	munmap(mf.start, mf.len);
	RESETMF();

    }
    mf_freeHeaders();
}

/*
 * lineno calculation, and moving
 */
void 
mf_sync_lineno()
{
    unsigned char *p;

    if(mf.disps == mf.maxdisps && mf.maxlinenoS >= 0)
    {
	mf.lineno = mf.maxlinenoS;
    } else {
	mf.lineno = 0;
	for (p = mf.start; p < mf.disps; p++)
	    if(*p == '\n')
		mf.lineno ++;

	if(mf.disps == mf.maxdisps && mf.maxlinenoS < 0)
	    mf.maxlinenoS = mf.lineno;
    }
}

MFPROTO int mf_backward(int); // used by mf_buildmaxdisps
MFPROTO int mf_forward(int); // used by mf_buildmaxdisps

void
mf_determinemaxdisps(int backlines, int update_by_offset)
{
    unsigned char *pbak = mf.disps, *mbak = mf.maxdisps;
    long lbak = mf.lineno;

    if(update_by_offset)
    {
	if(backlines > 0)
	{
	    /* tricky way because usually 
	     * mf_forward checks maxdisps.
	     */
	    mf.disps = mf.maxdisps;
	    mf.maxdisps = mf.end-1;
	    mf_forward(backlines);
	    mf_backward(0);
	} else
	    mf_backward(backlines);
    } else {
	mf.lineno = backlines;
	mf.disps = mf.end - 1;
	backlines = mf_backward(backlines);
    }

    if(mf.disps != mbak)
    {
	mf.maxdisps = mf.disps;
	if(update_by_offset)
	    mf.lastpagelines -= backlines;
	else
	    mf.lastpagelines = backlines;

	mf.maxlinenoS = -1;
#ifdef PMORE_PRELOAD_SIZE
	if(mf.len <= PMORE_PRELOAD_SIZE)
	    mf_sync_lineno(); // maxlinenoS will be automatically updated
#endif
    }
    mf.disps = pbak;
    mf.lineno = lbak;
}

/*
 * mf_backwards is also used for maxno determination,
 * so we cannot change anything in mf except these:
 *   mf.disps
 *   mf.lineno
 */
MFPROTO int 
mf_backward(int lines)
{
    int real_moved = 0;

    /* backward n lines means to find n times of '\n'. */

    /* if we're already in a line break, add one mark. */
   if (mf.disps < mf.end && *mf.disps == '\n')
       lines++, real_moved --;

    while (1)
    {
	if (mf.disps < mf.start || *mf.disps == '\n')
	{
	    real_moved ++;
	    if(lines-- <= 0 || mf.disps < mf.start)
		break;
	}
	mf.disps --;
    }

    /* now disps points to previous 1 byte of new address */
    mf.disps ++;
    real_moved --;
    mf.lineno -= real_moved;

    return real_moved;
}

MFPROTO int 
mf_forward(int lines)
{
    int real_moved = 0;

    while(mf.disps <= mf.maxdisps && lines > 0)
    {
	while (mf.disps <= mf.maxdisps && *mf.disps++ != '\n');

	if(mf.disps <= mf.maxdisps)
	    mf.lineno++, lines--, real_moved++;
    }

    if(mf.disps > mf.maxdisps)
	mf.disps = mf.maxdisps;

    /* please make sure you have lineno synced. */
    if(mf.disps == mf.maxdisps && mf.maxlinenoS < 0)
	mf.maxlinenoS = mf.lineno;

    return real_moved;
    /*
    if(lines > 0)
	return MFNAV_OK;
    else
	return MFNAV_EXCEED;
	*/
}

int 
mf_goTop()
{
    if(mf.disps == mf.start && mf.xpos > 0)
	mf.xpos = 0;
    mf.disps = mf.start;
    mf.lineno = 0;
    return MFNAV_OK;
}

int 
mf_goBottom()
{
    mf.disps = mf.maxdisps;
    mf_sync_lineno();

    return MFNAV_OK;
}

MFPROTO int 
mf_goto(int lineno)
{
    mf.disps = mf.start;
    mf.lineno = 0;
    return mf_forward(lineno);
}

MFPROTO int 
mf_viewedNone()
{
    return (mf.disps <= mf.start);
}

MFPROTO int 
mf_viewedAll()
{
    return (mf.dispe >= mf.end);
}
/*
 * search!
 */
int 
mf_search(int direction)
{
    unsigned char *s = sr.search_str;
    int l = sr.len;
    int flFound = 0;

    if(!s || !*s)
	return 0;

    if(direction ==  MFSEARCH_FORWARD) 
    {
	mf_forward(1);
	while(mf.disps < mf.end - l)
	{
	    if(sr.cmpfunc(mf.disps, s, l) == 0)
	    {
		flFound = 1;
		break;
	    } else
		mf.disps ++;
	}
	mf_backward(0);
	if(mf.disps > mf.maxdisps)
	    mf.disps = mf.maxdisps;
	mf_sync_lineno();
    } 
    else if(direction ==  MFSEARCH_BACKWARD) 
    {
	mf_backward(1);
	while (!flFound && mf.disps > mf.start)
	{
	    while(!flFound && mf.disps < mf.end-l && *mf.disps != '\n')
	    {
		if(sr.cmpfunc(mf.disps, s, l) == 0)
		{
		    flFound = 1;
		} else
		    mf.disps ++;
	    }
	    if(!flFound)
		mf_backward(1);
	}
	mf_backward(0);
	if(mf.disps < mf.start)
	    mf.disps = mf.start;
	mf_sync_lineno();
    }
    if(flFound)
	MFDISP_DIRTY();
    return flFound;
}

/* String Processing
 *
 * maybe you already have your string processors (or not).
 * whether yes or no, here we provides some.
 */

#define ISSPACE(x) (x <= ' ')

MFPROTO void 
pmore_str_strip_ansi(unsigned char *p)	// warning: p is NULL terminated
{
    unsigned char *pb = p;
    while (*p != 0)
    {
	if (*p == ESC_CHR)
	{
	    // ansi code sequence, ignore them.
	    pb = p++;
	    while (ANSI_IN_ESCAPE(*p))
		p++;
	    memmove(pb, p, strlen(p)+1);
	    p = pb;
	}
	else if (*p < ' ' || *p == 0xff)
	{
	    // control codes, ignore them.
	    // what is 0xff? old BBS does not handle telnet protocol
	    // so IACs were inserted.
	    memmove(p, p+1, strlen(p+1)+1);
	}
	else
	    p++;
    }
}

/* this chomp is a little different: 
 * it kills starting and trailing spaces.
 */
MFPROTO void 
pmore_str_chomp(unsigned char *p)
{
    unsigned char *pb = p + strlen(p)-1;

    while (pb >= p)
	if(ISSPACE(*pb))
	    *pb-- = 0;
	else
	    break;
    pb = p;
    while (*pb && ISSPACE(*pb))
	pb++;

    if(pb != p)
	memmove(p, pb, strlen(pb)+1);
}

#define PMORE_DBCS_LEADING(c) (c >= 0x80)
#if 0
int 
pmore_str_safe_big5len(unsigned char *p)
{
    return 0;
}
#endif

/*
 * Format Related
 */

void 
mf_freeHeaders()
{
    if(fh.lines > 0)
    {
	int i;

	for (i = 0; i < FH_HEADERS; i++)
	    if(fh.headers[i])
		free(fh.headers[i]);
	for (i = 0; i < sizeof(fh.floats) / sizeof(unsigned char*); i++)
	    free(fh.floats[i]);
	RESETFH();
    }
}

void 
mf_parseHeaders()
{
    /* file format:
     * AUTHOR: author BOARD: blah <- headers[0], flaots[0], floats[1]
     * XXX: xxx			  <- headers[1]
     * XXX: xxx			  <- headers[n]
     * [blank, fill with seperator] <- lines
     *
     * #define STR_AUTHOR1     "作者:"
     * #define STR_AUTHOR2     "發信人:"
     */
    unsigned char *pmf = mf.start;
    int i = 0;

    RESETFH();

    if(mf.len < LEN_AUTHOR2)
	return;

    if (strncmp(mf.start, STR_AUTHOR1, LEN_AUTHOR1) == 0)
    {
	fh.lines = 3;	// local
    } 
    else if (strncmp(mf.start, STR_AUTHOR2, LEN_AUTHOR2) == 0)
    {
	fh.lines = 4;
    }
    else 
	return;

    for (i = 0; i < fh.lines; i++)
    {
	unsigned char *p = pmf, *pb = pmf;
	int l;

	/* first, go to line-end */
	while(pmf < mf.end && *pmf != '\n')
	    pmf++;
	if(pmf >= mf.end)
	    break;
	p = pmf;
	pmf ++;	// move to next line.

	// p is pointing at a new line. (\n)
	l = (int)(p - pb);
	p = (unsigned char*) malloc (l+1);
	fh.headers[i] = p;
	memcpy(p, pb, l);
	p[l] = 0;

	// now, postprocess p.
	pmore_str_strip_ansi(p);

	// strip to quotes[+1 space]
	if((pb = strchr(p, ':')) != NULL)
	{
	    if(*(pb+1) == ' ') pb++;
	    memmove(p, pb, strlen(pb)+1);
	}

	// kill staring and trailing spaces
	pmore_str_chomp(p);

	// special case, floats are in line[0].
	if(i == 0 && (pb = strrchr(p, ':')) != NULL && *(pb+1))
	{
	    unsigned char *np = strdup(pb+1);

	    fh.floats[1] = np;
	    pmore_str_chomp(np);
	    // remove quote and traverse back
	    *pb-- = 0;
	    while (pb > p && *pb != ',' && !(ISSPACE(*pb)))
		pb--;

	    if (pb > p) {
		fh.floats[0] = strdup(pb+1);
		pmore_str_chomp(fh.floats[0]);
		*pb = 0;
		pmore_str_chomp(fh.headers[0]);
	    } else {
		fh.floats[0] = strdup("");
	    }
	}
    }
}

/*
 * mf_display utility macros
 */
MFPROTO void
MFDISP_SKIPCURLINE()
{ 
    while (mf.dispe < mf.end && *mf.dispe != '\n')
	mf.dispe++;
}

MFPROTO int
MFDISP_PREDICT_LINEWIDTH(unsigned char *p)
{
    /* predict from p to line-end, without ANSI seq.
     */
    int off = 0;
    int inAnsi = 0;

    while (p < mf.end && *p != '\n')
    {
	if(inAnsi)
	{
	    if(!ANSI_IN_ESCAPE(*p))
		inAnsi = 0;
	} else {
	    if(*p == ESC_CHR)
		inAnsi = 1;
	    else
		off ++;
	}
	p++;
    }
    return off;
}

MFPROTO int
MFDISP_DBCS_HEADERWIDTH(int originalw)
{
    return originalw - (originalw %2);
//    return (originalw >> 1) << 1;
}

#define MFDISP_FORCEUPDATE2TOP() { startline = 0; }
#define MFDISP_FORCEUPDATE2BOT() { endline   = MFDISP_PAGE - 1; }
#define MFDISP_FORCEDIRTY2BOT() \
    if(optimized == MFDISP_OPT_OPTIMIZED) { \
	optimized = MFDISP_OPT_FORCEDIRTY; \
	MFDISP_FORCEUPDATE2BOT(); \
    }

/*
 * display mf content from disps for MFDISP_PAGE
 */
void 
mf_display()
{
    int lines = 0, col = 0, currline = 0, wrapping = 0;
    int startline, endline;

    int optimized = MFDISP_OPT_CLEAR;

    /* why t_columns-1 here?
     * because BBS systems usually have a poor terminal system
     * and many stupid clients behave differently.
     * So we try to avoid using the last column, leave it for
     * BBS to place '\n' and CLRTOEOL.
     */
    const int headerw = MFDISP_DBCS_HEADERWIDTH(t_columns-1);
    const int dispw = headerw - (t_columns - headerw < 2);
    const int maxcol = dispw - 1;

    if(mf.wraplines || mf.trunclines)
	MFDISP_DIRTY();	// we can't scroll with wrapped lines.

    mf.wraplines = 0;
    mf.trunclines = 0;
    mf.dispedlines = 0;

    MFDISP_FORCEUPDATE2TOP();
    MFDISP_FORCEUPDATE2BOT();

#ifdef PMORE_USE_OPT_SCROLL
    /* process scrolling */
    if (mf.oldlineno >= 0 && mf.oldlineno != mf.lineno)
    {
	int scrll = mf.lineno - mf.oldlineno, i;
	int reverse = (scrll > 0 ? 0 : 1);

	if(reverse) 
	    scrll = -scrll;
	else
	{
	    /* because bottom status line is also scrolled,
	     * we have to erase it here.
	     */
	    pmore_clrtoeol(b_lines, 0);
	    // move(b_lines, 0);
	    // clrtoeol();
	}

	if(scrll > MFDISP_PAGE)
	    scrll = MFDISP_PAGE;

	i = scrll;
	while(i-- > 0)
	    if (reverse)
		rscroll();	// v
	    else
		scroll();	// ^

	if(reverse)
	{
	    endline = scrll-1;		// v
	    // clear the line which will be scrolled
	    // to bottom (status line position).
	    pmore_clrtoeol(b_lines, 0);
	    // move(b_lines, 0);
	    // clrtoeol();
	}
	else
	{
	    startline = MFDISP_PAGE - scrll; // ^
	}
	move(startline, 0);
	optimized = MFDISP_OPT_OPTIMIZED;
	// return;	// uncomment if you want to observe scrolling
    }
    else
#endif
	clear(), move(0, 0);

    mf.dispe = mf.disps;
    while (lines < MFDISP_PAGE) 
    {
	int inAnsi = 0;
	int newline = MFDISP_NEWLINE_CLEAR;
	int predicted_linewidth = -1;
	int xprefix = mf.xpos;

#ifdef PMORE_USE_DBCS_WRAP
	unsigned char *dbcs_incomplete = NULL;
#endif

	currline = mf.lineno + lines;
	col = 0;

	if(!wrapping && mf.dispe < mf.end)
	    mf.dispedlines++;

	if(optimized == MFDISP_OPT_FORCEDIRTY)
	{
	    /* btw, apparently this line should be visible. 
	     * if not, maybe something wrong.
	     */
	    pmore_clrtoeol(lines, 0);
	}

#ifdef PMORE_USE_ASCII_MOVIE
	if(mfmovie.mode == MFDISP_MOVIE_PLAYING_OLD &&
		mfmovie.compat24)
	{
	    if(mf.dispedlines == 23)
		return;
	} 
	else
	if(mfmovie.mode == MFDISP_MOVIE_UNKNOWN || 
		mfmovie.mode == MFDISP_MOVIE_PLAYING)
	{
	    if(mf_movieFrameHeader(mf.dispe))
		switch(mfmovie.mode)
		{
		    case MFDISP_MOVIE_UNKNOWN:
			mfmovie.mode = MFDISP_MOVIE_DETECTED;
			break;
		    case MFDISP_MOVIE_PLAYING:
			/*
			 * maybe we should do clrtobot() here,
			 * but it's even better if we do clear()
			 * all time. so we set dirty here for
			 * next frame, and please set dirty before
			 * playing.
			 */
			MFDISP_DIRTY();
			return;
		}
	}
#endif

	/* Is currentline visible? */
	if (lines < startline || lines > endline)
	{
	    MFDISP_SKIPCURLINE();
	    newline = MFDISP_NEWLINE_SKIP;
	}
	/* Now, consider what kind of line
	 * (header, seperator, or normal text)
	 * is current line.
	 */
	else if (currline == fh.lines && bpref.rawmode == MFDISP_RAW_NA)
	{
	    /* case 1, header seperator line */
	    if (bpref.seperator & MFDISP_SEP_LINE)
	    {
		outs("\033[1;40;36m╰───────────────────\033[30;47m y)回應  /)搜尋文字  =[]<>)主題式閱\讀 \033[36;40m╯");
		outs(ANSI_RESET);
	    }

	    /* Traditional 'more' adds seperator as a newline.
	     * This is buggy, however we can support this
	     * by using wrapping features.
	     * Anyway I(piaip) don't like this. And using wrap
	     * leads to slow display (we cannt speed it up with
	     * optimized scrolling.
	     */
	    if(bpref.seperator & MFDISP_SEP_WRAP) 
	    {
		/* we have to do all wrapping stuff
		 * in normal text section.
		 * make sure this is updated.
		 */
		wrapping = 1;
		mf.wraplines ++;
		MFDISP_FORCEDIRTY2BOT();
		if(mf.dispe > mf.start && 
			mf.dispe < mf.end &&
			*mf.dispe == '\n')
		    mf.dispe --;
	    }
	    else
		MFDISP_SKIPCURLINE();
	} 
	else if (currline < fh.lines && bpref.rawmode == MFDISP_RAW_NA )
	{
	    /* case 2, we're printing headers */
	    const char *val = fh.headers[currline];
	    const char *name = _fh_disp_heads[currline];
	    int w = headerw - FH_HEADER_LEN - 3;

	    prints("\033[1;36m%s\033[46;37m%s\033[m ", (currline == 0) ? "╭" : "│", name);

	    /* right floating stuff? */
	    if (currline == 0 && fh.floats[0])
	    {
		w -= strlen(fh.floats[0]) + strlen(fh.floats[1]) + 1;
	    }

	    prints("%-*.*s", w, w, 
		    (val ? val : ""));

	    if (currline == 0 && fh.floats[0])
	        prints("\033[1;33;46m%s\033[40;33m %s", fh.floats[0], fh.floats[1]);

	    outs("\033[1;36m");
	    outs((currline == 0) ? "╮" : "│");
	    outs(ANSI_RESET);
	    MFDISP_SKIPCURLINE();
	} 
	else if(mf.dispe < mf.end)
	{
	    /* case 3, normal text */
	    long dist = mf.end - mf.dispe;
	    long flResetColor = 0;
	    int  srlen = -1;
	    int breaknow = 0;

	    unsigned char c;

	    if(xprefix > 0 && !bpref.oldwrapmode && bpref.indicator)
	    {
		outs(MFDISP_WNAV_INDICATOR);
		col++;
	    }

	    // first check quote
	    if(bpref.rawmode == MFDISP_RAW_NA)
	    {
		if(dist > 1 && 
			(*mf.dispe == ':' || *mf.dispe == '>') && 
			*(mf.dispe+1) == ' ')
		{
		    outs(ANSI_COLOR(36));
		    flResetColor = 1;
		} else if (dist > 2 && 
			(!strncmp(mf.dispe, "※", 2) || 
			 !strncmp(mf.dispe, "==>", 3)))
		{
		    outs(ANSI_COLOR(32));
		    flResetColor = 1;
		}
	    }

	    while(!breaknow && mf.dispe < mf.end && (c = *mf.dispe) != '\n')
	    {
		if(inAnsi)
		{
		    if (!ANSI_IN_ESCAPE(c))
			inAnsi = 0;
		    /* whatever this is, output! */
		    mf.dispe ++;
		    switch(bpref.rawmode)
		    {
			case MFDISP_RAW_NOANSI:
			    /* TODO
			     * col++ here may be buggy. */
			    if(col < t_columns)
			    {
				/* we tried our best to determine */
				if(xprefix > 0)
				    xprefix --;
				else
				{
				    outc(c); 
				    col++;
				}
			    }
			    if(!inAnsi)
				outs(ANSI_RESET);
			    break;
			case MFDISP_RAW_PLAIN:
			    break;
			default:
			    outc(c);
			    break;
		    }
		    continue;

		} else {

		    if(c == ESC_CHR)
		    {
			inAnsi = 1;
			/* we can't output now because maybe
			 * ptt_prints wants to do something.
			 */
		    }
		    else if(sr.search_str && srlen < 0 &&  // support search
#ifdef PMORE_USE_DBCS_WRAP
			    dbcs_incomplete == NULL &&
#endif
			    mf.end - mf.dispe > sr.len &&
			    sr.cmpfunc(mf.dispe, sr.search_str, sr.len) == 0)
		    {
			    outs(ANSI_COLOR(7)); 
			    srlen = sr.len-1;
			    flResetColor = 1;
		    }

#ifdef PMORE_USE_PTT_PRINTS
		    /* special case to resolve dirty Ptt_Prints */
		    if(inAnsi && 
			    mf.end - mf.dispe > 2 &&
			    *(mf.dispe+1) == '*')
		    {
			int i;
			char buf[64];	// make sure ptt_prints will not exceed

			memset(buf, 0, sizeof(buf));
			strncpy(buf, mf.dispe, 3);  // ^[[*s
			mf.dispe += 2;

			if(bpref.rawmode)
			    buf[0] = '*';
			else
			    Ptt_prints(buf, NO_RELOAD); // result in buf
			i = strlen(buf);

			if (col + i > maxcol)
			    i = maxcol - col;
			if(i > 0)
			{
			    buf[i] = 0;
			    col += i;
			    outs(buf);
			}
			inAnsi = 0;
		    } else
#endif
		    if(inAnsi)
		    {
			switch(bpref.rawmode)
			{
			    case MFDISP_RAW_NOANSI:
				/* TODO
				 * col++ here may be buggy. */
				if(col < t_columns)
				{
				    /* we tried our best to determine */
				    if(xprefix > 0)
					xprefix --;
				    else
				    {
					outs(ANSI_COLOR(1) "*");
					col++;
				    }
				}
				break;
			    case MFDISP_RAW_PLAIN:
				break;
			    default:
				outc(ESC_CHR);
				break;
			}
		    } else {
			int canOutput = 0;
			/* if col > maxcol,
			 * because we have the space for
			 * "indicators" (one byte),
			 * so we can tolerate one more byte.
			 */
			if(col <= maxcol)	// normal case
			    canOutput = 1;
			else if (bpref.oldwrapmode && // oldwrapmode
			    col < t_columns)
			{
			    canOutput = 1;
			    newline = MFDISP_NEWLINE_MOVE;
			} else {
			    int off = 0;
			    // put more efforts to determine
			    // if we can use indicator space
			    // determine real offset between \n
			    if(predicted_linewidth < 0)
				predicted_linewidth = col + 1 +
				    MFDISP_PREDICT_LINEWIDTH(mf.dispe+1);
			    off = predicted_linewidth - (col + 1);

			    if (col + off <= (maxcol+1))
			    {
				canOutput = 1;	// indicator space
			    }
#ifdef PMORE_TRADITIONAL_FULLCOL
			    else if (col + off < t_columns)
			    {
				canOutput = 1;
				newline = MFDISP_NEWLINE_MOVE;
			    }
#endif
			}

			if(canOutput)
			{
			    /* the real place to output text
			     */
#ifdef PMORE_USE_DBCS_WRAP
			    if(mf.xpos > 0 && dbcs_incomplete && col < 2)
			    {
				/* col = 0 or 1 only */
				if(col == 0) /* no indicators */
				    c = ' ';
				else if(!bpref.oldwrapmode && bpref.indicator)
				    c = ' ';
			    }

			    if (dbcs_incomplete)
				dbcs_incomplete = NULL;
			    else if(PMORE_DBCS_LEADING(c)) 
				dbcs_incomplete = mf.dispe;
#endif
			    if(xprefix > 0)
				xprefix --;
			    else
			    {
				outc(c);
				col++;
			    }

			    if (srlen == 0)
				outs(ANSI_RESET);
			    if(srlen >= 0)
				srlen --;
			}
			else
			/* wrap modes */
			if(mf.xpos > 0 || bpref.wrapmode == MFDISP_WRAP_TRUNCATE)
			{
			    breaknow = 1;
			    mf.trunclines ++;
			    MFDISP_SKIPCURLINE();
			    wrapping = 0;
			}
			else if (bpref.wrapmode == MFDISP_WRAP_WRAP)
			{
			    breaknow = 1;
			    wrapping = 1;
			    mf.wraplines ++;
#ifdef PMORE_USE_DBCS_WRAP
			    if(dbcs_incomplete)
			    {
				mf.dispe = dbcs_incomplete;
				dbcs_incomplete = NULL;
				/* to be more dbcs safe,
				 * use the followings to
				 * erase printed character.
				 */
				if(col > 0) {
				    /* TODO BUG BUGGY
				     * using move is maybe actually non-sense
				     * because BBS terminal system cannot
				     * display this when ANSI escapes were used
				     * in same line.  However, on most
				     * situation this works.
				     * So we used an alternative, forced ANSI 
				     * move command.
				     */
				    // move(lines, col-1);
				    char ansicmd[16];
				    sprintf(ansicmd, ANSI_MOVETO(%d,%d),
					    lines+1, col-1+1);
				    /* to preven ANSI ESCAPE being tranlated as
				     * DBCS trailing byte. */
				    outc(' '); 
				    /* move back one column */
				    outs(ansicmd);
				    /* erase it (previous leading byte)*/
				    outc(' ');
				    /* go to correct position */
				    outs(ansicmd);
				}
			    }
#endif
			}
		    }
		}
		if(!breaknow)
		    mf.dispe ++;
	    }
	    if(flResetColor)
		outs(ANSI_RESET);

	    /* "wrapping" should be only in normal text section.
	     * We don't support wrap within scrolling,
	     * so if we have to wrap, invalidate all lines.
	     */
	    if(breaknow)
	    {
		if(wrapping)
		    MFDISP_FORCEDIRTY2BOT();

		if(!bpref.oldwrapmode && bpref.indicator && col < t_columns)
		{
		    if(wrapping)
			outs(MFDISP_WRAP_INDICATOR);
		    else
			outs(MFDISP_TRUNC_INDICATOR);
		} else {
		    outs(ANSI_RESET);
		}
	    }
	    else
		wrapping = 0;
	}

	if(mf.dispe < mf.end && *mf.dispe == '\n') 
	    mf.dispe ++;
	// else, we're in wrap mode.

	switch(newline)
	{
	    case MFDISP_NEWLINE_SKIP:
		break;
	    case MFDISP_NEWLINE_CLEAR:
		FORCE_CLRTOEOL();
		outc('\n');
		break;
	    case MFDISP_NEWLINE_MOVE:
		move(lines+1, 0);
		break;
	}
	lines ++;
    }
    /*
     * we've displayed the file.
     * but if we got wrapped lines in last page,
     * mf.maxdisps may be required to be larger.
     */
    if(mf.disps == mf.maxdisps && mf.dispe < mf.end)
    {
	/*
	 * never mind if that's caused by seperator
	 * however this code is rarely used now.
	 * only if no preload file.
	 */
	if (bpref.seperator & MFDISP_SEP_WRAP &&
	    mf.wraplines == 1 &&
	    mf.lineno < fh.lines)
	{
	    /*
	     * o.k, now we know maxline should be one line forward.
	     */
	    mf_determinemaxdisps(+1, 1);
	} else 
	{
	    /* not caused by seperator?
	     * ok, then it's by wrapped lines.
	     *
	     * old flavor: go bottom: 
	     *    mf_determinemaxdisps(0)
	     * however we have "update" method now,
	     * so we can achieve more user friendly behavior.
	     */
	    mf_determinemaxdisps(+mf.wraplines, 1);
	}
    }
    mf.oldlineno = mf.lineno;
}

/* --------------------- MAIN PROCEDURE ------------------------- */
static void
show_help(const char * const helptext[])
{
    const char     *str;
    int             i;

    clear();
    for (i = 0; (str = helptext[i]); i++) {
	if (*str == '\0')
	    prints(ANSI_COLOR(1) "【 %s 】" ANSI_COLOR(0) "\n", str + 1);
	else if (*str == '\01')
	    prints("\n" ANSI_COLOR(36) "【 %s 】" ANSI_RESET "\n", str + 1);
	else
	    prints("        %s\n", str);
    }
	pressanykey(NULL);
}

static const char    * const pmore_help[] = {
    "\0閱\讀文章功\能鍵使用說明",
    "\01游標移動功\能鍵",
    "(k/↑) (j/↓/Enter)   上捲/下捲一行",
    "(^B)(PgUp)(BackSpace) 上捲一頁",
    "(^F)(PgDn)(Space)(→) 下捲一頁",
    "(,/</S-TAB)(./>/TAB)  左/右捲動",
    "(0/g/Home) ($/G/End)  檔案開頭/結尾",
    "(;/:)                 跳至某行/某頁",
    "數字鍵 1-9            跳至輸入的頁數或行號",
    "\01其他功\能鍵",
    "(/" ANSI_COLOR(1;30) "/" ANSI_RESET 
       "s)                 搜尋字串",
    "(n/N)                 重複正/反向搜尋",
    "(Ctrl-T)              存入暫存檔",
    "(f/b)                 跳至下/上篇",
    "(a/A)                 跳至同一作者下/上篇",
    "(t/[-/]+)             主題式閱\讀:循序/前/後篇",
    "(\\/|)                 切換顯示原始內容", // this IS already aligned!
    "(w/W/l)               切換自動折行/折行符號/分隔線顯示方式",
    "(p/o)                 播放動畫/切換傳統模式(狀態列與折行方式)",
    "(q/←) (h/H/?/F1)     結束/本說明畫面",
#ifdef DEBUG
    "(d)                   切換除錯(debug)模式",
#endif
    /* You don't have to delete this copyright line.
     * It will be located in bottom of screen and overrided by
     * status line. Only user with big terminals will see this :)
     */
    "\01本系統使用 piaip 的新式瀏覽程式: pmore, piaip's more",
    NULL
};
/*
 * pmore utility macros
 */
MFPROTO void
PMORE_UINAV_FORWARDPAGE()
{
    /* Usually, a forward is just mf_forward(MFNAV_PAGE);
     * but because of wrapped lines...
     * This function is used when user tries to nagivate
     * with page request.
     * If you want to a real page forward, don't use this.
     * That's why we have this special function.
     */
    int i = mf.dispedlines - 1;

    if(mf_viewedAll())
	return;

    if(i < 1)
	i = 1;
    mf_forward(i);
}

MFPROTO void
PMORE_UINAV_FORWARDLINE()
{
    if(mf_viewedAll())
	return;
    mf_forward(1);
}

#define REENTRANT_RESTORE() { mf = bkmf; fh = bkfh; }

/*
 * piaip's more, a replacement for old more
 */
int 
pmore(char *fpath, int promptend)
{
    int flExit = 0, retval = 0;
    int ch = 0;
    int invalidate = 1;
    char *override_msg = NULL;

    /* simple re-entrant hack
     * I don't want to write pointers everywhere,
     * and pmore should be simple enough (inside itself)
     * so we can do so.
     */

    MmappedFile bkmf;
    MF_PrettyFormattedHeader bkfh;

#ifdef PMORE_USE_ASCII_MOVIE
    RESET_MOVIE();
#endif

    bkmf = mf; /* simple re-entrant hack */
    bkfh = fh;
    RESETMF();
    RESETFH();

#ifdef STATINC
    STATINC(STAT_MORE);
#endif
    if(!mf_attach(fpath))
    {
	REENTRANT_RESTORE();
	return -1;
    }

    clear();
    while(!flExit)
    {
	if(invalidate)
	{
	    mf_display(); 
	    invalidate = 0;
	}

	/* in current implementation,
	 * we want to invalidate for each keypress.
	 */
	invalidate = 1;

#ifdef	PMORE_TRADITIONAL_PROMPTEND

	if(promptend == NA)
	{
#ifdef PMORE_USE_ASCII_MOVIE
	    if(mfmovie.mode == MFDISP_MOVIE_DETECTED)
	    {
		/* quick auto play */
		mfmovie.mode = MFDISP_MOVIE_YES;
		RESET_MOVIE();
		mfmovie.mode = MFDISP_MOVIE_PLAYING;
		mf_determinemaxdisps(0, 0); // display until last line
		mf_movieNextFrame();
		MFDISP_DIRTY();
		continue;
	    } else if (mfmovie.mode != MFDISP_MOVIE_PLAYING)
#endif
	    break;
	}
#else
	if(promptend == NA && mf_viewedAll())
	    break;
#endif
	move(b_lines, 0);
	// clrtoeol(); // this shall be done in mf_display to speed up.

#ifdef PMORE_USE_ASCII_MOVIE
	switch (mfmovie.mode)
	{
	    case MFDISP_MOVIE_UNKNOWN:
		mfmovie.mode = MFDISP_MOVIE_NO;
		break;

	    case MFDISP_MOVIE_DETECTED:
		mfmovie.mode = MFDISP_MOVIE_YES;
		{
		    // query if user wants to play movie.

		    int w = t_columns-1;
		    const char *s = 
			" 這份文件是可播放的文字動畫，要開始播放嗎？ [Y/n]";
		    outs(ANSI_RESET ANSI_COLOR(1;33;44));
		    w -= strlen(s); outs(s);
		    while(w-- > 0) outc(' '); outs(ANSI_RESET);
		    w = tolower(igetch());
		    if(w != 'n' && 
			    w != KEY_UP && w != KEY_LEFT &&
			    w != 'q')
		    {
			RESET_MOVIE();
			mfmovie.mode = MFDISP_MOVIE_PLAYING;
			mf_determinemaxdisps(0, 0); // display until last line
			mf_movieNextFrame();
			MFDISP_DIRTY();
			continue;
		    }
		    /* else, we have to clean up. */
		    move(b_lines, 0);
		    clrtoeol();
		}
		break;

	    case MFDISP_MOVIE_PLAYING_OLD:
	    case MFDISP_MOVIE_PLAYING:
		{
		    int w = t_columns - 1;
		    const char *s = " >>> 播放動畫中... 可按任意鍵停止";

		    outs(ANSI_RESET ANSI_COLOR(1;30;47));
		    w -= strlen(s); outs(s); 
		    while(w-- > 0) outc(' '); outs(ANSI_RESET);
		}

		if(mf_movieSyncFrame())
		{
		    /* user did not hit anything.
		     * play next frame.
		     */
		    if(mfmovie.mode == MFDISP_MOVIE_PLAYING)
		    {
			if(!mf_movieNextFrame())
			{
			    mfmovie.mode = MFDISP_MOVIE_YES; // nothing more
			    mf_determinemaxdisps(MFNAV_PAGE, 0);
			    mf_forward(0);

			    if(promptend == NA)
			    {
				/* if we played to end,
				 * no need to prevent pressanykey().
				 */
				flExit = 1, retval = 0;
			    }
			}
		    }
		    else if(mfmovie.mode == MFDISP_MOVIE_PLAYING_OLD)
		    {
			if(mf_viewedAll())
			{
			    mfmovie.mode = MFDISP_MOVIE_NO;
			    mf_determinemaxdisps(MFNAV_PAGE, 0);
			    mf_forward(0);
			}
			else
			{
			    if(!mfmovie.compat24)
				PMORE_UINAV_FORWARDPAGE();
			    else
				mf_forward(22);
			}
		    }
		} else {
		    igetch();

		    /* TODO simple navigation here? */

		    /* stop playing */
		    if(mfmovie.mode == MFDISP_MOVIE_PLAYING)
		    {
			mfmovie.mode = MFDISP_MOVIE_YES;
			if(promptend == NA)
			{
			    flExit = 1, retval = 'f';
			}
		    }
		    else if(mfmovie.mode == MFDISP_MOVIE_PLAYING_OLD)
			mfmovie.mode = MFDISP_MOVIE_NO;

		    mf_determinemaxdisps(MFNAV_PAGE, 0);
		    mf_forward(0);
		}
		continue;
	}
#endif

	/* PRINT BOTTOM STATUS BAR */
#ifdef DEBUG
	if(debug)
	{
	    /* in debug mode don't print ANSI codes
	     * because themselves are buggy.
	     */
	    prints("L#%ld(w%ld,lp%ld) pmt=%d Dsp:%08X/%08X/%08X, "
		    "F:%08X/%08X(%d) tScr(%dx%d)",
		    mf.lineno, mf.wraplines, mf.lastpagelines,
		    promptend,
		    (unsigned int)mf.disps, 
		    (unsigned int)mf.maxdisps,
		    (unsigned int)mf.dispe,
		    (unsigned int)mf.start, (unsigned int)mf.end,
		    (int)mf.len,
		    t_columns,
		    t_lines
		  );
	}
	else
#endif
	{
	    char *printcolor;

	    char buf[256];	// orz
	    int prefixlen = 0;
	    int barlen = 0;
	    int postfix1len = 0, postfix2len = 0;

	    int progress  = 
		(int)((unsigned long)(mf.dispe-mf.start) * 100 / mf.len);
	    /*
	     * page determination is hard.
	     * should we use starting line, or finishing line?
	     */
	    int nowpage = 
		(int)((mf.lineno + mf.dispedlines/2) / MFNAV_PAGE)+1;
	    int allpages = -1; /* unknown yet */
	    if (mf.maxlinenoS >= 0)
	    {
		allpages = 
		(int)((mf.maxlinenoS + mf.lastpagelines -
		       ((bpref.seperator & MFDISP_SEP_WRAP) &&
			(fh.lines >= 0) ? 0:1)) / MFNAV_PAGE)+1;
		if (mf.lineno >= mf.maxlinenoS || nowpage > allpages)
		    nowpage = allpages;
		/*
		    nowpage = 
			(int)((mf.lineno + mf.dispedlines-2) / MFNAV_PAGE)+1 ;
			*/
	    }
	    /* why -2 and -1?
	     * because we want to determine by nav_page,
	     * and mf.dispedlines is based on disp_page (nav_page+1)
	     * mf.lastpagelines is based on nav_page
	     */

/*
	    if(mf_viewedAll())
		printcolor = ANSI_COLOR(37;44);
	    else if (mf_viewedNone())
		printcolor = ANSI_COLOR(33;45);
	    else
		printcolor = ANSI_COLOR(34;46);
*/
	    printcolor = COLOR2;
	    
	    outs(ANSI_RESET);
	    outs(printcolor);

	    if(bpref.oldstatusbar)
	    {

/*
		prints("  瀏覽 P.%d(%d%%)  %s  %-30.30s%s",
			nowpage,
			progress,
			ANSI_COLOR(31;47), 
			"(h)" 
			ANSI_COLOR(30) "求助  "
			ANSI_COLOR(31) "→↓[PgUp][",
			"PgDn][Home][End]"
			ANSI_COLOR(30) "游標移動  "
			ANSI_COLOR(31) "←[q]"
			ANSI_COLOR(30) "結束   ");
*/
	        prints("  瀏覽 P.%d(%d%%)  ", nowpage, progress);
	        prints("%s                 ←↑↓→|PgUp|PgDn|Home|End)導覽  h)說明 \033[m", COLOR3);
	    } else {

		if(allpages >= 0)
		    sprintf(buf,
			    "  瀏覽 第 %1d/%1d 頁 (%3d%%) ",
			    nowpage,
			    allpages,
			    progress
			   );
		else
		    sprintf(buf,
			    "  瀏覽 第 %1d 頁 (%3d%%) ",
			    nowpage,
			    progress
			   );
		outs(buf); prefixlen += strlen(buf);

		outs(ANSI_COLOR(1;30;47));

		if(override_msg)
		{
		    buf[0] = ' ';
		    snprintf(buf+1, sizeof(buf)-1, override_msg);
		    override_msg = NULL;
		}
		else
		if(mf.xpos > 0)
		{
		    snprintf(buf, sizeof(buf),
			    " 顯示範圍: %d~%d 欄位, %02d~%02d 行",
			    (int)mf.xpos+1, 
			    (int)(mf.xpos + t_columns-(mf.trunclines ? 2 : 1)),
			    (int)(mf.lineno + 1),
			    (int)(mf.lineno + mf.dispedlines)
			   );
		} else {
		    snprintf(buf, sizeof(buf),
			    " 目前顯示: 第 %02d~%02d 行",
			    (int)(mf.lineno + 1),
			    (int)(mf.lineno + mf.dispedlines)
			   );
		}

		outs(buf); prefixlen += strlen(buf);

		postfix1len = 12;	// check msg below
		postfix2len = 10;

		if (prefixlen + postfix1len + postfix2len + 1 > t_columns)
		{
		    postfix1len = 0;
		    if (prefixlen + postfix1len + postfix2len + 1 > t_columns)
			postfix2len = 0;
		}
		barlen = t_columns - 1 - postfix1len - postfix2len - prefixlen;

		while(barlen-- > 0)
		    outc(' ');

		if(postfix1len > 0)
		    outs(
			    ANSI_COLOR(0;30;47) "←|q)離開 "
			);

		if(postfix2len > 0)
		    outs(
			    ANSI_COLOR(0;30;47) "h)按鍵說明 "
			);

	    }
	    outs(ANSI_RESET);
	    FORCE_CLRTOEOL();
	}

	/* igetch() will do refresh(); */
	ch = igetkey();
	switch (ch) {
	    /* ------------------ EXITING KEYS ------------------ */
	    case 'r': case 'R':
	    case 'Y': case 'y':
	    case 'X':
	    case 'A':
	    case 'a':
	    case 'F': case 'f':
	    case 'B': case 'b':
	    case KEY_LEFT:
	    case 'q':

		/* from Kaede, thread reading */
	    case ']':
	    case '+':
	    case '[':
	    case '-':
	    case '=':
		flExit = 1,	retval = ch;
		break;
	    /* ------------------ NAVIGATION KEYS ------------------ */
	    /* Simple Navigation */
	    case 'k': case 'K':
		mf_backward(1);
		break;
	    case 'j': case 'J':
		PMORE_UINAV_FORWARDLINE();
		break;

	    case Ctrl('F'):
	    case KEY_PGDN:
		PMORE_UINAV_FORWARDPAGE();
		break;
	    case Ctrl('B'):
	    case KEY_PGUP:
		mf_backward(MFNAV_PAGE);
		break;

	    case '0':
	    case 'g':
	    case KEY_HOME:
		mf_goTop();
		break;
	    case '$':
	    case 'G':
	    case KEY_END:
		mf_goBottom();

#ifdef PMORE_ACCURATE_WRAPEND
		/* allright. in design of pmore,
		 * it's possible that when user navigates to file end,
		 * a wrapped line made nav not 100%.
		 */
		mf_display();
		invalidate = 0;

		if(!mf_viewedAll())
		{
		    /* one more try. */
		    mf_goBottom();
		    invalidate = 1;
		}
#endif
		break;

	    /* Compound Navigation */
	    case '.':
		if(mf.xpos == 0)
		    mf.xpos ++;
		mf.xpos ++;
		break;
	    case ',':
		if(mf.xpos > 0)
		    mf.xpos --;
		break;
	    case '\t':
	    case '>':
		//if(mf.xpos == 0 || mf.trunclines)
		    mf.xpos = (mf.xpos/8+1)*8;
		break;
		/* acronym form shift-tab, ^[[Z */
		/* however some terminals does not send that. */
	    case KEY_STAB:
	    case '<':
		mf.xpos = (mf.xpos/8-1)*8;
		if(mf.xpos < 0) mf.xpos = 0;
		break;
	    case '\r':
	    case '\n':
	    case KEY_DOWN:
		if (mf_viewedAll() ||
			(promptend == 2 && (ch == '\r' || ch == '\n')))
		    flExit = 1, retval = 'f';
		else
		    PMORE_UINAV_FORWARDLINE();
		break;

	    case ' ':
		if (mf_viewedAll())
		    flExit = 1, retval = 'f';
		else
		    PMORE_UINAV_FORWARDPAGE();
		break;
	    case KEY_RIGHT:
		if(mf_viewedAll())
		    promptend = 0, flExit = 1, retval = 0;
		else
		{
		    /* if mf.xpos > 0, widenav mode. */
		    /* because we have other keys to do so,
		     * disable it now.
		     */
		    /*
		    if(mf.trunclines > 0)
		    {
			if(mf.xpos == 0)
			    mf.xpos++;
			mf.xpos++;
		    }
		    else if (mf.xpos == 0)
		    */
			PMORE_UINAV_FORWARDPAGE();
		}
		break;

	    case KEY_UP:
		if(mf_viewedNone())
		    flExit = 1, retval = 'b';
		else
		    mf_backward(1);
		break;
	    case Ctrl('H'):
		if(mf_viewedNone())
		    flExit = 1, retval = 'b';
		else
		    mf_backward(MFNAV_PAGE);
		break;

	    case 't':
		if (mf_viewedAll())
		    flExit = 1, retval = ']';
		else
		    PMORE_UINAV_FORWARDPAGE();
		break;
	    /* ------------------ SEARCH  KEYS ------------------ */
	    case 's':
	    case '/':
		{
		    char sbuf[81] = "";
		    char ans[4] = "n";

		    if(sr.search_str) {
			free(sr.search_str);
			sr.search_str = NULL;
		    }

		    getdata(b_lines - 1, 0, "[搜尋]關鍵字:", sbuf,
			    40, DOECHO, 0);

		    if (sbuf[0]) {
			if (getdata(b_lines - 1, 0, "區分大小寫(Y/N/Q)? [N] ",
				    ans, sizeof(ans), LCECHO, 0) && *ans == 'y')
			    sr.cmpfunc = strncmp;
			else if (*ans == 'q')
			    sbuf[0] = 0;
			else
			    sr.cmpfunc = strncasecmp;
		    }
		    sr.len = strlen(sbuf);
		    if(sr.len) sr.search_str = strdup(sbuf);
		    mf_search(MFSEARCH_FORWARD);
		    MFDISP_DIRTY();
		}
		break;
	    case 'n':
		mf_search(MFSEARCH_FORWARD);
		break;
	    case 'N':
		mf_search(MFSEARCH_BACKWARD);
		break;
	    /* ------------------ SPECIAL KEYS ------------------ */
	    case '1': case '2': case '3': case '4': case '5':
	    case '6': case '7': case '8': case '9':
	    case ';': case ':':
		{
		    char buf[16] = "";
		    int  i = 0;
		    int  pageMode = (ch != ':');
		    if (ch >= '1' && ch <= '9')
			buf[0] = ch, buf[1] = 0;

		    pmore_clrtoeol(b_lines-1, 0);
		    getdata(b_lines-1, 0, 
			    (pageMode ? 
			     "跳至此頁(若要改指定行數請在結尾加.): " : 
			     "跳至此行: "),
			    buf, 8, DOECHO, 0);
		    if(buf[0]) {
			i = atoi(buf);
			if(buf[strlen(buf)-1] == '.')
			    pageMode = 0;
			if(i-- > 0)
			    mf_goto(i * (pageMode ? MFNAV_PAGE : 1));
		    }
		    MFDISP_DIRTY();
		}
		break;

	    case Ctrl('T'):
		{
		    char buf[10];
		    getdata(b_lines - 1, 0, "把這篇文章收入到暫存檔？[y/N] ",
			    buf, 4, LCECHO, 0);
		    if (buf[0] == 'y') {
			sethomefile(buf, cuser.userid, ask_tmpbuf(b_lines - 1));
                        //Copy(fpath, buf);
                        f_cp(fpath, buf, O_TRUNC);
		    }
		    MFDISP_DIRTY();
		}
		break;

	    case 'h': case 'H': case KEY_F1:
	    case '?':
		// help
		show_help(pmore_help);
		MFDISP_DIRTY();
		break;

	    case 'E':
		// admin edit any files other than ve help file
		if (/*HAS_PERM(PERM_SYSOP) && */strcmp(fpath, "etc/ve.hlp")) {
		    mf_detach();
		    //vedit(fpath, NA, NULL);
		    vedit(fpath, HAS_PERM(PERM_SYSOP) ? 0 : 2);
		    REENTRANT_RESTORE();
		    return 0;
		}
		break;
	    case 'w':
		switch(bpref.wrapmode)
		{
		    case MFDISP_WRAP_WRAP:
			bpref.wrapmode = MFDISP_WRAP_TRUNCATE;
			override_msg = ANSI_COLOR(31) "已設定為截行模式(不自動折行)";
			break;
		    case MFDISP_WRAP_TRUNCATE:
			bpref.wrapmode = MFDISP_WRAP_WRAP;
			override_msg = ANSI_COLOR(34) "已設定為自動折行模式";
			break;
		}
		MFDISP_DIRTY();
		break;
	    case 'W':
		bpref.indicator = !bpref.indicator;
		if(bpref.indicator)
		    override_msg = ANSI_COLOR(34) "顯示折行符號";
		else
		    override_msg = ANSI_COLOR(31) "不再顯示折行符號";
		MFDISP_DIRTY();
		break;
	    case 'o':
		bpref.oldwrapmode  = !bpref.oldwrapmode;
		bpref.oldstatusbar = !bpref.oldstatusbar;
		MFDISP_DIRTY();
		break;
	    case 'l':
		switch(bpref.seperator)
		{
		    case MFDISP_SEP_OLD:
			bpref.seperator = MFDISP_SEP_LINE;
			override_msg = ANSI_COLOR(31) "設定為單行分隔線";
			break;
		    case MFDISP_SEP_LINE:
			bpref.seperator = 0;
			override_msg = ANSI_COLOR(31) "設定為無分隔線";
			break;
		    default:
			bpref.seperator = MFDISP_SEP_OLD;
			override_msg =  ANSI_COLOR(34) "傳統分隔線加空行";
			break;
		}
		MFDISP_DIRTY();
		break;
	    case '\\':
	    case '|':
		if(ch == '|')
		    bpref.rawmode += MFDISP_RAW_MODES-1;
		else
		    bpref.rawmode ++;
		bpref.rawmode %= MFDISP_RAW_MODES;
		switch(bpref.rawmode)
		{
		    case MFDISP_RAW_NA:
			override_msg = ANSI_COLOR(34) "顯示預設格式化內容";
			break;
			/*
		    case MFDISP_RAW_NOFMT:
			override_msg = ANSI_COLOR(31) "省略自動格式化";
			break;
			*/
		    case MFDISP_RAW_NOANSI:
			override_msg = ANSI_COLOR(33) "顯示原始 ANSI 控制碼";
			break;
		    case MFDISP_RAW_PLAIN:
			override_msg = ANSI_COLOR(37) "顯示純文字";
			break;
		}
		MFDISP_DIRTY();
		break;
#ifdef PMORE_USE_ASCII_MOVIE
	    case 'p':
		/* play ascii movie again
		 */
		if(mfmovie.mode == MFDISP_MOVIE_YES)
		{
		    RESET_MOVIE();
		    mfmovie.mode = MFDISP_MOVIE_PLAYING;
		    mf_determinemaxdisps(0, 0); // display until last line
		    /* it is said that it's better not to go top. */
		    // mf_goTop();
		    mf_movieNextFrame();
		    MFDISP_DIRTY();
		} 
		else if (mfmovie.mode == MFDISP_MOVIE_NO)
		{
		    static char buf[10]="1";
		    //move(b_lines-1, 0);
		    
		    /* 
		     * TODO scan current page to confirm if this is a new style movie
		     */
		    pmore_clrtoeol(b_lines-1, 0);
		    getdata(b_lines - 1, 0, 
			    "這可能是傳統動畫檔, "
			    "若要直接播放請輸入速度(秒): "
			    ,
			    buf, 8, LCECHO, 0);
		    if(buf[0])
		    {
			float nf = 0;
			sscanf(buf, "%f", &nf);
			RESET_MOVIE();

			mfmovie.mode = MFDISP_MOVIE_PLAYING_OLD;
			float2tv(nf, &mfmovie.frameclk);
			mfmovie.compat24 = 0;
			/* are we really going to start? check termsize! */
			if (t_lines != 24)
			{
			    char ans[4];
			    pmore_clrtoeol(b_lines-1, 0);
			    getdata(b_lines - 1, 0, 
				"傳統動畫是以 24 行為單位設計的, "
				"要模擬 24 行嗎? (否則會用現在的行數)[Yn] "
				, ans, 3, LCECHO, 0);
			    if(ans[0] == 'n')
				mfmovie.compat24 = 0;
			    else
				mfmovie.compat24 = 1;
			}
			mf_determinemaxdisps(0, 0); // display until last line
			MFDISP_DIRTY();
		    }
		}
		break;
#endif

#ifdef DEBUG
	    case 'd':
		debug = !debug;
		MFDISP_DIRTY();
		break;
#endif
	}
	/* DO NOT DO ANYTHING HERE. NOT SAFE RIGHT NOW. */
    }

    mf_detach();
    if (retval == 0 && promptend) {
	pressanykey(NULL);
	clear();
    } else
	outs(reset_color);

    REENTRANT_RESTORE();
    return retval;
}

// ---------------------------------------------------- Extra modules

#ifdef PMORE_USE_ASCII_MOVIE
void 
float2tv(float f, struct timeval *ptv)
{
    if(f < MOVIE_MIN_FRAMECLK)
	f = MOVIE_MIN_FRAMECLK;
    ptv->tv_sec = (long) f;
    ptv->tv_usec = (f - (long)f) * MOVIE_SECOND_U;
}

/*
 * maybe you can use add_io or you have other APIs in
 * your I/O system, but we'll do it here.
 * override if you have better methods.
 */
int 
pmore_wait_input(struct timeval *ptv)
{
    int sel = 0;
    fd_set readfds;

    if(num_in_buf() > 0)
	return 1;

    FD_ZERO(&readfds);
    FD_SET(0, &readfds);

    refresh();

#ifdef STATINC
    STATINC(STAT_SYSSELECT);
#endif

    do {
	if(num_in_buf() > 0)	// for EINTR
	    return 1;

	sel = select(1, &readfds, NULL, NULL, ptv);
    } while (sel < 0 && errno == EINTR);
    /* EINTR, interrupted. I don't care! */

    if(sel == 0)
	return 0;

    return 1;
}

MFPROTO unsigned char * 
mf_movieFrameHeader(unsigned char *p)
{
    if(mf.end - p < 3)
	return NULL;

    if(*p == 12)	// ^L
	return p+1;
    if( *p == '^' &&
	    *(p+1) == 'L')
	return p+2;
    return NULL;
}

/*
 * return meaning:
 * I've got synchronized.
 * If no (user breaks), return 0
 */
int mf_movieSyncFrame()
{
    if (mfmovie.synctime.tv_sec > 0)
    {
	/* synchronize world timeline model */
	struct timeval dv;
	gettimeofday(&dv, NULL);
	dv.tv_sec = mfmovie.synctime.tv_sec - dv.tv_sec;
	if(dv.tv_sec < 0)
	    return 1;
	dv.tv_usec = mfmovie.synctime.tv_usec - dv.tv_usec;
	if(dv.tv_usec < 0) {
	    dv.tv_sec --;
	    dv.tv_usec += MOVIE_SECOND_U;
	}
	if(dv.tv_sec < 0)
	    return 1;
	return !pmore_wait_input(&dv);
    } else {
	/* synchronize each frame clock model */
	/* because Linux will change the timeval passed to select,
	 * let's use a temp value here.
	 */
	struct timeval dv = mfmovie.frameclk;
	return !pmore_wait_input(&dv);
    }
}

int 
mf_movieNextFrame()
{
    do 
    {
	unsigned char *p = mf_movieFrameHeader(mf.disps);
	if(p) 
	{
	    char buf[16];
	    int cbuf = 0;
	    float nf = 0;
	    
	    /* process leading */
	    if (*p == 'S') {
		gettimeofday(&mfmovie.synctime, NULL);
		p++;
	    }

	    while (p < mf.end && 
		    ((*p >= '0' && *p <= '9') || *p == '.'))
		buf[cbuf++] = *p++;

	    buf[cbuf] = 0;
	    if(cbuf)
	    {
		sscanf(buf, "%f", &nf);
		float2tv(nf, &mfmovie.frameclk);
	    }

	    if(mfmovie.synctime.tv_sec > 0)
	    {
		mfmovie.synctime.tv_usec += mfmovie.frameclk.tv_usec;
		mfmovie.synctime.tv_sec  += mfmovie.frameclk.tv_sec;
		mfmovie.synctime.tv_sec  += mfmovie.synctime.tv_usec / MOVIE_SECOND_U;
		mfmovie.synctime.tv_usec %= MOVIE_SECOND_U;
	    }

	    mf_forward(1);
	    return 1;
	}
    } while(mf_forward(1) > 0);

    return 0;
}
#endif

/* vim:sw=4:ts=8
 */
