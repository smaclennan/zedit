/* tinit.c - generic terminal init/fini routines
 * Copyright (C) 1988-2016 Sean MacLennan
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
#include <unistd.h>
#include "tinit.h"

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

int Prow, Pcol;
static int Srow = -1, Scol = -1;
static int Clrcol[ROWMAX];

/* Come here on SIGHUP or SIGTERM */
static void t_hang_up(int signo)
{
	printf("\r\nHang up! (%d)\r\n", signo);
	exit(1);
}

static void tfini(void)
{
	tcsetattr(0, TCSAFLUSH, &save_tty);
	tflush();
}

#ifdef TERMCAP
#define NUMCM	7
#define MUST	3
char *cm[NUMCM];
static char bp[1024];
static char area[1024];
char *termcap_end;

static void tlinit(void)
{
	static const char * const names[] = {
		"cm", "ce", "cl", "me", "so", "vb", "md"
	};
	char *was = area, *end = area, *term = getenv("TERM");
	int i;

	if (term == NULL) {
		printf("ERROR: environment variable TERM not set.\n");
		exit(1);
	}
	if (tgetent(bp, term) != 1) {
		printf("ERROR: Unable to get termcap entry for %s.\n", term);
		exit(1);
	}

	/* get the initialization string and send to stdout */
	tgetstr("is", &end);
	if (end != was)
		TPUTS(was);

	/* get the termcap strings needed - must be done last */
	for (i = 0; i < NUMCM; ++i) {
		cm[i] = end;
		tgetstr(names[i], &end);
		if (cm[i] == end) {
			if (i < MUST) {
				Dbg("Missing termcap entry for %s\n", names[i]);
				exit(1);
			} else
				cm[i] = "";
		}
	}

	termcap_end = end;

	termcap_keys();
}
#else
static void tlinit(void) {}
#endif

/* Initalize the terminal. */
void tinit(void)
{
	tcgetattr(0, &save_tty);
	tcgetattr(0, &settty);
	settty.c_iflag = 0;
#ifdef TAB3
	settty.c_oflag = TAB3;
#endif
	settty.c_lflag = ECHOE | ECHOK;
	settty.c_cc[VMIN] = (char) 1;
	settty.c_cc[VTIME] = (char) 1;
	tcsetattr(0, TCSANOW, &settty);

#ifdef SIGHUP
	signal(SIGHUP,  t_hang_up);
#endif
#ifdef SIGTERM
	signal(SIGTERM, t_hang_up);
#endif

	tlinit();
	atexit(tfini);
}

void tsize(int *rows, int *cols)
{
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
		if (sscanf(buf, "\033[%d;%dR", rows, cols) != 2) {
			*rows = 25;
			*cols = 80;
		}
	}
}

/* Optimized routines to minimize output */

/** Move the cursor to the current Prow+Pcol */
void tforce(void)
{
	if (Scol != Pcol || Srow != Prow) {
#ifdef TERMCAP
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
	putchar(ch);
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
		tforce();
#ifdef TERMCAP
		TPUTS(cm[1]);
#else
		fputs("\033[K", stdout);
		tflush();
#endif
		Clrcol[Prow] = Pcol;
	}
}

/** Clear the entire window (screen) */
void tclrwind(void)
{
#ifdef TERMCAP
	TPUTS(cm[2]);
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

#ifdef TERMCAP
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
	printf("\033[%dm", style);
#endif

	cur_style = style;
	tflush();
}
