/* termcap.c - termcap specific routines
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

#if TERMCAP

#include "keys.h"
#include <termcap.h>

static char *Term;

#define NUMCM	7
#define MUST	3
char *cm[NUMCM];

struct key_array Tkeys[] = {
	{ NULL,		"ku" },
	{ NULL,		"kd" },
	{ NULL,		"kr" },
	{ NULL,		"kl" },

	{ NULL,		"kI" },
	{ NULL,		"kD" },
	{ NULL,		"kP" },
	{ NULL,		"kN" },
	{ NULL,		"kh" },
	{ NULL,		"@7" },

	{ NULL,		"k1" },
	{ NULL,		"k2" },
	{ NULL,		"k3" },
	{ NULL,		"k4" },
	{ NULL,		"k5" },
	{ NULL,		"k6" },
	{ NULL,		"k7" },
	{ NULL,		"k8" },
	{ NULL,		"k9" },
	{ NULL,		"k;" },
	{ NULL,		"F1" },
	{ NULL,		"F2" },

	/* Hack the ctrl versions since they are not in termcap */
	{ "\033Oa",	"C-up" },
	{ "\033Ob",	"C-down" },
	{ "\033Oc",	"C-right" },
	{ "\033Od",	"C-left" },
	{ "\033[7^",	"C-home" },
	{ "\033[8^",	"C-end" },
};
#define N_KEYS (sizeof(Tkeys) / sizeof(struct key_array))

static char bp[1024];

void tlinit()
{
	/* NOTE: so and se must be last */
	static char *names[] = { "cm", "ce", "cl", "so", "se", "so", "vb" };
	static char area[1024];
	char *end;
	int i, j;

	Term = getenv("TERM");
	if (Term == NULL) {
		printf("ERROR: environment variable TERM not set.\n");
		exit(1);
	}
	if (tgetent(bp, Term) != 1) {
		printf("ERROR: Unable to get termcap entry for %s.\n", Term);
		exit(1);
	}

	/* get the initialziation string and send to stdout */
	end = area;
	tgetstr("is", &end);
	if (end != area)
		TPUTS(area);

	/* get the termcap strings needed - must be done last */
	end = area;
	for (i = 0; i < NUMCM; ++i) {
		cm[i] = end;
		tgetstr(names[i], &end);
		if (cm[i] == end) {
			if (i < MUST) {
				printf("Missing termcap entry for %s\n",
				       names[i]);
				exit(1);
			} else
				cm[i] = "";
		}
	}

	/* get the cursor and function key defines */
	Key_mask = 0;
	for (i = j = 0; i < NUM_SPECIAL; ++i)
		if (Tkeys[i].key)
			Key_mask |= 1 << i;
		else {
			Tkeys[i].key = end;
			tgetstr(Tkeys[i].label, &end);
			if (Tkeys[i].key != end) {
				Key_mask |= 1 << i;
				if (*Tkeys[i].key != '\033')
					Key_shortcut = 0;
			}
		}

	/* HACK */
	i = TC_F10 - SPECIAL_START;
	if ((Key_mask & (1 << i)) == 0) {
		Tkeys[i].key = end;
		tgetstr("k0", &end);
		if (Tkeys[i].key != end) {
			Key_mask |= 1 << i;
			Tkeys[i].label = "k0";
		}
	}
}

void tlfini() {}


void tsize(int *rows, int *cols)
{
	tgetent(bp, Term);
	*rows = tgetnum("li");
	*cols = tgetnum("co");
}

void tstyle(int style)
{
	static int cur_style = -1;

	if (style == cur_style)
		return;

	if (style < NUMCM) {
		cur_style = style;
		TPUTS(cm[style]);
	}
}

void tbell(void)
{
	if (VAR(VVISBELL) && *cm[6])
		TPUTS(cm[6]);
	else if (VAR(VSILENT) == 0)
		putchar('\7');
}
#endif
