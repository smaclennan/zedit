/* cursor.c - Zedit cursor commands
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

static void Scroll(Boolean);

void Zbegline()
{
	Bmove(-1);
	Tobegline();
}


void Zendline()
{
	Bmove1();
	Toendline();
}


/* Support routine to calc column to force to for line up and down */
int Forcecol()
{
	static int fcol;

	return zCursor() ? fcol : (fcol = Bgetcol(TRUE, 0));
}


static void ScrollLine(Boolean forward)
{
	struct mark save;

	if (VAR(VSINGLE)) {
		Bmrktopnt(&save);
		Bpnttomrk(Sstart);
		forward ? Bcsearch(NL) : Bcrsearch(NL);
		Tobegline();
		Bmrktopnt(Sstart);
		Bmove(-1);
		Bmrktopnt(Psstart);
		Bpnttomrk(&save);
		Sendp = FALSE;
	}
}

void Zprevline()
{
	int col = Forcecol();

	while (Arg-- > 0)
		Bcrsearch(NL);

	if (Bisbeforemrk(Sstart))
		ScrollLine(FALSE);

	Bmakecol(col, FALSE);
}

void Znextline()
{
	int col = Forcecol();

	while (Arg-- > 0)
		Bcsearch(NL);

	if (Sendp && !Bisbeforemrk(Send))
		ScrollLine(TRUE);

	Bmakecol(col, FALSE);
}

void Zprevchar()
{
	Bmove(-Arg);
	Arg = 0;
}

void Znextchar()
{
	Bmove(Arg);
	Arg = 0;
}

void Zprevpage()
{
	int i, n, col = Forcecol();

	Bpnttomrk(Sstart);
	for (n = i = Wheight() - Prefline() - 2; i > 0 && Bcrsearch(NL); --i)
		i -= Bgetcol(TRUE, 0) / Tmaxcol();
	Bmakecol(col, FALSE);
	Reframe();
}

void Znextpage()
{
	int i, col = Forcecol();

	Bpnttomrk(Sstart);
	for (i = Wheight() + Prefline() - 2; i > 0 && Bcsearch(NL); --i) {
		Bmove(-1);
		i -= Bgetcol(TRUE, 0) / Tmaxcol();
		Bmove1();
	}
	Bmakecol(col, FALSE);
	Reframe();
}

#define ISWORD	Istoken

void Zbword()
{
	Moveto(ISWORD, BACKWARD);
	Movepast(ISWORD, BACKWARD);
}

void Zfword()
{
	Movepast(ISWORD, FORWARD);
	Moveto(ISWORD, FORWARD);
}

void Ztostart()
{
	Btostart();
}

void Ztoend()
{
	Btoend();
}

void Zswapmrk()
{
	struct mark tmark;

	Arg = 0;
	Mrktomrk(&tmark, Curbuff->mark);
	Zsetmrk();
	Bpnttomrk(&tmark);
}

void Zopenline()
{
	Binsert(NL);
	Bmove(-1);
}

void Zlgoto()
{
	long line, cnt = 0;
	struct page *tpage;

	if (Argp)
		line = Arg;
	else {
		line = Getnum("Line: ");
		if (line == -1)
			return;
	}

	/* find the correct page */
	for (tpage = Curbuff->firstp; tpage->nextp; tpage = tpage->nextp) {
		if (tpage->lines == EOF) {
			Makecur(tpage);
			tpage->lines = Cntlines(Curplen);
		}
		cnt += tpage->lines;
		if (cnt >= line) {
			cnt -= tpage->lines;
			break;
		}
	}
	Makecur(tpage);
	Makeoffset(0);

	/* go to the correct offset */
	for (cnt = line - cnt - 1; cnt > 0; --cnt)
		Bcsearch(NL);
}

void Zcgoto()
{
	int col = (int)Getnum("Column: ");
	if (col == -1)
		return;
	Bmakecol(--col, TRUE);
}

long Getnum(char *prompt)
{
	char str[10];
	long num = -1;

	/* get the line number */
	*str = '\0';
	if (Argp) {
		num = Arg;
		Arg = 0;
	} else if (Getarg(prompt, str, 9) == 0)
		num = strtol(str, NULL, 0);
	return num;
}


struct mark *Bookmrks[BOOKMARKS];	/* stack of book marks */
char *Bookname[BOOKMARKS];		/* stack of book names */
int  Bookmark = -1;			/* current book mark */
int  Lastbook = -1;			/* last bookmark */


void Zsetbookmrk()
{
	if (Argp) {
		Arg = 0;
		*PawStr = '\0';
		if (Getarg("Bookmark name: ", PawStr, STRMAX))
			return;
	} else
		strcpy(PawStr, "Unamed");

	Bookmark = (Bookmark + 1) % BOOKMARKS;
	if (Bookname[Bookmark])
		free(Bookname[Bookmark]);
	Bookname[Bookmark] = strdup(PawStr);

	if (Bookmark > Lastbook) {
		Lastbook = Bookmark;
		Bookmrks[Bookmark] = Bcremrk();
	} else
		Bmrktopnt(Bookmrks[Bookmark]);

	if (Argp)
		sprintf(PawStr, "Book Mark %s(%d) Set",
			Bookname[Bookmark], Bookmark + 1);
	else
		sprintf(PawStr, "Book Mark %d Set", Bookmark + 1);
	Echo(PawStr);
}


void Znxtbookmrk()
{
	if (Bookmark < 0) {
		Echo("No bookmarks set.");
		return;
	}

	if (Argp) {
		int b;

		Arg = 0;
		b = Getplete("Bookmark name: ", NULL, Bookname, sizeof(char *),
			     Lastbook + 1);
		if (b == -1)
			return;
	}

	if (Bookmrks[Bookmark]->mbuff != Curbuff) {
		strcpy(Lbufname, Curbuff->bname);
		Bgoto(Bookmrks[Bookmark]->mbuff);
	}
	Bpnttomrk(Bookmrks[Bookmark]);
	Curwdo->modeflags = INVALID;
	sprintf(PawStr, "Book Mark %d", Bookmark + 1);
	Echo(PawStr);
	if (--Bookmark < 0)
		Bookmark = Lastbook;
}

void Zviewline()
{
	struct mark pmark;

	Bmrktopnt(&pmark);
	Tobegline();
	Bmrktopnt(Sstart);
	Bmove(-1);
	Bmrktopnt(Psstart);
	Sendp = FALSE;
	Bpnttomrk(&pmark);
}

void Zredisplay()
{
#ifndef BORDER3D
	struct wdo *wdo;
#endif

	Wsize();
#ifdef BORDER3D
	Curwdo->modeflags = INVALID;
#else
	for (wdo = Whead; wdo; wdo = wdo->next)
		wdo->modeflags = INVALID;
#endif
	Redisplay();
#if COMMENTBOLD
	Recomment();
#endif
#ifdef BORDER3D
	/* Draw the boarder around the window */
	DrawBorders();
#endif
}

void Zbegwind()
{
	Bpnttomrk(Sstart);
}

void Zendwind()
{
	int i;

	Bpnttomrk(Sstart);
	for (i = Wheight() - 1; i && Bcsearch(NL); --i)
		;
}

void Zscrollup()
{
	Scroll(FALSE);
}

void Zscrolldown()
{
	Scroll(TRUE);
}

static void Scroll(Boolean forward)
{
	struct mark *pmark = Bcremrk();

	Bpnttomrk(Sstart);
	if (forward)
		while (Arg-- > 0 && Bcsearch(NL))
			;
	else
		while (Arg-- > 0 && Bcrsearch(NL))
			;
	Tobegline();
	Bmrktopnt(Sstart);
	Bmove(-1);
	Bmrktopnt(Psstart);
	Sendp = FALSE;

	if (Mrkaftermrk(Sstart, pmark))
		Bmove1();
	else
		Bpnttomrk(pmark);

	Unmark(pmark);
}
