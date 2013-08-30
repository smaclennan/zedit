/* ansi.c - low level ansi terminal interface
 * Copyright (C) 1988-2010 Sean MacLennan
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

#ifdef ANSI
#include "keys.h"

struct key_array Tkeys[] = {
	{ "\033[A",	"up" },
	{ "\033[B",	"down" },
	{ "\033[C",	"right" },
	{ "\033[D",	"left" },
	{ "\033[7~",	"home" },
	{ NULL,		"back" },
	{ "\033[11~",	"f1" },
	{ "\033[12~",	"f2" },
	{ "\033[13~",	"f3" },
	{ "\033[14~",	"f4" },
	{ "\033[15~",	"f5" },
#ifdef ANSI_KEY_HACK
	/* I do not know if this is a Linux bug or we are supposed to
	 * skip 16 and 22 */
	{ "\033[17~",	"f6" },
	{ "\033[18~",	"f7" },
	{ "\033[19~",	"f8" },
	{ "\033[20~",	"f9" },
	{ "\033[21~",	"f10" },
	{ "\033[23~",	"f11" },
	{ "\033[24~",	"f12" },
#else
	{ "\033[16~",	"f6" },
	{ "\033[17~",	"f7" },
	{ "\033[18~",	"f8" },
	{ "\033[19~",	"f9" },
	{ "\033[20~",	"f10" },
	{ "\033[21~",	"f11" },
	{ "\033[22~",	"f12" },
#endif
	{ "\033[8~",	"end" },
	{ "\033[6~",	"page down" },
	{ "\033[5~",	"page up" },
	{ "\033[2~",	"insert" },
	{ "\033[3~",	"delete" },

	{ "\033Oa",	"C-up" },
	{ "\033Ob",	"C-down" },
	{ "\033Oc",	"C-right" },
	{ "\033Od",	"C-left" },
	{ "\033[7^",	"C-home" },
	{ "\033[8^",	"C-end" },
};
#define N_KEYS ((int)(sizeof(Tkeys) / sizeof(struct key_array)))


void tlinit(void)
{
	int i;

#if DBG
	if (N_KEYS != NUMKEYS - TC_UP) {
		printf("Mismatch N_KEYS %d NUMKEYS %d\n",
		       N_KEYS, NUMKEYS - TC_UP);
		exit(1);
	}
#endif

	for (i = 0; i < N_KEYS; ++i)
		if (Tkeys[i].key)
			Key_mask |= 1 << i;
}

void tsize(int *rows, int *cols)
{	/* Let termsize default it */
	*rows = *cols = 0;
}

void tlfini(void)
{
#if COMMENTBOLD
	tstyle(T_NORMAL);
#endif
}


void tstyle(int style)
{
	static int cur_style = -1;

	if (style == cur_style)
		return;

	switch (cur_style = style) {
#if COMMENTBOLD
	case T_NORMAL:
		TPUTS("\033[0;30m"); break; /* black */
	case T_COMMENT:
		TPUTS("\033[31m"); break; /* red */
#else
	case T_NORMAL:
		TPUTS("\033[0m"); break;
#endif
	case T_STANDOUT:
		TPUTS("\033[7m"); break;
	case T_REVERSE:
		TPUTS("\033[7m"); break;
	case T_BOLD:
		TPUTS("\033[1m"); break;
	}
	fflush(stdout);
}
#endif
