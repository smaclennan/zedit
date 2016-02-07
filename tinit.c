/* tinit.c - generic terminal init/fini routines
 * Copyright (C) 2016 Sean MacLennan
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "tinit.h"

#ifdef __unix__
#include <unistd.h>

#ifdef HAVE_TERMIO
#include <termio.h>
static struct termio save_tty;
static struct termio settty;
#define tcgetattr(fd, tty) ioctl(fd, TCGETA, tty)
#define tcsetattr(fd, type, tty) ioctl(fd, TCSETAW, tty)
#undef TCSAFLUSH
#define TCSAFLUSH TCSETAF
#else
#include <termios.h>
static struct termios save_tty;
static struct termios settty;
#endif
#elif defined(WIN32)
HANDLE hstdin, hstdout;	/* Console in and out handles */
#endif

int Prow, Pcol;
static int Srow = -1, Scol = -1;
static int Clrcol[ROWMAX];

/* App specific init and fini */
void __attribute__ ((weak)) tainit(void) {}
void __attribute__ ((weak)) tafini(void) {}

/* Come here on SIGHUP or SIGTERM */
void __attribute__ ((weak)) hang_up(int signo)
{
	printf("\r\nThey hung up! (%d)\r\n", signo);
	exit(1);
}

static void tfini(void)
{
#ifdef __unix__
	tcsetattr(fileno(stdin), TCSAFLUSH, &save_tty);
#endif

	tafini();
	tflush();
}

/* Initalize the terminal. */
void tinit(void)
{
#ifdef __unix__
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
#endif

#ifdef SIGHUP
	signal(SIGHUP,  hang_up);
#endif
#ifdef SIGTERM
	signal(SIGTERM, hang_up);
#endif

	/* Must set this before tainit() in case it calls exit */
	atexit(tfini);

	tainit();
}


void tsize(int *rows, int *cols)
{
#ifdef WIN32
	static int first_time = 1;

	if (first_time) {
		/* Win8 creates a huge screen buffer (300 lines) */
		COORD size;
		size.X = *cols = 80;
		size.Y = *rows = 25;
		SetConsoleScreenBufferSize(hstdout, size);
		first_time = 0;
	} else {
		CONSOLE_SCREEN_BUFFER_INFO info;
		if (GetConsoleScreenBufferInfo(hstdout, &info)) {
			*cols = info.dwSize.X;
			*rows = info.dwSize.Y;
		}
	}
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
#elif defined(TERMINFO)
		TPUTS(tparm(cursor_address, Prow, Pcol));
#elif defined(TERMCAP)
		TPUTS(tgoto(cm[0], Pcol, Prow));
#else
		printf("\033[%d;%dH", Prow + 1, Pcol + 1);
		tflush();
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
	WriteConsole(hstdout, &c, 1, &written, NULL);
#else
	putchar(ch);
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
#ifdef TERMINFO
		TPUTS(clr_eol);
#elif defined(TERMCAP)
		TPUTS(cm[1]);
#else
		fputs("\033[K", stdout);
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
#elif defined(TERMINFO)
	TPUTS(clear_screen);
#elif defined(TERMCAP)
	TPUTS(cm[2]);
#else
	fputs("\033[2J", stdout);
#endif
	memset(Clrcol, 0, ROWMAX);
	Prow = Pcol = 0;
	tflush();
}

/* Keyboard input */

/** The size of the keyboard input stack. Must be a power of 2 */
#define CSTACK 16
static Byte cstack[CSTACK]; /**< The keyboard input stack */
static int cptr = -1; /**< Current pointer in keyboard input stack. */
static int cpushed; /**< Number of bytes pushed on the keyboard input stack. */

/** This is the lowest level keyboard routine. It reads the keys into
 * a stack then returns the keys one at a time. When the stack is
 * consumed it reads again.
 *
 * The read can block.
 */
Byte tgetkb(void)
{
	cptr = (cptr + 1) & (CSTACK - 1);
	if (cpushed)
		--cpushed;
	else {
		Byte buff[CSTACK];
		int i, p = cptr;

		cpushed = read(0, (char *)buff, CSTACK) - 1;
		if (cpushed < 0)
			hang_up(1);	/* we lost connection */
		for (i = 0; i <= cpushed; ++i) {
			cstack[p] = buff[i];
			p = (p + 1) & (CSTACK - 1);
		}
	}
	return cstack[cptr];
}
