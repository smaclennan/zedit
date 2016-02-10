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
void hang_up(int signo)
{
	struct zbuff *tbuff;

	InPaw = true;	/* Kludge to turn off error */
	foreachbuff(tbuff)
		if (tbuff->buff->bmodf && !(tbuff->bmode & SYSBUFF) && Curbuff->fname)
			bwritefile(tbuff->buff, tbuff->fname, file_mode());
	unvoke(NULL);
	checkpipes(0);
	printf("\r\nThey hung up! (%d)\r\n", signo);
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

static void tafini(void)
{
	tlfini();

	set_mouse(false);

	clrpaw();
	t_goto(Rowmax - 1, 0);
	tstyle(T_NORMAL);
}

/* Initalize the terminal. */
void tainit(void)
{
#ifdef WIN32
	/* We want everything else disabled */
	SetConsoleMode(hstdin, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
#endif

	tlinit();

	set_mouse(true);

	/* Override the signal handler in tinit() */
#ifdef SIGHUP
	signal(SIGHUP,  hang_up);
#endif
#ifdef SIGTERM
	signal(SIGTERM, hang_up);
#endif
#ifdef SIGWINCH
	signal(SIGWINCH, sigwinch); /* window has changed size - update */
#endif
#ifdef DOPIPES
	siginit();
#endif

	/* Must be after setting up tty */
	termsize();

	if (Rowmax < 3) {
		/* screen too small */
		exit(1);
	}

	tsetcursor(false);

	initline();		/* Curwdo not defined yet */

	atexit(tafini);
}

void setmark(bool prntchar)
{
	tstyle(T_REVERSE);
	tprntchar(prntchar ? Buff() : ' ');
	tstyle(T_NORMAL);
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
		tputchar(ichar);
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

int prefline(void)
{
	int line, w;

	w = wheight();
	line = PREFLINE * w / (Rowmax - 2);
	return line < w ? line : w >> 1;
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
#elif defined(TERMCAP)
	switch (style) {
	case T_NORMAL:
		TPUTS(cm[3]);
		break;
	case T_STANDOUT: /* modeline */
		if (ring_bell && *cm[5])
			TPUTS(cm[5]);
		else
			TPUTS(cm[4]);
		break;
	case T_REVERSE:
	case T_REGION:
		TPUTS(cm[4]);
		break;
	case T_COMMENT:
		TPUTS(cm[6]);
		break;
	}
#else
	switch (style) {
	case T_NORMAL:
		fputs("\033[0m", stdout); break;
	case T_STANDOUT: /* modeline */
		fputs(ring_bell ? "\033[41m" : "\033[7m", stdout); break;
	case T_REVERSE:
	case T_REGION:
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

void tsetcursor(bool hide)
{
#ifdef WIN32
	CONSOLE_CURSOR_INFO cursorinfo;

	if (Curbuff->bmode & OVERWRITE)
		cursorinfo.dwSize = 25; /* default */
	else
		cursorinfo.dwSize = 100; /* solid */
	cursorinfo.bVisible = true;
	SetConsoleCursorInfo(hstdout, &cursorinfo);
#endif
}

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
	struct mark tmark;
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

	bmrktopnt(Bbuff, &tmark);

	/* Move the point to row */
	if (row > Prow)
		while (Prow < row) {
			bcsearch(Bbuff, '\n');
			++Prow;
		}
	else if (row <= Prow) {
		while (Prow > row) {
			bcrsearch(Bbuff, '\n');
			--Prow;
		}
		tobegline(Bbuff);
	}

	/* Move the point to col */
	atcol = 0;
	while (col > 0 && !bisend(Bbuff) && Buff() != '\n') {
		int n = chwidth(Buff(), atcol, false);
		bmove1(Bbuff);
		col -= n;
		atcol += n;
	}

	if (set_mark) {
		Zset_mark(); /* mark to point */
		bpnttomrk(Bbuff, &tmark); /* reset mark */
	}
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

#if !defined(TERMCAP) && !defined(TERMCAP_KEYS)
void tlinit(void) {}
void tlfini(void) {}
#endif


