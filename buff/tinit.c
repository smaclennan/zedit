/* tinit.c - generic terminal init/fini routines
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

#include <signal.h>
#include "tinit.h"

/* \cond skip */
#ifdef __unix__
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
HANDLE hstdout;	/* Console out handle */
#endif

/* Come here on SIGHUP or SIGTERM */
static void t_hang_up(int signo)
{
	if (signo == SIGTERM)
		terror("\r\nTerminate!\r\n");
#ifdef SIGHUP /* windows doesn't have SIGHUP */
	else if (signo == SIGHUP)
		terror("\r\nHang up!\r\n");
#endif
	else
		terror("\r\nGot signal!\r\n");
	exit(1);
}

static void tfini(void)
{
#ifdef __unix__
	tcsetattr(0, TCSAFLUSH, &save_tty);
#endif
	tflush();
}

#ifdef TERMCAP
#define NUMCM	7
#define MUST	3
char *cm[NUMCM];
static char bp[1024];
static char area[1024];
char *termcap_end;

static char *key_names[] = {
	"ku",
	"kd",
	"kr",
	"kl",

	"kI",
	"kD",
	"kP",
	"kN",
	"kh",
	"@7",

	"k1",
	"k2",
	"k3",
	"k4",
	"k5",
	"k6",
	"k7",
	"k8",
	"k9",
	"k;",
	"F1",
	"F2",
};

extern void set_tkey(int i, char *key);

static void tlinit(void)
{
	static const char * const names[] = {
		"cm", "ce", "cl", "me", "so", "vb", "md"
	};
	char *key, *was = area, *end = area, *term = getenv("TERM");
	int i;

	if (term == NULL) {
		terror("ERROR: environment variable TERM not set.\n");
		exit(1);
	}
	if (tgetent(bp, term) != 1) {
		terror("ERROR: Unable to get termcap entry.\n");
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

	/* get the cursor and function key defines */
	for (i = 0; i < 22; ++i) {
		key = termcap_end;
		tgetstr(key_names[i], &termcap_end);
		if (key != termcap_end)
			if (*key == 033)
				set_tkey(i, key);
	}
}
#else
static void tlinit(void) {}
#endif
/* \endcond */

/** @addtogroup term
 * @{
*/

/** Initalize the terminal. Sets the terminal up for raw character at
 * a time with no echo input.
 */
void tinit(void)
{
#ifdef __unix__
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
#elif defined(WIN32)
	tkbdinit();
	hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

#ifdef SIGHUP
	signal(SIGHUP,  t_hang_up);
#endif
#ifdef SIGTERM
	signal(SIGTERM, t_hang_up);
#endif

	tlinit();
	atexit(tfini);
}
/* @} */
