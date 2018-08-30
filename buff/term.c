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

void tsize(int *rows, int *cols)
{
#ifdef WIN32
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD size;

	if (GetConsoleScreenBufferInfo(hstdout, &info)) {
		size.Y = info.srWindow.Bottom - info.srWindow.Top + 1;
		size.X = info.srWindow.Right - info.srWindow.Left + 1;
	}

	if (size.X != *cols || size.Y != *rows) {
		if (size.X > COLMAX) size.X = COLMAX;
		if (size.Y > ROWMAX) size.Y = ROWMAX;
		*cols = size.X;
		*rows = size.Y;
		SetConsoleScreenBufferSize(hstdout, size);
	}
#else
	char buf[12], *p;
	int n, r, c = 0;

	/* Save cursor position */
	twrite("\033[s", 3);
	/* Send the cursor to the extreme right corner */
	twrite("\033[999;999H", 10);
	/* Ask where we really ended up */
	twrite("\033[6n", 4);
	n = read(0, buf, sizeof(buf) - 1);
	/* Restore cursor */
	twrite("\033[u", 3);

	if (n > 0) {
		buf[n] = '\0';
		if (buf[0] == '\033' && buf[1] == '[') {
			r = strtol(buf + 2, &p, 10);
			if (*p++ == ';')
				c = strtol(p, NULL, 10);
		}
		if (r > 0 && c > 0) {
			*rows = r;
			*cols = c;
		}
	}
#endif
}

/* Optimized routines to minimize output */

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
		char str[64], *p = str;

		*p++ = '\033'; *p++ = '[';
		p = _utoa(Prow + 1, p);
		*p++ = ';';
		p = _utoa(Pcol + 1, p);
		*p++ = 'H';
		twrite(str, p - str);
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

void tstyle(int style)
{
	static int cur_style = -1;

	if (style == cur_style)
		return;

#ifdef WIN32
	switch (style) {
	case T_NORMAL:
		SetConsoleTextAttribute(hstdout, ATTR_NORMAL);
		break;
	case T_REVERSE:
		SetConsoleTextAttribute(hstdout, ATTR_REVERSE);
		break;
	case T_BOLD:
		SetConsoleTextAttribute(hstdout,
					ATTR_NORMAL | FOREGROUND_INTENSITY);
		break;
	case T_FG + T_BLACK:
		SetConsoleTextAttribute(hstdout, ATTR_NORMAL); break;
	case T_FG + T_RED:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED); break;
	case T_FG + T_GREEN:
		SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN); break;
	case T_FG + T_YELLOW:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_GREEN); break;
	case T_FG + T_BLUE:
		SetConsoleTextAttribute(hstdout, FOREGROUND_BLUE); break;
	case T_FG + T_MAGENTA:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_BLUE); break;
	case T_FG + T_CYAN:
		SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN | FOREGROUND_BLUE); break;
	case T_FG + T_WHITE:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	case T_BG + T_BLACK:
		SetConsoleTextAttribute(hstdout, ATTR_REVERSE); break;
	case T_BG + T_RED:
		SetConsoleTextAttribute(hstdout, BACKGROUND_RED); break;
	case T_BG + T_GREEN:
		SetConsoleTextAttribute(hstdout, BACKGROUND_GREEN); break;
	case T_BG + T_YELLOW:
		SetConsoleTextAttribute(hstdout, BACKGROUND_RED | BACKGROUND_GREEN); break;
	case T_BG + T_BLUE:
		SetConsoleTextAttribute(hstdout, BACKGROUND_BLUE); break;
	case T_BG + T_MAGENTA:
		SetConsoleTextAttribute(hstdout, BACKGROUND_RED | BACKGROUND_BLUE); break;
	case T_BG + T_CYAN:
		SetConsoleTextAttribute(hstdout, BACKGROUND_GREEN | BACKGROUND_BLUE); break;
	case T_BG + T_WHITE:
		SetConsoleTextAttribute(hstdout, BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
		break;
	}
#elif defined(TERMCAP)
	switch (style) {
	case T_NORMAL:
		TPUTS(cm[3]);
		break;
	case T_REVERSE:
		TPUTS(cm[4]);
		break;
	case T_BELL:
		TPUTS(cm[5]);
		break;
	case T_BOLD:
		TPUTS(cm[6]);
		break;
	default:
		return;
	}
#else
	char str[32], *p = str;

	*p++ = '\033'; *p++ = '[';
	p = _utoa(style, p);
	*p++ = 'm';
	twrite(str, p - str);
#endif

	cur_style = style;
	tflush();
}
