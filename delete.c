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
	bdelbuff(Killbuff);
	bdelbuff(Paw);
}

void delinit(void)
{
	Killbuff = bcreate();
	Paw = bcreate();
	atexit(delfini);
}

static void copytomrk(struct mark *tmark)
{
	if (delcmd())
		btoend(Killbuff);
	else
		bempty(Killbuff);
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

	if (tbuff == Bbuff)
		return 0;

	flip = bisaftermrk(Bbuff, tmark);
	if (flip)
		bswappnt(Bbuff, tmark);

	if (!(ltmrk = bcremrk(Bbuff)))
		return 0;

	sbuff = Bbuff;
	while (bisbeforemrk(Bbuff, tmark)) {
		if (Curpage == tmark->mpage)
			srclen = tmark->moffset - Curchar;
		else
			srclen = Curpage->plen - Curchar;
		spnt = Curcptr;

		bswitchto(tbuff);
		dstlen = PSIZE - Curpage->plen;
		if (dstlen == 0) {
			if (pagesplit(Bbuff, HALFP))
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
		foreach_pagemark(Bbuff, btmrk, Curpage)
			if (btmrk->moffset > Curchar)
					btmrk->moffset += dstlen;
		makeoffset(Bbuff, Curchar + dstlen);
		vsetmod();
		Bbuff->bmodf = true;
		bswitchto(sbuff);
		bmove(Bbuff, dstlen);
	}

	bpnttomrk(Bbuff, ltmrk);
	unmark(ltmrk);

	if (flip)
		bswappnt(Bbuff, tmark);

	return copied;
}

/* Delete from the point to the Mark. */
void bdeltomrk(struct mark *tmark)
{
	if (bisaftermrk(Bbuff, tmark))
		bswappnt(Bbuff, tmark);
	while (bisbeforemrk(Bbuff, tmark))
		if (Curpage == tmark->mpage)
			bdelete(Bbuff, tmark->moffset - Curchar);
		else
			bdelete(Bbuff, Curpage->plen - Curchar);
}

void killtomrk(struct mark *tmark)
{
	copytomrk(tmark);
	bdeltomrk(tmark);
}

void Zappend_kill(void) {}

void Zdelete_char(void)
{
	bdelete(Bbuff, Arg);
	Arg = 0;
}

void Zdelete_previous_char(void)
{
	bmove(Bbuff, -Arg);
	bdelete(Bbuff, Arg);
	Arg = 0;
}

void Zdelete_to_eol(void)
{
	if (!bisend(Bbuff) && Buff() == NL)
		bdelete(Bbuff, 1);
	else {
		bool atstart = bpeek(Bbuff) == NL;
		struct mark *tmark = zcreatemrk();
		toendline(Bbuff);
		if (atstart)
			bmove1(Bbuff); /* delete the NL */
		killtomrk(tmark);
		unmark(tmark);
	}
}

void Zdelete_line(void)
{
	struct mark *tmark;

	tobegline(Bbuff);
	tmark = zcreatemrk();
	bcsearch(Bbuff, NL);
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
		bempty(Bbuff);
		First = false;
	}

	mrktomrk(&save, Send);
	tbuff = Bbuff;
#if !UNDO
	/* This leaves the mark at the start of the yank and
	 * the point at the end. */
	set_umark(NULL);
#endif
	bswitchto(Killbuff);
	btoend(Bbuff);
	tmark = zcreatemrk();
	btostart(Bbuff);
#if UNDO
	undo_add(bcopyrgn(tmark, tbuff));
#else
	bcopyrgn(tmark, tbuff);
#endif
	unmark(tmark);
	bswitchto(tbuff);
	if (bisaftermrk(Bbuff, &save))
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
		zswitchto(Buff_save);
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
		bpnttomrk(Bbuff, tmark); /* move Point back */
		unmark(tmark);
		unmark(start);
	}
	Arg = 0;
}

void Zdelete_blanks(void)
{
	struct mark *tmark, *pmark;

	pmark = zcreatemrk();
	if (bcrsearch(Bbuff, NL)) {
		bmove1(Bbuff);
		tmark = zcreatemrk();
		movepast(bisspace, BACKWARD);
		if (!bisstart(Bbuff))
			bcsearch(Bbuff, NL);
		if (bisbeforemrk(Bbuff, tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	if (bcsearch(Bbuff, NL)) {
		tmark = zcreatemrk();
		movepast(bisspace, FORWARD);
		if (bcrsearch(Bbuff, NL))
			bmove1(Bbuff);
		if (bisaftermrk(Bbuff, tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	bpnttomrk(Bbuff, pmark);
	unmark(pmark);
}

void Zjoin(void)
{
	toendline(Bbuff);
	bdelete(Bbuff, 1);
	Ztrim_white_space();
	binsert(Bbuff, ' ');
}

void Zempty_buffer(void)
{
	if (ask("Empty buffer? ") != YES)
		return;
	bempty(Bbuff);
	Bbuff->bmodf = true;
}
