/* doskbd.c - DOS keyboard input routines
 * Copyright (C) 2014 Sean MacLennan
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
#include <time.h>

int Cmdpushed = -1;
static int kb_hit;

static int alts[] = { /* 0x10 to 0x86 inclusive */
	M('Q'), M('W'), M('E'), M('R'), M('T'), M('Y'),
	M('U'), M('I'), M('O'), M('P'), M('['), M(']'), 0, 0,
	M('A'), M('S'), M('D'), M('F'), M('G'), M('H'),
	M('J'), M('K'), M('L'), M(';'), M('\''), 0, 0, 0,
	M('Z'), M('X'), M('C'), M('V'), M('B'), M('N'),
	M('M'), M(','), M('.'), '/', 0, 0, 0, 0, 0,
	TC_F1, TC_F2, TC_F3, TC_F4, TC_F5, TC_F6, TC_F7, TC_F8, TC_F9, TC_F10,
	0, 0, TC_HOME, TC_UP, TC_PGUP, 0,
	TC_LEFT, 0, TC_RIGHT, 0, TC_END,
	TC_DOWN, TC_PGDOWN, TC_INSERT, TC_DELETE,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 54 to 5F */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /* 60 to 6F */
	0, 0, 0, 0, 0, TC_C_END, 0, TC_C_HOME,
	M('!'), M('@'), M('#'), M('$'), M('%'), M('^'), M('&'), M('*'),
	M('('), M(')'), M('-'), M('+'), 0, TC_F11, TC_F12,
};

int tgetcmd(void)
{
	extern int ring_bell;
	int cmd;

	if (Cmdpushed >= 0) {
		cmd = Cmdpushed;
		Cmdpushed = -1;
		return cmd;
	}

	cmd = getch() & 0x7f;
	kb_hit = 0;
	if (ring_bell) {
		ring_bell = 0;
		Curwdo->modeflags = INVALID;
	}
	if (cmd == 0) {
		cmd = getch();
		if (cmd >= 0x10 && cmd <= 0x86)
			return alts[cmd - 0x10];
		return TC_UNKNOWN;
	}

	/* Unfortunately DOS uses C-H for backspace */
	if (cmd == 8 && Keys[Cmd] != ZCTRL_X) cmd = 127;

	return cmd;
}

bool tkbrdy(void)
{
	/* Once kbhit returns true you cannot call it again
	 * without calling getch or it will block. */
	if (!kb_hit)
		kb_hit = kbhit();

	return kb_hit;
}

bool tdelay(int ms)
{
#if 0
	/* This works. Except if you hit C-C during a clock call then the C-C
	 * will abort Zedit. The C-C interrupt handler doesn't seem to help.
	 * The delay call has the same problem.
	 */
	clock_t end;

	if (InPaw)
		return false;

	end = clock() + (clock_t)(ms / 55); /* clock in 55ms increments */
	while (!tkbrdy() && clock() <= end) ;
	return !tkbrdy();
#else
	((void)ms);

	if (InPaw)
		return false;
	else
		return !tkbrdy();
#endif
}
