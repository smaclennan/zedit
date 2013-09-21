/* kbd.c - keyboard input routines
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
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "keys.h"


unsigned Key_mask;

static unsigned Cmdpushed, Cmdstack[10];	/* stack and vars for T[un]getcmd */

void tpushcmd(int cmd)
{
	Cmdstack[Cmdpushed++] = cmd;
}

static void tungetkb(void);

static int check_specials(void)
{
	int i, j, bit, mask = Key_mask;

	for (j = 0; mask; ++j) {
		int cmd = tgetkb() & 0x7f;
		for (bit = 1, i = 0; i < NUMKEYS - SPECIAL_START; ++i, bit <<= 1)
			if ((mask & bit) && cmd == Tkeys[i].key[j]) {
				if (Tkeys[i].key[j + 1] == '\0')
					return i + SPECIAL_START;
			} else
				mask &= ~bit;
	}

	/* No match - push back the chars and try to handle
	 * the first one. */
	while (j-- > 0)
		tungetkb();

	return tgetkb() & 0x7f;
}

int tgetcmd(void)
{
	if (Cmdpushed)
		return Cmdstack[--Cmdpushed];

#if ANSI
	int cmd = tgetkb() & 0x7f;

	/* All ansi special keys start with ESC */
	if (cmd == '\033')
		if (tkbrdy()) {
			tungetkb();
			return check_specials();
		}

	return cmd;
#else
	return check_specials();
#endif
}


/* stack and vars for t[un]getkb / tkbrdy */
#define CSTACK		20
static Byte cstack[CSTACK];
static int cptr = -1;
int cpushed;	/* needed in z.c */
static int Pending = FALSE;


Byte tgetkb(void)
{
	cptr = (cptr + 1) % CSTACK;
	if (cpushed)
		--cpushed;
	else {
		Byte buff[CSTACK >> 1];
		int i, p;

		cpushed = read(0, (char *)buff, CSTACK >> 1) - 1;
		if (cpushed < 0)
			hangup(1);	/* we lost connection */
		for (i = 0, p = cptr; i <= cpushed; ++i, p = (p + 1) % CSTACK)
			cstack[p] = buff[i];
	}
	Pending = FALSE;
	return cstack[cptr];
}


static void tungetkb(void)
{
	if (--cptr < 0)
		cptr = CSTACK - 1;
	++cpushed;
}

#ifdef HAVE_POLL
static struct pollfd stdin_fd = { .fd = 1, .events = POLLIN };
#endif

int tkbrdy(void)
{
#ifdef HAVE_POLL
	if (cpushed || Pending)
		return TRUE;

	return Pending = poll(&stdin_fd, 1, 0) == 1;
#else
	static struct timeval poll = { 0, 0 };
	int fds = 1;

	if (cpushed || Pending)
		return TRUE;

	return Pending = select(1, (fd_set *)&fds, NULL, NULL, &poll);
#endif
}

Boolean delay(int ms)
{
#ifdef HAVE_POLL
	if (InPaw || tkbrdy())
		return FALSE;

	return poll(&stdin_fd, 1, ms) != 1;
#else
#error No-poll
#endif
}

