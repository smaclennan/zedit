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
	bmove(-1);
	tobegline();
}

void Zend_of_line(void)
{
	bmove1();
	toendline();
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
		bcrsearch(NL);
	bmakecol(col, false);
}

void Znext_line(void)
{
	int col = forcecol();
	while (Arg-- > 0)
		bcsearch(NL);
	bmakecol(col, false);
}

void Zprevious_char(void)
{
	bmove(-Arg);
	Arg = 0;
}

void Znext_char(void)
{
	bmove(Arg);
	Arg = 0;
}

void Zprevious_page(void)
{
	int i, col = forcecol();

	bpnttomrk(Sstart);
	for (i = wheight() - prefline() - 2; i > 0 && bcrsearch(NL); --i)
		i -= bgetcol(true, 0) / Colmax;
	bmakecol(col, false);
	reframe();
}

void Znext_page(void)
{
	int i, col = forcecol();

	bpnttomrk(Sstart);
	for (i = wheight() + prefline() - 2; i > 0 && bcsearch(NL); --i) {
		bmove(-1);
		i -= bgetcol(true, 0) / Colmax;
		bmove1();
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
	btostart();
}

void Zend_of_buffer(void)
{
	btoend();
}

void Zswap_mark(void)
{
	struct mark tmark;

	Arg = 0;
	NEED_UMARK;

	mrktomrk(&tmark, Curbuff->umark);
	Zset_mark();
	bpnttomrk(&tmark);
}

void Zopen_line(void)
{
	binsert(NL);
	bmove(-1);
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

	btostart();
	while (--line > 0)
		bcsearch(NL);
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

int set_bookmark(char *bookname)
{
	Bookmark = (Bookmark + 1) % BOOKMARKS;
	if (Bookname[Bookmark])
		free(Bookname[Bookmark]);
	if (bookname)
		Bookname[Bookmark] = strdup(bookname);

	if (Bookmark > Lastbook) {
		Lastbook = Bookmark;
		Bookmrks[Bookmark] = bcremrk();
	} else
		bmrktopnt(Bookmrks[Bookmark]);
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

void cleanup_bookmarks(void)
{
	int i;

	for (i = 0; i < BOOKMARKS; ++i)
		if (Bookname[i])
			free(Bookname[i]);
}

void Zredisplay(void)
{
	struct buff *buff;

	wsize();
	redisplay();

	for (buff = Bufflist; buff; buff = buff->next)
		uncomment(buff);
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

void Zscroll_up(void)
{
	scroll(bcrsearch);
}

void Zscroll_down(void)
{
	scroll(bcsearch);
}
