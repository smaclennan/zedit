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

void Zbeginning_of_line(void)
{
	bmove(Bbuff, -1);
	tobegline(Bbuff);
}

void Zend_of_line(void)
{
	bmove1(Bbuff);
	toendline(Bbuff);
}

/* Support routine to calc column to force to for line up and down */
static int forcecol(void)
{
	static int fcol;

	switch (Lfunc) {
	case ZPREVIOUS_LINE:
	case ZNEXT_LINE:
	case ZPREVIOUS_PAGE:
	case ZNEXT_PAGE:
		break;

	default:
		if (Buff() == NL)
			fcol = COLMAX + 1;
		else
			fcol = bgetcol(true, 0);
	}

	return fcol;
}

void Zprevious_line(void)
{
	int col = forcecol();
	while (Arg-- > 0)
		bcrsearch(Bbuff, NL);
	bmakecol(col, false);
}

void Znext_line(void)
{
	int col = forcecol();
	while (Arg-- > 0)
		bcsearch(Bbuff, NL);
	bmakecol(col, false);
}

void Zprevious_char(void)
{
	bmove(Bbuff, -Arg);
	Arg = 0;
}

void Znext_char(void)
{
	bmove(Bbuff, Arg);
	Arg = 0;
}

/* Return current screen col of point. */
int bgetcol(bool flag, int col)
{
	struct mark pmark;

	bmrktopnt(Bbuff, &pmark);
	if (bcrsearch(Bbuff, '\n'))
		bmove1(Bbuff);
	while (!bisatmrk(Bbuff, &pmark) && !bisend(Bbuff)) {
		col += chwidth(*Curcptr, col, flag);
		bmove1(Bbuff);
	}
	return col;
}

/* Try to put Point in a specific column.
 * Returns actual Point column.
 */
int bmakecol(int col, bool must)
{
	int tcol = 0;

	if (bcrsearch(Bbuff, '\n'))
		bmove1(Bbuff);
	while (tcol < col && *Curcptr != '\n' && !bisend(Bbuff)) {
		tcol += chwidth(*Curcptr, tcol, !must);
		bmove1(Bbuff);
	}
	if (must && tcol < col) {
		int wid = chwidth('\t', tcol, true);
		if (tcol + wid < col)
			tcol -= Tabsize - wid;
		tindent(col - tcol);
	}
	return tcol;
}

void Zprevious_page(void)
{
	int i, col = forcecol();

	bpnttomrk(Bbuff, Sstart);
	for (i = wheight() - prefline() - 2; i > 0 && bcrsearch(Bbuff, NL); --i)
		i -= bgetcol(true, 0) / Colmax;
	bmakecol(col, false);
	reframe();
}

void Znext_page(void)
{
	int i, col = forcecol();

	bpnttomrk(Bbuff, Sstart);
	for (i = wheight() + prefline() - 2; i > 0 && bcsearch(Bbuff, NL); --i) {
		bmove(Bbuff, -1);
		i -= bgetcol(true, 0) / Colmax;
		bmove1(Bbuff);
	}
	bmakecol(col, false);
	reframe();
}

#define ISWORD	bistoken

void Zprevious_word(void)
{
	moveto(ISWORD, BACKWARD);
	movepast(ISWORD, BACKWARD);
}

void Znext_word(void)
{
	movepast(ISWORD, FORWARD);
	moveto(ISWORD, FORWARD);
}

void Zbeginning_of_buffer(void)
{
	btostart(Bbuff);
}

void Zend_of_buffer(void)
{
	btoend(Bbuff);
}

void Zswap_mark(void)
{
	struct mark tmark;

	Arg = 0;
	NEED_UMARK;

	mrktomrk(&tmark, UMARK);
	Zset_mark();
	bpnttomrk(Bbuff, &tmark);
}

void Zopen_line(void)
{
	binsert(Bbuff, NL);
	bmove(Bbuff, -1);
}

static long getnum(const char *prompt)
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

void Zgoto_line(void)
{
	long line;

	if (Argp)
		line = Arg;
	else {
		line = getnum("Line: ");
		if (line == -1)
			return;
	}

	btostart(Bbuff);
	while (--line > 0)
		bcsearch(Bbuff, NL);
}

void Zout_to(void)
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

static void cleanup_bookmarks(void)
{
	int i;

	for (i = 0; i < BOOKMARKS; ++i)
		if (Bookname[i])
			free(Bookname[i]);
}

int set_bookmark(char *bookname)
{
	if (Bookmark == -1)
		atexit(cleanup_bookmarks);
	Bookmark = (Bookmark + 1) % BOOKMARKS;
	if (Bookname[Bookmark])
		free(Bookname[Bookmark]);
	if (bookname)
		Bookname[Bookmark] = strdup(bookname);

	if (Bookmark > Lastbook) {
		Lastbook = Bookmark;
		Bookmrks[Bookmark] = zcreatemrk();
	} else
		bmrktopnt(Bbuff, Bookmrks[Bookmark]);
	return Bookmark;
}

void Zset_bookmark(void)
{
	if (Argp) {
		Arg = 0;
		*PawStr = '\0';
		if (getarg("Bookmark name: ", PawStr, STRMAX) == 0) {
			set_bookmark(PawStr);
			putpaw("Book Mark %s(%d) Set",
				   Bookname[Bookmark], Bookmark + 1);
		}
	} else {
		set_bookmark(NULL);
		putpaw("Book Mark %d Set", Bookmark + 1);
	}
}

void Znext_bookmark(void)
{
	if (Bookmark < 0) {
		putpaw("No bookmarks set.");
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

	if (Bookmrks[Bookmark]->mbuff != Bbuff) {
		strcpy(Lbufname, zapp(Curbuff)->bname);
		wgoto(Bookmrks[Bookmark]->mbuff);
	}
	bpnttomrk(Bbuff, Bookmrks[Bookmark]);
	Curwdo->modeflags = INVALID;
	putpaw("Book Mark %d", Bookmark + 1);
	if (--Bookmark < 0)
		Bookmark = Lastbook;
}

void Zredisplay(void)
{
	struct zbuff *buff;

	wsize();
	redisplay();

	foreachbuff(buff)
		uncomment(buff);
}

static void scroll(bool (*search)(struct buff *buff, Byte what))
{
	struct mark *pmark = zcreatemrk();

	bpnttomrk(Bbuff, Sstart);
	while (Arg-- > 0 && search(Bbuff, NL))
		;
	tobegline(Bbuff);
	bmrktopnt(Bbuff, Sstart);
	bmove(Bbuff, -1);
	bmrktopnt(Bbuff, Psstart);
	Sendp = false;

	if (mrkaftermrk(Sstart, pmark))
		bmove1(Bbuff);
	else
		bpnttomrk(Bbuff, pmark);

	unmark(pmark);
}

void Zscroll_up(void)
{
	scroll(bcrsearch);
}

void Zscroll_down(void)
{
	scroll(bcsearch);
}
