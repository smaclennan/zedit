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
#include "keys.h"
#include "ansi.h"
#include <poll.h>


static unsigned Cmdpushed, Cmdstack[10]; /* stack and vars for T[un]getcmd */

void tpushcmd(int cmd)
{
	Cmdstack[Cmdpushed++] = cmd;
}

static void tungetkb(int j);

static int check_specials(void)
{
	int i, j, bit, mask = Key_mask;

	for (j = 0; mask; ++j) {
		int cmd = tgetkb() & 0x7f;
		for (bit = 1, i = 0; i < NUM_SPECIAL; ++i, bit <<= 1)
			if ((mask & bit) && cmd == Tkeys[i].key[j]) {
				if (Tkeys[i].key[j + 1] == '\0')
					return i + SPECIAL_START;
			} else
				mask &= ~bit;
	}

	/* No match - push back the chars and try to handle
	 * the first one. */
	tungetkb(j);

	return tgetkb() & 0x7f;
}

int tgetcmd(void)
{
	int cmd;

	if (Cmdpushed)
		return Cmdstack[--Cmdpushed];

	cmd = tgetkb() & 0x7f;

	/* All special keys start with ESC */
	if (cmd == '\033')
		if (tkbrdy()) {
			tungetkb(1);
			return check_specials();
		}

	return cmd;
}


/* stack and vars for t[un]getkb / tkbrdy */
#define CSTACK 16 /* must be power of 2 */
static Byte cstack[CSTACK];
static int cptr = -1;
int cpushed;	/* needed in shell.c */
static bool Pending;

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
	Pending = false;
	return cstack[cptr];
}

static void tungetkb(int j)
{
	cptr = (cptr - j) & (CSTACK - 1);
	cpushed += j;
}

#ifdef NO_POLL
static int do_select(int ms)
{
	struct timeval timeout;
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(0, &fds);

	timeout.tv_sec = 0;
	timeout.tv_usec = ms * 1000;

	return select(1, &fds, NULL, NULL, &timeout);
}
#else
static struct pollfd stdin_fd = { .fd = 1, .events = POLLIN };
#endif

int tkbrdy(void)
{
	if (cpushed || Pending)
		return true;

#ifdef NO_POLL
	return Pending = do_select(0);
#else
	return Pending = poll(&stdin_fd, 1, 0) == 1;
#endif
}

bool delay(int ms)
{
	if (InPaw || cpushed || Pending)
		return false;

#ifdef NO_POLL
	return do_select(ms) <= 0;
#else
	return poll(&stdin_fd, 1, ms) != 1;
#endif
}
