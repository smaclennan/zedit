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
#ifdef TERMINFO
	TPUTS(reset_1string);
	resetterm();
#endif
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
			set_tkey(i, key);
	}
}
#elif defined(TERMINFO)
static void tlinit(void)
{
	int rc, i;

	char *Term = getenv("TERM");
	if (Term == NULL) {
		printf("FATAL ERROR: environment variable TERM not set.\n");
		exit(1);
	}

	setupterm(Term, 1, &rc);
	if (rc != 1) {
		printf("FATAL ERROR: Unable to get terminfo entry for %s.\n",
			   Term);
		exit(1);
	}
	if (!clear_screen || !clr_eol || !cursor_address) {
		printf("FATAL ERROR: Terminfo entry for %s incomplete.\n",
			   Term);
		exit(1);
	}

	if (!exit_attribute_mode)
		enter_reverse_mode = enter_standout_mode = enter_bold_mode = NULL;

	/* initialize the terminal */
	TPUTS(init_1string);
	TPUTS(init_2string);
	TPUTS(init_3string);

	set_tkey(0, key_up);
	set_tkey(1, key_down);
	set_tkey(2, key_right);
	set_tkey(3, key_left);

	set_tkey(4, key_ic);
	set_tkey(5, key_dc);
	set_tkey(6, key_ppage);
	set_tkey(7, key_npage);
	set_tkey(8, key_home);
	set_tkey(9, key_end);

	if (key_f0) { /* old school */
		set_tkey(10, key_f0);
		set_tkey(11, key_f1);
		i = 12;
	} else {
		set_tkey(10, key_f1);
		set_tkey(21, key_f12);
		i = 11;
	}
	set_tkey(i++, key_f2);
	set_tkey(i++, key_f3);
	set_tkey(i++, key_f4);
	set_tkey(i++, key_f5);
	set_tkey(i++, key_f6);
	set_tkey(i++, key_f7);
	set_tkey(i++, key_f8);
	set_tkey(i++, key_f9);
	set_tkey(i++, key_f10);
	set_tkey(i++, key_f11);

#ifdef SAM_NO
	Key_mask = 0x00c00000; /* C-Home and C-End not in terminfo */
	for (k = 0; k < i; ++k)
		if (Tkeys[k] && *Tkeys[k])
			Key_mask |= 1 << k;

	if (verbose) {
		for (k = 0; k < i; ++k)
			dump_key(k, Tkeys[k], NULL);

		Dbg("Key Mask %x\n", Key_mask);
	}
#endif
}
#else
static void tlinit(void)
{
	char *term = getenv("TERM");

	/* I like st, but they have a weird mix of ansi and posix and the
	 * terminfo entry lies.
	 */
	if(strcmp(term, "st") == 0 || strncmp(term, "st-", 3) == 0)
		st_hack();
}
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
