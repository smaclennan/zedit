/* ansi.c - low level ansi terminal interface
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

#if ANSI
#include <termios.h>
#include "keys.h"

struct key_array Tkeys[] = {
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


void tlinit(void) { Key_mask = 0xfffffff; }

void tlfini(void) {}

void tsize(int *rows, int *cols)
{
	char buf[12];
	int n, w;

	*rows = *cols = 0;

	/* Save cursor position */
	w = write(0, "\033[s", 3);
	/* Send the cursor to the extreme right corner */
	w += write(0, "\033[999;999H", 10);
	/* Ask where we really ended up */
	w += write(0, "\033[6n", 4);
	n = read(0, buf, sizeof(buf) - 1);
	/* Restore cursor */
	w += write(0, "\033[u", 3);

	if (n > 0) {
		buf[n] = '\0';
		sscanf(buf, "\033[%d;%dR", rows, cols);
	}
}

void tstyle(int style)
{
	static int cur_style = -1;

	if (style == cur_style)
		return;

	switch (cur_style = style) {
	case T_NORMAL:
		fputs("\033[0m", stdout); break;
	case T_STANDOUT:
	case T_REVERSE:
		fputs("\033[7m", stdout); break;
	case T_BOLD:
		fputs("\033[1m", stdout); break;
	case T_COMMENT:
		fputs("\033[31m", stdout); break; /* red */
	}
	fflush(stdout);
}

void tbell(void)
{
	if (VAR(VVISBELL)) {
		fputs("\033[?5h", stdout);
		fflush(stdout);
		usleep(100000);
		fputs("\033[?5l", stdout);
	} else if (VAR(VSILENT) == 0)
		putchar('\7');
}
#endif
