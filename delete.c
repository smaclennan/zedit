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


static struct buff *Killbuff;

static void delfini(void)
{
	_bdelbuff(Killbuff);
	_bdelbuff(Paw);
}

void delinit(void)
{
	Killbuff = _bcreate();
	Paw = _bcreate();
	atexit(delfini);
}

static void copytomrk(struct mark *tmark)
{
	struct buff *save = Curbuff;
	bswitchto(Killbuff);
	if (delcmd())
		btoend(Curbuff);
	else
		bempty(Curbuff);
	bswitchto(save);
	bcopyrgn(tmark, Killbuff);
}

/* Copy from Point to tmark to tbuff. Returns number of bytes
 * copied. Caller must handle undo. */
int bcopyrgn(struct mark *tmark, struct buff *tbuff)
{
	struct buff *sbuff;
	struct mark *ltmrk, *btmrk;
	bool flip;
	int  srclen, dstlen;
	Byte *spnt;
	int copied = 0;

	if (tbuff == Curbuff)
		return 0;

	flip = bisaftermrk(tmark);
	if (flip)
		bswappnt(tmark);

	if (!(ltmrk = _bcremrk(Curbuff)))
		return 0;

	sbuff = Curbuff;
	while (bisbeforemrk(tmark)) {
		if (Curpage == tmark->mpage)
			srclen = tmark->moffset - Curchar;
		else
			srclen = Curpage->plen - Curchar;
		spnt = Curcptr;

		bswitchto(tbuff);
		dstlen = PSIZE - Curpage->plen;
		if (dstlen == 0) {
			if (pagesplit(Curbuff, HALFP))
				dstlen = PSIZE - Curpage->plen;
			else {
				bswitchto(sbuff);
				break;
			}
		}
		if (srclen < dstlen)
			dstlen = srclen;
		/* Make a gap */
		memmove(Curcptr + dstlen, Curcptr, Curpage->plen - Curchar);
		/* and fill it in */
		memmove(Curcptr, spnt, dstlen);
		Curpage->plen += dstlen;
		copied += dstlen;
		foreach_pagemark(Curbuff, btmrk, Curpage)
			if (btmrk->moffset > Curchar)
					btmrk->moffset += dstlen;
		makeoffset(Curbuff, Curchar + dstlen);
		vsetmod();
		Curbuff->bmodf = true;
		bswitchto(sbuff);
		bmove(Curbuff, dstlen);
	}

	bpnttomrk(ltmrk);
	unmark(ltmrk);

	if (flip)
		bswappnt(tmark);

	return copied;
}

/* Delete from the point to the Mark. */
void bdeltomrk(struct mark *tmark)
{
	if (bisaftermrk(tmark))
		bswappnt(tmark);
	while (bisbeforemrk(tmark))
		if (Curpage == tmark->mpage)
			bdelete(Curbuff, tmark->moffset - Curchar);
		else
			bdelete(Curbuff, Curpage->plen - Curchar);
}

void killtomrk(struct mark *tmark)
{
	copytomrk(tmark);
	bdeltomrk(tmark);
}

void Zappend_kill(void) {}

void Zdelete_char(void)
{
	bdelete(Curbuff, Arg);
	Arg = 0;
}

void Zdelete_previous_char(void)
{
	bmove(Curbuff, -Arg);
	bdelete(Curbuff, Arg);
	Arg = 0;
}

void Zdelete_to_eol(void)
{
	if (!bisend(Curbuff) && Buff() == NL)
		bdelete(Curbuff, 1);
	else {
		bool atstart = _bpeek(Curbuff) == NL;
		struct mark *tmark = zcreatemrk();
		toendline(Curbuff);
		if (atstart)
			bmove1(Curbuff); /* delete the NL */
		killtomrk(tmark);
		unmark(tmark);
	}
}

void Zdelete_line(void)
{
	struct mark *tmark;

	tobegline(Curbuff);
	tmark = zcreatemrk();
	bcsearch(Curbuff, NL);
	killtomrk(tmark);
	unmark(tmark);
}

void Zdelete_region(void)
{
	NEED_UMARK;
	killtomrk(UMARK);
	CLEAR_UMARK;
}

void Zcopy_region(void)
{
	NEED_UMARK;
	copytomrk(UMARK);
	CLEAR_UMARK;
}


void Zyank(void)
{
	struct buff *tbuff;
	struct mark *tmark, save;	/* save must NOT be a pointer */

	if (InPaw && First) {
		bempty(Curbuff);
		First = false;
	}

	mrktomrk(&save, Send);
	tbuff = Curbuff;
#if !UNDO
	/* This leaves the mark at the start of the yank and
	 * the point at the end. */
	set_umark(NULL);
#endif
	bswitchto(Killbuff);
	btoend(Curbuff);
	tmark = zcreatemrk();
	btostart(Curbuff);
#if UNDO
	undo_add(bcopyrgn(tmark, tbuff));
#else
	bcopyrgn(tmark, tbuff);
#endif
	unmark(tmark);
	bswitchto(tbuff);
	if (bisaftermrk(&save))
		reframe();
}

void Zdelete_word(void)
{
	struct mark *tmark;

	tmark = zcreatemrk();
	moveto(bisword, FORWARD);
	movepast(bisword, FORWARD);
	killtomrk(tmark);
	unmark(tmark);
}

void Zdelete_previous_word(void)
{
	struct mark *tmark;

	tmark = zcreatemrk();
	Zprevious_word();
	killtomrk(tmark);
	unmark(tmark);
}

void Zcopy_word(void)
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
		tmark = zcreatemrk();	/* save current Point */
		moveto(bistoken, FORWARD); /* find start of word */
		movepast(bistoken, BACKWARD);
		start = zcreatemrk();
		movepast(bistoken, FORWARD); /* move Point to end of word */
		copytomrk(start); /* copy to Kill buffer */
		bpnttomrk(tmark); /* move Point back */
		unmark(tmark);
		unmark(start);
	}
	Arg = 0;
}

void Zdelete_blanks(void)
{
	struct mark *tmark, *pmark;

	pmark = zcreatemrk();
	if (bcrsearch(Curbuff, NL)) {
		bmove1(Curbuff);
		tmark = zcreatemrk();
		movepast(bisspace, BACKWARD);
		if (!bisstart(Curbuff))
			bcsearch(Curbuff, NL);
		if (bisbeforemrk(tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	if (bcsearch(Curbuff, NL)) {
		tmark = zcreatemrk();
		movepast(bisspace, FORWARD);
		if (bcrsearch(Curbuff, NL))
			bmove1(Curbuff);
		if (bisaftermrk(tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	bpnttomrk(pmark);
	unmark(pmark);
}

void Zjoin(void)
{
	toendline(Curbuff);
	bdelete(Curbuff, 1);
	Ztrim_white_space();
	binsert(Curbuff, ' ');
}

void Zempty_buffer(void)
{
	if (ask("Empty buffer? ") != YES)
		return;
	bempty(Curbuff);
	Curbuff->bmodf = true;
}
