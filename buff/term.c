/* generic terminal routines
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

/** @addtogroup term
 * @{
 */

int Prow; /**< Current Point row. */
int Pcol; /**< Current Point column. */

/* \cond skip */
static int Srow = -1, Scol = -1;
static int Clrcol[ROWMAX];
/* \endcond */

/* Optimized routines to minimize output */

#ifdef TBUFFERED

/* a nice screen full */
#define MAX_BUF 2048 /**< twrite() buffer size */

/* \cond skip */
static Byte tbuffer[MAX_BUF];
static int tcur;
/* \endcond */

/** Low-level buffered terminal output routine. tputchar() is the
 * preferred function if you are using the optimized functions.
 * @param buf The output buffer.
 * @param len The length of the output buffer.
 * @return Same return as write() command.
 */
int twrite(const void *buf, int len)
{
again:
	if (tcur + len < MAX_BUF) {
		memcpy(tbuffer + tcur, buf, len);
		tcur += len;
		return len;
	}

	tflush();

	if (len >= MAX_BUF)
		return write(1, buf, len);

	goto again;
}

/** Buffered write optimized for 1 byte. It has the same prototype as
 * putchar() so it can be used by termcap.
 * @param c The byte to write.
 * @return Returns the byte written.
 */
int tputc(int c)
{
	if (tcur >= MAX_BUF)
		tflush();
	tbuffer[tcur++] = (Byte)c;
	return c;
}

/** Buffered terminal flush. Flushes the buffered output. */
void tflush(void)
{
	if (tcur) {
		_twrite(1, tbuffer, tcur);
		tcur = 0;
	}
}
#endif

/* \cond skip */
/** Optimized move the cursor to the current Prow+Pcol. You probably
 * want t_goto().
 */
static void tforce(void)
{
	if (Scol != Pcol || Srow != Prow) {
#if defined(TERMCAP)
		TPUTS(tgoto(cm[0], Pcol, Prow));
#elif defined(TERMINFO)
		TPUTS(tparm(cursor_address, Prow, Pcol));
#else
		char str[64];
		int n = strfmt(str, sizeof(str),
					   "\033[%d;%dH", Prow + 1, Pcol + 1);
		twrite(str, n);
#endif
		Srow = Prow;
		Scol = Pcol;
	}
}
/* \endcond */

/** Print a character at the current Prow+Pcol. May be buffered.
 * @param ch The character to put to the terminal.
 */
void tputchar(Byte ch)
{
	tforce();
	tputc(ch);
	++Scol;
	++Pcol;
	if (Clrcol[Prow] < Pcol)
		Clrcol[Prow] = Pcol;
}

/** Optimized move the cursor to row+col.
 * @param row Row, zero based.
 * @param col Column, zero based.
 */
void t_goto(int row, int col)
{
	Prow = row;
	Pcol = col;
	tforce();
}

/** Optimized clear from Prow+Pcol to end of line. */
void tcleol(void)
{
	if (Prow >= ROWMAX)
		Prow = ROWMAX - 1;

	if (Pcol < Clrcol[Prow]) {
		tforce();
#if defined(TERMCAP)
		TPUTS(cm[1]);
#elif defined(TERMINFO)
		TPUTS(clr_eol);
#else
		twrite("\033[K", 3);
		tflush();
#endif
		Clrcol[Prow] = Pcol;
	}
}

/** Clear the entire window (screen). This performs a flush to make
 * sure the screen is clear.
 */
void tclrwind(void)
{
#if defined(TERMCAP)
	TPUTS(cm[2]);
#elif defined(TERMINFO)
	TPUTS(clear_screen);
#else
	twrite("\033[2J", 4);
#endif
	memset(Clrcol, 0, sizeof(Clrcol));
	Prow = Pcol = 0;
	tflush();
}
/* @} */
