/* buff.c - low level buffer commands for Zedit
 * Copyright (C) 1988-2017 Sean MacLennan
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

#include "buff.h"

/** Move point to the beginning of the line. */
void tobegline(struct buff *buff)
{
	if (buff->curchar > 0 && *(buff->curcptr - 1) == '\n')
		return;
	if (bcrsearch(buff, '\n'))
		bmove1(buff);
}

/** Move point to the end of the line. */
void toendline(struct buff *buff)
{
	if (bcsearch(buff, '\n'))
		bmove(buff, -1);
}

/** Return the length of the buffer. */
unsigned long blength(struct buff *tbuff)
{
	struct page *tpage;
	unsigned long len = 0;

	for (tpage = tbuff->firstp; tpage; tpage = tpage->nextp)
		len += tpage->plen;

	return len;
}

/** Return the current position of the point as an index. */
unsigned long blocation(struct buff *buff)
{
	struct page *tpage;
	unsigned long len = 0;

	for (tpage = buff->firstp; tpage != buff->curpage; tpage = tpage->nextp)
		len += tpage->plen;

	return len + buff->curchar;
}

/** Delete all bytes from a buffer and leave it with one empty page
 * (ala bcreate()). More efficient than bdelete(blength(buff)) since it
 * works on pages rather than bytes.
 */
void bempty(struct buff *buff)
{
	struct mark *btmark;

#if HUGE_FILES
	bhugecleanup(buff);
#endif

	makecur(buff, buff->firstp, 0);
	curplen(buff) = 0;
	while (buff->curpage->nextp)
		freepage(buff, buff->curpage->nextp);

	foreach_global_buffmark(buff, btmark)
		if (btmark->mpage) {
			btmark->mpage = buff->firstp;
			btmark->moffset = 0;
		}
	foreach_buffmark(buff, btmark)
		if (btmark->mpage) {
			btmark->mpage = buff->firstp;
			btmark->moffset = 0;
		}

	undo_clear(buff);
	bsetmod(buff);
}

/** Peek the previous byte. Does not move the point. Returns LF at
 * start of buffer.
 */
Byte bpeek(struct buff *buff)
{
	Byte ch;

	if (buff->curchar > 0)
		return *(buff->curcptr - 1);
	if (bisstart(buff))
		/* Pretend we are at the start of a line.
		 * Needed for delete-to-eol and step in reg.c.
		 */
		return '\n';
	bmove(buff, -1);
	ch = *buff->curcptr;
	bmove1(buff);
	return ch;
}

/** Move the point to a given absolute offset in the buffer. */
void boffset(struct buff *buff, unsigned long offset)
{
	struct page *tpage;

	/* find the correct page */
	for (tpage = buff->firstp; tpage->nextp; tpage = tpage->nextp) {
		if (tpage->plen >= offset)
			break;
		offset -= tpage->plen;
	}

	makecur(buff, tpage, offset);
}

/** Go forward or back past a thingy */
void bmovepast(struct buff *buff, int (*pred)(int), bool forward)
{
	if (forward)
		while (!bisend(buff) && pred(*buff->curcptr))
			bmove1(buff);
	else {
		bmove(buff, -1);
		while (!bisstart(buff) && pred(*buff->curcptr))
			bmove(buff, -1);
		if (!pred(*buff->curcptr))
			bmove1(buff);
	}
}

/** Go forward or back to a thingy */
void bmoveto(struct buff *buff, int (*pred)(int), bool forward)
{
	if (forward)
		while (!bisend(buff) && !pred(*buff->curcptr))
			bmove1(buff);
	else {
		bmove(buff, -1);
		while (!bisstart(buff) && !pred(*buff->curcptr))
			bmove(buff, -1);
		if (!bisstart(buff))
			bmove1(buff);
	}
}

/** Copy from Point to mark to buffer 'to'. Returns bytes copied. */
long bcopyrgn(struct mark *tmark, struct buff *to)
{
	struct mark *ltmrk, *btmrk;
	bool flip;
	int  srclen, dstlen;
	long copied = 0;
	struct buff *from = tmark->mbuff;

	flip = bisaftermrk(from, tmark);
	if (flip)
		bswappnt(from, tmark);

	ltmrk = bcremark(from);
	if (!ltmrk)
		return 0;

	while (bisbeforemrk(from, tmark)) {
		if (from->curpage == tmark->mpage)
			srclen = tmark->moffset - from->curchar;
		else
			srclen = curplen(from) - from->curchar;

		dstlen = PGSIZE - curplen(to);
		if (dstlen == 0) {
			if (pagesplit(to, HALFP))
				dstlen = PGSIZE - curplen(to);
			else
				break;
		}
		if (srclen < dstlen)
			dstlen = srclen;
		/* Make a gap */
		memmove(to->curcptr + dstlen,
			to->curcptr,
			curplen(to) - to->curchar);
		/* and fill it in */
		memmove(to->curcptr, from->curcptr, dstlen);
		curplen(to) += dstlen;
		copied += dstlen;
		foreach_global_pagemark(btmrk, to->curpage)
			if (btmrk->moffset > to->curchar)
					btmrk->moffset += dstlen;
		foreach_pagemark(to, btmrk, to->curpage)
			if (btmrk->moffset > to->curchar)
					btmrk->moffset += dstlen;
		makeoffset(to, to->curchar + dstlen);
		bsetmod(to);
		to->bmodf = true;
		bmove(from, dstlen);
	}

	bpnttomrk(from, ltmrk);
	bdelmark(ltmrk);

	if (flip)
		bswappnt(from, tmark);

	return copied;
}

/** Delete from the point to the Mark. Returns bytes deleted. */
long bdeltomrk(struct mark *tmark)
{
	long amount, deleted = 0;
	struct buff *buff = tmark->mbuff;

	if (bisaftermrk(buff, tmark))
		bswappnt(buff, tmark);
	while (bisbeforemrk(buff, tmark)) {
		if (buff->curpage == tmark->mpage)
			amount = tmark->moffset - buff->curchar;
		else
			amount = curplen(buff) - buff->curchar;
		bdelete(buff, amount);
		deleted += amount;
	}

	return deleted;
}

/** Return the current line of the point. */
unsigned long bline(struct buff *buff)
{
	struct mark tmark;
	unsigned long line = 1;

	bmrktopnt(buff, &tmark);
	btostart(buff);
	while (bcsearch(buff, '\n') && !bisaftermrk(buff, &tmark))
		++line;
	bpnttomrk(buff, &tmark);
	return line;
}

