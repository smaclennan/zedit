/* term.c - generic terminal routines
 * Copyright (C) 1988-2017 Sean MacLennan
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

#include "tinit.h"

int Prow, Pcol;
static int Srow = -1, Scol = -1;
static int Clrcol[ROWMAX];

/* Optimized routines to minimize output */

#ifdef TBUFFERED

#define MAX_BUF 2048 /* a nice screen full */
static Byte tbuffer[MAX_BUF];
static int tcur;

int twrite(const void *buf, int len)
{
again:
	if (tcur + len < MAX_BUF) {
		memcpy(tbuffer + tcur, buf, len);
		tcur += len;
		return len;
	}

	tflush();

	if (len >= MAX_BUF) {
		write(1, buf, len);
		return len;
	}

	goto again;
}

void tflush(void)
{
	if (tcur) {
		write(1, tbuffer, tcur);
		tcur = 0;
	}
}
#endif

/** Move the cursor to the current Prow+Pcol */
void tforce(void)
{
	if (Scol != Pcol || Srow != Prow) {
#ifdef WIN32
		COORD where;

		where.X = Pcol;
		where.Y = Prow;
		SetConsoleCursorPosition(hstdout, where);
#elif defined(TERMCAP)
		TPUTS(tgoto(cm[0], Pcol, Prow));
#else
		char str[64];
		int n = strfmt(str, sizeof(str), "\033[%d;%dH", Prow + 1, Pcol + 1);
		twrite(str, n);
#endif
		Srow = Prow;
		Scol = Pcol;
	}
}

/** Print a character at the current Prow+Pcol */
void tputchar(Byte ch)
{
	tforce();
#ifdef WIN32
	DWORD written;
	WriteConsole(hstdout, &ch, 1, &written, NULL);
#else
	twrite(&ch, 1);
#endif
	++Scol;
	++Pcol;
	if (Clrcol[Prow] < Pcol)
		Clrcol[Prow] = Pcol;
}

/** Move the cursor to row+col */
void t_goto(int row, int col)
{
	tsetpoint(row, col);
	tforce();
}

/** Clear from Prow+Pcol to end of line */
void tcleol(void)
{
	if (Prow >= ROWMAX)
		Prow = ROWMAX - 1;

	if (Pcol < Clrcol[Prow]) {
#ifdef WIN32
		COORD where;
		DWORD written;

		where.X = Pcol;
		where.Y = Prow;
		FillConsoleOutputCharacter(hstdout, ' ', Clrcol[Prow] - Pcol,
					   where, &written);

		/* This is to clear a possible mark */
		if (Clrcol[Prow])
			where.X = Clrcol[Prow] - 1;
		FillConsoleOutputAttribute(hstdout, ATTR_NORMAL, 1,
					   where, &written);
#else
		tforce();
#ifdef TERMCAP
		TPUTS(cm[1]);
#else
		twrite("\033[K", 3);
		tflush();
#endif
#endif
		Clrcol[Prow] = Pcol;
	}
}

/** Clear the entire window (screen) */
void tclrwind(void)
{
#ifdef WIN32
	COORD where;
	DWORD written;
	where.X = where.Y = 0;
	FillConsoleOutputAttribute(hstdout, ATTR_NORMAL, COLMAX * ROWMAX,
				   where, &written);
	FillConsoleOutputCharacter(hstdout, ' ', COLMAX * ROWMAX,
				   where, &written);
#elif defined(TERMCAP)
	TPUTS(cm[2]);
#else
	twrite("\033[2J", 4);
#endif
	memset(Clrcol, 0, sizeof(Clrcol));
	Prow = Pcol = 0;
	tflush();
}
