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

#ifdef __unix__
# ifdef HAVE_TERMIO
#include <termio.h>
static struct termio save_tty;
static struct termio settty;
#define tcgetattr(fd, tty) ioctl(fd, TCGETA, tty)
#define tcsetattr(fd, type, tty) ioctl(fd, TCSETAW, tty)
#undef TCSAFLUSH
#define TCSAFLUSH TCSETAF
# else
#include <termios.h>
static struct termios save_tty;
static struct termios settty;
# endif
#elif defined(WIN32)
HANDLE hstdin, hstdout;	/* Console in and out handles */

#define ATTR_NORMAL	(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY)
#define ATTR_REVERSE	(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY)
#define ATTR_REGION	(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)
#elif defined(DOS)
#include <conio.h>
#else
#error No term driver
#endif

static int Clrcol[ROWMAX + 1];		/* Clear if past this */

int Prow, Pcol;				/* Point row and column */
static int Srow = -1 , Scol = -1;	/* Saved row and column */
int Colmax = EOF, Rowmax;		/* Row and column maximums */

int ring_bell;				/* tbell called */

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

/* Come here on SIGHUP or SIGTERM */
void hang_up(int signal)
{
	struct buff *tbuff;
	((void)signal);

	InPaw = true;	/* Kludge to turn off error */
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next) {
		if (tbuff->bmodf && !(tbuff->bmode & SYSBUFF) && Curbuff->fname) {
			bswitchto(tbuff);
			bwritefile(Curbuff->fname);
		}
		if (tbuff->child != EOF)
			unvoke(tbuff);
	}
	checkpipes(0);
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
#if defined(__unix__)
	tcgetattr(fileno(stdin), &save_tty);
	tcgetattr(fileno(stdin), &settty);
	settty.c_iflag = 0;
	settty.c_oflag = TAB3;
	settty.c_lflag = ECHOE | ECHOK;
	settty.c_cc[VMIN] = (char) 1;
	settty.c_cc[VTIME] = (char) 1;
	tcsetattr(fileno(stdin), TCSANOW, &settty);
#elif defined(WIN32)
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	/* We want everything else disabled */
	SetConsoleMode(hstdin, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
#elif defined(DOS)
	install_ints();
#endif

	set_mouse(true);

#ifdef SIGHUP
	signal(SIGHUP,  hang_up);
#endif
#ifdef SIGTERM
	signal(SIGTERM, hang_up);
#endif
#ifdef SIGWINCH
	signal(SIGWINCH, sigwinch); /* window has changed size - update */
#endif
#if DOPIPES
	siginit();
#endif

	/* Must be after setting up tty */
	termsize();

	if (Rowmax < 3) {
		/* screen too small */
		tfini();
		exit(1);
	}

	tsetcursor(false);

	initline();		/* Curwdo not defined yet */
}

void tfini(void)
{
#ifdef __unix__
	tcsetattr(fileno(stdin), TCSAFLUSH, &save_tty);
#endif

	set_mouse(false);

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
#ifdef WIN32
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
#elif defined(DOS)
	*rows = 25;
	*cols = 80;
#else
	char buf[12];
	int n, w;

	/* Save cursor position */
	w = write(0, "\033[s", 3);
	/* Send the cursor to the extreme right corner */
	w += write(0, "\033[999;999H", 10);
	/* Ask where we really ended up */
	w += write(0, "\033[6n", 4);
	n = read(0, buf, sizeof(buf) - 1);
	/* Restore cursor */
	w += write(0, "\033[u", 3);

	if (n > 0) {
		buf[n] = '\0';
		sscanf(buf, "\033[%d;%dR", rows, cols);
	}
#endif
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
		if (col + wid >= Colmax)
			wid = Colmax - col + Tabsize - 1;
		if (!adjust)
			wid = MIN(wid, Tabsize);
		return InPaw ? 2 : wid;
	default:
		wid = ((ch & 0x80) && !isprint(ch & 0x7f)) ? 3 : 2;
		if (adjust) {
			int delta = col + wid - Colmax;
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
#ifdef WIN32
		COORD where;

		where.X = Pcol;
		where.Y = Prow;
		SetConsoleCursorPosition(hstdout, where);
#elif defined(DOS)
		gotoxy(Pcol + 1, Prow + 1);
#else
		printf("\033[%d;%dH", Prow + 1, Pcol + 1);
#endif
		Srow = Prow;
		Scol = Pcol;
	}
}

void tcleol(void)
{
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
#elif defined(DOS)
		tforce();
		clreol();
#else
		tforce();
		fputs("\033[K", stdout);
#endif
		Clrcol[Prow] = Pcol;
	}
}

void tclrwind(void)
{
#ifdef WIN32
	COORD where;
	DWORD written;
	where.X = where.Y = 0;
	FillConsoleOutputAttribute(hstdout, ATTR_NORMAL, Colmax * Rowmax,
				   where, &written);
	FillConsoleOutputCharacter(hstdout, ' ', Colmax * Rowmax,
				   where, &written);
#elif defined(DOS)
	tstyle(T_NORMAL);
	clrscr();
#else
	fputs("\033[2J", stdout);
#endif
	memset(Clrcol, 0, ROWMAX);
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
	case T_STANDOUT: /* modeline */
		if (ring_bell)
			SetConsoleTextAttribute(hstdout, BACKGROUND_RED);
		else
			SetConsoleTextAttribute(hstdout, ATTR_REVERSE);
		break;
	case T_REVERSE:
		SetConsoleTextAttribute(hstdout, ATTR_REVERSE);
		break;
	case T_BOLD:
		SetConsoleTextAttribute(hstdout,
					ATTR_NORMAL | FOREGROUND_INTENSITY);
		break;
	case T_COMMENT:
		SetConsoleTextAttribute(hstdout, FOREGROUND_RED);
		break;
	case T_REGION:
		SetConsoleTextAttribute(hstdout, ATTR_REGION);
		break;
	}
#elif defined(DOS)
	if (cur_style == T_BOLD)
		normvideo();

	switch (style) {
	case T_NORMAL:
		textcolor(WHITE);
		textbackground(BLACK);
		break;
	case T_STANDOUT: /* modeline */
		textcolor(BLACK);
		textbackground(ring_bell ? RED : WHITE);
		break;
	case T_REVERSE:
		textcolor(BLACK);
		textbackground(WHITE);
		break;
	case T_BOLD:
		highvideo();
		break;
	case T_COMMENT:
		textcolor(RED);
		break;
	case T_REGION:
		textcolor(WHITE);
		textbackground(BLUE);
		break;
	}
#else
	switch (style) {
	case T_NORMAL:
		fputs("\033[0m", stdout); break;
	case T_STANDOUT: /* modeline */
		fputs(ring_bell ? "\033[41m" : "\033[7m", stdout); break;
	case T_REVERSE:
		fputs("\033[7m", stdout); break;
	case T_BOLD:
		fputs("\033[1m", stdout); break;
	case T_COMMENT:
		fputs("\033[31m", stdout); break; /* red */
	case T_REGION:
		fputs("\033[7m", stdout); break;
	}
#endif

	cur_style = style;
	tflush();
}

void tsetcursor(bool hide)
{
#ifdef DOS
	if (hide)
		_setcursortype(_NOCURSOR);
	else if (Curbuff->bmode & OVERWRITE)
		_setcursortype(_NORMALCURSOR);
	else
		_setcursortype(_SOLIDCURSOR);
#elif defined(WIN32)
	CONSOLE_CURSOR_INFO cursorinfo;

	if (Curbuff->bmode & OVERWRITE)
		cursorinfo.dwSize = 25; /* default */
	else
		cursorinfo.dwSize = 100; /* solid */
	cursorinfo.bVisible = true;
	SetConsoleCursorInfo(hstdout, &cursorinfo);
#endif
}

#ifdef WIN32
void tputchar(Byte c)
{
	DWORD written;
	WriteConsole(hstdout, &c, 1, &written, NULL);
}
#endif

void mouse_scroll(int row, bool down)
{
	struct wdo *wdo = wfind(row);
	if (!wdo) {
		error("Not on a window."); /* XEmacs-ish */
		return;
	}

	wswitchto(wdo);

	Arg = 3;
	down ? Znext_line() : Zprevious_line();
}

void mouse_point(int row, int col, bool set_mark)
{
	int atcol;
	struct mark *tmark;
	struct wdo *wdo = wfind(row);
	if (!wdo) {
		error("Not on a window."); /* XEmacs-ish */
		return;
	}

	if (wdo != Curwdo) {
		wswitchto(wdo);
		/* We need Prow and Pcol to be correct. */
		zrefresh();
	}

	tmark = zcreatemrk();

	/* Move the point to row */
	if (row > Prow)
		while (Prow < row) {
			bcsearch('\n');
			++Prow;
		}
	else if (row <= Prow) {
		while (Prow > row) {
			bcrsearch('\n');
			--Prow;
		}
		tobegline();
	}

	/* Move the point to col */
	atcol = 0;
	while (col > 0 && !bisend() && *Curcptr != '\n') {
		int n = chwidth(*Curcptr, atcol, false);
		bmove1();
		col -= n;
		atcol += n;
	}

	if (set_mark) {
		Zset_mark(); /* mark to point */
		bpnttomrk(tmark); /* reset mark */
	}

	unmark(tmark);
}

#undef tbell
void tbell(void)
{
	ring_bell = 1;
	if (Curwdo)
		Curwdo->modeflags = INVALID;
}

void tbell_dbg(char *func, int line)
{
	Dbg("tbell %s:%d\n", func, line);
	tbell();
}
