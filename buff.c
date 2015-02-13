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

#include "buff.h"
#include "mark.h"
#include "page.h"

#ifdef ZEDIT
#include "z.h"
#else
static inline void vsetmod(bool flag) {}
static inline void undo_add(int size) {}
static inline void undo_clear(struct buff *buff) {}
#endif

#if ZLIB
#undef Byte
#include <zlib.h>

#define bread(a, b, c) gzread(gz, b, c)
#define bclose(a) gzclose(gz)
#else
#define bread(a, b, c) read(a, b, c)
#define bclose(a) close(a)
#endif

#ifndef CRLF
#define COMPRESSED			0x0008
#define CRLF				0x0010
#endif

/* If set, this function will be called on bdelbuff */
void (*app_cleanup)(struct buff *buff);

bool Curmodf;		/* page modified?? */
struct buff *Curbuff;		/* the current buffer */

struct buff *Bufflist;		/* the buffer list */

#define Cpstart (Curbuff->curpage->pdata)

int raw_mode;

/* stats only */
int NumBuffs;
int NumPages;

static struct page *newpage(struct page *curpage);
static void freepage(struct page **firstp, struct page *page);

/* Create a buffer but don't add it to the buffer list. */
struct buff *_bcreate(void)
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
		buf->curpage = fpage;
		makecur(buf, fpage, 0);
		++NumBuffs;
	}

	return buf;
}

void _bdelbuff(struct buff *tbuff)
{
	if (!tbuff)
		return;

	if (tbuff->fname)
		free(tbuff->fname);
	if (tbuff->bname)
		free(tbuff->bname);
	if (tbuff->app && app_cleanup)
		app_cleanup(tbuff);

	while (tbuff->firstp)	/* delete the pages */
		freepage(&tbuff->firstp, tbuff->firstp);

	free((char *)tbuff);	/* free the buffer proper */

	--NumBuffs;
}

#ifndef HAVE_THREADS
static void bfini(void)
{
	Curbuff = NULL;

	while (Bufflist)
		/* bdelbuff will update Bufflist */
		bdelbuff(Bufflist);

#ifdef DOS_EMS
	ems_free();
#endif
}

static void binit(void)
{
	static int binitialized = 0;

	if (!binitialized) {
#ifdef DOS_EMS
		ems_init();
#endif
		atexit(bfini);
		binitialized = 1;
	}
}

/* Create a buffer. Returns a pointer to the buffer descriptor. */
struct buff *bcreate(void)
{
	struct buff *buf;

	binit();

	if ((buf = _bcreate())) {
		/* add the buffer to the head of the list */
		if (Bufflist)
			Bufflist->prev = buf;
		buf->next = Bufflist;
		Bufflist = buf;
	}

	return buf;
}

/* Delete the buffer and its pages. */
bool bdelbuff(struct buff *tbuff)
{
	if (!tbuff)
		return true;

	if (tbuff == Curbuff) { /* switch to a safe buffer */
		if (tbuff->next)
			bswitchto(tbuff->next);
		else if (tbuff->prev)
			bswitchto(tbuff->prev);
		else
			return false;
	}

	if (tbuff == Bufflist)
		Bufflist = tbuff->next;
	if (tbuff->prev)
		tbuff->prev->next = tbuff->next;
	if (tbuff->next)
		tbuff->next->prev = tbuff->prev;

	_bdelbuff(tbuff);

	return true;
}
#endif

/* Insert a character in the current buffer. */
bool _binsert(struct buff *buff, Byte byte)
{
	if (curplen(buff) == PSIZE && !bpagesplit(buff))
		return false;
	memmove(buff->curcptr + 1, buff->curcptr, curplen(buff) - buff->curchar);
	*buff->curcptr++ = byte;
	buff->curchar++;
	curplen(buff)++;
	buff->bmodf = true;
	Curmodf = true;

	undo_add(1);

#ifdef HAVE_MARKS
	struct mark *btmark;

	foreach_pagemark(btmark, buff->curpage)
		if (btmark->moffset >= buff->curchar)
			++(btmark->moffset);
#endif

	vsetmod(false);
	return true;
}

bool bappend(Byte *data, int size)
{
	btoend();

	/* Fill the current page */
	int n, left = PSIZE - Curpage->plen;
	if (left > 0) {
		n = MIN(left, size);
		memcpy(Curcptr, data, n);
		Curpage->plen += n;
		size -= n;
		data += n;
		Curmodf = true;
	}

	/* Put the rest in new pages */
	while (size > 0) {
		struct page *npage = newpage(Curpage);
		if (!npage)
			return false;
		makecur(Curbuff, npage, 0);

		n = MIN(PSIZE, size);
		memcpy(Cpstart, data, n);
		Curpage->plen = n;
		size -= n;
		data += n;
		Curmodf = true;
	}

	btoend();
	return true;
}

/* Simple version to start.
 * Can use size / PSIZE + 1 + 1 pages.
 */
int bindata(Byte *data, int size)
{
	struct page *npage;
	int copied = 0;

	int n = Curpage->plen - Curchar;
	if (n > 0) {
		struct mark *btmark;

		/* Make a new page and move the end of this page to the new page */
		if (!(npage = newpage(Curpage)))
			return 0;
		memcpy(npage->pdata, Curcptr, n);
		npage->plen = n;

#ifdef HAVE_MARKS
		/* Fix marks that are now in new page */
		foreach_pagemark(btmark, Curpage)
			if (btmark->moffset >= Curchar) {
				btmark->mpage = npage;
				btmark->moffset -= n;
			}
#endif

		/* Copy as much as possible to the end of this page */
		n = MIN(n, size);
		memcpy(Curcptr, data, n);
		data += n;
		size -= n;
		copied += n;
		Curcptr += n;
		Curchar += n;
		Curpage->plen = Curchar;
	}

	while (size > 0) {
		if (!(npage = newpage(Curpage)))
			break;

		n = MIN(PSIZE, size);
		memcpy(npage->pdata, data, n);
		data += n;
		size -= n;
		copied += n;

		makecur(Curbuff, npage, n);

		Curpage->plen = n;
	}

	return copied;
}

/* Delete quantity characters. */
void _bdelete(struct buff *buff, int quantity)
{
	int quan, noffset;
	struct page *tpage, *curpage = buff->curpage;
	struct mark *tmark;

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
		Curmodf = true;
		if (curplen(buff) == 0 && (curpage->nextp || curpage->prevp)) {
			/* We deleted entire page. */
			tpage = curpage->nextp;
			noffset = 0;
			if (tpage == NULL) {
				tpage = curpage->prevp;
				noffset = tpage->plen;
			}
#ifdef HAVE_MARKS
			foreach_pagemark(tmark, curpage) {
				tmark->mpage = tpage;
				tmark->moffset = noffset;
			}
#endif
			freepage(&buff->firstp, curpage);
		} else {
			tpage = curpage;
			noffset = buff->curchar;
			if ((noffset >= curplen(buff)) && curpage->nextp) {
				tpage = curpage->nextp;
				noffset = 0;
			}
#ifdef HAVE_MARKS
			foreach_pagemark(tmark, curpage)
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
	vsetmod(true);
}

bool _bcrsearch(struct buff *buff, Byte what)
{
	while (1) {
		if (buff->curchar <= 0) {
			if (buff->curpage == buff->firstp)
				return false;
			else
				makecur(buff, buff->curpage->prevp, buff->curpage->plen - 1);
		} else {
			--buff->curchar;
			--buff->curcptr;
		}
		if (*buff->curcptr == what)
			return true;
	}
}

bool _bcsearch(struct buff *buff, Byte what)
{
	Byte *n;

	if (_bisend(buff))
		return false;

	while ((n = (Byte *)memchr(buff->curcptr, what, buff->curpage->plen - buff->curchar)) == NULL)
		if (lastp(buff->curpage)) {
			makeoffset(buff, buff->curpage->plen);
			return false;
		} else
			makecur(buff, buff->curpage->nextp, 0);

	makeoffset(buff, n - buff->curpage->pdata);
	_bmove1(buff);
	return true;
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
unsigned long blocation(void)
{
	struct page *tpage;
	unsigned long len = 0;

	for (tpage = Curbuff->firstp; tpage != Curpage; tpage = tpage->nextp)
		len += tpage->plen;

	return len + Curchar;
}

#define MAXMOVE		(0x7fff - 1024)

void boffset(unsigned long off)
{	/* This works even if int is 16 bits */
	btostart();
	for (; off > MAXMOVE; off -= MAXMOVE)
		bmove(MAXMOVE);
	bmove(off);
}

/* Move the point relative to its current position.
 *
 * This routine is the most time-consuming routine in the editor.
 * Because of this, it is highly optimized. makeoffset() calls have
 * been inlined here.
 *
 * Since bmove(1) is used the most, a special call has been made.
 */
bool _bmove(struct buff *buff, int dist)
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
			makecur(buff, curpage->prevp, curplen(buff));
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

void _bmove1(struct buff *buff)
{
	if (++buff->curchar < buff->curpage->plen)
		/* within current page */
		++buff->curcptr;
	else if (buff->curpage->nextp)
		/* goto start of next page */
		makecur(buff, buff->curpage->nextp, 0);
	else
		/* Already at EOB */
		--buff->curchar;
}

void bempty(void)
{
	struct mark *btmark;

	makecur(Curbuff, Curbuff->firstp, 0);
	while (Curpage->nextp)
		freepage(&Curbuff->firstp, Curpage->nextp);
#ifdef HAVE_MARKS
	for (btmark = Mrklist; btmark; btmark = btmark->prev)
		if (btmark->mpage && btmark->mbuff == Curbuff) {
			btmark->mpage = Curpage;
			btmark->moffset = 0;
		}
#endif
	Curpage->plen = Curchar = 0;		/* reset to start of page */
	Curcptr = Cpstart;
	Curmodf = true;

	undo_clear(Curbuff);
	vsetmod(true);
}

void bswitchto(struct buff *buf)
{
	if (buf && buf != Curbuff) {
		Curbuff = buf;
		makecur(Curbuff, buf->curpage, buf->curchar);
	}
}


/* Set the point to the start of the buffer. */
void _btostart(struct buff *buff)
{
	makecur(buff, buff->firstp, 0);
}

/* Set the point to the end of the buffer */
void _btoend(struct buff *buff)
{
	struct page *lastp = buff->curpage->nextp;
	if (lastp) {
		while (lastp->nextp)
			lastp = lastp->nextp;
		makecur(buff, lastp, lastp->plen);
	} else
		makeoffset(buff, buff->curpage->plen);
}

static void crfixup(void)
{
	char *p = (char *)memchr(Cpstart + 1, '\n', Curpage->plen - 1);
	if (!p)
		return;

	if (*(p - 1) != '\r')
		return;

	if (raw_mode)
		return;

	Curbuff->bmode |= CRLF;

	while (bcsearch('\r'))
		if (*Curcptr == '\n') {
			bmove(-1);
			bdelete(1);
		}

	btostart();
}

/*
 * Load the file 'fname' into the current buffer.
 * Returns  0  successfully opened file
 * > 0 (errno) on error
 * -1 on gzdopen error
 */
int breadfile(const char *fname)
{
	char buf[PSIZE];
	struct stat sbuf;
	int fd, len;

	fd = open(fname, O_RDONLY | O_BINARY);
	if (fd < 0)
		return errno;

	if (fstat(fd, &sbuf) == 0)
		Curbuff->mtime = sbuf.st_mtime;
	else
		Curbuff->mtime = -1;

	bempty();

#if ZLIB
	gzFile gz = gzdopen(fd, "rb");
	if (!gz) {
		close(fd);
		return -1;
	}

	/* Ubuntu 12.04 has a bug where zero length files are reported as
	 * compressed.
	 */
	if (sbuf.st_size && gzdirect(gz) == 0)
		Curbuff->bmode |= COMPRESSED;
#endif

	while ((len = bread(fd, buf, PSIZE)) > 0) {
		Curmodf = true;
		if (Curpage->plen) {
			if (!newpage(Curpage)) {
				bempty();
				bclose(fd);
				return ENOMEM;
			}
			makecur(Curbuff, Curpage->nextp, 0);
		}
		memcpy(Curcptr, buf, len);
		Curcptr += len;
		Curchar += len;
		Curpage->plen += len;
	}
	(void)bclose(fd);

	btostart();

	if (Curpage->plen && !(Curbuff->bmode & COMPRESSED))
		crfixup();

	Curbuff->bmodf = false;

	return 0;
}

#if ZLIB
static bool bwritegzip(int fd)
{
	struct page *tpage;
	int status = true;

	gzFile gz = gzdopen(fd, "wb");
	if (!gz) {
		close(fd);
		return false;
	}

	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int n = gzwrite(gz, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	gzclose(gz); /* also closes fd */

	return status;
}
#endif

#if HAVE_MARKS
static bool bwritefd(int fd)
{
	struct mark smark;
	struct page *tpage;
	int n, status = true;

	bmrktopnt(&smark);
	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			makecur(Curbuff, tpage, 0); /* DOS_EMS requires */
			n = write(fd, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	close(fd);

	bpnttomrk(&smark);
	return status;
}

static bool bwritedos(int fd)
{
	struct mark smark;
	struct page *tpage;
	int i, n, status = true;
	Byte buf[PSIZE * 2], *p;

	bmrktopnt(&smark);
	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int len = tpage->plen;
			makecur(Curbuff, tpage, 0); /* DOS_EMS requires */
			p = buf;
			for (i = 0; i < tpage->plen; ++i) {
				if (tpage->pdata[i] == '\n') {
					*p++ = '\r';
					++len;
				}
				*p++ = tpage->pdata[i];
			}

			n = write(fd, buf, len);
			status = n == len;
		}

	close(fd);

	bpnttomrk(&smark);
	return status;
}

/*	Write the current buffer to the file 'fname'.
 *	Handles the backup scheme according to VAR(VBACKUP).
 *	Returns:	true	if write successful
 *				false	if write failed
 */
bool bwritefile(char *fname)
{
	static int Cmask;
	int fd, mode, status = true;
	struct stat sbuf;

	if (!fname)
		return true;

	/* If the file existed, check to see if it has been modified. */
	if (Curbuff->mtime && stat(fname, &sbuf) == 0) {
		if (sbuf.st_mtime > Curbuff->mtime) {
			/* file has been modified */
		}
		mode  = sbuf.st_mode;
	} else {
		if (Cmask == 0) {
			Cmask = umask(0);	/* get the current umask */
			umask(Cmask);		/* set it back */
			Cmask = ~Cmask & 0666;	/* make it usable */
		}
		mode  = Cmask;
	}

	/* Write the output file */
	fd = open(fname, WRITE_MODE, mode);
	if (fd != EOF) {
#if ZLIB
		if (Curbuff->bmode & COMPRESSED)
			status = bwritegzip(fd);
		else
#endif
			if (Curbuff->bmode & CRLF)
				status = bwritedos(fd);
			else
				status = bwritefd(fd);
	} else
		status = false;

	/* cleanup */
	if (status) {
		if (stat(fname, &sbuf) == 0)
			Curbuff->mtime = sbuf.st_mtime;
		else
			Curbuff->mtime = -1;
		Curbuff->bmodf = false;
	}

	return status;
}
#endif

/* Make page current at dist */
void makecur(struct buff *buff, struct page *page, int dist)
{
	if (buff->curpage != page) {
#ifdef DOS_EMS
		ems_makecur(page, Curmodf);
#endif
		Curmodf = false;
		buff->curpage = page;
	}

	buff->curchar = dist;
	buff->curcptr = page->pdata + dist;
}

bool _bisstart(struct buff *buff)
{
	return (buff->curpage == buff->firstp) && (buff->curchar == 0);
}

bool _bisend(struct buff *buff)
{
	return lastp(buff->curpage) && (buff->curchar >= curplen(buff));
}

/* Peek the previous byte */
Byte bpeek(void)
{
	if (Curchar > 0)
		return *(Curcptr - 1);
	else if (bisstart())
		/* Pretend we are at the start of a line.
		 * Needed for delete-to-eol and step in reg.c. */
		return '\n';
	else {
		Byte ch;
		bmove(-1);
		ch = Buff();
		bmove1();
		return ch;
	}
}

/* Convert the next portion of buffer to integer. Skip leading ws. */
int batoi(void)
{
	int num;

	while (Buff() == ' ' || Buff() == '\t')
		bmove1();
	for (num = 0; isdigit(Buff()); bmove1())
		num = num * 10 + Buff() - '0';
	return num;
}

void bgoto_char(long offset)
{
	struct page *tpage;

	/* find the correct page */
	for (tpage = Curbuff->firstp; tpage->nextp; tpage = tpage->nextp)
		if (tpage->plen >= offset)
			break;
		else
			offset -= tpage->plen;

	makecur(Curbuff, tpage, offset);
}

void tobegline(void)
{
	if (Curchar > 0 && *(Curcptr - 1) == '\n')
		return;
	if (bcrsearch('\n'))
		bmove1();
}

void toendline(void)
{
	if (bcsearch('\n'))
		bmove(-1);
}

/* Low level memory page routines */

/* Create a new memory page and link into chain after curpage */
static struct page *newpage(struct page *curpage)
{
	struct page *page = (struct page *)calloc(1, sizeof(struct page));
	if (!page)
		return NULL;

#ifdef DOS_EMS
	if (!ems_newpage(page)) {
		free(page);
		return NULL;
	}
#endif

	if (curpage) {
		page->prevp = curpage;
		page->nextp = curpage->nextp;
		curpage->nextp = page;
	}

	++NumPages;
	return page;
}

/* Split a full page. */
static struct page *pagesplit(struct page *curpage)
{
	struct page *newp = newpage(curpage);
	if (newp) {
#ifdef DOS_EMS
		ems_pagesplit(newp);
#else
		memmove(newp->pdata, curpage->pdata + HALFP, HALFP);
#endif
		curpage->plen = HALFP;
		newp->plen = HALFP;
	}

	return newp;
}

/* Free a memory page */
static void freepage(struct page **firstp, struct page *page)
{
#ifdef DOS_EMS
	ems_freepage(page);
#endif

	if (page->nextp)
		page->nextp->prevp = page->prevp;
	if (page->prevp)
		page->prevp->nextp = page->nextp;
	else if (firstp)
		*firstp = page->nextp;
	free((char *)page);
	--NumPages;
}

#ifdef DOS_EMS
/* ems.c needs to know the page structure */
#include "win32/ems.c"
#endif

/* Split a full page. */
bool bpagesplit(struct buff *buff)
{
	struct page *newp = pagesplit(buff->curpage);
	if (!newp)
		return false;

#if HAVE_MARKS
	struct mark *btmark;

	foreach_pagemark(btmark, buff->curpage)
		if (btmark->moffset >= HALFP) {
			btmark->mpage = newp;
			btmark->moffset -= HALFP;
		}
#endif
	if (buff->curchar >= HALFP)
		/* new page has Point in it */
		makecur(buff, newp, buff->curchar - HALFP);
	Curmodf = true;
	return true;
}
