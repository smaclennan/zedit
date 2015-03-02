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
}

void delinit(void)
{
	Killbuff = bcreate();
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
/* No global */
int bcopyrgn(struct mark *tmark, struct buff *to)
{
	struct mark *ltmrk, *btmrk;
	bool flip;
	int  srclen, dstlen;
	int copied = 0;
	struct buff *from = tmark->mbuff;

	flip = bisaftermrk(from, tmark);
	if (flip)
		bswappnt(from, tmark);

	if (!(ltmrk = bcremrk(from)))
		return 0;

	while (bisbeforemrk(from, tmark)) {
		if (from->curpage == tmark->mpage)
			srclen = tmark->moffset - from->curchar;
		else
			srclen = curplen(from) - from->curchar;

		dstlen = PSIZE - curplen(to);
		if (dstlen == 0) {
			if (pagesplit(to, HALFP))
				dstlen = PSIZE - curplen(to);
			else
				break;
		}
		if (srclen < dstlen)
			dstlen = srclen;
		/* Make a gap */
		memmove(to->curcptr + dstlen, to->curcptr, curplen(to) - to->curchar);
		/* and fill it in */
		memmove(to->curcptr, from->curcptr, dstlen);
		curplen(to) += dstlen;
		copied += dstlen;
		foreach_pagemark(to, btmrk, to->curpage)
			if (btmrk->moffset > to->curchar)
					btmrk->moffset += dstlen;
		makeoffset(to, to->curchar + dstlen);
		vsetmod_callback(to);
		to->bmodf = true;
		bmove(from, dstlen);
	}

	bpnttomrk(from, ltmrk);
	unmark(ltmrk);

	if (flip)
		bswappnt(from, tmark);

	return copied;
}

/* Delete from the point to the Mark. */
void bdeltomrk(struct mark *tmark)
{
	if (bisaftermrk(Bbuff, tmark))
		bswappnt(Bbuff, tmark);
	while (bisbeforemrk(Bbuff, tmark))
		if (Bbuff->curpage == tmark->mpage)
			bdelete(Bbuff, tmark->moffset - Bbuff->curchar);
		else
			bdelete(Bbuff, curplen(Bbuff) - Bbuff->curchar);
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
	struct mark *tmark, save;	/* save must NOT be a pointer */

	if (InPaw && First) {
		bempty(Bbuff);
		First = false;
	}

	mrktomrk(&save, Send);
#if !UNDO
	/* This leaves the mark at the start of the yank and
	 * the point at the end. */
	set_umark(NULL);
#endif
	btoend(Killbuff);
	if (!(tmark = bcremrk(Killbuff))) {
		error("out of memory");
		return;
	}
	btostart(Killbuff);
#if UNDO
	undo_add(bcopyrgn(tmark, Bbuff));
#else
	bcopyrgn(tmark, Bbuff);
#endif
	unmark(tmark);
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
		zswitchto(Paw);
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
