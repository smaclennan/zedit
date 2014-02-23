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

#if 0
static int alts[] = {
	/* 10 */ 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0, 0,
	/* 1E */ 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', 0, 0,
	/* 2C */ 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/'
	/* 36 */ };
#endif

int tgetcmd(void)
{
	int cmd;

	if (Cmdpushed >= 0) {
		cmd = Cmdpushed;
		Cmdpushed = -1;
		return cmd;
	}

	kb_hit = 0;
	cmd = getch() & 0x7f;
	if (cmd == 0) {
		cmd = getch();
		if (cmd >= 0x3b && cmd <= 0x44)
			return TC_F1 + cmd - 0x3B;
		switch (cmd) {
		case 0x47: return TC_HOME;
		case 0x48: return TC_UP;
		case 0x49: return TC_PGUP;
		case 0x4B: return TC_LEFT;
		case 0x4D: return TC_RIGHT;
		case 0x4F: return TC_END;
		case 0x50: return TC_DOWN;
		case 0x51: return TC_PGDOWN;
		case 0x52: return TC_INSERT;
		case 0x53: return TC_DELETE;
		case 0x85: return TC_F11;
		case 0x86: return TC_F12;
		default:   return TC_UNKNOWN;
		}
	}

	/* Unfortunately DOS uses C-H for backspace */
	if (cmd == 8) cmd = 127;

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
	if (InPaw)
		return false;
	else
		return !tkbrdy();
#endif
}
