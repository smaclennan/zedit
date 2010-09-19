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


void Zmakedel() {}

void Zdelchar()
{
	Bdelete(Arg);
	Arg = 0;
}

void Zrdelchar()
{
	Bmove(-Arg);
	Bdelete(Arg);
	Arg = 0;
}

void Zdeleol()
{
	struct mark *tmark = Bcremrk();

	if (!Bisend() && Buff() == NL)
		Bmove1();
	else if (VAR(VKILLLINE)) {
		Boolean atstart;

		Tobegline();
		atstart = Bisatmrk(tmark);
		Toendline();
		if (atstart)
			Bmove1(); /* delete the NL */
	} else
		Toendline();
	Killtomrk(tmark);
	Unmark(tmark);
}

void Zdelline()
{
	struct mark *tmark;

	Tobegline();
	tmark = Bcremrk();
	Bcsearch(NL);
	Killtomrk(tmark);
	Unmark(tmark);
}

/* Delete from the point to the mark */
void Zdelrgn()
{
	Killtomrk(Curbuff->mark);
}


/* Copy from point to the mark into delbuff */
void Zcopyrgn()
{
	Copytomrk(Curbuff->mark);
}


/* Insert the delete buffer at the point */
void Zyank()
{
	struct buff *tbuff;
	int yanked;
	struct mark *tmark, save;	/* save must NOT be a pointer */

	if (InPaw && First) {
		Bdelete(Curplen);
		First = FALSE;
	}

	Mrktomrk(&save, Send);
	tbuff = Curbuff;
	Bmrktopnt(Curbuff->mark);
	Bswitchto(Killbuff);
	Btoend();
	tmark = Bcremrk();
	Btostart();
	yanked = Bcopyrgn(tmark, tbuff);
	Unmark(tmark);
	Bswitchto(tbuff);
	undo_add(yanked);
	if (Bisaftermrk(&save))
		Reframe();
}

void Zdelword()
{
	struct mark *tmark;

	tmark = Bcremrk();
	Moveto(Isword, FORWARD);
	Movepast(Isword, FORWARD);
	Killtomrk(tmark);
	Unmark(tmark);
}

void Zrdelword()
{
	struct mark *tmark;

	tmark = Bcremrk();
	Zbword();
	Killtomrk(tmark);
	Unmark(tmark);
}

void Zgetbword()
{
	char word[STRMAX], *ptr;
	struct mark *tmark, *start;

	if (InPaw) {
		Bswitchto(Buff_save);
		Getbword(word, STRMAX, Istoken);
		Bswitchto(Paw);
		for (ptr = word; *ptr; ++ptr) {
			Cmd = *ptr;
			Pinsert();
		}
	} else {
		tmark = Bcremrk();	/* save current Point */
		Moveto(Istoken, FORWARD); /* find start of word */
		Movepast(Istoken, BACKWARD);
		start = Bcremrk();
		Movepast(Istoken, FORWARD); /* move Point to end of word */
		Copytomrk(start); /* copy to Kill buffer */
		Bpnttomrk(tmark); /* move Point back */
		Unmark(tmark);
		Unmark(start);
	}
	Arg = 0;
}

void Zdelblanks()
{
	struct mark *tmark, *pmark;

	pmark = Bcremrk();
	if (Bcrsearch(NL)) {
		Bmove1();
		tmark = Bcremrk();
		Movepast(Isspace, BACKWARD);
		if (!Bisstart())
			Bcsearch(NL);
		if (Bisbeforemrk(tmark))
			Bdeltomrk(tmark);
		Unmark(tmark);
	}
	if (Bcsearch(NL)) {
		tmark = Bcremrk();
		Movepast(Isspace, FORWARD);
		if (Bcrsearch(NL))
			Bmove1();
		if (Bisaftermrk(tmark))
			Bdeltomrk(tmark);
		Unmark(tmark);
	}
	Bpnttomrk(pmark);
	Unmark(pmark);
}

void Zjoin()
{
	Toendline();
	Bdelete(1);
	Zdelwhite();
	Binsert(' ');
}

void Zempty()
{
	if (Ask("Empty buffer? ") != YES)
		return;
	Bempty();
	Curbuff->bmodf = MODIFIED;
}


void Copytomrk(struct mark *tmark)
{
	struct buff *save = Curbuff;
	Bswitchto(Killbuff);
	if (Delcmd())
		Btoend();
	else
		Bempty();
	Bswitchto(save);
	Bcopyrgn(tmark, Killbuff);
}
