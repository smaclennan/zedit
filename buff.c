/* buff.c - low level buffer commands for Zedit
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include "buff.h"
#ifdef HAVE_MARKS
#include "mark.h"
#endif

static void dummy_bsetmod(struct buff *buff) {}
void (*bsetmod)(struct buff *buff) = dummy_bsetmod;

/* If set, this function will be called on bdelbuff */
void (*app_cleanup)(struct buff *buff);

int NumBuffs;
int NumPages;

/* Create a buffer but don't add it to the buffer list. */
struct buff *bcreate(void)
{
	struct buff *buf = (struct buff *)calloc(1, sizeof(struct buff));
	if (buf) {
		struct page *fpage;

		if (!(fpage = newpage(NULL))) {
			/* bad news, de-allocate */
			free(buf);
			return NULL;
		}
		buf->firstp = fpage;
		makecur(buf, fpage, 0);
		++NumBuffs;
	}

	return buf;
}

void bdelbuff(struct buff *tbuff)
{
	if (!tbuff)
		return;

	if (app_cleanup)
		app_cleanup(tbuff);

	while (tbuff->firstp)	/* delete the pages */
		freepage(tbuff, tbuff->firstp);

#if defined(HAVE_MARKS) && !defined(HAVE_GLOBAL_MARKS)
	while (tbuff->marks) /* delete the marks */
		unmark(tbuff->marks);
#endif

	free((char *)tbuff);	/* free the buffer proper */

	--NumBuffs;
}

/* Insert a character in the current buffer. */
bool binsert(struct buff *buff, Byte byte)
{
	if (curplen(buff) == PSIZE && !pagesplit(buff, HALFP))
		return false;
	memmove(buff->curcptr + 1, buff->curcptr, curplen(buff) - buff->curchar);
	*buff->curcptr++ = byte;
	buff->curchar++;
	curplen(buff)++;
	buff->bmodf = true;

#if UNDO
	undo_add(1);
#endif

#ifdef HAVE_MARKS
	struct mark *btmark;

	foreach_pagemark(buff, btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);
#endif

	bsetmod(false);
	return true;
}

/* Delete quantity characters. */
void bdelete(struct buff *buff, int quantity)
{
	int quan, noffset;
	struct page *tpage, *curpage = buff->curpage;

	while (quantity) {
		/* Delete as many characters as possible from this page */
		quan = MIN(curplen(buff) - buff->curchar, quantity);
		if (quan < 0)
			quan = 0; /* May need to switch pages */

#if UNDO
		undo_del(quan);
#endif

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
#ifdef HAVE_MARKS
			struct mark *tmark;
			foreach_pagemark(buff, tmark, curpage) {
				tmark->mpage = tpage;
				tmark->moffset = noffset;
			}
#endif
			freepage(buff, curpage);
		} else {
			tpage = curpage;
			noffset = buff->curchar;
			if ((noffset >= curplen(buff)) && curpage->nextp) {
				tpage = curpage->nextp;
				noffset = 0;
			}
#ifdef HAVE_MARKS
			struct mark *tmark;
			foreach_pagemark(buff, tmark, curpage)
				if (tmark->moffset >= buff->curchar) {
					if (tmark->moffset >= buff->curchar + quan)
						tmark->moffset -= quan;
					else {
						tmark->mpage = tpage;
						tmark->moffset = noffset;
					}
				}
#endif
		}
		makecur(buff, tpage, noffset);
	}
	bsetmod(buff);
}

/* Move the point relative to its current position.
 *
 * This routine is the most time-consuming routine in the editor.
 * Because of this, it is highly optimized. makeoffset() calls have
 * been inlined here.
 *
 * Since bmove(1) is used the most, a special call has been made.
 */
bool bmove(struct buff *buff, int dist)
{
	while (dist) {
		struct page *curpage = buff->curpage;

		dist += buff->curchar;
		if (dist >= 0 && dist < curplen(buff)) {
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

void bmove1(struct buff *buff)
{
	if (++buff->curchar < buff->curpage->plen)
		/* within current page */
		++buff->curcptr;
	else if (buff->curpage->nextp)
		/* goto start of next page */
		makecur(buff, buff->curpage->nextp, 0);
	else
		/* Already at EOB */
		makeoffset(buff, curplen(buff));
}

bool bisstart(struct buff *buff)
{
	return (buff->curpage == buff->firstp) && (buff->curchar == 0);
}

bool bisend(struct buff *buff)
{
	return lastp(buff->curpage) && (buff->curchar >= curplen(buff));
}

void btostart(struct buff *buff)
{
	makecur(buff, buff->firstp, 0);
}

void btoend(struct buff *buff)
{
	struct page *lastp = buff->curpage->nextp;
	if (lastp) {
		while (lastp->nextp)
			lastp = lastp->nextp;
		makecur(buff, lastp, lastp->plen);
	} else
		makeoffset(buff, buff->curpage->plen);
}

void tobegline(struct buff *buff)
{
	if (buff->curchar > 0 && *(buff->curcptr - 1) == '\n')
		return;
	if (bcrsearch(buff, '\n'))
		bmove1(buff);
}

void toendline(struct buff *buff)
{
	if (bcsearch(buff, '\n'))
		bmove(buff, -1);
}

/* Returns the length of the buffer. */
unsigned long blength(struct buff *tbuff)
{
	struct page *tpage;
	unsigned long len = 0;

	for (tpage = tbuff->firstp; tpage; tpage = tpage->nextp)
		len += tpage->plen;

	return len;
}

/* Return the current position of the point. */
unsigned long blocation(struct buff *buff)
{
	struct page *tpage;
	unsigned long len = 0;

	for (tpage = buff->firstp; tpage != buff->curpage; tpage = tpage->nextp)
		len += tpage->plen;

	return len + buff->curchar;
}

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

void bempty(struct buff *buff)
{
	makecur(buff, buff->firstp, 0);
	curplen(buff) = 0;
	while (buff->curpage->nextp)
		freepage(buff, buff->curpage->nextp);

#ifdef HAVE_MARKS
	struct mark *btmark;

	foreach_buffmark(buff, btmark)
		if (btmark->mpage) {
			btmark->mpage = buff->firstp;
			btmark->moffset = 0;
		}
#endif

#if UNDO
	undo_clear(buff);
#endif
	bsetmod(buff);
}

/* Peek the previous byte */
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

/* You must guarantee we are at the end of the page */
static int bappendpage(struct buff *buff, Byte *data, int size)
{
	int appended = 0;

	/* Fill the current page */
	int n, left = PSIZE - curplen(buff);
	if (left > 0) {
		n = MIN(left, size);
		memcpy(buff->curcptr, data, n);
		buff->curcptr += n;
		buff->curchar += n;
		curplen(buff) += n;
		size -= n;
		data += n;
		appended += n;
	}

	/* Put the rest in new pages */
	while (size > 0) {
		struct page *npage = newpage(buff->curpage);
		if (!npage)
			return appended;
		makecur(buff, npage, 0);

		n = MIN(PSIZE, size);
		memcpy(buff->curcptr, data, n);
		curplen(buff) = n;
		makeoffset(buff, n);
		size -= n;
		data += n;
		appended += n;
	}

	return appended;
}

int bappend(struct buff *buff, Byte *data, int size)
{
	btoend(buff);
	return bappendpage(buff, data, size);
}

/* Simple version to start.
 * Can use size / PSIZE + 1 + 1 pages.
 */
int bindata(struct buff *buff, Byte *data, int size)
{
	struct page *npage;
	int n, copied = 0;

	/* If we can append... use append */
	if (buff->curchar == curplen(buff))
		return bappendpage(buff, data, size);

	n = PSIZE - curplen(buff);
	if (n >= size) {
		/* fits in this page */
		n = curplen(buff) - buff->curchar;
		memmove(buff->curcptr + size, buff->curcptr, n);
		memcpy(buff->curcptr, data, size);
#ifdef HAVE_MARKS
		struct mark *m;
		foreach_pagemark(buff, m, buff->curpage)
			if (m->moffset >= buff->curchar)
				m->moffset += size;
#endif
		buff->curcptr += size;
		buff->curchar += size;
		curplen(buff) += size;
		return size;
	}

	n = curplen(buff) - buff->curchar;
	if (n > 0) {
		if (!pagesplit(buff, buff->curchar))
			return copied;

		/* Copy as much as possible to the end of this page */
		n = MIN(PSIZE - buff->curchar, size);
		memcpy(buff->curcptr, data, n);
		data += n;
		size -= n;
		copied += n;
		buff->curcptr += n;
		buff->curchar += n;
		curplen(buff) = buff->curchar;
	}

	while (size > 0) {
		if (!(npage = newpage(buff->curpage)))
			break;

		n = MIN(PSIZE, size);
		memcpy(npage->pdata, data, n);
		data += n;
		size -= n;
		copied += n;

		makecur(buff, npage, n);
		curplen(buff) = n;
	}

	return copied;
}

#define BREAD_THRESHOLD (PSIZE / 4)

/* Simple version. Can do multiple reads! */
int bread(struct buff *buff, int fd, int size)
{
	int n, ret = 0, left = PSIZE - curplen(buff);

	/* try to fill current page */
	if (left >= size || left > BREAD_THRESHOLD) {
		if (buff->curchar != curplen(buff)) {
			/* Move data after point into new page */
			if (!pagesplit(buff, buff->curchar))
				return -ENOMEM;
			left = PSIZE - buff->curchar;
		}
		n = read(fd, buff->curcptr, MIN(size, left));
		if (n <= 0)
			return n;
		buff->curchar += n;
		buff->curcptr += n;
		curplen(buff) += n;
		ret += n;
		size -= n;
	}

	/* now use full pages */
	while (size > 0) {
		struct page *npage = newpage(buff->curpage);
		if (!npage)
			return ret > 0 ? ret : -ENOMEM;
		n = read(fd, npage->pdata, MIN(size, PSIZE));
		if (n > 0) {
			makecur(buff, npage, n);
			curplen(buff) = n;
		} else {
			freepage(buff, npage);
			return ret > 0 ? ret : n;
		}
	}

	return ret;
}

/* Writes are easy! Leaves the point at the end of the write. */
int bwrite(struct buff *buff, int fd, int size)
{
	struct iovec iovs[MAX_IOVS];
	struct page *pg;
	int i, n;

	int have = curplen(buff) - buff->curchar;
	iovs[0].iov_base = buff->curcptr;
	iovs[0].iov_len = MIN(have, size);
	size -= iovs[0].iov_len;

	for (pg = buff->curpage->nextp, i = 1;
		 i < MAX_IOVS && size > 0 && pg;
		 ++i, pg = pg->nextp) {
		iovs[i].iov_base = pg->pdata;
		iovs[i].iov_len = MIN(pg->plen, size);
		size -= iovs[i].iov_len;
	}

	do
		n = writev(fd, iovs, i);
	while (n < 0 && errno == EINTR);

	if (n > 0)
		bmove(buff, n);

	return n;
}

/* Low level memory page routines */

/* Make page current at dist */
void makecur(struct buff *buff, struct page *page, int dist)
{
	buff->curpage = page;
	buff->curchar = dist;
	buff->curcptr = page->pdata + dist;
}

/* Create a new memory page and link into chain after curpage */
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

/* Split the current full page. Leaves dist in curpage. */
struct page *pagesplit(struct buff *buff, int dist)
{
	if (dist < 0 || dist > PSIZE) return NULL;

	struct page *curpage = buff->curpage;
	struct page *newp = newpage(curpage);
	if (!newp)
		return NULL;

	int newsize = curpage->plen - dist;
	memmove(newp->pdata, curpage->pdata + dist, newsize);
	curpage->plen = dist;
	newp->plen = newsize;

#if HAVE_MARKS
	struct mark *btmark;

	foreach_pagemark(buff, btmark, curpage)
		if (btmark->moffset >= dist) {
			btmark->mpage = newp;
			btmark->moffset -= dist;
		}
#endif

	if (buff->curchar >= dist)
		/* new page has Point in it */
		makecur(buff, newp, buff->curchar - dist);

	return newp;
}

/* Free a memory page */
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
