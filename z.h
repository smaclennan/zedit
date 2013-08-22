/* z.h - Main Zedit include file
 * Copyright (C) 1988-2010 Sean MacLennan
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

#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>

#include "typedefs.h"
#include "vars.h"
#include "term.h"
#include "funcs.h"
#include "buff.h"

extern int Verbose;

extern struct passwd *Me;

#ifdef SYSV4
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>
#endif

#define ZSTR	"Zedit"
#define VERSION	"5.0"
#define ZFMT	"%s %s  (%s)  %s: "

/* These are portable across different Unix's */
#define TOLOWER(c)		(isupper(c) ? tolower(c) : c)
#define TOUPPER(c)		(islower(c) ? toupper(c) : c)

#ifndef TRUE
#define FALSE		0
#define TRUE		1
#endif
#define INVALID		-1

#define READ_MODE	O_RDONLY
#define READ_BINARY	O_RDONLY
#define WRITE_MODE	(O_WRONLY | O_CREAT | O_TRUNC)
#define UPDATE_MODE	(O_RDWR   | O_CREAT | O_TRUNC)

#define Psep(c)		(c == '/')
#define PSEP		'/'

#define ZSHFILE		".zshXXXXXX"
#define ZBFILE		".bindings.z"
#define ZCFILE		".config.z"
#define ZEFILE		".expand.z"
#define ZSFILE		".save.z"
#define ZCLIP		".zclip"
#define ZDBGFILE	"z.out"
#define ZHFILE		"help.z"

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
#define ALLFILES	"*.*"
#define NUMASCII	256			/* number of ascii chars */
#define ESIZE		256			/* reg exp buffer size */
#define BOOKMARKS	10			/* number of book marks */

#define FINDPATHS	4

/*
 * BUFFER MODES
 *
 * CCxSMMMm
 *	m minor  mode
 *	M majour mode
 *	S super  mode
 *	C comment char
 */
/* minor modes - 0 to all allowed */
#define OVERWRITE			0x001
#define EXACT				0x002
/* majour modes - only one allowed */
#define NORMAL				0x010
#define CMODE				0x020
#define TEXT				0x040
#define ASMMODE				0x080
#define SHMODE				0x100
/* super modes - 0 to all allowed */
#define VIEW				0x40000
#define SYSBUFF				0x80000
/* some handy macro for buffer modes */
#define MAJORMODE			(0xfff0)
#define PROGMODE			(CMODE | ASMMODE | SHMODE)
#define MODEMASK			(~(NORMAL | TEXT | PROGMODE))

/* System buffer names */
#define UNTITLED	"NoFile"	/* for buffers with no fname */
#define MAINBUFF	"*scratch*"
#define PAWBUFNAME	"*PAW*"		/* Paw - should never be display */
#define HELPBUFF	"*help*"
#define MAKEBUFF	"*compile*"
#define MANBUFF		"*man*"
#define SHELLBUFF	"*shell*"
#define TAGBUFNAME	"*tags*"
#define SPELLBUFF	"*spell*"
#define LISTBUFF	"*list*"
#define REFBUFF		"*ref*"

/* ask return values */
#define BANG		2
#define YES		1
#define NO		0
#define ABORT		-1
#define BADCHAR		-2

/* Help Types */
#define H_NONE				0
#define H_MISC				1
#define H_VAR				2
#define H_CURSOR			3
#define H_DELETE			4
#define H_SEARCH			5
#define H_FILE				6
#define H_BUFF				7
#define H_DISP				8
#define H_MODE				9
#define H_HELP				10
#define H_BIND				11
#define H_SHELL				12

/* GENERAL STRUCTURE DEFS */

/* general linked list structure */
struct llist {
	char fname[STRMAX];
	struct llist *prev, *next;
};

struct cnames {
	char *name;
	short fnum;
	short htype;
};
#define CNAMESIZE sizeof(struct cnames)

extern Boolean Argp;
extern int Arg;				/* must be signed */
extern Boolean InPaw;		/* Are we in the Paw window? */
extern char PawStr[];		/* handy string to put text in */
extern int Pawcol, Pawlen, Pshift;
extern Byte tline[];

extern char *Cwd;
extern int Cmask;
extern unsigned Cmd;
extern char *Shell;
extern char *Thispath;
extern char *ConfigDir;

extern struct cnames Cnames[];
extern void (*Cmds[][3])();
extern int Curcmds;
extern Byte Keys[], Lfunc;
extern Boolean First;

#define CMD(n) (*Cmds[n][Curcmds])()

extern int cpushed;
extern fd_set SelectFDs;
extern int NumFDs;

extern Boolean Sendp;
extern struct buff *Killbuff;
extern char Lbufname[];
extern char Fname[];
extern struct avar Vars[];
extern struct buff *Bufflist, *Paw;
extern struct buff *Buff_save;
extern struct mark *Sstart, *Psstart, *Send, *REstart;
extern struct mark Scrnmarks[];

extern Boolean Initializing;
extern Boolean Insearch;

extern char **Bnames;
extern int Numbuffs;
extern unsigned Nextpart;

extern Byte CRdefault;

extern char G_start[], G_end[];
extern char Calc_str[];
extern char savetag[];
extern char Command[];
extern char mkcmd[];
extern char grepcmd[];
extern char old[], new[];
extern Boolean searchdir[];

extern int circf;

#define MIN(a, b)	(a < b ? a : b)
#define MAX(a, b)	(a > b ? a : b)


#define echo(s)		putpaw(s, FALSE)
#define error(s)	putpaw(s, TRUE)
#define clrecho(void)	putpaw("", 2)

#include "proto.h"

#endif /* _Z_H_ */
