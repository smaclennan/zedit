/* delete.c - Zedit delete commands
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


struct buff *Killbuff;


void Zmakedel(void) {}

void Zdelchar(void)
{
	bdelete(Arg);
	Arg = 0;
}

void Zrdelchar(void)
{
	bmove(-Arg);
	bdelete(Arg);
	Arg = 0;
}

void Zdeleol(void)
{
	struct mark *tmark = bcremrk();

	if (!Bisend() && Buff() == NL)
		bmove1();
	else if (VAR(VKILLLINE)) {
		Boolean atstart;

		Tobegline();
		atstart = Bisatmrk(tmark);
		Toendline();
		if (atstart)
			bmove1(); /* delete the NL */
	} else
		Toendline();
	Killtomrk(tmark);
	unmark(tmark);
}

void Zdelline(void)
{
	struct mark *tmark;

	Tobegline();
	tmark = bcremrk();
	bcsearch(NL);
	Killtomrk(tmark);
	unmark(tmark);
}

/* Delete from the point to the mark */
void Zdelrgn(void)
{
	Killtomrk(Curbuff->mark);
}


/* Copy from point to the mark into delbuff */
void Zcopyrgn(void)
{
	Copytomrk(Curbuff->mark);
}


/* Insert the delete buffer at the point */
void Zyank(void)
{
	struct buff *tbuff;
	int yanked;
	struct mark *tmark, save;	/* save must NOT be a pointer */

	if (InPaw && First) {
		bdelete(Curplen);
		First = FALSE;
	}

	Mrktomrk(&save, Send);
	tbuff = Curbuff;
	bmrktopnt(Curbuff->mark);
	bswitchto(Killbuff);
	btoend();
	tmark = bcremrk();
	btostart();
	yanked = bcopyrgn(tmark, tbuff);
	unmark(tmark);
	bswitchto(tbuff);
	undo_add(yanked);
	if (bisaftermrk(&save))
		Reframe();
}

void Zdelword(void)
{
	struct mark *tmark;

	tmark = bcremrk();
	Moveto(Isword, FORWARD);
	Movepast(Isword, FORWARD);
	Killtomrk(tmark);
	unmark(tmark);
}

void Zrdelword(void)
{
	struct mark *tmark;

	tmark = bcremrk();
	Zbword();
	Killtomrk(tmark);
	unmark(tmark);
}

void Zgetbword(void)
{
	char word[STRMAX], *ptr;
	struct mark *tmark, *start;

	if (InPaw) {
		bswitchto(Buff_save);
		Getbword(word, STRMAX, Istoken);
		bswitchto(Paw);
		for (ptr = word; *ptr; ++ptr) {
			Cmd = *ptr;
			Pinsert();
		}
	} else {
		tmark = bcremrk();	/* save current Point */
		Moveto(Istoken, FORWARD); /* find start of word */
		Movepast(Istoken, BACKWARD);
		start = bcremrk();
		Movepast(Istoken, FORWARD); /* move Point to end of word */
		Copytomrk(start); /* copy to Kill buffer */
		bpnttomrk(tmark); /* move Point back */
		unmark(tmark);
		unmark(start);
	}
	Arg = 0;
}

void Zdelblanks(void)
{
	struct mark *tmark, *pmark;

	pmark = bcremrk();
	if (bcrsearch(NL)) {
		bmove1();
		tmark = bcremrk();
		Movepast(Isspace, BACKWARD);
		if (!Bisstart())
			bcsearch(NL);
		if (bisbeforemrk(tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	if (bcsearch(NL)) {
		tmark = bcremrk();
		Movepast(Isspace, FORWARD);
		if (bcrsearch(NL))
			bmove1();
		if (bisaftermrk(tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	bpnttomrk(pmark);
	unmark(pmark);
}

void Zjoin(void)
{
	Toendline();
	bdelete(1);
	Zdelwhite();
	binsert(' ');
}

void Zempty(void)
{
	if (Ask("Empty buffer? ") != YES)
		return;
	bempty();
	Curbuff->bmodf = MODIFIED;
}


void Copytomrk(struct mark *tmark)
{
	struct buff *save = Curbuff;
	bswitchto(Killbuff);
	if (Delcmd())
		btoend();
	else
		bempty();
	bswitchto(save);
	bcopyrgn(tmark, Killbuff);
}
