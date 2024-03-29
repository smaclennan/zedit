/* Copyright (C) 1988-2018 Sean MacLennan */

#include "z.h"
#include <signal.h>

/** @addtogroup zedit
 * @{
 */

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
		if (tbuff->buff->bmodf &&
			!(tbuff->bmode & SYSBUFF) &&
			Curbuff->fname)
			bwritefile(tbuff->buff, tbuff->fname, file_mode());
	unvoke(NULL);
	checkpipes(0);
	strfmt(PawStr, PAWSTRLEN, "\r\nThey hung up! (%d)\r\n", signo);
	terror(PawStr);
	exit(1);
}

/* This is called before the windows are created */
static void initline(void)
{
	int i = strconcat(PawStr, PAWSTRLEN, ZSTR,
					  " ", VERSION, "  Initializing", NULL);
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
	clrpaw();
	t_goto(Rowmax - 1, 0);
	tstyle(T_NORMAL);
}

/* Initalize the terminal. */
void tainit(void)
{
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

	initline();		/* Curwdo not defined yet */

	atexit(tafini);
}

void termsize(void)
{
	int rows = 0, cols = 0;

	/* Get the defaults from the low level interface */
	tsize(&rows, &cols);

	Rowmax = rows <= 0 ? 24 : MIN(rows, ROWMAX);
	Colmax = cols <= 0 ? 80 : MIN(cols, COLMAX);
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

	switch (ichar) {
	case ' '...'~':
		tputchar(ichar);
		break;
	case '\t':
		if (InPaw)
			tprntstr("^I");
		else
			for (tcol = tabsize(Pcol); tcol > 0; --tcol)
				tputchar(' ');
		break;
	case 0x89:
		tstyle(T_BOLD);
		tprntstr("~^I");
		tstyle(T_NORMAL);
		break;
	default:
		tstyle(T_BOLD);
		if (ichar & 0x80) {
			tputchar('~');
			tprntchar(ichar & 0x7f);
		} else {
			tputchar('^');
			tprntchar(ichar ^ '@');
		}
		tstyle(T_NORMAL);
	}
}

/* Calculate the width of a character.
 * The 'adjust' parameter adjusts for the end of line.
 */
int chwidth(Byte ch, int col, bool adjust)
{
	int wid;

	switch (ch) {
	case ' '...'~':
		return 1;
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

void tbell(void)
{
	ring_bell = 1;
	if (Curwdo)
		Curwdo->modeflags = INVALID;
}
/* @} */
