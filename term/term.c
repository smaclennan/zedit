/* Copyright (C) 1988-2017 Sean MacLennan */

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

// This is very specific to ansi_force.
static int rowcol2ascii(char **str, int rowcol)
{
	int n = 0;

	if (rowcol == 0) {
		*(*str)-- = '0';
		return 1;
	}

	while (rowcol > 0) {
		*(*str)-- = (rowcol % 10) + '0';
		rowcol /= 10;
		++n;
	}

	str--;
	return n;
}

// SAM FIXME
static void ansi_force(void)
{
	// The longest string is 10 + 10 + 4
	char str[32], *p = str + 31;

#ifdef ZERO_BASED
	Prow++; Pcol++;
#endif

	// Build the string backwards
	*p-- = 'H';
	int ncol = rowcol2ascii(&p, Pcol);
	*p-- = ';';
	int nrow = rowcol2ascii(&p, Prow);
	*p-- = '[';
	*p = '\033';
	twrite(p, ncol + nrow + 4);

#ifdef ZERO_BASED
	Prow--; Pcol--;
#endif
}

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
		ansi_force();
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

/** Print a character at the current Prow+Pcol. May be buffered.
 * @param ch The character to put to the terminal.
 */
void tputstr(char *str)
{
	tforce();
	while (*str) {
		tputc(*str);
		++str;
		++Scol;
		++Pcol;
	}
	if (Clrcol[Prow] < Pcol)
		Clrcol[Prow] = Pcol;
}

/** Optimized move the cursor to row+col.
 * @param row The row, zero based.
 * @param col The column, zero based.
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
