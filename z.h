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

#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include "global.h"
#include "vars.h"
#include "term.h"
#include "funcs.h"
#include "buff.h"

#include <errno.h>
#ifndef errno
extern int errno;
#endif
extern int Verbose;

#include <pwd.h>
extern struct passwd *Me;

#if SYSV2
#ifdef MAXNAMLEN
/* KLUDGE for SYSV machines that have "BSD" compatibility */
#define BSDDIR	1
#endif
#endif

#if SYSV4
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>
#endif

#include <fcntl.h>

#include <stdlib.h>


#define ZSTR		"Zedit"
#define VERSION	"5 beta"
#define ZFMT		"%s %s  (%s)  %s: "



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

/* Zedit globals */
#define VERSTR		"1.4"		/* save file version string */

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
#define TCL					0x100
/* super modes - 0 to all allowed */
#define VIEW				0x40000
#define SYSBUFF				0x80000
/* some handy macro for buffer modes */
#define MAJORMODE			(0xfff0)
#define PROGMODE			(CMODE | ASMMODE | TCL)
#define MODEMASK			(~(NORMAL | TEXT | PROGMODE))

/* System buffer names */
#define UNTITLED	"NoFile"	/* for buffers with no fname */
#define MAINBUFF	"Main"		/* not a system buffer */
#define PAWBUFNAME	"PAW"		/* Paw - should never be display */
#define DIRBUFNAME	".dir"
#define HELPBUFF	".help"
#define MAKEBUFF	".make"
#define MANBUFF		".man"
#define SHELLBUFF	".shell"
#define TAGBUFNAME	".tags"
#define SPELLBUFF	".spell"
#define LISTBUFF	".list"
#define REFBUFF		".ref"

/* Ask return values */
#define YES		1
#define NO		0
#define ABORT		-1
#define BADCHAR		-2

#define FORMSTRING	">FIELD<"


/* GENERAL STRUCTURE DEFS */

/* general linked list structure */
struct llist {
	char fname[STRMAX];
	struct llist *prev, *next;
};


extern Boolean Argp;
extern int Arg;				/* must be signed */
extern Boolean InPaw;		/* Are we in the Paw window? */
extern char PawStr[];		/* handy string to put text in */

extern int Cmask;

extern Proc (**Funcs)();
extern Byte Keys[], Lfunc;
extern Boolean First;

extern int cpushed;
extern fd_set SelectFDs;
extern int NumFDs;

extern Boolean Sendp;
extern Buffer *Killbuff;
extern char Lbufname[];
extern struct avar Vars[];
extern Buffer *Bufflist, *Paw;
extern Mark *Sstart, *Psstart, *Send, *REstart;

extern char **Bnames;
extern int Numbuffs;
extern unsigned Nextpart;


#define Stricmp				strcasecmp
#define Strnicmp			strncasecmp

#define MIN(a, b)			(a < b ? a : b)
#define MAX(a, b)			(a > b ? a : b)


#define Echo(s)				PutPaw(s, FALSE)
#define Error(s)			PutPaw(s, TRUE)


#if DBG
extern Byte Dbgstr[];
extern int Dbgint;
#endif

#define ASSERT(n)	if (!(n)) Hangup(n)

#include "proto.h"

#if SUNBSD
#include "sun.h"
#endif
