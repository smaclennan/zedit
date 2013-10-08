/* cursor.c - Zedit cursor commands
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

void Zbegline(void)
{
	bmove(-1);
	tobegline();
}

void Zendline(void)
{
	bmove1();
	toendline();
}

/* Support routine to calc column to force to for line up and down */
static int forcecol(void)
{
	static int fcol;

	if (Lfunc != ZPREVLINE && Lfunc != ZNEXTLINE &&
	    Lfunc != ZPREVPAGE && Lfunc != ZNEXTPAGE) {
		if (Buff() == NL)
			fcol = COLMAX + 1;
		else
			fcol = bgetcol(true, 0);
	}

	return fcol;
}

static void ScrollLine(bool (*search)(Byte what))
{
	struct mark save;

	bmrktopnt(&save);
	bpnttomrk(Sstart);
	search(NL);
	tobegline();
	bmrktopnt(Sstart);
	bmove(-1);
	bmrktopnt(Psstart);
	bpnttomrk(&save);
	Sendp = false;
}

void Zprevline(void)
{
	int col = forcecol();

	while (Arg-- > 0)
		bcrsearch(NL);

	if (VAR(VSINGLE))
		if (bisbeforemrk(Sstart))
			ScrollLine(bcrsearch);

	bmakecol(col, false);
}

void Znextline(void)
{
	int col = forcecol();

	while (Arg-- > 0)
		bcsearch(NL);

	if (VAR(VSINGLE))
		if (Sendp && !bisbeforemrk(Send))
			ScrollLine(bcsearch);

	bmakecol(col, false);
}

void Zprevchar(void)
{
	bmove(-Arg);
	Arg = 0;
}

void Znextchar(void)
{
	bmove(Arg);
	Arg = 0;
}

void Zprevpage(void)
{
	int i, col = forcecol();

	bpnttomrk(Sstart);
	for (i = wheight() - prefline() - 2; i > 0 && bcrsearch(NL); --i)
		i -= bgetcol(true, 0) / tmaxcol();
	bmakecol(col, false);
	reframe();
}

void Znextpage(void)
{
	int i, col = forcecol();

	bpnttomrk(Sstart);
	for (i = wheight() + prefline() - 2; i > 0 && bcsearch(NL); --i) {
		bmove(-1);
		i -= bgetcol(true, 0) / tmaxcol();
		bmove1();
	}
	bmakecol(col, false);
	reframe();
}

#define ISWORD	bistoken

void Zbword(void)
{
	moveto(ISWORD, BACKWARD);
	movepast(ISWORD, BACKWARD);
}

void Zfword(void)
{
	movepast(ISWORD, FORWARD);
	moveto(ISWORD, FORWARD);
}

void Ztostart(void)
{
	btostart();
}

void Ztoend(void)
{
	btoend();
}

void Zswapmrk(void)
{
	struct mark tmark;

	Arg = 0;
	mrktomrk(&tmark, Curbuff->mark);
	Zsetmrk();
	bpnttomrk(&tmark);
}

void Zopenline(void)
{
	binsert(NL);
	bmove(-1);
}

static long getnum(char *prompt)
{
	char str[10];
	long num = -1;

	/* get the line number */
	*str = '\0';
	if (Argp) {
		num = Arg;
		Arg = 0;
	} else if (getarg(prompt, str, 9) == 0)
		num = strtol(str, NULL, 0);
	return num;
}

void Zlgoto(void)
{
	long line, cnt = 0;
	struct page *tpage;

	if (Argp)
		line = Arg;
	else {
		line = getnum("Line: ");
		if (line == -1)
			return;
	}

	/* find the correct page */
	for (tpage = Curbuff->firstp; tpage->nextp; tpage = tpage->nextp) {
		if (tpage->plines == EOF) {
			makecur(tpage);
			tpage->plines = cntlines(Curplen);
		}
		cnt += tpage->plines;
		if (cnt >= line) {
			cnt -= tpage->plines;
			break;
		}
	}
	makecur(tpage);
	makeoffset(0);

	/* go to the correct offset */
	for (cnt = line - cnt - 1; cnt > 0; --cnt)
		bcsearch(NL);
}

void Zcgoto(void)
{
	int col = (int)getnum("Column: ");
	if (col == -1)
		return;
	bmakecol(--col, true);
}

#define BOOKMARKS	10			/* number of book marks */
static struct mark *Bookmrks[BOOKMARKS];	/* stack of book marks */
static char *Bookname[BOOKMARKS];		/* stack of book names */
static int  Bookmark = -1;			/* current book mark */
static int  Lastbook = -1;			/* last bookmark */


void Zsetbookmrk(void)
{
	if (Argp) {
		Arg = 0;
		*PawStr = '\0';
		if (getarg("Bookmark name: ", PawStr, STRMAX))
			return;
	} else
		strcpy(PawStr, "Unamed");

	Bookmark = (Bookmark + 1) % BOOKMARKS;
	if (Bookname[Bookmark])
		free(Bookname[Bookmark]);
	Bookname[Bookmark] = strdup(PawStr);

	if (Bookmark > Lastbook) {
		Lastbook = Bookmark;
		Bookmrks[Bookmark] = bcremrk();
	} else
		bmrktopnt(Bookmrks[Bookmark]);

	if (Argp)
		putpaw("Book Mark %s(%d) Set",
		       Bookname[Bookmark], Bookmark + 1);
	else
		putpaw("Book Mark %d Set", Bookmark + 1);
}


void Znxtbookmrk(void)
{
	if (Bookmark < 0) {
		echo("No bookmarks set.");
		return;
	}

	if (Argp) {
		int b;

		Arg = 0;
		b = getplete("Bookmark name: ", NULL, Bookname, sizeof(char *),
			     Lastbook + 1);
		if (b == -1)
			return;
	}

	if (Bookmrks[Bookmark]->mbuff != Curbuff) {
		strcpy(Lbufname, Curbuff->bname);
		bgoto(Bookmrks[Bookmark]->mbuff);
	}
	bpnttomrk(Bookmrks[Bookmark]);
	Curwdo->modeflags = INVALID;
	putpaw("Book Mark %d", Bookmark + 1);
	if (--Bookmark < 0)
		Bookmark = Lastbook;
}

void Zviewline(void)
{
	struct mark pmark;

	bmrktopnt(&pmark);
	tobegline();
	bmrktopnt(Sstart);
	bmove(-1);
	bmrktopnt(Psstart);
	Sendp = false;
	bpnttomrk(&pmark);
}

void Zredisplay(void)
{
	struct wdo *wdo;

	wsize();
	for (wdo = Whead; wdo; wdo = wdo->next)
		wdo->modeflags = INVALID;
	redisplay();
	recomment();
}

void Zbegwind(void)
{
	bpnttomrk(Sstart);
}

void Zendwind(void)
{
	int i;

	bpnttomrk(Sstart);
	for (i = wheight() - 1; i && bcsearch(NL); --i)
		;
}

static void scroll(bool (*search)(Byte what))
{
	struct mark *pmark = bcremrk();

	bpnttomrk(Sstart);
	while (Arg-- > 0 && search(NL))
		;
	tobegline();
	bmrktopnt(Sstart);
	bmove(-1);
	bmrktopnt(Psstart);
	Sendp = false;

	if (mrkaftermrk(Sstart, pmark))
		bmove1();
	else
		bpnttomrk(pmark);

	unmark(pmark);
}

void Zscrollup(void)
{
	scroll(bcrsearch);
}

void Zscrolldown(void)
{
	scroll(bcsearch);
}
