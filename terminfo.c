/* terminfo.c - terminfo specific routines
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

#if TERMINFO
#include "zterminfo.h"
#include "keys.h"


struct key_array Tkeys[] = {
	{ NULL,		"kcuu1" },
	{ NULL,		"kcud1" },
	{ NULL,		"kcub1" },
	{ NULL,		"kcuf1" },

	{ NULL,		"kich1" },
	{ NULL,		"kdch1" },
	{ NULL,		"kpp" },
	{ NULL,		"knp" },
	{ NULL,		"khome" },
	{ NULL,		"kend" },

	{ NULL,		"kf0" },
	{ NULL,		"kf1" },
	{ NULL,		"kf2" },
	{ NULL,		"kf3" },
	{ NULL,		"kf4" },
	{ NULL,		"kf5" },
	{ NULL,		"kf6" },
	{ NULL,		"kf7" },
	{ NULL,		"kf8" },
	{ NULL,		"kf9" },
	{ NULL,		"kf10" },
	{ NULL,		"kf11" },

	/* Hack the ctrl versions since they are not in the terminfo */
	{ "\033Oa",	"C-up" },
	{ "\033Ob",	"C-down" },
	{ "\033Oc",	"C-right" },
	{ "\033Od",	"C-left" },
	{ "\033[7^",	"C-home" },
	{ "\033[8^",	"C-end" },

	{ NULL,		"kbs" }
};
#define N_KEYS (sizeof(Tkeys) / sizeof(struct key_array))

static char *Term;

void tlinit()
{
	int rc, i;

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
		enter_reverse_mode = enter_standout_mode = enter_bold_mode = 0;

	if (!flash_screen)
		VAR(VVISBELL) = 0;

	/* initialize the terminal */
	TPUTS(init_1string);
	TPUTS(init_2string);
	TPUTS(init_3string);

	Tkeys[0].key = key_up;
	Tkeys[1].key = key_down;
	Tkeys[2].key = key_right;
	Tkeys[3].key = key_left;

	Tkeys[4].key = key_ic;
	Tkeys[5].key = key_dc;
	Tkeys[6].key = key_ppage;
	Tkeys[7].key = key_npage;
	Tkeys[8].key = key_home;
	Tkeys[9].key = key_end;

	Tkeys[10].key = key_f0;
	Tkeys[11].key = key_f1;
	Tkeys[12].key = key_f2;
	Tkeys[13].key = key_f3;
	Tkeys[14].key = key_f4;
	Tkeys[15].key = key_f5;
	Tkeys[16].key = key_f6;
	Tkeys[17].key = key_f7;
	Tkeys[18].key = key_f8;
	Tkeys[19].key = key_f9;
	Tkeys[20].key = key_f10;
	Tkeys[21].key = key_f11;

	Tkeys[28].key = key_backspace;

	for (i = 0; i < N_KEYS; ++i)
		if (Tkeys[i].key && *Tkeys[i].key)
			Key_mask |= 1 << i;
}

void tsize(int *rows, int *cols)
{	/* default to values in terminfo file */
	*rows = lines;
	*cols = columns;
}

void tlfini()
{
	TPUTS(reset_1string);
	resetterm();
}

void tstyle(int style)
{
	static int cur_style = -1;

	if (style == cur_style)
		return;

	cur_style = style;
	switch (cur_style) {
	case T_NORMAL:
		TPUTS(exit_attribute_mode);
		break;
	case T_COMMENT:
		TPUTS(tparm(set_a_foreground, COLOR_RED));
		break;
	case T_STANDOUT:
		TPUTS(enter_standout_mode);
		break;
	case T_REVERSE:
		TPUTS(enter_reverse_mode);
		break;
	case T_BOLD:
		TPUTS(enter_bold_mode);
		break;
	}
	fflush(stdout);
}

void tbell(void)
{
	if (VAR(VVISBELL))
		TPUTS(flash_screen);
	else if (VAR(VSILENT) == 0)
		putchar('\7');
}

/* for tputs this must be a function */
#ifdef TERMIOS
int _putchar(int ch)
#else
int _putchar(char ch)
#endif
{
	putchar(ch);
	return 0;	/*shutup*/
}
#endif
