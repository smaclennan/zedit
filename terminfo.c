/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

#if TERMINFO
#undef TRUE
#undef FALSE
#undef SYSV2
#undef ISPRINT
#include <curses.h>
#undef ISPRINT		/* guarentee we don't get the wrong one! */
#include <term.h>
#include "keys.h"


struct key_array Tkeys[] =
{
	{ NULL,		"kcuu1" },
	{ NULL,		"kcud1" },
	{ NULL,		"kcub1" },
	{ NULL,		"kcuf1" },
	{ NULL,		"khome" },
	{ NULL,		"kbs" },
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
	{ NULL,		"kend" },
	{ NULL,		"knp" },
	{ NULL,		"kpp" },
	{ NULL,		"kich1" },
	{ NULL,		"kdch1" },

	/* Hack the ctrl versions since they are not in the terminfo */
	{ "\033Oa",	"C-up" },
	{ "\033Ob",	"C-down" },
	{ "\033Oc",	"C-right" },
	{ "\033Od",	"C-left" },
	{ "\033[7^",	"C-home" },
	{ "\033[8^",	"C-end" },
};
#define N_KEYS (sizeof(Tkeys) / sizeof(struct key_array))


void TIinit()
{
	char *n;
	int rc, i;

#if DBG
	if (N_KEYS != NUMKEYS - TC_UP) {
		printf("Mismatch N_KEYS %d NUMKEYS %d\n", N_KEYS, NUMKEYS - TC_UP);
		exit(1);
	}
#endif

	if((Term = getenv("TERM")) == NULL) {
		printf("FATAL ERROR: environment variable TERM not set.\n");
		exit(1);
	}

#ifdef __linux__
	/* The key bindings are broken for the Linux xterm entry. */
	if (strcmp(Term, "xterm") == 0)
		Term = "linux";
#endif

	setupterm(Term, 1, &rc);
	if(rc != 1)
	{
		printf("FATAL ERROR: Unable to get terminfo entry for %s.\n", Term);
		exit(1);
	}
	if(!clear_screen || !clr_eol || !cursor_address)
	{
		printf("FATAL ERROR: Terminfo entry for %s incomplete.\n",  Term);
		exit(1);
	}

	/* setup missing modes */
	if((n = enter_reverse_mode) == 0 && (n = enter_standout_mode) == 0)
		n = enter_bold_mode;
	if(!enter_reverse_mode)		enter_reverse_mode = n;
	if(!enter_standout_mode)	enter_standout_mode = n;
	if(!exit_attribute_mode &&
	  (exit_attribute_mode = exit_standout_mode) == 0)
			enter_reverse_mode = enter_standout_mode = enter_bold_mode = 0;

	if (!flash_screen)
		Vars[VVISBELL].val = 0;

	/* initialize the terminal */
	TPUTS(init_1string);
	TPUTS(init_2string);
	TPUTS(init_3string);

	Tkeys[ 0].key = key_up;
	Tkeys[ 1].key = key_down;
	Tkeys[ 2].key = key_right;
	Tkeys[ 3].key = key_left;
	Tkeys[ 4].key = key_home;
	Tkeys[ 5].key = key_backspace;
	Tkeys[ 6].key = key_f0;
	Tkeys[ 7].key = key_f1;
	Tkeys[ 8].key = key_f2;
	Tkeys[ 9].key = key_f3;
	Tkeys[10].key = key_f4;
	Tkeys[11].key = key_f5;
	Tkeys[12].key = key_f6;
	Tkeys[13].key = key_f7;
	Tkeys[14].key = key_f8;
	Tkeys[15].key = key_f9;
	Tkeys[16].key = key_f10;
	Tkeys[17].key = key_f11;
	Tkeys[18].key = key_end;
	Tkeys[19].key = key_npage;
	Tkeys[20].key = key_ppage;
	Tkeys[21].key = key_ic;
	Tkeys[22].key = key_dc;

	for(i = 0; i <= 24; ++i)
		if(Tkeys[i].key && *Tkeys[i].key)
			Key_mask |= 1 << i;
}


void Tsize(rows, cols)
int *rows, *cols;
{
#if HAS_RESIZE
	FILE *fp;
	char buf[1024], name[STRMAX + 1];
	int n;
#endif

	/* default to values in terminfo file */
	*rows = lines;
	*cols = columns;

#if HAS_RESIZE
	if((fp = popen("resize -u", "r")))
	{
		while(fgets(buf, sizeof(buf), fp))
			if(sscanf(buf, "%[^=]=%d;\n", name, &n) == 2) {
				if(strcmp(name, "COLUMNS") == 0)
					*cols = n;
				else if(strcmp(name, "LINES") == 0)
					*rows = n;
			}
		pclose(fp);
	}
#endif
}


void TIfini()
{
#if COMMENTBOLD
	Tstyle(T_NORMAL);
#endif
#if !LINUX
  /* SAM This fails under linux... */
	TPUTS(reset_1string);
	resetterm();
#endif
}


void Tstyle(style)
int style;
{
	static int cur_style = -1;

	if(style == cur_style) return;

	switch(cur_style = style)
	{
#if COMMENTBOLD
	case T_NORMAL:
		TPUTS(exit_attribute_mode);
// SAM		TPUTS(tparm(set_a_foreground, COLOR_WHITE));
				TPUTS(tparm(set_a_foreground, COLOR_BLACK));
		break;
	case T_COMMENT:
		TPUTS(tparm(set_a_foreground, COLOR_RED));
		break;
	case T_CPP:
		TPUTS(tparm(set_a_foreground, COLOR_GREEN));
		break;
	case T_CPPIF:
		TPUTS(tparm(set_a_foreground, COLOR_MAGENTA));
		break;
#else
	case T_NORMAL:		TPUTS(exit_attribute_mode); break;
#endif
	case T_STANDOUT:	TPUTS(enter_standout_mode);	break;
	case T_REVERSE:		TPUTS(enter_reverse_mode);	break;
	case T_BOLD:		TPUTS(enter_bold_mode);	break;
	}
	fflush(stdout);
}

#endif
