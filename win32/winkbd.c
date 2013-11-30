/* winkbd.c - keyboard input routines
 * Copyright (C) 2013 Sean MacLennan
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

/* Note: We can currently only have 32 specials */
static struct key_array {
	const char *key;
	const char *label;
} Tkeys[] = {
	{ "\033[A",	"up" },
	{ "\033[B",	"down" },
	{ "\033[C",	"right" },
	{ "\033[D",	"left" },

	{ "\033[2~",	"insert" },
	{ "\033[3~",	"delete" },
	{ "\033[5~",	"page up" },
	{ "\033[6~",	"page down" },
	{ "\033[7~",	"home" },
	{ "\033[8~",	"end" },

	{ "\033[11~",	"f1" },
	{ "\033[12~",	"f2" },
	{ "\033[13~",	"f3" },
	{ "\033[14~",	"f4" },
	{ "\033[15~",	"f5" },
	{ "\033[17~",	"f6" },
	{ "\033[18~",	"f7" },
	{ "\033[19~",	"f8" },
	{ "\033[20~",	"f9" },
	{ "\033[21~",	"f10" },
	{ "\033[23~",	"f11" },
	{ "\033[24~",	"f12" },

	{ "\033Oa",	"C-up" },
	{ "\033Ob",	"C-down" },
	{ "\033Oc",	"C-right" },
	{ "\033Od",	"C-left" },
	{ "\033[7^",	"C-home" },
	{ "\033[8^",	"C-end" },
};
#define N_KEYS ((int)(sizeof(Tkeys) / sizeof(struct key_array)))
#define Key_mask 0x0fffffff

int Cmdpushed = -1;

/* stack and vars for t[un]getkb / tkbrdy */
#define CSTACK 16 /* must be power of 2 */
static Byte cstack[CSTACK];
static int cptr = -1;
int cpushed;	/* needed in shell.c */
static bool Pending;

static void tungetkb(int j)
{
	cptr = (cptr - j) & (CSTACK - 1);
	cpushed += j;
}

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

	/* No match - push back the chars */
	tungetkb(j);

	/* If it is an unknown CSI string, suck it up */
	if (cstack[(cptr + 2) & (CSTACK - 1)] == '[')
		cpushed = 2;

	return tgetkb() & 0x7f;
}

int tgetcmd(void)
{
	int cmd;

	if (Cmdpushed >= 0) {
		cmd = Cmdpushed;
		Cmdpushed = -1;
	} else {
		cmd = tgetkb() & 0x7f;

		/* All special keys start with ESC */
		if (cmd == '\033')
			if (tkbrdy()) {
				tungetkb(1);
				return check_specials();
			}
	}

	return cmd;
}

static bool convertKey(KEY_EVENT_RECORD *event)
{
	bool meta = event->dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED);
	bool ctrl = event->dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED);
	bool shift = event->dwControlKeyState & (CAPSLOCK_ON | SHIFT_PRESSED);
}

Byte tgetkb(void)
{
	Pending = false;

	cptr = (cptr + 1) & (CSTACK - 1);
	if (cpushed) {
		--cpushed;
		return cstack[cptr];
	}

	INPUT_RECORD input[CSTACK];
	DWORD i, n;
	int p = cptr;

again:
	if (!ReadConsoleInput(hstdin, input, CSTACK, &n))
		hang_up(1);	/* we lost connection */

	for (i = 0; i < n; ++i)
		switch (input[i].EventType) {
		case KEY_EVENT: /* 1 */
			if (input[i].Event.KeyEvent.bKeyDown == 0) {
				cstack[p] = (Byte)input[i].Event.KeyEvent.wVirtualKeyCode;
				p = (p + 1) & (CSTACK - 1);
				++cpushed;
			}
			break;
		case WINDOW_BUFFER_SIZE_EVENT:
			Rowmax = input[i].Event.WindowBufferSizeEvent.dwSize.Y;
			Colmax = input[i].Event.WindowBufferSizeEvent.dwSize.X;
			Zredisplay();		/* update the windows */
			zrefresh();		/* force a screen update */
			break;
		case FOCUS_EVENT:
		case MENU_EVENT:
		case MOUSE_EVENT:
			break;
		}

	if (cpushed == 0)
		goto again; /* keep reading till we get a key */

	return cstack[cptr];
}

#ifdef NO_POLL
static int do_select(int ms)
{
#ifdef WIN32
	return false; /* SAM HACK */
#else
	struct timeval timeout;
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(0, &fds);

	timeout.tv_sec = 0;
	timeout.tv_usec = ms * 1000;

	return select(1, &fds, NULL, NULL, &timeout);
#endif
}
#else
#include <poll.h>

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

char *dispkey(unsigned key, char *s)
{
	char *p;
	int j;

	*s = '\0';
	if (key > SPECIAL_START)
		return strcpy(s, Tkeys[key - SPECIAL_START].label);
	if (key > 127)
		strcpy(s, key < 256 ? "M-" : "C-X ");
	j = key & 0x7f;
	if (j == 27)
		strcat(s, "ESC");
	else if (j < 32 || j == 127) {
		strcat(s, "C-");
		p = s + strlen(s);
		*p++ = j ^ '@';
		*p = '\0';
	} else if (j == 32)
		strcat(s, "Space");
	else {
		p = s + strlen(s);
		*p++ = j;
		*p = '\0';
	}
	return s;
}
