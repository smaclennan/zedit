/****************************************************************************
 *																			*
 *				 The software found in this file is the						*
 *					  Copyright of Sean MacLennan							*
 *						  All rights reserved.								*
 *																			*
 ****************************************************************************/
#include "z.h"

/* This files contains PC and MSDOS specific functions */

#ifdef PC
/* These routines are for fast direct memory mapped screen access on the PC.
 * Note that the ANSI driver can be used on the PC.
 */

Short *Lptr[ROWMAX + 1];		/* Pointers to video ram lines */
Byte FGcolor = 7, BGcolor = 0;	/* Attribute variables */
extern Byte Attr;


void PCinit()
{
	extern Byte Vtype;
	extern unsigned Savecursor;
	int i;
	unsigned *vseg;

	/* Get the video type and setup the line pointer array */
	Getvtype();
	vseg = Vtype == MONO ? (unsigned *)0xb0000000 : (unsigned *)0xb8000000;
	for(i = 0; i < ROWMAX; ++i) Lptr[i] = vseg + (i * 80);

	/* set the default colors */
	Setcolors(Vars[VCOLORS].val);
}


void Tstyle(style)
int style;
{
	switch(style)
	{
		case T_BOLD:	if(Vars[VBOLDLINE].val == 0) Attr |= 8; break;
		case T_STANDOUT:
		case T_REVERSE:	Attr = (FGcolor << 4) | BGcolor; break;
		case T_NORMAL:	Attr = (BGcolor << 4) | FGcolor; break;
	}
}


void Setcolors(colors)
unsigned colors;
{
	extern Byte Vtype;

	if(Vtype != MONO)
	{
		FGcolor = (colors / 10) & 7;
		BGcolor = (colors % 10) & 7;
	}
	Tstyle(T_NORMAL);		/* Set Attr */
}


void Boldline()
{
	static int bline = -1;

	if(Vars[VBOLDLINE].val == 0) return;
	if(bline != Tgetrow() && bline != -1)
		Chattr(Lptr[bline], 0);
	Chattr(Lptr[Tgetrow()], 8);
	bline = Tgetrow();
}


static char scrn[ 4000 ];

void Screen(row)
char row;
{
	extern unsigned *Lptr[];
	static int saverow;

	if(row != -1)
	{
		memcpy(scrn, (char *)Lptr[0], 4000);
		saverow = row;
	}
	else
	{
		memcpy((char *)Lptr[0], scrn, 4000);
		Tgoto(saverow, 0);
	}
}


void Border()
{
	WDO *wdo = Whead;
	int trow, tcol;
	
	trow = Prow; tcol = Pcol;
	Tsetpoint(0, 0);
	Tputchar(0xd5);
	for(Pcol = 1; Pcol < Colmax - 1; ++Pcol) Tputchar(0xcd);
	Tputchar(0xb8);
	for(Prow = 1; Prow < Rowmax - 2; ++Prow)
		if(Prow == wdo->last)
			/* don't munch the modelines */
			wdo = wdo->next ? wdo->next : Whead;
		else
		{
			Pcol = 0;
			Tputchar(0xb3);
			Pcol = Colmax - 1;
			Tputchar(0xb3);
		}
	Tsetpoint(trow, tcol);
}
#endif

#if MSDOS && !defined(__TURBOC__)
/* strtok for Microsoft C does not work.
 * This is a hacked version that accepts 1 character s2's.
 */
char *strtok(s1, s2)
char *s1;
const char *s2;
{
	extern char *strchr();
	static char *p = NULL;
	char *r = NULL;
	
	if(s1) p = s1;
	if(p && *p)
	{
		r = p;
		if((p = strchr(p, *s2)) != NULL)
			*p++ = '\0';
	}
	return r;
}

ExtendedLineMarker()
{
	Pcol = Tmaxcol() - 1;
	Tputchar( 0x10 );
}
#endif


#ifdef IBMALLOC
/* This version uses the correct malloc! */
char *Strdup(str)
char *str;
{
	char *new;

	if((new = IBMalloc(strlen(str) + 1)) != NULL)
		strcpy(new, str);
	return new;
}
#endif
