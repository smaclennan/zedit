/* term.c - generic terminal commands
 * Copyright (C) 1988-2010 Sean MacLennan
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
#include "keys.h"
#ifdef TERMINFO
#include <term.h>
#endif

#include <signal.h>
#include <sys/wait.h>	/* need for WNOWAIT */

#ifndef XWINDOWS
#if defined(LINUX)
#include <termios.h>
static struct termios savetty;
static struct termios settty;
#elif defined(SYSV2)
#include <termio.h>
static struct termio savetty;
static struct termio settty;
#elif defined(BSD)
#include <sgtty.h>
static struct sgttyb savetty;
static struct sgttyb settty;
static struct tchars savechars;
static struct tchars setchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
static struct ltchars savelchars;
static struct ltchars setlchars = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#endif
#endif

size_t Clrcol[ROWMAX + 1];	/* Clear if past this */

int Prow, Pcol;				/* Point row and column */
int Srow, Scol;				/* saved row and column */
size_t Colmax, Rowmax;			/* Row and column maximums */
int Tstart;					/* Start column and row */

#ifndef XWINDOWS
#ifdef SIGWINCH
/* This is called if the window has changed size.
 * If Exitflag is set, we are not ready to update display yet.
 */
static void sigwinch(int sig)
{
	if (Exitflag)
		termsize();
	else {
		Zredisplay();		/* update the windows */
		refresh();			/* force a screen update */
	}

#ifdef SYSV2
	signal(SIGWINCH, sigwinch);
#endif
}
#endif

/* Initalize the terminal. */
void tinit(void)
{
	/* Initialize the low level interface.
	 * Do this first - it may exit */
	tlinit();

	termsize();

#ifdef LINUX
	tcgetattr(fileno(stdin), &savetty);
	tcgetattr(fileno(stdin), &settty);
	settty.c_iflag = VAR(VFLOW) ? (IXON | IXOFF) : 0;
	settty.c_oflag = TAB3;
	settty.c_lflag = ECHOE | ECHOK;
	settty.c_cc[VMIN] = (char) 1;
	settty.c_cc[VTIME] = (char) 1;
	tcsetattr(fileno(stdin), TCSANOW, &settty);
#elif defined(SYSV2)
	ioctl(fileno(stdin), TCGETA, &savetty);
	ioctl(fileno(stdin), TCGETA, &settty);
	settty.c_iflag = VAR(VFLOW) ? (IXON | IXOFF) : 0;
	settty.c_oflag = TAB3;
	settty.c_lflag = ECHOE | ECHOK;
	settty.c_cc[VMIN] = (char) 1;
	settty.c_cc[VTIME] = (char) 1;
	ioctl(fileno(stdin), TCSETAW, &settty);
#elif defined(BSD)
	gtty(fileno(stdin), &savetty);
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
#endif

	signal(SIGHUP,  Hangup);
	signal(SIGTERM, Hangup);
#ifdef PIPESH
#if !defined(SYSV4) || !defined(WNOWAIT)
	signal(SIGCLD,  Sigchild);
#endif
	signal(SIGPIPE, Sigchild);
#endif
#ifdef BSD
	signal(SIGTSTP, SIG_DFL);		/* set signals so that we can */
	signal(SIGCONT, tinit);		/* suspend & restart Zedit */
#endif
#ifdef SIGWINCH
	signal(SIGWINCH, sigwinch); /* window has changed size - update */
#endif

	if (Rowmax < 3) {
		/* screen too small */
		tfini();
		exit(1);
	}

	Srow = Scol = -1;	/* undefined */
	if (Exitflag)
		Initline();		/* Curwdo not defined yet */
	else
		Zredisplay();
}

void tfini(void)
{
#if defined(LINUX)
	tcsetattr(fileno(stdin), TCSAFLUSH, &savetty);
#elif defined(SYSV2)
	ioctl(fileno(stdin), TCSETAF, &savetty);
#elif defined(BSD)
	stty(fileno(stdin), &savetty);
	ioctl(fileno(stdin), TIOCSETC, &savechars);
	ioctl(fileno(stdin), TIOCSLTC, &savelchars);
#endif

	if (VAR(VCLEAR))
		tclrwind();
	else {
		clrecho();
		tgoto(Rowmax - 1, 0);
	}
	tflush();
	tlfini();
}

void tbell(void)
{
	if (VAR(VVISBELL)) {
#ifdef __linux__
		fputs("\033[?5h", stdout);
		fflush(stdout);
		usleep(100000);
		fputs("\033[?5l", stdout);
#elif defined(TERMINFO)
		TPUTS(flash_screen);
#endif
	} else if (VAR(VSILENT) == 0)
		putchar('\7');
}

void setmark(Boolean prntchar)
{
	tstyle(T_REVERSE);
	tprntchar(prntchar ? Buff() : ' ');
	tstyle(T_NORMAL);
}

/*
 * Set Rowmax and Colmax.
 *		1. Use environment variables.
 *		2. Use given variables.
 *		3. Default to 24x80
 *		4. Limit to ROWMAXxCOLMAX.
 */
void termsize(void)
{
	int rows, cols;
	FILE *fp;

	/* Get the defaults from the low level interface */
	tsize(&rows, &cols);

	/* If we have resize, trust it */
	fp = popen("resize -u", "r");
	if (fp) {
		char buf[1024], name[STRMAX + 1];
		int n;

		while (fgets(buf, sizeof(buf), fp))
			if (sscanf(buf, "%[^=]=%d;\n", name, &n) == 2) {
				if (strcmp(name, "COLUMNS") == 0)
					cols = n;
				else if (strcmp(name, "LINES") == 0)
					rows = n;
			}
		pclose(fp);
	} else { /* Check the environment */
		char *p = getenv("LINES");
		if (p)
			rows = atoi(p);
		p = getenv("COLUMNS");
		if (p)
			cols = atoi(p);
	}

	Rowmax = rows <= 0 ? 24 : rows;
	if (Rowmax > ROWMAX)
		Rowmax = ROWMAX;

	Colmax = cols <= 0 ? 80 : cols;
	if (Colmax > COLMAX)
		Colmax = COLMAX;
}
#endif /* !XWINDOWS */

void extendedlinemarker(void)
{
	int col;

	for (col = tgetcol(); col < tmaxcol() - 1; ++col)
		tprntchar(' ');
	tstyle(T_BOLD);
	tprntchar('>');
	tstyle(T_NORMAL);
}

/* Print a char. */
void tprntchar(Byte ichar)
{
	int tcol;

	if (ISPRINT(ichar)) {
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
			else {
				/* optimize for most used tab sizes */
				if (Tabsize == 4 || Tabsize == 8)
					tcol = Tabsize - (Pcol & (Tabsize - 1));
				else
					tcol = Tabsize - (Pcol % Tabsize);
				for (; tcol > 0; --tcol)
					tprntchar(' ');
			}
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
int chwidth(Byte ch, int col, Boolean adjust)
{
	int wid;

	if (ISPRINT(ch))
		return 1;
	if (InPaw && (ch == '\n' || ch == '\t'))
		return 2;
	if (ch == '\n')
		return 0;

	if (ch == '\t') {
		if (Tabsize == 4 || Tabsize == 8)
			wid = Tabsize - (col & (Tabsize - 1));
		else
			wid = Tabsize - (col % Tabsize);
		if (col + wid >= tmaxcol())
			wid = tmaxcol() - col + Tabsize - 1 - Tstart;
		if (!adjust)
			wid = MIN(wid, Tabsize);
	} else {
		int delta;

		wid = ((ch & 0x80) && !isprint(ch & 0x7f)) ? 3 : 2;
		if (adjust) {
			delta = col + wid - tmaxcol() - Tstart;
			if (delta >= 0)
				wid += delta + 1 - Tstart;
		}
	}
	return wid;
}

void tprntstr(char *str)
{
	while (*str)
		tprntchar(*str++);
}

void tgoto(int row, int col)
{
	tsetpoint(row, col);
	tforce();
}

/* Print a decimal number. */
void titot(unsigned cntr)
{
	if (cntr > 9)
		titot(cntr / 10);
	tprntchar(cntr % 10 + '0');
}

int prefline(void)
{
	int line, w;

	w = wheight();
	line = PREFLINE * w / (Rowmax - 2);
	return line < w ? line : w >> 1;
}

#ifndef XWINDOWS
void tforce(void)
{
	if (Scol != Pcol || Srow != Prow) {
#ifdef TERMINFO
		TPUTS(tparm(cursor_address, Prow, Pcol));
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
		tforce();
#ifdef TERMINFO
		TPUTS(clr_eol);
#else
		TPUTS("\033[K");
#endif
		Clrcol[Prow] = Pcol;
	}
}

void tclrwind(void)
{
#ifdef TERMINFO
	TPUTS(clear_screen);
#else
	TPUTS("\033[2J");
#endif
	memset(Clrcol, 0, ROWMAX);
	Prow = Pcol = 0;
	tflush();
}

/* for tputs this must be a function */
#ifdef LINUX
int _putchar(int ch)
#else
int _putchar(char ch)
#endif
{
	putchar(ch);
	return 0;	/*shutup*/
}

void newtitle(char *str) {}
#endif /* !XWINDOWS */
