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
char *setmodes(struct buff *buff)
{
	if (!InPaw)	/* we should never be in the Paw but .... */
		Curcmds = (buff->bmode & VIEW) ? 1 : 0;

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
void initline(void)
{
	int i;

	sprintf(PawStr, "%s %s  Initializing", ZSTR, VERSION);
	tclrwind();
	tgoto(Rowmax - 2, 0);
	tstyle(T_STANDOUT);
	tprntstr(PawStr);
	for (i = strlen(PawStr) + 1; i < Colmax; ++i)
		tprntchar(' ');
	tstyle(T_NORMAL);
	tflush();
}

/* Redraw the modeline except for flags. */
static void modeline(struct wdo *wdo)
{
	char str[COLMAX + 1]; /* can't use PawStr because of setmodes */
	int len;

	tsetpoint(wdo->last, 0);
	tstyle(T_STANDOUT);
	sprintf(str, ZFMT, ZSTR, VERSION, setmodes(wdo->wbuff),
		wdo->wbuff->bname);
	tprntstr(str);
	if (wdo->wbuff->fname) {
		len = (VAR(VLINES) ? 13 : 3) + strlen(str);
		tprntstr(limit(wdo->wbuff->fname, len));
	}
	wdo->modecol = tgetcol();

	/* space pad the line */
	for (len = tmaxcol() - tgetcol(); len > 0; --len)
		tprntchar(' ');
	tstyle(T_NORMAL);
}

/* This routine will call modeline if wdo->modeflags == INVALID */
void modeflags(struct wdo *wdo)
{
	unsigned trow, tcol, line, col, mask;

	trow = tgetrow();
	tcol = tgetcol();

	if (wdo->modeflags == INVALID)
		modeline(wdo);

	tstyle(T_STANDOUT);

	if (VAR(VLINES)) {
		struct buff *was = Curbuff;
		bswitchto(wdo->wbuff);
		blocation(&line);
		col = bgetcol(FALSE, 0) + 1;
		if (col > 999)
			sprintf(PawStr, "%5u:???", line);
		else
			sprintf(PawStr, "%5u:%-3u", line, col);
		PawStr[9] = '\0';
		tsetpoint(wdo->last, tmaxcol() - 9);
		tprntstr(PawStr);
		bswitchto(was);
	}

	mask = delcmd() | (wdo->wbuff->bmodf ? 2 : 0);
	if (!InPaw && wdo->modeflags != mask) {
		tsetpoint(wdo->last, wdo->modecol);
		tprntchar(mask & 2 ? '*' : ' ');
		tprntchar(mask & 1 ? '+' : ' ');
		wdo->modeflags = mask;
	}

	tstyle(T_NORMAL);
	tgoto(trow, tcol);
}
#endif
