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

#if DOPIPES
#include <sys/wait.h>	/* need for WNOWAIT */
#endif

#if defined(HAVE_TERMIOS)
#include <termios.h>
static struct termios save_tty;
static struct termios settty;
#elif defined(HAVE_TERMIO)
#include <termio.h>
static struct termio save_tty;
static struct termio settty;
#elif defined(HAVE_SGTTY)
#include <sgtty.h>
static struct sgttyb save_tty;
static struct sgttyb settty;
static struct tchars savechars;
static struct tchars setchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static struct ltchars savelchars;
static struct ltchars setlchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#elif defined(WIN32)
HANDLE hstdin, hstdout;	/* Console in and out handles */

#define WHITE_ON_BLACK (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)
#define BLACK_ON_WHITE (BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED)
#elif defined(DOS)
#include <conio.h>
#else
#error No term driver
#endif

int Clrcol[ROWMAX + 1];		/* Clear if past this */

int Prow, Pcol;			/* Point row and column */
static int Srow, Scol;		/* Saved row and column */
int Colmax = EOF, Rowmax;	/* Row and column maximums */

int ring_bell;			/* tbell called */

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

	InPaw = true;	/* Kludge to turn off error */
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next) {
		if (tbuff->bmodf && !(tbuff->bmode & SYSBUFF) && bfname()) {
			bswitchto(tbuff);
			bwritefile(bfname());
		}
		if (tbuff->child != EOF)
			unvoke(tbuff, false);
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
#ifdef HAVE_TERMIOS
	tcgetattr(fileno(stdin), &save_tty);
	tcgetattr(fileno(stdin), &settty);
	settty.c_iflag = 0;
	settty.c_oflag = TAB3;
	settty.c_lflag = ECHOE | ECHOK;
	settty.c_cc[VMIN] = (char) 1;
	settty.c_cc[VTIME] = (char) 1;
	tcsetattr(fileno(stdin), TCSANOW, &settty);
#elif defined(HAVE_TERMIO)
	ioctl(fileno(stdin), TCGETA, &save_tty);
	ioctl(fileno(stdin), TCGETA, &settty);
	settty.c_iflag = 0;
	settty.c_oflag = TAB3;
	settty.c_lflag = ECHOE | ECHOK;
	settty.c_cc[VMIN] = (char) 1;
	settty.c_cc[VTIME] = (char) 1;
	ioctl(fileno(stdin), TCSETAW, &settty);
#elif defined(HAVE_SGTTY)
	gtty(fileno(stdin), &save_tty);
	gtty(fileno(stdin), &settty);

	/* set CBREAK (raw) mode no ECHO, leave C-Ms alone so we can
	 * read them */
	settty.sg_flags |= CBREAK;
	settty.sg_flags &= ~(ECHO | CRMOD);
	stty(fileno(stdin), &settty);

	ioctl(fileno(stdin), TIOCGETC, &savechars);
	ioctl(fileno(stdin), TIOCSETC, &setchars);

	ioctl(fileno(stdin), TIOCGLTC, &savelchars);
	ioctl(fileno(stdin), TIOCSLTC, &setlchars);
#elif defined(WIN32)
	hstdin = GetStdHandle(STD_INPUT_HANDLE);
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

	/* We want everything else disabled */
	SetConsoleMode(hstdin, ENABLE_WINDOW_INPUT);
#endif

#ifdef SIGHUP
	signal(SIGHUP,  hang_up);
#endif
#ifdef SIGTERM
	signal(SIGTERM, hang_up);
#endif
#if DOPIPES
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

	tsetcursor();

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
		FillConsoleOutputAttribute(hstdout, WHITE_ON_BLACK, 1,
					   where, &written);
#elif defined(DOS)
		int i, len = Clrcol[Prow] - Pcol;

		tforce();
		for (i = 0; i < len; ++i)
			putch(' ');
		/* We need to move the cursor back.
		 * We cannot call tforce since it has saved the position. */
		gotoxy(Pcol + 1, Prow + 1);
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
	FillConsoleOutputAttribute(hstdout, WHITE_ON_BLACK, Colmax * Rowmax,
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
	}
#else
	switch (style) {
	case T_NORMAL:
		fputs("\033[0m", stdout); break;
	case T_STANDOUT:
	case T_REVERSE:
		fputs("\033[7m", stdout); break;
	case T_BOLD:
		fputs("\033[1m", stdout); break;
	case T_COMMENT:
		fputs("\033[31m", stdout); break; /* red */
	}
#endif

	cur_style = style;
	tflush();
}

void tbell(void)
{
#ifdef WIN32
	COORD where;
	DWORD written;
	where.X = 0;
	where.Y = Rowmax - 2;
	FillConsoleOutputAttribute(hstdout, WHITE_ON_BLACK, Colmax,
				   where, &written);
	Beep(440, 250);
	FillConsoleOutputAttribute(hstdout, BLACK_ON_WHITE, Colmax,
				   where, &written);
#elif defined(DOS)
	Curwdo->modeflags = INVALID;
#else
	fputs("\033[?5h", stdout);
	fflush(stdout);
	usleep(100000);
	fputs("\033[?5l", stdout);
#endif

	ring_bell = 1;
}

void tsetcursor(void)
{
#ifdef DOS
	if (Curbuff->bmode & OVERWRITE)
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
/* SAM we can get much smarter with tputchar and tflush... */
void tputchar(Byte c)
{
	DWORD written;
	WriteConsole(hstdout, &c, 1, &written, NULL);
}

void tflush(void) {}
#endif
