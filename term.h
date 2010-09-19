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

#if TERMINFO
#define TPUTS(s)		tputs(s, 1, _putchar)
#elif ANSI
#define TPUTS(s)		fputs(s, stdout)
#endif

#define Tsetpoint(r, c)		(Prow = r, Pcol = c)
#define Tgetrow()		Prow
#define Tgetcol()		Pcol
#define Tmaxrow()		Rowmax
#define Tmaxcol()		Colmax
extern int Tabsize;
#define Twidth(ch)		Width(ch, Pcol - Tstart, FALSE)
#define Bwidth(ch, col)		Width(ch, col, TRUE)
#define Sindent(arg)		while (arg-- > 0) Binsert(' ')

#ifdef XWINDOWS
#define Tforce()
#else
#define ShowCursor(x)
#define ShowMark(x)
#define Tputchar(c)		putchar(c)
#define Tflush()		fflush(stdout)
#endif

extern unsigned Cmdpushed, Cmdstack[];
#define Popcmd()		Cmdstack[--Cmdpushed]
#define Pushcmd(cmd)		Cmdstack[Cmdpushed++] = cmd

#define Wheight()		(Curwdo->last - Curwdo->first)

/* this is MUCH faster than an isascii isprint pair */
#define ISPRINT(c)		(c >= ' ' && c <= '~')


/* terminal variables */
/* SAM Why must be byte??? */
extern size_t Clrcol[ROWMAX + 1];	/* Clear if past this - must be Byte */
extern int Prow, Pcol;			/* Point row and column */
extern int Srow, Scol;			/* Saved row and column */
extern size_t Colmax, Rowmax;		/* Row and column maximums */
extern int Tstart;			/* Start column and row */
extern int Tlrow;			/* Last row displayed (-1 for none) */

#define PNUMCOLS		3	/* default columns for Pout */
#define PCOLSIZE		26	/* default column size */

#define ISNL(c)			((c) == '\n')
#define STRIP(c)		(c)
