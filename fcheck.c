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

#include "z.h"
#include "cnames.h"
#include "vars-array.h"


int main(int argc, char *argv[])
{
	int err = 0;

	if (NUMVARS != VARNUM) {
		/* haven't setup term stuff yet */
		printf("Mismatch in NUMVARS and VARNUM %d:%d\n",
		       NUMVARS, VARNUM);
		err = 1;
	}

	/* check sizes of various stuff */
	int s1 = sizeof(Cnames) / sizeof(struct cnames);
	if (s1 != NUMFUNCS) {
		printf("Cnames: %d NUMFUNCS: %d\n", s1, NUMFUNCS);
		exit(1); /* stop since the loop below might segfault */
	}

	/* validate the Cnames array the best we can */
	for (s1 = 1; s1 < NUMFUNCS; ++s1) {
		if (strcasecmp(Cnames[s1].name,
			       Cnames[s1 - 1].name) <= 0) {
			printf("Problem: (%d) %s and %s\n",
			    s1, Cnames[s1 - 1].name, Cnames[s1].name);
			err = 1;
		}
		if (strlen(Cnames[s1].name) > (size_t)30) {
			printf("%s too long\n", Cnames[s1].name);
			err = 1;
		}
		if (strncmp(Cnames[s1].name, "Top", 3) == 0) {
			printf("Zhelp() Top: %s\n", Cnames[s1].name);
			err = 1;
		}
	}

	return err;
}
