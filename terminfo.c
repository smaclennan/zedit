/* terminfo.c - terminfo specific routines
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

#ifdef TERMINFO
static char *Term;

void tlinit(void)
{
	int rc, i, k;

	Term = getenv("TERM");
	if (Term == NULL) {
		printf("FATAL ERROR: environment variable TERM not set.\n");
		exit(1);
	}

	setupterm(Term, 1, &rc);
	if (rc != 1) {
		printf("FATAL ERROR: Unable to get terminfo entry for %s.\n",
		       Term);
		exit(1);
	}
	if (!clear_screen || !clr_eol || !cursor_address) {
		printf("FATAL ERROR: Terminfo entry for %s incomplete.\n",
		       Term);
		exit(1);
	}

	if (!exit_attribute_mode)
		enter_reverse_mode = enter_standout_mode = enter_bold_mode = NULL;

	/* initialize the terminal */
	TPUTS(init_1string);
	TPUTS(init_2string);
	TPUTS(init_3string);

	Tkeys[0] = key_up;
	Tkeys[1] = key_down;
	Tkeys[2] = key_right;
	Tkeys[3] = key_left;

	Tkeys[4] = key_ic;
	Tkeys[5] = key_dc;
	Tkeys[6] = key_ppage;
	Tkeys[7] = key_npage;
	Tkeys[8] = key_home;
	Tkeys[9] = key_end;

	if (key_f0) { /* old school */
		Tkeys[10] = key_f0;
		Tkeys[11] = key_f1;
		i = 12;
	} else {
		Tkeys[10] = key_f1;
		Tkeys[21] = key_f12;
		i = 11;
	}
	Tkeys[i++] = key_f2;
	Tkeys[i++] = key_f3;
	Tkeys[i++] = key_f4;
	Tkeys[i++] = key_f5;
	Tkeys[i++] = key_f6;
	Tkeys[i++] = key_f7;
	Tkeys[i++] = key_f8;
	Tkeys[i++] = key_f9;
	Tkeys[i++] = key_f10;
	Tkeys[i++] = key_f11;

	Key_mask = 0x00c00000; /* C-Home and C-End not in terminfo */
	for (k = 0; k < i; ++k)
		if (Tkeys[k] && *Tkeys[k])
			Key_mask |= 1 << k;

	if (verbose) {
		for (k = 0; k < i; ++k)
			dump_key(k, Tkeys[k], NULL);

		Dbg("Key Mask %x\n", Key_mask);
	}
}

void tlfini(void)
{
	TPUTS(reset_1string);
	resetterm();
}
#endif
