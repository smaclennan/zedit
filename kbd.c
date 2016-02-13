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

#include "tinit.h"
#include "keys.h"
#include <poll.h>
#include <signal.h>
#include <unistd.h>

/** The special multi-byte keys. Note: We can currently only have 32 specials */
static char *Tkeys[] = {
	"\033[A",	/* up */
	"\033[B",	/* down */
	"\033[C",	/* right */
	"\033[D",	/* left */

	"\033[2~",	/* insert */
	"\033[3~",	/* delete */
	"\033[5~",	/* page up */
	"\033[6~",	/* page down */
	"\033[7~",	/* home */
	"\033[8~",	/* end */

	"\033[11~",	/* f1 */
	"\033[12~",	/* f2 */
	"\033[13~",	/* f3 */
	"\033[14~",	/* f4 */
	"\033[15~",	/* f5 */
	"\033[17~",	/* f6 */
	"\033[18~",	/* f7 */
	"\033[19~",	/* f8 */
	"\033[20~",	/* f9 */
	"\033[21~",	/* f10 */
	"\033[23~",	/* f11 */
	"\033[24~",	/* f12 */

	"\033[7^",	/* C-home */
	"\033[8^"	/* C-end */
};

/** The size of the keyboard input stack. Must be a power of 2 */
#define CSTACK 16
static Byte cstack[CSTACK]; /**< The keyboard input stack */
static int cptr = -1; /**< Current pointer in keyboard input stack. */
static int cpushed; /**< Number of bytes pushed on the keyboard input stack. */
static bool Pending; /**< Set to true if poll stdin detected input. */

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
			kill(getpid(), SIGHUP); /* we lost connection */
		for (i = 0; i <= cpushed; ++i) {
			cstack[p] = buff[i];
			p = (p + 1) & (CSTACK - 1);
		}
	}
	return cstack[cptr];
}

/** Push back n keys */
void tungetkb(int n)
{
	cptr = (cptr - n) & (CSTACK - 1);
	cpushed += n;
}

/** Peek the key at a given offset */
Byte tpeek(int offset)
{
	return cstack[(cptr + offset) & (CSTACK - 1)];
}

/** Check if the keyboard input is "special", i.e. One of the
 * multi-byte #Tkeys.
 */
static int check_specials(void)
{
	int i, j, bit, mask = KEY_MASK;

	for (j = 1; mask; ++j) {
		int cmd = tgetkb() & 0x7f;
		for (bit = 1, i = 0; i < NUM_SPECIAL; ++i, bit <<= 1)
			if ((mask & bit) && cmd == Tkeys[i][j]) {
				if (Tkeys[i][j + 1] == '\0')
					return i + SPECIAL_START;
			} else
				mask &= ~bit;
	}

	/* No match - push back the chars */
	tungetkb(j);

	if (tpeek(2) == '0')
		/* Check for ESC O keys - happens a lot on ssh */
		switch (tpeek(3)) {
		case 'A'...'D':
			/* rewrite the arrow keys */
			Tkeys[0] = "\033OA";	/* up */
			Tkeys[1] = "\033OB";	/* down */
			Tkeys[2] = "\033OC";	/* right */
			Tkeys[3] = "\033OD";	/* left */
			/* skip the ESC */
			tgetkb();
			return check_specials();
		}

	return tgetkb() & 0x7f;
}

/** Get keyboard input. Handles the special keys. */
int tgetcmd(void)
{
	int cmd = tgetkb() & 0x7f;
	Pending = false;

	/* All special keys start with ESC */
	if (cmd == '\033' && tkbrdy())
		return check_specials();

	return cmd;
}

/** A static pollfd for stdin. */
static struct pollfd stdin_fd = { .fd = 0, .events = POLLIN };

/** Is there a key waiting? Non-blocking command. */
bool tkbrdy(void)
{
	if (cpushed || Pending)
		return true;

	return Pending = poll(&stdin_fd, 1, 0) == 1;
}

/** Delay for a set time or until there is keyboard input. */
bool tdelay(int ms)
{
	if (cpushed || Pending)
		return false;

	return poll(&stdin_fd, 1, ms) != 1;
}

#ifdef TERMCAP
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

void termcap_keys(void)
{
	extern char *termcap_end;
	int i;
	char *key;

	/* get the cursor and function key defines */
	for (i = 0; i < 22; ++i) {
		key = termcap_end;
		tgetstr(key_names[i], &termcap_end);
		if (key != termcap_end) {
			if (*key != 033) {
				/* This is mainly to catch 0177 for delete */
				if (verbose)
					dump_key(i, key, "skipped");
			} else {
				Tkeys[i] = key;
				if (verbose)
					dump_key(i, key, NULL);
			}
		}
	}
}
#endif
