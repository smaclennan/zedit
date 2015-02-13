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

#include "page.h" /* for bcopyrgn */


static struct buff *Killbuff;

static void delfini(void)
{
	bdelbuff(Killbuff);
	bdelbuff(Paw);
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
		btoend();
	else
		bempty();
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
#ifdef DOS_EMS
	Byte spnt[PSIZE];
#else
	Byte *spnt;
#endif
	int copied = 0;

	if (tbuff == Curbuff)
		return 0;

	flip = bisaftermrk(tmark);
	if (flip)
		bswappnt(tmark);

	if (!(ltmrk = bcremrk()))
		return 0;

	sbuff = Curbuff;
	while (bisbeforemrk(tmark)) {
		if (Curpage == tmark->mpage)
			srclen = tmark->moffset - Curchar;
		else
			srclen = Curpage->plen - Curchar;
		Curmodf = true;
#ifdef DOS_EMS
		memcpy(spnt, Curcptr, srclen);
#else
		spnt = Curcptr;
#endif

		bswitchto(tbuff);
		dstlen = PSIZE - Curpage->plen;
		if (dstlen == 0) {
			if (bpagesplit(Curbuff))
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
		for (btmrk = Mrklist; btmrk; btmrk = btmrk->prev)
			if (btmrk->mpage == Curpage &&
				btmrk->moffset > Curchar)
					btmrk->moffset += dstlen;
		makeoffset(Curbuff, Curchar + dstlen);
		vsetmod(false);
		Curmodf = true;
		Curbuff->bmodf = true;
		bswitchto(sbuff);
		bmove(dstlen);
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
			bdelete(tmark->moffset - Curchar);
		else
			bdelete(Curpage->plen - Curchar);
}

void killtomrk(struct mark *tmark)
{
	copytomrk(tmark);
	bdeltomrk(tmark);
}

void Zappend_kill(void) {}

void Zdelete_char(void)
{
	bdelete(Arg);
	Arg = 0;
}

void Zdelete_previous_char(void)
{
	bmove(-Arg);
	bdelete(Arg);
	Arg = 0;
}

void Zdelete_to_eol(void)
{
	if (!bisend() && Buff() == NL)
		bdelete(1);
	else {
		bool atstart = _bpeek(Curbuff) == NL;
		struct mark *tmark = zcreatemrk();
		toendline();
		if (atstart)
			bmove1(); /* delete the NL */
		killtomrk(tmark);
		unmark(tmark);
	}
}

void Zdelete_line(void)
{
	struct mark *tmark;

	tobegline();
	tmark = zcreatemrk();
	bcsearch(NL);
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
		bempty();
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
	btoend();
	tmark = zcreatemrk();
	btostart();
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
	if (bcrsearch(NL)) {
		bmove1();
		tmark = zcreatemrk();
		movepast(bisspace, BACKWARD);
		if (!bisstart())
			bcsearch(NL);
		if (bisbeforemrk(tmark))
			bdeltomrk(tmark);
		unmark(tmark);
	}
	if (bcsearch(NL)) {
		tmark = zcreatemrk();
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
	Ztrim_white_space();
	binsert(' ');
}

void Zempty_buffer(void)
{
	if (ask("Empty buffer? ") != YES)
		return;
	bempty();
	Curbuff->bmodf = true;
}
