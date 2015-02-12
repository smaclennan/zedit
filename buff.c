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
static Byte *Cpstart;		/* pim data start */
Byte *Curcptr;			/* current character */
int Curchar;			/* current offset in Cpstart */
struct buff *Bufflist;		/* the buffer list */
struct buff *Curbuff;		/* the current buffer */
struct page *Curpage;		/* the current page */

int raw_mode;

/* stats only */
int NumBuffs;
int NumPages;

static struct page *newpage(struct page *curpage);
static void freepage(struct page **firstp, struct page *page);

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

/* Create a buffer but don't add it to the buffer list. You probably don't want this. */
struct buff *_bcreate(void)
{
	struct buff *buf = (struct buff *)calloc(1, sizeof(struct buff));
	if (buf) {
		struct page *fpage;

		binit();
		if (!(fpage = newpage(NULL))) {
			/* bad news, de-allocate */
			free(buf);
			return NULL;
		}
		buf->firstp = fpage;
		buf->curpage = fpage;
		++NumBuffs;
	}

	return buf;
}

/* Create a buffer. Returns a pointer to the buffer descriptor. */
struct buff *bcreate(void)
{
	struct buff *buf = _bcreate();
	if (buf) {
		/* add the buffer to the head of the list */
		if (Bufflist)
			Bufflist->prev = buf;
		buf->next = Bufflist;
		Bufflist = buf;

		if (Curbuff == NULL)
			bswitchto(buf);
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

	if (tbuff->fname)
		free(tbuff->fname);
	if (tbuff->bname)
		free(tbuff->bname);
	if (tbuff->app && app_cleanup)
		app_cleanup(tbuff);

	while (tbuff->firstp)	/* delete the pages */
		freepage(&tbuff->firstp, tbuff->firstp);
	if (tbuff == Bufflist)	/* unlink from the list */
		Bufflist = tbuff->next;
	if (tbuff->prev)
		tbuff->prev->next = tbuff->next;
	if (tbuff->next)
		tbuff->next->prev = tbuff->prev;

	free((char *)tbuff);	/* free the buffer proper */

	--NumBuffs;

	return true;
}

/* Insert a character in the current buffer. */
bool binsert(Byte byte)
{
	struct mark *btmark;

	if (Curpage->plen == Curpage->psize && !bpagesplit())
		return false;
	memmove(Curcptr + 1, Curcptr, Curpage->plen - Curchar);
	*Curcptr++ = byte;
	++Curpage->plen;
	++Curchar;
	Curbuff->bmodf = true;
	Curmodf = true;

	undo_add(1);

#ifdef HAVE_MARKS
	foreach_pagemark(btmark, Curpage)
		if (btmark->moffset >= Curchar)
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
		makecur(npage);

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
		makeoffset(Curchar + n);
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

		makecur(npage);
		makeoffset(n);

		Curpage->plen = n;
	}

	return copied;
}

/* Delete quantity characters. */
void bdelete(int quantity)
{
	int quan, noffset;
	struct page *tpage;
	struct mark *tmark;

	while (quantity) {
		/* Delete as many characters as possible from this page */
		if (Curchar + quantity > Curpage->plen)
			quan = Curpage->plen - Curchar;
		else
			quan = quantity;
		if (quan < 0)
			quan = 0; /* May need to switch pages */

#if UNDO
		undo_del(quan);
#endif

		Curpage->plen -= quan;

		memmove(Curcptr, Curcptr + quan, Curpage->plen - Curchar);
		if (lastp(Curpage))
			quantity = 0;
		else
			quantity -= quan;
		Curbuff->bmodf = true;
		Curmodf = true;
		if (Curpage->plen == 0 && (Curpage->nextp || Curpage->prevp)) {
			/* We deleted entire page. */
			tpage = Curpage->nextp;
			noffset = 0;
			if (tpage == NULL) {
				tpage = Curpage->prevp;
				noffset = tpage->plen;
			}
#ifdef HAVE_MARKS
			foreach_pagemark(tmark, Curpage) {
				tmark->mpage = tpage;
				tmark->moffset = noffset;
			}
#endif
			freepage(&Curbuff->firstp, Curpage);
			Curpage = NULL;
		} else {
			tpage = Curpage;
			noffset = Curchar;
			if ((noffset >= Curpage->plen) && Curpage->nextp) {
				tpage = Curpage->nextp;
				noffset = 0;
			}
#ifdef HAVE_MARKS
			foreach_pagemark(tmark, Curpage)
				if (tmark->moffset >= Curchar) {
					if (tmark->moffset >= Curchar + quan)
						tmark->moffset -= quan;
					else {
						tmark->mpage = tpage;
						tmark->moffset = noffset;
					}
				}
#endif
		}
		makecur(tpage);
		makeoffset(noffset);
	}
	vsetmod(true);
}

bool bcrsearch(Byte what)
{
	while (1) {
		if (Curchar <= 0)
			if (Curpage == Curbuff->firstp)
				return false;
			else {
				makecur(Curpage->prevp);
				makeoffset(Curpage->plen - 1);
			}
		else
			makeoffset(Curchar - 1);
		if (*Curcptr == what)
			return true;
	}
}

bool bcsearch(Byte what)
{
	Byte *n;

	if (bisend())
		return false;

	while ((n = (Byte *)memchr(Curcptr, what, Curpage->plen - Curchar)) == NULL)
		if (lastp(Curpage)) {
			makeoffset(Curpage->plen);
			return false;
		} else {
			makecur(Curpage->nextp);
			makeoffset(0);
		}

	makeoffset(n - Cpstart);
	bmove1();
	return true;
}

/* Insert a string into the current buffer. */
void binstr(const char *str)
{
	while (*str)
		binsert(*str++);
}

/* Returns the length of the buffer. */
unsigned long blength(struct buff *tbuff)
{
	struct page *tpage, *spage = Curpage;
	unsigned long len = 0;

	for (tpage = tbuff->firstp; tpage; tpage = tpage->nextp) {
		makecur(tpage);
		len += tpage->plen;
	}
	makecur(spage);

	return len;
}

/* Return the current position of the point. */
unsigned long blocation(void)
{
	struct page *tpage, *spage = Curpage;
	unsigned long len = 0;

	for (tpage = Curbuff->firstp; tpage != spage; tpage = tpage->nextp) {
		makecur(tpage);
		len += tpage->plen;
	}
	makecur(spage);
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
bool bmove(int dist)
{
	while (dist) {
		dist += Curchar;
		if (dist >= 0 && dist < Curpage->plen) {
			/* within current page makeoffset dist */
			Curchar = dist;
			Curcptr = Cpstart + dist;
			return true;
		}
		if (dist < 0) { /* goto previous page */
			if (Curpage == Curbuff->firstp) {
				/* past start of buffer */
				makeoffset(0);
				return false;
			}
			makecur(Curpage->prevp);
			Curchar = Curpage->plen; /* makeoffset curplen */
			Curcptr = Cpstart + Curpage->plen;
		} else {	/* goto next page */
			if (lastp(Curpage)) {
				/* past end of buffer */
				makeoffset(Curpage->plen);
				return false;
			}
			dist -= Curpage->plen; /* must use this curplen */
			makecur(Curpage->nextp);
			Curchar = 0; /* makeoffset 0 */
			Curcptr = Cpstart;
		}
	}
	return true;
}

void bmove1(void)
{
	if (++Curchar < Curpage->plen)
		/* within current page */
		++Curcptr;
	else if (Curpage->nextp) {
		/* goto start of next page */
		makecur(Curpage->nextp);
		Curchar = 0;
		Curcptr = Cpstart;
	} else {
		/* At EOB */
		Curchar = Curpage->plen;
		Curcptr = Cpstart + Curpage->plen;
	}
}

void bempty(void)
{
	struct mark *btmark;

	makecur(Curbuff->firstp);
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
		if (Curbuff) {
			Curbuff->curpage   = Curpage;
			Curbuff->curchar = Curchar;
			Curbuff->curcptr = Curcptr;
		}
		makecur(buf->curpage);
		makeoffset(buf->curchar);
		Curbuff = buf;
	}
}


/* Set the point to the end of the buffer */
void btoend(void)
{
	if (Curpage->nextp) {
		struct page *lastp;

		for (lastp = Curpage->nextp; lastp->nextp; lastp = lastp->nextp) ;
		makecur(lastp);
	}
	makeoffset(Curpage->plen);
}


/* Set the point to the start of the buffer. */
void btostart(void)
{
	makecur(Curbuff->firstp);
	makeoffset(0);
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
			makecur(Curpage->nextp);
			makeoffset(0);
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

static bool bwritefd(int fd)
{
	struct page *cur = Curpage;
	struct page *tpage;
	int n, status = true;

	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			makecur(tpage); /* DOS_EMS requires */
			n = write(fd, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	close(fd);

	makecur(cur);
	return status;
}

static bool bwritedos(int fd)
{
	struct page *cur = Curpage;
	struct page *tpage;
	int i, n, status = true;
	Byte buf[PSIZE * 2], *p;

	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
			int len = tpage->plen;
			makecur(tpage); /* DOS_EMS requires */
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

	makecur(cur);
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

/* Make the point be dist chars into the page. */
void makeoffset(int dist)
{
	Curchar = dist;
	Curcptr = Cpstart + dist;
}

/* Make page current*/
void makecur(struct page *page)
{
	if (Curpage == page)
		return;

#ifdef DOS_EMS
	ems_makecur(page, Curmodf);
#endif

	Curpage = page;
	Cpstart = page->pdata;
	Curmodf = false;
}

bool bisend(void)
{
	return lastp(Curpage) && (Curchar >= Curpage->plen);
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

	makecur(tpage);
	makeoffset(offset);
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

	page->psize = PSIZE;
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
bool bpagesplit(void)
{
	struct mark *btmark;
	struct page *newp = pagesplit(Curpage);
	if (!newp)
		return false;

	Curmodf = true;
#if HAVE_MARKS
	foreach_pagemark(btmark, Curpage)
		if (btmark->moffset >= HALFP) {
			btmark->mpage = newp;
			btmark->moffset -= HALFP;
		}
#endif
	if (Curchar >= HALFP) {
		/* new page has Point in it */
		makecur(newp);
		makeoffset(Curchar - HALFP);
	}
	return true;
}
