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
#include <poll.h>
#if GPM_MOUSE
#include <gpm.h>
#endif

/* Note: We can currently only have 32 specials */
static const char * const Tkeys[] = {
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

/* stack and vars for t[un]getkb / tkbrdy */
#define CSTACK 16 /* must be power of 2 */
static Byte cstack[CSTACK];
static int cptr = -1;
static int cpushed;
static bool Pending;

static Byte tgetkb(void)
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

/* Xterm supports motion events but rxvt does not.
 * Hint: Rxvt*termName: rxvt
 */
void set_mouse(bool enable)
{
	static int is_real_xterm = -1;
	int n = 0;

	if (enable) {
		char *term = getenv("TERM");
		if (term) {
			if (strncmp(term, "xterm", 5) == 0) {
				is_real_xterm = 1;
				n = write(1, "\033[?1002h", 8);
			} else if (strncmp(term, "rxvt", 4) == 0) {
				is_real_xterm = 0;
				n = write(1, "\033[?1000h", 8);
			}
#if GPM_MOUSE
			else {
				Gpm_Connect conn;
				memset(&conn, 0, sizeof(conn));
				conn.eventMask  = GPM_DOWN | GPM_UP | GPM_DRAG;
				conn.defaultMask = GPM_MOVE; /* so that mouse cursor displayed */
				gpm_zerobased = 1;

				if (Gpm_Open(&conn, 0) < 0) {
					error("Cannot connect to mouse server\n");
					gpm_fd = -1; /* paranoia */
					return;
				}
			}
#endif
		}
	} else {
		if (is_real_xterm == 1)
			n = write(1, "\033[?1002l", 8);
		else if (is_real_xterm == 0)
			n = write(1, "\033[?1000l", 8);
#if GPM_MOUSE
		else if (gpm_fd	> 0)
			Gpm_Close();
#endif
	}

	if (n < 0) Dbg("Unable to set mouse mode.\n");
}


static int do_mouse(void)
{
	Byte button, col, row;

	tgetkb(); tgetkb(); tgetkb(); /* suck up \033[M */
	button = tgetkb();
	col = tgetkb() - 33; /* zero based */
	row = tgetkb() - 33; /* zero based */

	switch (button & 0x60) {
	case 0x20:
		/* button event */
		switch (button & 3) {
		case 0:
		case 1:
			mouse_point(row, col, false);
			break;
		case 2:
			mouse_point(row, col, true);
			break;
		case 3:
			/* release */
			break;
		}
		break;
	case 0x40:
		/* motion event */
		mouse_point(row, col, true);
		break;
	case 0x60:
		/* scroll wheel */
		mouse_scroll(row, button & 1);
		break;
	default:
		printf("Unknown event\n");
	}

	return TC_MOUSE;
}

#if GPM_MOUSE
static Gpm_Event event;
static int need_mouse_cursor;

void handle_gpm_mouse(void)
{
	Gpm_GetEvent(&event);
	
	switch (GPM_BARE_EVENTS(event.type)) {
	case GPM_DOWN:
		switch (event.buttons) {
		case 1: mouse_point(event.y, event.x, true); break;
		case 2:
		case 4: mouse_point(event.y, event.x, false); break;
		}
		break;
	case GPM_DRAG: mouse_point(event.y, event.x, true); break;
	case GPM_UP: break;
	case GPM_MOVE:
		switch (event.wdy) {
		case 0:  need_mouse_cursor = 1; break;
		case -1: mouse_scroll(event.y, true); break;
		case 1:  mouse_scroll(event.y, false); break;
		}
	}
}

void handle_mouse_cursor(void)
{
	if (need_mouse_cursor) {
		GPM_DRAWPOINTER(&event);
		need_mouse_cursor = 0;
	}
}
#endif

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
	cptr = (cptr - j) & (CSTACK - 1);
	cpushed += j;

	/* If it is an unknown CSI string, suck it up */
	if (cstack[(cptr + 2) & (CSTACK - 1)] == '[') {
		if (cstack[(cptr + 3) & (CSTACK - 1)] == 'M')
			return do_mouse();
		else
			cpushed = 2;
	}

	return tgetkb() & 0x7f;
}

int tgetcmd(void)
{
	int cmd = tgetkb() & 0x7f;

	/* All special keys start with ESC */
	if (cmd == '\033' && tkbrdy())
		return check_specials();

	return cmd;
}

static struct pollfd stdin_fd = { .fd = 0, .events = POLLIN };

bool tkbrdy(void)
{
	if (cpushed || Pending)
		return true;

	return Pending = poll(&stdin_fd, 1, 0) == 1;
}

bool tdelay(int ms)
{
	if (InPaw || cpushed || Pending)
		return false;

	return poll(&stdin_fd, 1, ms) != 1;
}
