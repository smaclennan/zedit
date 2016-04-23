/* buff.c - low level buffer commands for Zedit
 * Copyright (C) 1988-2016 Sean MacLennan
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

/** Default #bsetmod function. Does nothing ;) */
static void dummy_bsetmod(struct buff *buff) {}
/** If you need to know when the buffer is modified, this callback
 * will get called to notify you. By default it is set to a dummy
 * function that does nothing.
 */
void (*bsetmod)(struct buff *buff) = dummy_bsetmod;

int NumBuffs; /**< Current number of buffers. Not thread safe. */
int NumPages; /**< Current number of pages. Not thread safe. */

/** Create a buffer and allocate the first page. */
struct buff *bcreate(void)
{
	struct buff *buf = (struct buff *)calloc(1, sizeof(struct buff));
	if (buf) {
		if (!(buf->firstp = newpage(NULL))) {
			/* bad news, de-allocate */
			free(buf);
			return NULL;
		}
		makecur(buf, buf->firstp, 0);
		++NumBuffs;
	}
	return buf;
}

/** Delete a buffer. Buffer can be NULL. */
void bdelbuff(struct buff *tbuff)
{
	if (!tbuff)
		return;

	while (tbuff->firstp)	/* delete the pages */
		freepage(tbuff, tbuff->firstp);

#ifdef HAVE_BUFFER_MARKS
	while (tbuff->marks) /* delete the marks */
		bdelmark(tbuff->marks);
#endif

	free(tbuff);	/* free the buffer proper */

	--NumBuffs;
}

/** Insert a byte into a buffer. Returns false if more space is needed
 * but we cannot allocate it.
 */
bool binsert(struct buff *buff, Byte byte)
{
	if (curplen(buff) == PSIZE && !pagesplit(buff, HALFP))
		return false;
	memmove(buff->curcptr + 1, buff->curcptr, curplen(buff) - buff->curchar);
	*buff->curcptr++ = byte;
	buff->curchar++;
	curplen(buff)++;
	buff->bmodf = true;

	undo_add(buff, 1, false);

	struct mark *btmark;

	foreach_global_pagemark(buff, btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);
	foreach_pagemark(buff, btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);

	bsetmod(buff);
	return true;
}

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
	if (*fmt == 'u' || *fmt == 'd') *saw_neg = 0;
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

/** Insert a string. Uses variable arguments.
 *
 * Supports a subset of printf: %%s, %%d, %%u. Format can contain a width
 * and a minus (-) for left justify.
 */
bool binstr(struct buff *buff, const char *fmt, ...)
{
	va_list ap;
	char tmp[21];
	int saw_neg, len, n;
	bool rc = true;

	va_start(ap, fmt);
	while (*fmt && rc) {
		if (*fmt == '%') {
			switch (valid_format(fmt, &saw_neg, &len, &n)) {
			case 's':
				fmt += n;
				rc = out_str(buff, va_arg(ap, char *), saw_neg, len);
				break;
			case 'd':
				fmt += n;
				snprintf(tmp, sizeof(tmp), "%d", va_arg(ap, int));
				rc = out_str(buff, tmp, saw_neg, len);
				break;
			case 'u':
				fmt += n;
				snprintf(tmp, sizeof(tmp), "%u", va_arg(ap, unsigned));
				rc = out_str(buff, tmp, saw_neg, len);
				break;
			default:
				rc = binsert(buff, *fmt);
			}
		} else
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
	struct page *tpage, *curpage = buff->curpage;

	while (quantity > 0) {
		/* Delete as many characters as possible from this page */
		quan = MIN(curplen(buff) - buff->curchar, quantity);
		if (quan < 0)
			quan = 0; /* May need to switch pages */

		undo_del(buff, quan);

		curplen(buff) -= quan;

		memmove(buff->curcptr, buff->curcptr + quan, curplen(buff) - buff->curchar);
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
			struct mark *tmark;
			foreach_global_pagemark(buff, tmark, curpage) {
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
			struct mark *tmark;
			foreach_global_pagemark(buff, tmark, curpage)
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

/** Move the point relative to its current position. The move can be
 * forward (positive dist) or backwards (negative dist). Returns true
 * if the full move was possible.
 */
bool bmove(struct buff *buff, int dist)
{
	while (dist) {
		struct page *curpage = buff->curpage;

		dist += buff->curchar;
		if (dist >= 0 && (unsigned)dist < curplen(buff)) {
			/* within current page makeoffset dist */
			makeoffset(buff, dist);
			return true;
		}
		if (dist < 0) { /* goto previous page */
			if (curpage == buff->firstp) {
				/* past start of buffer */
				makeoffset(buff, 0);
				return false;
			}
			makecur(buff, curpage->prevp, curpage->prevp->plen);
		} else {	/* goto next page */
			if (lastp(curpage)) {
				/* past end of buffer */
				makeoffset(buff, curplen(buff));
				return false;
			}
			dist -= curplen(buff); /* must use this curplen */
			makecur(buff, curpage->nextp, 0);
		}
	}
	return true;
}

/** Move point to start of buffer. */
void btostart(struct buff *buff)
{
	makecur(buff, buff->firstp, 0);
}

/** Move point to end of buffer. */
void btoend(struct buff *buff)
{
	struct page *lastp = buff->curpage->nextp;
	if (lastp) {
		while (lastp->nextp)
			lastp = lastp->nextp;
		makecur(buff, lastp, lastp->plen);
	} else
		makeoffset(buff, curplen(buff));
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

/** Search forward for a single byte `what'. If what found leaves
 * point at the byte after what and returns true. If not found leaves
 * point at the end of buffer and returns false.
 */
bool bcsearch(struct buff *buff, Byte what)
{
	Byte *n;

	if (bisend(buff))
		return false;

	while ((n = (Byte *)memchr(buff->curcptr, what, buff->curpage->plen - buff->curchar)) == NULL)
		if (lastp(buff->curpage)) {
			makeoffset(buff, buff->curpage->plen);
			return false;
		} else
			makecur(buff, buff->curpage->nextp, 0);

	makeoffset(buff, n - buff->curpage->pdata);
	bmove1(buff);
	return true;
}

/** Search backward for a single byte. If byte found leaves point at
 * byte and returns true. If not found leaves point at the start of
 * buffer and returns false.
 *
 * Not as efficient as bcsearch() since we cannot use memchr.
 */
bool bcrsearch(struct buff *buff, Byte what)
{
	while (1) {
		if (buff->curchar <= 0) {
			if (buff->curpage == buff->firstp)
				return false;
			else
				makecur(buff, buff->curpage->prevp, buff->curpage->prevp->plen - 1);
		} else {
			--buff->curchar;
			--buff->curcptr;
		}
		if (*buff->curcptr == what)
			return true;
	}
}

/** Delete all bytes from a buffer and leave it with one empty page
 * (ala bcreate()). More efficient than bdlete(blength(buff)) since it
 * works on pages rather than bytes.
 */
void bempty(struct buff *buff)
{
	makecur(buff, buff->firstp, 0);
	curplen(buff) = 0;
	while (buff->curpage->nextp)
		freepage(buff, buff->curpage->nextp);

	struct mark *btmark;

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

/** Peek the previous byte. Does not move the point. Returns LF at start of buffer. */
Byte bpeek(struct buff *buff)
{
	if (buff->curchar > 0)
		return *(buff->curcptr - 1);
	else if (bisstart(buff))
		/* Pretend we are at the start of a line.
		 * Needed for delete-to-eol and step in reg.c. */
		return '\n';
	else {
		Byte ch;
		bmove(buff, -1);
		ch = *buff->curcptr;
		bmove1(buff);
		return ch;
	}
}

/** Move the point to a given absolute offset in the buffer. */
void boffset(struct buff *buff, unsigned long offset)
{
	struct page *tpage;

	/* find the correct page */
	for (tpage = buff->firstp; tpage->nextp; tpage = tpage->nextp)
		if (tpage->plen >= offset)
			break;
		else
			offset -= tpage->plen;

	makecur(buff, tpage, offset);
}

/** Go forward or back past a thingy */
void bmovepast(struct buff *buff, int (*pred)(int), bool forward)
{
	if (!forward)
		bmove(buff, -1);
	while (!(forward ? bisend(buff) : bisstart(buff)) && pred(*buff->curcptr))
		bmove(buff, forward ? 1 : -1);
	if (!forward && !pred(*buff->curcptr))
		bmove1(buff);
}

/** Go forward or back to a thingy */
void bmoveto(struct buff *buff, int (*pred)(int), bool forward)
{
	if (!forward)
		bmove(buff, -1);
	while (!(forward ? bisend(buff) : bisstart(buff)) && !pred(*buff->curcptr))
		bmove(buff, forward ? 1 : -1);
	if (!forward && !bisstart(buff))
		bmove1(buff);
}

/* Low level memory page routines */

/** Create a new memory page and link into chain after curpage. */
struct page *newpage(struct page *curpage)
{
	struct page *page = (struct page *)calloc(1, sizeof(struct page));
	if (!page)
		return NULL;

	if (curpage) {
		page->prevp = curpage;
		page->nextp = curpage->nextp;
		if (curpage->nextp)
			curpage->nextp->prevp = page;
		curpage->nextp = page;
	}

	++NumPages;
	return page;
}

/** Split the current full page and return a new page. Leaves dist in
 * curpage. Point is moved to the new page if required.
 */
struct page *pagesplit(struct buff *buff, unsigned dist)
{
	if (dist > PSIZE) return NULL;

	struct page *curpage = buff->curpage;
	struct page *newp = newpage(curpage);
	if (!newp)
		return NULL;

	int newsize = curpage->plen - dist;
	memmove(newp->pdata, curpage->pdata + dist, newsize);
	curpage->plen = dist;
	newp->plen = newsize;

	struct mark *btmark;

	foreach_global_pagemark(buff, btmark, curpage)
		if (btmark->moffset >= dist) {
			btmark->mpage = newp;
			btmark->moffset -= dist;
		}
	foreach_pagemark(buff, btmark, curpage)
		if (btmark->moffset >= dist) {
			btmark->mpage = newp;
			btmark->moffset -= dist;
		}

	if (buff->curchar >= dist)
		/* new page has Point in it */
		makecur(buff, newp, buff->curchar - dist);

	return newp;
}

/** Free a memory page. */
void freepage(struct buff *buff, struct page *page)
{
	if (page->nextp)
		page->nextp->prevp = page->prevp;
	if (page->prevp)
		page->prevp->nextp = page->nextp;
	else
		buff->firstp = page->nextp;

	free(page);
	--NumPages;
}
