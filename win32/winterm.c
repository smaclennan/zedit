/* term.c - generic terminal commands
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

#include "z.h"
#include <signal.h>

int Clrcol[ROWMAX + 1];		/* Clear if past this */

int Prow, Pcol;			/* Point row and column */
static int Srow, Scol;		/* Saved row and column */
int Colmax = EOF, Rowmax;	/* Row and column maximums */

#ifdef SIGWINCH
/* This is called if the window has changed size. */
static void sigwinch(int sig)
{
	if (Initializing)
		termsize();
	else {
		Zredisplay();		/* update the windows */
		zrefresh();		/* force a screen update */
	}

#ifdef SYSV4
	signal(SIGWINCH, sigwinch);
#endif
}
#endif

HANDLE hstdin, hstdout;	/* Console in and out handles */

/* Come here on SIGHUP or SIGTERM */
void hang_up(int signal)
{
	struct buff *tbuff;

	InPaw = true;	/* Kludge to turn off error */
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next) {
		if (tbuff->bmodf && !(tbuff->bmode & SYSBUFF) && bfname()) {
			bswitchto(tbuff);
			bwritefile(bfname());
		}
#if SHELL
		if (tbuff->child != EOF)
			unvoke(tbuff, false);
#endif
	}
#if SHELL
	checkpipes(0);
#endif
	tfini();
	exit(1);
}

/* This is called before the windows are created */
static void initline(void)
{
	int i = sprintf(PawStr, "%s %s  Initializing", ZSTR, VERSION);
	tclrwind();
	t_goto(Rowmax - 2, 0);
	tstyle(T_STANDOUT);
	tprntstr(PawStr);
	for (++i; i < Colmax; ++i)
		tprntchar(' ');
	tstyle(T_NORMAL);
	t_goto(0, 0);
	tflush();
}

/* Initalize the terminal. */
void tinit(void)
{
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	/* We want everything else disabled */
	SetConsoleMode(hstdin, ENABLE_WINDOW_INPUT);

#ifdef SIGHUP
	signal(SIGHUP,  hang_up);
#endif
#ifdef SIGTERM
	signal(SIGTERM, hang_up);
#endif
#if SHELL
#if !defined(WNOWAIT)
	signal(SIGCLD,  sigchild);
#endif
	signal(SIGPIPE, sigchild);
#endif
#ifdef SIGWINCH
	signal(SIGWINCH, sigwinch); /* window has changed size - update */
#endif

	/* Must be after setting up tty */
	termsize();

	if (Rowmax < 3) {
		/* screen too small */
		tfini();
		exit(1);
	}

	Srow = Scol = -1;	/* undefined */
	initline();		/* Curwdo not defined yet */
}

void tfini(void)
{
#ifdef HAVE_TERMIOS
	tcsetattr(fileno(stdin), TCSAFLUSH, &save_tty);
#elif defined(HAVE_TERMIO)
	ioctl(fileno(stdin), TCSETAF, &save_tty);
#elif defined(HAVE_SGTTY)
	stty(fileno(stdin), &save_tty);
	ioctl(fileno(stdin), TIOCSETC, &savechars);
	ioctl(fileno(stdin), TIOCSLTC, &savelchars);
#endif

	clrpaw();
	t_goto(Rowmax - 1, 0);
	tstyle(T_NORMAL);
	tflush();
}

void setmark(bool prntchar)
{
	tstyle(T_REVERSE);
	tprntchar(prntchar ? Buff() : ' ');
	tstyle(T_NORMAL);
}

static void tsize(int *rows, int *cols)
{
	if (Colmax == EOF) {
		/* Win8 creates a huge screen buffer (300 lines) */
		COORD size;
		size.X = *cols = 80;
		size.Y = *rows = 25;
		SetConsoleScreenBufferSize(hstdout, size);
	} else {
		CONSOLE_SCREEN_BUFFER_INFO info;

		if (GetConsoleScreenBufferInfo(hstdout, &info)) {
			*cols = info.dwSize.X;
			*rows = info.dwSize.Y;
		}
	}
}

void termsize(void)
{
	int rows = 0, cols = 0;

	/* Get the defaults from the low level interface */
	tsize(&rows, &cols);

	Rowmax = rows <= 0 ? 24 : rows;
	if (Rowmax > ROWMAX)
		Rowmax = ROWMAX;

	Colmax = cols <= 0 ? 80 : cols;
	if (Colmax > COLMAX)
		Colmax = COLMAX;
}

static int tabsize(int col)
{	/* optimize for most used tab sizes */
	switch (Tabsize) {
	case 2:
	case 4:
	case 8:
		return Tabsize - (col & (Tabsize - 1));
	default:
		return Tabsize - (col % Tabsize);
	}
}

/* Print a char. */
void tprntchar(Byte ichar)
{
	int tcol;

	if (ZISPRINT(ichar)) {
		tforce();
		tputchar(ichar);
		++Scol;
		++Pcol;
		if (Clrcol[Prow] < Pcol)
			Clrcol[Prow] = Pcol;
	} else
		switch (ichar) {
		case '\t':
			if (InPaw)
				tprntstr("^I");
			else
				for (tcol = tabsize(Pcol); tcol > 0; --tcol)
					tprntchar(' ');
			break;

		case 0x89:
			tstyle(T_BOLD);
			tprntstr("~^I");
			tstyle(T_NORMAL);
			break;

		default:
			tstyle(T_BOLD);
			if (ichar & 0x80) {
				tprntchar('~');
				tprntchar(ichar & 0x7f);
			} else {
				tprntchar('^');
				tprntchar(ichar ^ '@');
			}
			tstyle(T_NORMAL);
			break;
	}
}

/* Calculate the width of a character.
 * The 'adjust' parameter adjusts for the end of line.
*/
int chwidth(Byte ch, int col, bool adjust)
{
	int wid;

	if (ZISPRINT(ch))
		return 1;
	switch (ch) {
	case '\n':
		return InPaw ? 2 : 0;
	case '\t':
		wid = tabsize(col);
		if (col + wid >= tmaxcol())
			wid = tmaxcol() - col + Tabsize - 1;
		if (!adjust)
			wid = MIN(wid, Tabsize);
		return InPaw ? 2 : wid;
	default:
		wid = ((ch & 0x80) && !isprint(ch & 0x7f)) ? 3 : 2;
		if (adjust) {
			int delta = col + wid - tmaxcol();
			if (delta >= 0)
				wid += delta + 1;
		}
		return wid;
	}
}

void tprntstr(const char *str)
{
	while (*str)
		tprntchar(*str++);
}

void t_goto(int row, int col)
{
	tsetpoint(row, col);
	tforce();
}

int prefline(void)
{
	int line, w;

	w = wheight();
	line = PREFLINE * w / (Rowmax - 2);
	return line < w ? line : w >> 1;
}

void tforce(void)
{
	if (Scol != Pcol || Srow != Prow) {
		COORD where;

		where.X = Pcol;
		where.Y = Prow;
		SetConsoleCursorPosition(hstdout, where);
		Srow = Prow;
		Scol = Pcol;
	}
}

void tcleol(void)
{
	if (Pcol < Clrcol[Prow]) {
		COORD where;
		DWORD written;

		where.X = Pcol;
		where.Y = Prow;
		FillConsoleOutputCharacter(hstdout, ' ', Clrcol[Prow] - Pcol,
					   where, &written);

		/* This is to clear a possible mark */
		if (Clrcol[Prow])
			where.X = Clrcol[Prow] - 1;
		FillConsoleOutputAttribute(hstdout, WHITE_ON_BLACK, 1,
					   where, &written);

		Clrcol[Prow] = Pcol;
	}
}

void tclrwind(void)
{
	memset(Clrcol, 0, ROWMAX);
	Prow = Pcol = 0;

	COORD where;
	DWORD written;
	where.X = where.Y = 0;
	FillConsoleOutputAttribute(hstdout, WHITE_ON_BLACK, Colmax * Rowmax,
				   where, &written);
	FillConsoleOutputCharacter(hstdout, ' ', Colmax * Rowmax,
				   where, &written);
}

#define WHITE_ON_BLACK (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BLACK_ON_WHITE (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)

void tstyle(int style)
{
	static int cur_style = -1;

	if (style == cur_style)
		return;

	switch (cur_style = style) {
	case T_NORMAL:
		SetConsoleTextAttribute(hstdout, WHITE_ON_BLACK);
		break;
	case T_STANDOUT:
	case T_REVERSE:
		SetConsoleTextAttribute(hstdout, BLACK_ON_WHITE);
		break;
	case T_BOLD:
		SetConsoleTextAttribute(hstdout,
					WHITE_ON_BLACK | FOREGROUND_INTENSITY);
		break;
	case T_COMMENT:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED);
		break;
	}
	fflush(stdout);
}

void tbell(void)
{
	if (VAR(VBELL)) {
		COORD where;
		DWORD written;
		where.X = 0;
		where.Y = Rowmax - 2;
		FillConsoleOutputAttribute(hstdout, WHITE_ON_BLACK, Colmax,
					   where, &written);
		Beep(440, 250);
		FillConsoleOutputAttribute(hstdout, BLACK_ON_WHITE, Colmax,
					   where, &written);
	}
}

/* SAM we can get much smarter with tputchar and tflush... */
void tputchar(Byte c)
{
	DWORD written;
	WriteConsole(hstdout, &c, 1, &written, NULL);
}

void tflush(void) {}
