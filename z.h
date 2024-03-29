/* Copyright (C) 1988-2018 Sean MacLennan */

#ifndef _Z_H_
#define _Z_H_

#ifdef __STRICT_ANSI__
#define _XOPEN_SOURCE
#define _POSIX_SOURCE
#define _SVID_SOURCE
#define _DEFAULT_SOURCE
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <strings.h> /* needed for c11 */
#include <sys/stat.h>

#include "config.h"

struct zbuff;

#include "buff.h"
#include "reg.h"
#include "vars.h"
#include "funcs.h"
#include "keys.h"
#include "calc.h"
#include "tinit.h"

/** @defgroup zedit Zedit functions
 * The core buffer routines including the mark functions.
 */

/** @addtogroup zedit
 * @{
 */

#define ZSTR	"Zedit"
#define VERSION	"6.3"

#define INVALID		-1

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

/** Huge file size. If HUGE_FILES is defined, files bigger than
 * HUGE_SIZE will be read on demand.
 */
#define HUGE_SIZE (2UL << 30)

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
#define SYSBUFF				0x2000
/* File modes in buff.h */
/* #define COMPRESSED		0x10000 */
/* #define CRLF				0x20000 */
/* some handy macro for buffer modes */
#define MAJORMODE			0x1f00
#define PROGMODE			(CMODE | SHMODE)

/* System buffer names */
#define UNTITLED	"NoFile"	/* for buffers with no fname */
#define MAINBUFF	"*scratch*"
#define HELPBUFF	"*help*"
#define SHELLBUFF	"*shell*"
#define LISTBUFF	"*list*"
#define CONFBUFF	"*config*"
#define TAGBUFF		"*tags*"
#define LIFEBUFF    "*life*"

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
/* getplete structs must be aligned */
_Static_assert((sizeof(struct cnames) % sizeof(char *)) == 0,
	       "cnames not aligned");

struct wdo {
	struct wdo *next;
	struct zbuff *wbuff;		/* buffer window looks on */
	struct mark *wpnt;		/* saved Point */
	struct mark *wmrk;		/* saved Mark */
	struct mark *wstart;		/* screen start */
	int umark_set;
	int first, last;		/* screen line boundries */
	int modecol;			/* column for modeflags */
	int modeflags;			/* flags for modeflags */
};

#define foreachwdo(wdo) for (wdo = Whead; wdo; wdo = wdo->next)
#define wheight() (Curwdo->last - Curwdo->first)

extern struct wdo *Curwdo, *Whead;
extern struct zbuff *Curbuff;
extern struct buff *Bbuff;

struct zbuff {
	struct zbuff *prev;	/**< list of buffers */
	struct zbuff *next;	/**< list of buffers */
	char *bname;            /**< buffer name */
	char *fname;            /**< file associated with buffer */
	struct mark *umark;     /**< user mark */
	struct buff *buff;	/**< low-level buffer */
	time_t mtime;           /**< file modified time */
	void *chead;		/**< list of comments in file */
	void *ctail;		/**< list of comments in file */
	Byte comchar;		/**< single char comment character */
	unsigned int bmode;	/**< buffer mode */
};

#define foreachbuff(b) for ((b) = Bufflist; (b); (b) = (b)->next)

#define Buff()  (*Bbuff->curcptr)

extern char *Home;
extern int Homelen;
extern bool Argp;
extern int Arg;				/* must be signed */
extern int InPaw;		/* Are we in the Paw window? */
extern char PawStr[];		/* handy string to put text in */
#define PAWSTRLEN (COLMAX + 10)
extern int Pawcol, Pawlen, Pshift;
extern struct zbuff *Paw;
extern int verbose;

extern unsigned int Cmd;
extern int Cmdpushed;

extern struct cnames Cnames[];
extern void (*Cmds[][2])();
extern int Curcmds;
extern Byte Keys[], Lfunc;
extern bool First;
extern int ring_bell;
extern int raw_mode;
extern int text_mode;

#define CMD(n) ((*Cmds[n][Curcmds])())

extern struct zbuff *Bufflist;
extern struct zbuff *Buff_save;
extern struct mark *Sstart;
extern bool Initializing;
extern bool Insearch;
extern int circf;

extern void (*Nextpart)(void);

#define clrpaw()	putpaw("")

#define PREFLINE			10

#define	CR	('\r')
#define	NL	('\n')

/* Zedit specific attributes */
#define T_STANDOUT		T_REVERSE
#ifdef TERMCAP
#define T_COMMENT		T_BOLD
#else
#define T_COMMENT		(T_FG | T_RED)
#endif
#define T_REGION		T_REVERSE

void set_last_bufname(struct zbuff *buff);

char *zgetcwd(char *cwd, int len);

/* this is MUCH faster than an isascii isprint pair */
#define ZISPRINT(c)		(c >= ' ' && c <= '~')

/* terminal variables */
extern int Prow, Pcol;			/* Point row and column */
extern int Colmax, Rowmax;		/* Row and column maximums */
extern int Tabsize;

#define ISNL(c)			((c) == '\n')

#define UMARK_SET (Curbuff->umark)
#define NEED_UMARK do { if (!UMARK_SET) { tbell(); return; } } while (0)
#define UMARK (Curbuff->umark) /* Must guarantee umark set! */
#define CLEAR_UMARK clear_umark()

#include "proto.h"

/* @} */

#endif /* _Z_H_ */

/* The NEED_UMARK macro is just too useful */
/*
 * Local Variables:
 * my-checkpatch-ignores: "SPDX_LICENSE_TAG,MACRO_WITH_FLOW_CONTROL"
 * End:
 */
