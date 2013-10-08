/* delete.c - Zedit delete commands
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


struct buff *Killbuff;

static void copytomrk(struct mark *tmark)
{
	struct buff *save = Curbuff;
	bswitchto(Killbuff);
	if (delcmd())
		btoend();
	else
		bempty();
	bswitchto(save);
	bcopyrgn(tmark, Killbuff);
}

void killtomrk(struct mark *tmark)
{
	copytomrk(tmark);
	bdeltomrk(tmark);
}

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

	if (!bisend() && Buff() == NL)
		bmove1();
	else if (VAR(VKILLLINE)) {
		bool atstart;

		tobegline();
		atstart = bisatmrk(tmark);
		toendline();
		if (atstart)
			bmove1(); /* delete the NL */
	} else
		toendline();
	killtomrk(tmark);
	unmark(tmark);
}

void Zdelline(void)
{
	struct mark *tmark;

	tobegline();
	tmark = bcremrk();
	bcsearch(NL);
	killtomrk(tmark);
	unmark(tmark);
}

/* Delete from the point to the mark */
void Zdelrgn(void)
{
	killtomrk(Curbuff->mark);
}


/* Copy from point to the mark into delbuff */
void Zcopyrgn(void)
{
	copytomrk(Curbuff->mark);
}


/* Insert the delete buffer at the point */
void Zyank(void)
{
	struct buff *tbuff;
	int yanked;
	struct mark *tmark, save;	/* save must NOT be a pointer */

	if (InPaw && First) {
		bdelete(Curplen);
		First = false;
	}

	mrktomrk(&save, Send);
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
		reframe();
}

void Zdelword(void)
{
	struct mark *tmark;

	tmark = bcremrk();
	moveto(bisword, FORWARD);
	movepast(bisword, FORWARD);
	killtomrk(tmark);
	unmark(tmark);
}

void Zrdelword(void)
{
	struct mark *tmark;

	tmark = bcremrk();
	Zbword();
	killtomrk(tmark);
	unmark(tmark);
}

void Zgetbword(void)
{
	char word[STRMAX], *ptr;
	struct mark *tmark, *start;

	if (InPaw) {
		bswitchto(Buff_save);
		getbword(word, STRMAX, bistoken);
		bswitchto(Paw);
		for (ptr = word; *ptr; ++ptr) {
			Cmd = *ptr;
			pinsert();
		}
	} else {
		tmark = bcremrk();	/* save current Point */
		moveto(bistoken, FORWARD); /* find start of word */
		movepast(bistoken, BACKWARD);
		start = bcremrk();
		movepast(bistoken, FORWARD); /* move Point to end of word */
		copytomrk(start); /* copy to Kill buffer */
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
		movepast(bisspace, BACKWARD);
		if (!bisstart())
			bcsearch(NL);
		if (bisbeforemrk(tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	if (bcsearch(NL)) {
		tmark = bcremrk();
		movepast(bisspace, FORWARD);
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
	toendline();
	bdelete(1);
	Zdelwhite();
	binsert(' ');
}

void Zempty(void)
{
	if (ask("Empty buffer? ") != YES)
		return;
	bempty();
	Curbuff->bmodf = MODIFIED;
}
