/* fcheck.c - Zedit helper program
 * Copyright (C) 1988-2018 Sean MacLennan
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

#include "varray.c"
#include "funcs.c"
#include "cnames.c"
#include "bind.c"
#include "buff/kbd.c"

/* Need to include this after buff/kbd.c for -std=c11 */
#include <stdio.h>

#define VARSNUM		((int)(sizeof(Vars) / sizeof(struct avar)))

#define N_KEYS ((int)(sizeof(Tkeys) / sizeof(char *)))

void Dbg(const char *fmt, ...) {}

int main(int argc, char *argv[])
{
	int i, s1, s2, err = 0;
	Byte array[97];

	if (NUMVARS != VARSNUM) {
		printf("Mismatch in NUMVARS and VARNUM %d:%d\n",
			   NUMVARS, VARSNUM);
		err = 1;
	}

	if (N_KEYS != NUM_SPECIAL) {
		printf("Mismatch N_KEYS %d NUM_SPECIAL %d\n",
			   N_KEYS, NUM_SPECIAL);
		err = 1;
	}

	if (NUM_SPECIAL > 32) {
		printf("Too many special keys\n");
		err = 1;
	} else {
		unsigned long mask = 0;

		for (i = 0; i < NUM_SPECIAL; ++i)
			mask = (mask << 1) | 1;
		if (mask != KEY_MASK) {
			printf("Mismatch mask %08lx and %08lx\n",
				   mask, (unsigned long)KEY_MASK);
			err = 1;
		}
	}

	/* Spot check keys array */
	if (sizeof(Keys) != NUMKEYS) {
		printf("Problems with Keys array\n");
		err = 1;
	} else {
		memset(array, ZINSERT, sizeof(array));
		array[0] = ZUNDO;
		array[96] = ZDELETE_PREVIOUS_CHAR;
		if (memcmp(array, Keys + 31, sizeof(array))) {
			printf("Problems with Keys array 1\n");
			err = 1;
		}

		if (Keys[CX('1')] != ZONE_WINDOW ||
			Keys[CX('=')] != ZPOSITION ||
			Keys[CX('O')] != ZNEXT_WINDOW ||
			Keys[CX('Z')] != ZZAP_TO_CHAR ||
			Keys[CX(127)] != ZDELETE_PREVIOUS_WORD) {
			printf("Problems with Keys array 2\n");
			err = 1;
		}

		if (Keys[M('A')] != ZAGAIN ||
			Keys[M('Z')] != ZSAVE_AND_EXIT ||
			Keys[TC_UP] != ZPREVIOUS_LINE ||
			Keys[TC_F12] != ZREVERT_FILE ||
			Keys[M(127)] != ZDELETE_PREVIOUS_WORD) {
			printf("Problems with Keys array 3\n");
			err = 1;
		}
	}

	/* check sizes of various stuff */
	if (NUMFUNCS >= 256) {
		printf("Cnames[].fnum is a byte. Too many functions.\n");
		err = 1;
	}
	s1 = sizeof(Cnames) / sizeof(struct cnames);
	s2 = (sizeof(Cmds) / sizeof(void *) / 2) - 1;
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

	/* getplete structs must be aligned */
	if (sizeof(struct cnames) % sizeof(char *)) {
		printf("struct cnames not aligned [%d/%d]\n",
			   (int)sizeof(struct cnames), (int)sizeof(char *));
		err = 1;
	}
	if (sizeof(struct avar) % sizeof(char *)) {
		printf("struct avar not aligned [%d/%d]\n",
			   (int)sizeof(struct avar), (int)sizeof(char *));
		err = 1;
	}

	{   /* verify the __MRKSIZE */
		struct mark dummy_mrk, *dummy = &dummy_mrk;
		struct mark mrk1 = { NULL, NULL, 0x1234, NULL, NULL };
		struct mark mrk2;

		memset(&mrk2, 0xea, sizeof(struct mark));
		mrk2.prev = mrk2.next = dummy;

		mrktomrk(&mrk2, &mrk1);
		if (mrk2.moffset != 0x1234 || mrk2.prev != dummy) {
			printf("Problems with __MRKSIZE\n");
			err = 1;
		}
	}

	free(NULL); /* paranoia */

	return err;
}
