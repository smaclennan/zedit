/* term.h - terminal defines
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

#include <stdlib.h>

/* The memory usage for screen stuff is approx:  (ROWMAX + 1) x 25 + COLMAX */
/*                               Xwindows adds:	 ROWMAX * 4 + COLMAX * 4    */

/* NOTE: We assume COLMAX >= ROWMAX (xinit.c) */
#define	ROWMAX				110
#define	COLMAX				256

#define PREFLINE			10

#define	CR	'\r'
#define	NL	'\n'
#define	ESC	'\33'
#define DEL	'\177'

/* attributes - offesets into cm array in termcaps */
#define T_STANDOUT			3
#define T_NORMAL			4
#define T_REVERSE			5
#define T_BOLD				6
#define T_COMMENT			10	/* COMMENTBOLD only */
#define T_CPP				11	/* COMMENTBOLD only */
#define T_CPPIF				12	/* COMMENTBOLD only */

#ifdef LINUX
int _putchar(int);
#else
int _putchar(char);
#endif

#if defined(TERMINFO)
#define TPUTS(s)		tputs(s, 1, _putchar)
#elif defined(ANSI)
#define TPUTS(s)		fputs(s, stdout)
#endif

#define tsetpoint(r, c)		(Prow = r, Pcol = c)
#define tgetrow()		Prow
#define tgetcol()		Pcol
#define tmaxrow()		Rowmax
#define tmaxcol()		Colmax
extern int Tabsize;
#define twidth(ch)		chwidth(ch, Pcol - Tstart, FALSE)
#define bwidth(ch, col)		chwidth(ch, col, TRUE)
#define Sindent(arg)		while (arg-- > 0) binsert(' ')

#ifdef XWINDOWS
#define tforce()
#else
#define showcursor(x)
#define showmark(x)
#define tputchar(c)		putchar(c)
#define tflush()		fflush(stdout)
#endif

extern unsigned Cmdpushed, Cmdstack[];
#define POPCMD()		Cmdstack[--Cmdpushed]
#define PUSHCMD(cmd)		Cmdstack[Cmdpushed++] = cmd

#define wheight()		(Curwdo->last - Curwdo->first)

/* this is MUCH faster than an isascii isprint pair */
#define ISPRINT(c)		(c >= ' ' && c <= '~')


/* terminal variables */
/* SAM Why must be byte??? */
extern size_t Clrcol[ROWMAX + 1];	/* Clear if past this - must be Byte */
extern int Prow, Pcol;			/* Point row and column */
extern int Srow, Scol;			/* saved row and column */
extern size_t Colmax, Rowmax;		/* Row and column maximums */
extern int Tstart;			/* Start column and row */
extern int Tlrow;			/* Last row displayed (-1 for none) */

#define PNUMCOLS		3	/* default columns for pout */
#define PCOLSIZE		26	/* default column size */

#define ISNL(c)			((c) == '\n')
#define STRIP(c)		(c)
