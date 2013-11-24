/* fcheck.c - Zedit helper program
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

#define FCHECK
#include "z.h"
#include "vars-array.h"

#include "funcs.c"
#include "kbd.c"

#define VARSNUM		((int)(sizeof(Vars) / sizeof(struct avar)))

int InPaw;

void hang_up(int sig) {}

void Dbg(char *fmt, ...) {}

int main(int argc, char *argv[])
{
	int i, err = 0;

	if (NUMVARS != VARSNUM) {
		printf("Mismatch in NUMVARS and VARNUM %d:%d\n",
		       NUMVARS, VARSNUM);
		err = 1;
	}

	if (N_KEYS != NUM_SPECIAL) {
		printf("Mismatch N_KEYS %d NUMKEYS %d\n",
		       N_KEYS, NUM_SPECIAL);
		err = 1;
	}
	if (NUM_SPECIAL > 32) {
		printf("Too many special keys\n");
		err = 1;
	} else {
		unsigned mask = 0;

		for (i = 0; i < NUM_SPECIAL; ++i)
			mask = (mask << 1) | 1;
		if (mask != Key_mask) {
			printf("Mismatch mask %08x and %08;x\n", mask, Key_mask);
			err = 1;
		}
	}

	/* check sizes of various stuff */
	int s1 = sizeof(Cnames) / sizeof(struct cnames);
	int s2 = (sizeof(Cmds) / sizeof(void *) / 2) - 1;
	if (s1 != NUMFUNCS || s2 != NUMFUNCS) {
		printf("Cnames: %d Cmds: %d NUMFUNCS: %d\n", s1, s2, NUMFUNCS);
		exit(1); /* stop since the loop below might segfault */
	}

	/* validate the Cnames array the best we can */
	for (s1 = 1; s1 < NUMFUNCS; ++s1) {
		if (strcasecmp(Cnames[s1].name, Cnames[s1 - 1].name) <= 0) {
			printf("Problem: (%d) %s and %s\n",
			    s1, Cnames[s1 - 1].name, Cnames[s1].name);
			err = 1;
		}
		if (strlen(Cnames[s1].name) > (size_t)30) {
			printf("%s too long\n", Cnames[s1].name);
			err = 1;
		}
	}

	/* validate the Vars array */
	for (s1 = 1; s1 < NUMVARS; ++s1)
		if (strcasecmp(Vars[s1].vname, Vars[s1 - 1].vname) <= 0) {
			printf("Problem: (%d) %s and %s\n",
			    s1, Vars[s1 - 1].vname, Vars[s1].vname);
			err = 1;
		}

	return err;
}
