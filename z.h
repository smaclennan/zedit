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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "config.h"
#include "vars.h"
#include "funcs.h"
#include "buff.h"
#include "proto.h"

#define ZSTR	"Zedit"
#define VERSION	"5.2"

#define INVALID		-1

#define READ_MODE	O_RDONLY
#define WRITE_MODE	(O_WRONLY | O_CREAT | O_TRUNC)

#define ZBFILE		".bindings.z"
#define ZCFILE		".config.z"

/* the first three must follow:  (define + 1) % 4 = 0 */
#define BUFNAMMAX	31			/* max buffer name */
#define PATHMAX		255			/* max file name */
#define STRMAX		79			/* string max */

#define BACKWARD	0			/* go forwards */
#define FORWARD		1			/* go backwards */
#define REGEXP		2			/* reg exp search */
#define QUERY		3			/* query replace */
#define SGLOBAL		3			/* global search */
#define AGAIN		-1			/* go again */
#define NUMASCII	256			/* number of ascii chars */
#define ESIZE		256			/* reg exp buffer size */

/*
 * BUFFER MODES
 */
/* minor modes - 0 to all allowed */
#define OVERWRITE			0x001
#define EXACT				0x002
#define VIEW				0x004
/* majour modes - only one allowed */
#define NORMAL				0x010
#define CMODE				0x020
#define TEXT				0x040
#define SHMODE				0x080
/* super modes - 0 to all allowed */
#define SYSBUFF				0x1000
/* some handy macro for buffer modes */
#define MAJORMODE			(0xff0)
#define PROGMODE			(CMODE | SHMODE)

/* System buffer names */
#define UNTITLED	"NoFile"	/* for buffers with no fname */
#define MAINBUFF	"*scratch*"
#define PAWBUFNAME	"*PAW*"		/* Paw - should never be display */
#define HELPBUFF	"*help*"
#define MAKEBUFF	"*compile*"
#define SHELLBUFF	"*shell*"
#define LISTBUFF	"*list*"
#define CONFBUFF	"*config*"

#define MAKE_CMD	"make "
#define GREP_CMD	"grep -n"

/* ask return values */
#define BANG		2
#define YES		1
#define NO		0
#define ABORT		-1
#define BADCHAR		-2

struct cnames {
	char *name;
	int fnum;
	char *doc;
};
#define CNAMESIZE sizeof(struct cnames)

extern char *Home;
extern bool Argp;
extern int Arg;				/* must be signed */
extern int InPaw;		/* Are we in the Paw window? */
extern char PawStr[];		/* handy string to put text in */
extern int Pawcol, Pawlen, Pshift;
extern Byte tline[];

extern char *Cwd;
extern unsigned Cmd;

extern struct cnames Cnames[];
extern void (*Cmds[][2])();
extern int Curcmds;
extern Byte Keys[], Lfunc;
extern bool First;

#define CMD(n) (*Cmds[n][Curcmds])()

extern int cpushed;
extern bool Sendp;
extern struct buff *Killbuff;
extern char Lbufname[];
extern struct avar Vars[];
extern struct buff *Bufflist, *Paw;
extern struct buff *Buff_save;
extern struct mark *Sstart, *Psstart, *Send, *REstart;
extern struct mark Scrnmarks[];
extern bool Initializing;
extern bool Insearch;
extern Byte CRdefault;
extern int circf;
extern unsigned long undo_total;

extern void (*Nextpart)(void);

#define MIN(a, b)	(a < b ? a : b)
#define MAX(a, b)	(a > b ? a : b)

#define clrpaw(void)	putpaw("")
#define error(...)	do { tbell(); putpaw(__VA_ARGS__); } while (0)

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

#define tsetpoint(r, c)		(Prow = r, Pcol = c)
#define tgetrow()		Prow
#define tgetcol()		Pcol
#define tmaxrow()		Rowmax
#define tmaxcol()		Colmax
extern int Tabsize;

#define tputchar(c)		putchar(c)
#define tflush()		fflush(stdout)

#define wheight()		(Curwdo->last - Curwdo->first)

/* this is MUCH faster than an isascii isprint pair */
#define ZISPRINT(c)		(c >= ' ' && c <= '~')

/* terminal variables */
extern int Clrcol[ROWMAX + 1];		/* Clear if past this - must be Byte */
extern int Prow, Pcol;			/* Point row and column */
extern int Colmax, Rowmax;		/* Row and column maximums */
extern int Tlrow;			/* Last row displayed (-1 for none) */

#define ISNL(c)			((c) == '\n')
#define STRIP(c)		(c)

#endif /* _Z_H_ */
