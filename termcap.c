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

#if defined(TERMCAP) || defined(TERMCAP_KEYS)
#include "zterm.h"

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

static char bp[1024];
static char area[1024];

static char *termcap_keys(void)
{
	int i;
	char *key, *end = area;

	char *term = getenv("TERM");
	if (term == NULL) {
		printf("ERROR: environment variable TERM not set.\n");
		return NULL;
	}
	if (tgetent(bp, term) != 1) {
		printf("ERROR: Unable to get termcap entry for %s.\n", term);
		NULL;
	}

	/* get the cursor and function key defines */
	for (i = 0; i < 22; ++i) {
		key = end;
		tgetstr(key_names[i], &end);
		if (key != end) {
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

	return end;
}

#ifdef TERMCAP
#define NUMCM	7
#define MUST	3
char *cm[NUMCM];

void tlinit(void)
{
	static char *names[] = { "cm", "ce", "cl", "me", "so", "vb", "md" };
	int i;

	char *end = termcap_keys();
	if (end == NULL)
		exit(1);

	/* get the initialziation string and send to stdout */
	char *was = end;
	tgetstr("is", &end);
	if (end != was)
		TPUTS(was);

	/* get the termcap strings needed - must be done last */
	for (i = 0; i < NUMCM; ++i) {
		cm[i] = end;
		tgetstr(names[i], &end);
		if (cm[i] == end) {
			if (i < MUST || verbose)
				printf("Missing termcap entry for %s\n", names[i]);
			if (i < MUST)
				exit(1);
			else
				cm[i] = "";
		}
	}
}
#else
void tlinit(void) { termcap_keys(); }
#endif

void tlfini(void) {}
#endif /* TERMCAP || TERMCAP_KEYS */
