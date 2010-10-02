/* modes.c - draws the modeline
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


/* local routine to set PawStr to the correct mode */
char *Setmodes(struct buff *buff)
{
	if (!InPaw)	/* we should never be in the Paw but .... */
		Funcs = (buff->bmode & VIEW) ? Vcmds : Cmds;

	/* set all keys back to default */
	Keys[CR] = CRdefault;
	Keys[' '] = Keys['\175'] = Keys['#'] = Keys[':'] = ZINSERT;
#if COMMENTBOLD
	Keys['/'] = ZINSERT;
#endif

	/* Set PawStr to majour mode */
	switch (buff->bmode & MAJORMODE) {
	case CMODE:
		strcpy(PawStr, "C");		break;
	case ASMMODE:
		strcpy(PawStr, "ASM");		break;
	case TCL:
		strcpy(PawStr, "TCL");		break;
	case TEXT:
		strcpy(PawStr, "Text");		break;
	default:
		strcpy(PawStr, "Normal");	break;
	}

	/* Now setup any special keys */
	if (buff->bmode & VIEW) {
		/* view preempts the other modes */
		Keys[CR] = ZNEXTLINE;
		strcat(PawStr, " RO");
	} else
		switch (buff->bmode & MAJORMODE) {
		case CMODE:
			Keys[CR] = ZCINDENT;
			Keys['\175'] = ZCINSERT; /* end brace */
			Keys['#'] = ZCINSERT;
			Keys[':'] = ZCINSERT;
#if COMMENTBOLD
			Keys['/'] = ZCINSERT;
#endif
			break;
		case ASMMODE:
			break;
		case TEXT:
			Keys[' '] = ZFILLCHK;
			Keys[CR] = ZFILLCHK;
			break;
		}

	if (buff->bmode & OVERWRITE)
		strcat(PawStr, " OVWRT");
	Settabsize(buff->bmode);
	return PawStr;
}

#ifndef XWINDOWS
/* This is called before the windows are created */
void Initline(void)
{
	int i;

	sprintf(PawStr, "%s %s  Initializing", ZSTR, VERSION);
	Tclrwind();
	Tgoto(Rowmax - 2, 0);
	Tstyle(T_STANDOUT);
	Tprntstr(PawStr);
	for (i = strlen(PawStr) + 1; i < Colmax; ++i)
		Tprntchar(' ');
	Tstyle(T_NORMAL);
	Tflush();
}

/* Redraw the modeline except for flags. */
static void Modeline(struct wdo *wdo)
{
	char str[COLMAX + 1]; /* can't use PawStr because of Setmodes */
	int len;

	Tsetpoint(wdo->last, 0);
	Tstyle(T_STANDOUT);
	sprintf(str, ZFMT, ZSTR, VERSION, Setmodes(wdo->wbuff),
		wdo->wbuff->bname);
	Tprntstr(str);
	if (wdo->wbuff->fname) {
		len = (VAR(VLINES) ? 13 : 3) + strlen(str);
		Tprntstr(Limit(wdo->wbuff->fname, len));
	}
	wdo->modecol = Tgetcol();

	/* space pad the line */
	for (len = Tmaxcol() - Tgetcol(); len > 0; --len)
		Tprntchar(' ');
	Tstyle(T_NORMAL);
}

/* This routine will call Modeline if wdo->modeflags == INVALID */
void Modeflags(struct wdo *wdo)
{
	unsigned trow, tcol, line, col, mask;

	trow = Tgetrow();
	tcol = Tgetcol();

	if (wdo->modeflags == INVALID)
		Modeline(wdo);

	Tstyle(T_STANDOUT);

	if (VAR(VLINES)) {
		struct buff *was = Curbuff;
		Bswitchto(wdo->wbuff);
		Blocation(&line);
		col = Bgetcol(FALSE, 0) + 1;
		if (col > 999)
			sprintf(PawStr, "%5u:???", line);
		else
			sprintf(PawStr, "%5u:%-3u", line, col);
		PawStr[9] = '\0';
		Tsetpoint(wdo->last, Tmaxcol() - 9);
		Tprntstr(PawStr);
		Bswitchto(was);
	}

	mask = Delcmd() | (wdo->wbuff->bmodf ? 2 : 0);
	if (!InPaw && wdo->modeflags != mask) {
		Tsetpoint(wdo->last, wdo->modecol);
		Tprntchar(mask & 2 ? '*' : ' ');
		Tprntchar(mask & 1 ? '+' : ' ');
		wdo->modeflags = mask;
	}

	Tstyle(T_NORMAL);
	Tgoto(trow, tcol);
}
#endif
