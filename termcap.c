/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"
#include "keys.h"

#if TERMCAP

#include <termcap.h>

char *Term;

#define NUMCM	6
#define MUST	NUMCM - 3	/* all but so's and se */
char *cm[ NUMCM ];		/* Get from termcap */

struct key_array Tkeys[] =
{
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

	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },
	{ NULL },

	{ NULL,		"kb" },
};
#define N_KEYS (sizeof(Tkeys) / sizeof(struct key_array))

static char bp[ 1024 ];
static int SGnum; // SAM?

void tlinit()
{
	/* NOTE: so and se must be last */
	static char *names[] = { "cm", "ce", "cl", "so", "se", "so" };
	static char area[ 1024 ];
	char *end;
	int i, j;

	if((Term = getenv("TERM")) == NULL)
	{
		printf("FATAL ERROR: environment variable TERM not set.\n");
		exit(1);
	}
	if(tgetent(bp, Term) != 1)
	{
		printf("FATAL ERROR: Unable to get termcap entry for %s.\n", Term);
		exit(1);
	}

	/* get the initialziation string and send to stdout */
	end = area;
	tgetstr( "is", &end );
	if( end != area ) TPUTS( area );

	/* get the termcap strings needed - must be done last */
	end = area;
	for( i = 0; i < NUMCM; ++i )
	{
		cm[ i ] = end;
		tgetstr( names[i], &end );
		if( cm[i] == end )
		{
			if( i < MUST )
			{
				printf( "Missing termcap entry for %s\n", names[i] );
				exit( 1 );
			}
			else
				cm[ i ] = "";
		}
	}

	/* get the cursor and function key defines */
	Key_mask = 0;
	for( i = j = 0; i < NUMKEYS - SPECIAL_START; ++i )
		if (Tkeys[i].label) {
			Tkeys[i].key = end;
			tgetstr( Tkeys[i].label, &end );
			if( Tkeys[i].key != end )
				Key_mask |= 1 << i;
		}

	/* look for an sg - if positive, no reverse allowed on mark */
	if( (SGnum = tgetnum("sg")) == -1 ) SGnum = 0;
	if( SGnum > 0 ) cm[ T_REVERSE ] = "";
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

	if(style < NUMCM) {
		cur_style = style;
		TPUTS(cm[style]);
	}
}

void tbell(void)
{
	if (VAR(VSILENT) == 0)
		putchar('\7');
}
#endif
