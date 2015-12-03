/* z.h - Main Zedit include file
 * Copyright (C) 1988-2013 Sean MacLennan
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _Z_H_
#define _Z_H_

#ifdef __STRICT_ANSI__
#define _XOPEN_SOURCE
#define _POSIX_SOURCE
#define _SVID_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>

#include "config.h"

#ifdef WIN32
#include "zwin32.h"
#else
#include <unistd.h>
#include <dirent.h>
#include <strings.h> /* needed for c11 */
#endif

struct zbuff;

#include "buff.h"
#include "mark.h"
#include "vars.h"
#include "funcs.h"
#include "proto.h"
#include "keys.h"

#define ZSTR	"Zedit"
#define VERSION	"6.1"

#define INVALID		-1

#define ZCFILE		".config.z"

/* the first three must follow:  (define + 1) % 4 = 0 */
#define BUFNAMMAX	31			/* max buffer name */
#define PATHMAX		255			/* max file name */
#define STRMAX		79			/* string max */

#define BACKWARD	0			/* go forwards */
#define FORWARD		1			/* go backwards */
#define REGEXP		2			/* reg exp search */
#define QUERY		3			/* query replace */
#define AGAIN		-1			/* go again */
#define AGAIN_WRAP	128			/* wrap and go again */

/*
 * BUFFER MODES
 */
/* minor modes - 0 to all allowed */
#define OVERWRITE			0x0001
#define EXACT				0x0002
#define VIEW				0x0004
/* majour modes - only one allowed */
#define NORMAL				0x0100
#define CMODE				0x0200
#define TXTMODE				0x0400
#define SHMODE				0x0800
/* super modes - 0 to all allowed */
#define SYSBUFF				0x1000
/* File modes in buff.h */
/* #define COMPRESSED		0x10000 */
/* #define CRLF				0x20000 */
/* some handy macro for buffer modes */
#define MAJORMODE			0x0f00
#define PROGMODE			(CMODE | SHMODE)

/* System buffer names */
#define UNTITLED	"NoFile"	/* for buffers with no fname */
#define MAINBUFF	"*scratch*"
#define HELPBUFF	"*help*"
#define SHELLBUFF	"*shell*"
#define LISTBUFF	"*list*"
#define CONFBUFF	"*config*"
#define TAGBUFF		"*tags*"

/* ask return values */
#define BANG		2
#define YES		1
#define NO		0
#define ABORT		-1
#define BADCHAR		-2

struct cnames {
	const char *name;
	Byte fnum;
	Byte flags;
	const char *doc;
};
#define CNAMESIZE sizeof(struct cnames)

struct wdo {
	struct zbuff *wbuff;		/* buffer window looks on */
	struct mark *wpnt;		/* saved Point */
	struct mark *wmrk;		/* saved Mark */
	struct mark *wstart;		/* screen start */
	int umark_set;
	int first, last;		/* screen line boundries */
	int modecol;			/* column for modeflags */
	int modeflags;			/* flags for modeflags */
	struct wdo *next;
};

#define foreachwdo(wdo) for (wdo = Whead; wdo; wdo = wdo->next)
#define wheight() (Curwdo->last - Curwdo->first)

extern struct wdo *Curwdo, *Whead;
extern struct zbuff *Curbuff;
extern struct buff *Bbuff;

struct zbuff {
	char *bname;            /* buffer name */
	char *fname;            /* file associated with buffer */
	unsigned bmode;		    /* buffer mode - unused in core */
	time_t mtime;           /* file modified time */
	void *chead, *ctail;	/* list of comments in file */
	Byte comchar;			/* single char comment character */
	void *undo_tail;        /* list of undos */
	struct mark *umark;     /* user mark */
	struct buff *buff;	    /* low-level buffer */
	struct zbuff *prev, *next;	/* list of buffers */
};

#define foreachbuff(b) for ((b) = Bufflist; (b); (b) = (b)->next)

#define Curcptr (Bbuff->curcptr)
#define Buff()  (*Bbuff->curcptr)
#define unmark bdelmark

#define movepast(a, b) bmovepast(Bbuff, a, b)
#define moveto(a, b) bmoveto(Bbuff, a, b)

extern char *Home;
extern bool Argp;
extern int Arg;				/* must be signed */
extern int InPaw;		/* Are we in the Paw window? */
extern char PawStr[];		/* handy string to put text in */
extern int Pawcol, Pawlen, Pshift;
extern struct zbuff *Paw;
extern int verbose;

extern unsigned Cmd;
extern int Cmdpushed;

extern struct cnames Cnames[];
extern void (*Cmds[][2])();
extern int Curcmds;
extern Byte Keys[], Lfunc;
extern bool First;
extern int ring_bell;
extern int raw_mode;

#define CMD(n) (*Cmds[n][Curcmds])()

extern bool Sendp;
extern char Lbufname[];
extern struct zbuff *Bufflist;
extern struct zbuff *Buff_save;
extern struct mark *Sstart, *Psstart, *Send;
extern bool Initializing;
extern bool Insearch;
extern int circf;

extern void (*Nextpart)(void);

#define clrpaw()	_putpaw("")

/* The memory usage for screen stuff is approx:  (ROWMAX + 1) x 25 + COLMAX */
#define	ROWMAX				110
#define	COLMAX				256

#define PREFLINE			10

#define	CR	('\r')
#define	NL	('\n')

/* attributes */
#define T_NORMAL			0
#define T_STANDOUT			1
#define T_REVERSE			2
#define T_BOLD				3
#define T_COMMENT			4
#define T_REGION			5

#define tsetpoint(r, c)		(Prow = r, Pcol = c)

char *zgetcwd(char *cwd, int len);
#ifdef WIN32
char *gethomedir(void);
void tputchar(Byte c);
#define tflush()
#else
#define gethomedir()		getenv("HOME")
#define tputchar(c)		putchar(c)
#define tflush()		fflush(stdout)
#endif

/* this is MUCH faster than an isascii isprint pair */
#define ZISPRINT(c)		(c >= ' ' && c <= '~')

/* terminal variables */
extern int Prow, Pcol;			/* Point row and column */
extern int Colmax, Rowmax;		/* Row and column maximums */
extern int Tlrow;			/* Last row displayed (-1 for none) */
extern int Tabsize;

#define ISNL(c)			((c) == '\n')

void tbell_dbg(char *func, int line);
#if 0
#define tbell() tbell_dbg(__FILE__, __LINE__)
#endif

#define UMARK_SET (Curbuff->umark)
#define NEED_UMARK do if (!UMARK_SET) { tbell(); return; } while (0)
#define UMARK (Curbuff->umark) /* Must guarantee umark set! */
#define CLEAR_UMARK clear_umark()

#endif /* _Z_H_ */
