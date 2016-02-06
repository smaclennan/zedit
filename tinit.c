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
#include <signal.h>
#include "tinit.h"

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
