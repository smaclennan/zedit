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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "buff.h"

/** Helper function for binstr(). */
static char valid_format(const char *fmt, int *saw_neg, int *len, int *n)
{
	*saw_neg = *len = *n = 0;
	++fmt; ++*n; /* skip % */
	if (*fmt == '-') {
		*saw_neg = 1;
		++fmt; ++*n;
	}
	while (isdigit(*fmt)) {
		*len = *len * 10 + *fmt - '0';
		++fmt; ++*n;
	}
	if (*fmt == 'u' || *fmt == 'd')
		*saw_neg = 0;
	return *fmt;
}

/** Helper function for binstr(). */
static bool out_str(struct buff *buff, const char *s, int saw_neg, int len)
{
	int slen = strlen(s);

	if (saw_neg == 0)
		while (slen++ < len)
			if (!binsert(buff, ' '))
				return false;
	while (*s)
		if (!binsert(buff, *s++))
			return false;
	while (slen++ < len)
		if (!binsert(buff, ' '))
			return false;
	return true;
}

static bool handle_format(struct buff *buff, const char **fmt, va_list ap)
{
	char tmp[21];
	int saw_neg, len, n;

	switch (valid_format(*fmt, &saw_neg, &len, &n)) {
	case 's':
		*fmt += n;
		return out_str(buff, va_arg(ap, char *), saw_neg, len);
	case 'd':
		*fmt += n;
		snprintf(tmp, sizeof(tmp), "%d", va_arg(ap, int));
		return out_str(buff, tmp, saw_neg, len);
	case 'u':
		*fmt += n;
		snprintf(tmp, sizeof(tmp), "%u", va_arg(ap, unsigned));
		return out_str(buff, tmp, saw_neg, len);
	default:
		return binsert(buff, **fmt);
	}
}

/** Insert a string. Uses variable arguments.
 *
 * Supports a subset of printf: %%s, %%d, %%u. Format can contain a width
 * and a minus (-) for left justify.
 */
bool binstr(struct buff *buff, const char *fmt, ...)
{
	va_list ap;
	bool rc = true;

	va_start(ap, fmt);
	while (*fmt && rc) {
		if (*fmt == '%')
			rc = handle_format(buff, &fmt, ap);
		else
			rc = binsert(buff, *fmt);
		++fmt;
	}
	va_end(ap);

	return rc;
}

/** Delete quantity characters from the buffer at the current point. */
void bdelete(struct buff *buff, unsigned quantity)
{
	unsigned quan, noffset;
	struct mark *tmark;
	struct page *tpage, *curpage = buff->curpage;

	while (quantity > 0) {
		/* Delete as many characters as possible from this page */
		quan = MIN(curplen(buff) - buff->curchar, quantity);

		undo_del(buff, quan);

		curplen(buff) -= quan;

		memmove(buff->curcptr,
			buff->curcptr + quan,
			curplen(buff) - buff->curchar);
		if (lastp(curpage))
			quantity = 0;
		else
			quantity -= quan;
		buff->bmodf = true;
		if (curplen(buff) == 0 && (curpage->nextp || curpage->prevp)) {
			/* We deleted entire page. */
			tpage = curpage->nextp;
			noffset = 0;
			if (tpage == NULL) {
				tpage = curpage->prevp;
				noffset = tpage->plen;
			}
			foreach_global_pagemark(tmark, curpage) {
				tmark->mpage = tpage;
				tmark->moffset = noffset;
			}
			foreach_pagemark(buff, tmark, curpage) {
				tmark->mpage = tpage;
				tmark->moffset = noffset;
			}
			freepage(buff, curpage);
		} else {
			tpage = curpage;
			noffset = buff->curchar;
			if ((noffset >= curplen(buff)) && curpage->nextp) {
				tpage = curpage->nextp;
				noffset = 0;
			}
			foreach_global_pagemark(tmark, curpage)
				if (tmark->moffset >= buff->curchar) {
					if (tmark->moffset >= buff->curchar + quan)
						tmark->moffset -= quan;
					else {
						tmark->mpage = tpage;
						tmark->moffset = noffset;
					}
				}
			foreach_pagemark(buff, tmark, curpage)
				if (tmark->moffset >= buff->curchar) {
					if (tmark->moffset >= buff->curchar + quan)
						tmark->moffset -= quan;
					else {
						tmark->mpage = tpage;
						tmark->moffset = noffset;
					}
				}
		}
		makecur(buff, tpage, noffset);
	}
	bsetmod(buff);
}

/** Move point to end of buffer. */
void btoend(struct buff *buff)
{
	struct page *lastp;

	/* For huge files we don't want to make every page current */
	for (lastp = buff->curpage; lastp->nextp; lastp = lastp->nextp)
		;
	makecur(buff, lastp, lastp->plen);
}

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

