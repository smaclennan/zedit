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

#ifdef ZEDIT
#include "z.h"
#else
static inline void vsetmod(bool flag) {}
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

static bool Curmodf;		/* page modified?? */
static Byte *Cpstart;		/* pim data start */
Byte *Curcptr;			/* current character */
int Curchar;			/* current offset in Cpstart */
static int Curplen;			/* current page length */
struct buff *Bufflist;		/* the buffer list */
struct buff *Curbuff;		/* the current buffer */
struct page *Curpage;		/* the current page */

static int binitialized;

static struct mark *Mrklist;	/* the marks list tail */
static struct mark *mhead;

int raw_mode;

/* Keeping just one mark around is a HUGE win for a trivial amount of code. */
static struct mark *freemark;
/* Same for user mark */
static struct mark *freeumark;

/* stats only */
static int NumBuffs;
static int NumPages;
static int NumMarks;

/* Generally, the bigger the page size the faster the editor however
 * the more wasted memory. A page size of 1k seems to be a very good trade off.
 * NOTE: DOS *requires* 1k pages for DOS_EMS.
 */
#define PSIZE		1024		/* size of page */

struct page {
#ifdef DOS_EMS
	Byte *pdata;			/* the page data */
	Byte emmpage;			/* 16k page */
	Byte emmoff;			/* offset in page */
#elif defined(ONE_PAGE)
	Byte *pdata;			/* the page data */
#else
	Byte pdata[PSIZE];		/* the page data */
#endif
	int psize;			/* allocated page size */
	int plen;			/* current length of the page */
	struct page *nextp, *prevp;	/* list of pages in buffer */
};

static struct page *newpage(struct buff *tbuff,
			    struct page *ppage, struct page *npage);
static void freepage(struct buff *tbuff, struct page *page);
static bool pagesplit(void);

static void bfini(void)
{
	Curbuff = NULL;

	while (Bufflist)
		/* bdelbuff will update Bufflist */
		bdelbuff(Bufflist);

	if (mhead) {
		if (mhead->next)
			mhead->next->prev = NULL;
		else
			Mrklist = NULL;
	}

	while (Mrklist)
		unmark(Mrklist);

	if (freemark)
		free(freemark);

#ifdef DOS_EMS
	ems_free();
#endif
}

static void binit(void)
{
	if (!binitialized) {
#ifdef DOS_EMS
		ems_init();
#endif
		atexit(bfini);
		binitialized = 1;
	}
}

void minit(struct mark *preallocated)
{
	Mrklist = preallocated;
	mhead = preallocated;
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
			srclen = Curplen - Curchar;
		Curmodf = true;
#ifdef DOS_EMS
		memcpy(spnt, Curcptr, srclen);
#else
		spnt = Curcptr;
#endif

		bswitchto(tbuff);
		dstlen = PSIZE - Curplen;
		if (dstlen == 0) {
			if (pagesplit())
				dstlen = PSIZE - Curplen;
			else {
				bswitchto(sbuff);
				break;
			}
		}
		if (srclen < dstlen)
			dstlen = srclen;
		/* Make a gap */
		memmove(Curcptr + dstlen, Curcptr, Curplen - Curchar);
		/* and fill it in */
		memmove(Curcptr, spnt, dstlen);
		Curplen += dstlen;
		copied += dstlen;
		for (btmrk = Mrklist; btmrk; btmrk = btmrk->prev)
			if (btmrk->mpage == Curpage &&
			    btmrk->moffset > Curchar)
					btmrk->moffset += dstlen;
		makeoffset(Curchar + dstlen);
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

/* Create a buffer but don't add it to the buffer list. You probably don't want this. */
struct buff *_bcreate(void)
{
	struct buff *buf = (struct buff *)calloc(1, sizeof(struct buff));
	if (buf) {
		struct page *fpage;

		binit();
		if (!(fpage = newpage(buf, NULL, NULL))) {
			/* bad news, de-allocate */
			free(buf);
			return NULL;
		}
		buf->pnt_page = fpage;
#ifdef ZEDIT
		buf->child = EOF;
#endif
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

/* Create a mark at the current point and add it to the list.
 * If we are unable to alloc, longjmp.
 */
struct mark *bcremrk(void)
{
	struct mark *mrk;

	if (freemark) {
		mrk = freemark;
		freemark = NULL;
	} else {
		mrk = (struct mark *)calloc(1, sizeof(struct mark));
		if (!mrk)
			return NULL;
	}

	bmrktopnt(mrk);
	mrk->prev = Mrklist;		/* add to end of list */
	mrk->next = NULL;
	if (Mrklist)
		Mrklist->next = mrk;
	Mrklist = mrk;
	++NumMarks;
	return mrk;
}

/* Free up the given mark and remove it from the list.
 * Cannot free a scrnmark!
 */
void unmark(struct mark *mptr)
{
	if (mptr) {
		if (mptr->prev)
			mptr->prev->next = mptr->next;
		if (mptr->next)
			mptr->next->prev = mptr->prev;
		if (mptr == Mrklist)
			Mrklist = mptr->prev;

		if (!freemark)
			freemark = mptr;
		else
			free((char *)mptr);
		--NumMarks;
	}
}


bool bcrsearch(Byte what)
{
	while (1) {
		if (Curchar <= 0)
			if (Curpage == Curbuff->firstp)
				return false;
			else {
				makecur(Curpage->prevp);
				makeoffset(Curplen - 1);
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

	while ((n = (Byte *)memchr(Curcptr, what, Curplen - Curchar)) == NULL)
		if (Curpage == Curbuff->lastp) {
			makeoffset(Curplen);
			return false;
		} else {
			makecur(Curpage->nextp);
			makeoffset(0);
		}

	makeoffset(n - Cpstart);
	bmove1();
	return true;
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

#ifdef ZEDIT
	while (tbuff->child != EOF) {
		unvoke(tbuff);
		checkpipes(1);
	}

	uncomment(tbuff);
#endif

	if (tbuff->fname)
		free(tbuff->fname);
	if (tbuff->bname)
		free(tbuff->bname);
	if (tbuff->app)
		free(tbuff->app);

	while (tbuff->firstp)	/* delete the pages */
		freepage(tbuff, tbuff->firstp);
	unmark(tbuff->umark);	/* free the user mark */
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

/* Delete quantity characters. */
void bdelete(int quantity)
{
	int quan, noffset;
	struct page *tpage;
	struct mark *tmark;

	while (quantity) {
		/* Delete as many characters as possible from this page */
		if (Curchar + quantity > Curplen)
			quan = Curplen - Curchar;
		else
			quan = quantity;
		if (quan < 0)
			quan = 0; /* May need to switch pages */

#if UNDO
		undo_del(quan);
#endif

		Curplen -= quan;

		memmove(Curcptr, Curcptr + quan, Curplen - Curchar);
		if (Curpage == Curbuff->lastp)
			quantity = 0;
		else
			quantity -= quan;
		Curbuff->bmodf = true;
		Curmodf = true;
		if (Curplen == 0 && (Curpage->nextp || Curpage->prevp)) {
			/* We deleted entire page. */
			tpage = Curpage->nextp;
			noffset = 0;
			if (tpage == NULL) {
				tpage = Curpage->prevp;
				noffset = tpage->plen;
			}
			for (tmark = Mrklist; tmark; tmark = tmark->prev)
				if (tmark->mpage == Curpage) {
					tmark->mpage = tpage;
					tmark->moffset = noffset;
				}
			freepage(Curbuff, Curpage);
			Curpage = NULL;
		} else {
			tpage = Curpage;
			noffset = Curchar;
			if ((noffset >= Curplen) && Curpage->nextp) {
				tpage = Curpage->nextp;
				noffset = 0;
			}
			for (tmark = Mrklist; tmark; tmark = tmark->prev)
				if (tmark->mpage == Curpage &&
				    tmark->moffset >= Curchar) {
					if (tmark->moffset >= Curchar + quan)
						tmark->moffset -= quan;
					else {
						tmark->mpage = tpage;
						tmark->moffset = noffset;
					}
				}
		}
		makecur(tpage);
		makeoffset(noffset);
	}
	vsetmod(true);
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
			bdelete(Curplen - Curchar);
}


/* Insert a character in the current buffer. */
bool binsert(Byte byte)
{
	struct mark *btmark;

	if (Curplen == Curpage->psize && !pagesplit())
		return false;
	memmove(Curcptr + 1, Curcptr, Curplen - Curchar);
	*Curcptr++ = byte;
	++Curplen;
	++Curchar;
	Curbuff->bmodf = true;
	Curmodf = true;

#ifdef ZEDIT
	undo_add(1);
#endif

	for (btmark = Mrklist; btmark; btmark = btmark->prev)
		if (btmark->mpage == Curpage && btmark->moffset >= Curchar)
			++(btmark->moffset);
	vsetmod(false);
	return true;
}

bool bappend(Byte *data, int size)
{
	btoend();

	/* Fill the current page */
	int n, left = PSIZE - Curplen;
	if (left > 0) {
		n = MIN(left, size);
		memcpy(Curcptr, data, n);
		Curplen += n;
		size -= n;
		data += n;
		Curmodf = true;
	}

	/* Put the rest in new pages */
	while (size > 0) {
		struct page *npage = newpage(Curbuff, Curpage, NULL);
		if (!npage)
			return false;
		makecur(npage);

		n = MIN(PSIZE, size);
		memcpy(Cpstart, data, n);
		Curplen = n;
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
#ifdef ONE_PAGE
	/* No optimization for now */
	int i;
	for (i = 0; i < size; ++i)
		if (!binsert(data[i]))
			break;
	return i;
#else
	struct page *npage;
	int copied = 0;

	int n = Curplen - Curchar;
	if (n > 0) {
		struct mark *btmark;

		/* Make a new page and move the end of this page to the new page */
		if (!(npage = newpage(Curbuff, Curpage, Curpage->nextp)))
			return 0;
		memcpy(npage->pdata, Curcptr, n);
		npage->plen = n;

		/* Fix marks that are now in new page */
		for (btmark = Mrklist; btmark; btmark = btmark->prev)
			if (btmark->mpage == Curpage && btmark->moffset >= Curchar) {
				btmark->mpage = npage;
				btmark->moffset -= n;
			}

		/* Copy as much as possible to the end of this page */
		n = MIN(n, size);
		memcpy(Curcptr, data, n);
		data += n;
		size -= n;
		copied += n;
		makeoffset(Curchar + n);
		Curplen = Curchar;
	}

	while (size > 0) {
		if (!(npage = newpage(Curbuff, Curpage, Curpage->nextp)))
			break;

		n = MIN(PSIZE, size);
		memcpy(npage->pdata, data, n);
		data += n;
		size -= n;
		copied += n;

		makecur(npage);
		makeoffset(n);

		Curplen = n;
	}

	return copied;
#endif
}

void bconvert(int (*to)(int c))
{
	*Curcptr = to(*Curcptr);
	Curbuff->bmodf = Curmodf = true;
}

/* Insert a string into the current buffer. */
void binstr(const char *str)
{
	while (*str)
		binsert(*str++);
}


/* Returns true if point is after the mark. */
bool bisaftermrk(struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != Curbuff)
		return false;
	if (tmark->mpage == Curpage)
		return Curchar > tmark->moffset;
	for (tp = Curpage->prevp; tp && tp != tmark->mpage; tp = tp->prevp)
		;
	return tp != NULL;
}


/* True if the point precedes the mark. */
bool bisbeforemrk(struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != Curbuff)
		return false;
	if (tmark->mpage == Curpage)
		return Curchar < tmark->moffset;
	for (tp = Curpage->nextp; tp && tp != tmark->mpage; tp = tp->nextp)
		;
	return tp != NULL;
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

/* Return the current line of the point. */
unsigned long bline(void)
{
	struct mark tmark;
	unsigned long line = 1;

	bmrktopnt(&tmark);
	btostart();
	while (bcsearch('\n') && !bisaftermrk(&tmark))
		++line;
	bpnttomrk(&tmark);
	return line;
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
		if (dist >= 0 && dist < Curplen) {
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
			Curchar = Curplen; /* makeoffset Curplen */
			Curcptr = Cpstart + Curplen;
		} else {	/* goto next page */
			if (Curpage == Curbuff->lastp) {
				/* past end of buffer */
				makeoffset(Curplen);
				return false;
			}
			dist -= Curplen; /* must use this Curplen */
			makecur(Curpage->nextp);
			Curchar = 0; /* makeoffset 0 */
			Curcptr = Cpstart;
		}
	}
	return true;
}

void bmove1(void)
{
	if (++Curchar < Curplen)
		/* within current page */
		++Curcptr;
	else if (Curpage->nextp) {
		/* goto start of next page */
		makecur(Curpage->nextp);
		Curchar = 0;
		Curcptr = Cpstart;
	} else {
		/* At EOB */
		Curchar = Curplen;
		Curcptr = Cpstart + Curplen;
	}
}

/* Fairly special routine. Pushes the char one past the end of the
 * buffer. */
void bshove(void)
{
	btoend();
	++Curcptr;
	++Curchar;
}

/* Put the mark where the point is. */
void bmrktopnt(struct mark *tmark)
{
	tmark->mbuff   = Curbuff;
	tmark->mpage   = Curpage;
	tmark->moffset = Curchar;
}


/* Put the current buffer point at the mark */
void bpnttomrk(struct mark *tmark)
{
	if (tmark->mpage) {
		if (tmark->mbuff != Curbuff)
			bswitchto(tmark->mbuff);
		makecur(tmark->mpage);
		makeoffset(tmark->moffset);
	}
}

void bempty(void)
{
	struct mark *btmark;

	makecur(Curbuff->firstp);
	while (Curpage->nextp)
		freepage(Curbuff, Curpage->nextp);
	for (btmark = Mrklist; btmark; btmark = btmark->prev)
		if (btmark->mpage && btmark->mbuff == Curbuff) {
			btmark->mpage = Curpage;
			btmark->moffset = 0;
			btmark->modf = true;
		}
	Curplen = Curchar = 0;		/* reset to start of page */
	Curcptr = Cpstart;
	Curmodf = true;

#ifdef ZEDIT
	undo_clear(Curbuff);
#endif
}

/* Swap the point and the mark. */
void bswappnt(struct mark *tmark)
{
	struct mark tmp;

	tmp.mbuff	= Curbuff; /* Point not moved out of its buffer */
	tmp.mpage	= tmark->mpage;
	tmp.moffset	= tmark->moffset;
	bmrktopnt(tmark);
	bpnttomrk(&tmp);
}


void bswitchto(struct buff *buf)
{
	if (buf && buf != Curbuff) {
		if (Curbuff) {
			Curbuff->pnt_page   = Curpage;
			Curbuff->pnt_offset = Curchar;
		}
		makecur(buf->pnt_page);
		makeoffset(buf->pnt_offset);
		Curbuff = buf;
	}
}


/* Set the point to the end of the buffer */
void btoend(void)
{
	makecur(Curbuff->lastp);
	makeoffset(Curplen);
}


/* Set the point to the start of the buffer. */
void btostart(void)
{
	makecur(Curbuff->firstp);
	makeoffset(0);
}

static void crfixup(void)
{
	char *p = (char *)memchr(Cpstart + 1, '\n', Curplen - 1);
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
		if (Curplen) {
			if (!newpage(Curbuff, Curpage, NULL)) {
				bempty();
				bclose(fd);
				return ENOMEM;
			}
#ifndef ONE_PAGE
			makecur(Curpage->nextp);
			makeoffset(0);
#endif
		}
		memcpy(Curcptr, buf, len);
		Curcptr += len;
		Curchar += len;
		Curplen += len;
	}
	(void)bclose(fd);

	btostart();

	if (Curplen && !(Curbuff->bmode & COMPRESSED))
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

	Curpage->plen = Curplen;
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

	Curpage->plen = Curplen;
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

	Curpage->plen = Curplen;
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

/* True if mark1 follows mark2 */
bool mrkaftermrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return false;        /* marks in different buffers */
	if (mark1->mpage == mark2->mpage)
		return mark1->moffset > mark2->moffset;
	for (tpage = mark1->mpage->prevp;
	     tpage && tpage != mark2->mpage;
	     tpage = tpage->prevp)
		;

	return tpage != NULL;
}

/* True if mark1 is at mark2 */
bool mrkatmrk(struct mark *mark1, struct mark *mark2)
{
	return  mark1->mbuff == mark2->mbuff &&
		mark1->mpage == mark2->mpage &&
		mark1->moffset == mark2->moffset;
}

/* True if mark1 precedes mark2 */
bool mrkbeforemrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return false;        /* Marks not in same buffer */
	if (mark1->mpage == mark2->mpage)
		return mark1->moffset < mark2->moffset;
	for (tpage = mark1->mpage->nextp;
	     tpage && tpage != mark2->mpage;
	     tpage = tpage->nextp)
		;
	return tpage != NULL;
}


/* Make page current*/
void makecur(struct page *page)
{
	if (Curpage == page)
		return;
	if (Curpage)
		Curpage->plen = Curplen;

#ifdef DOS_EMS
	ems_makecur(page, Curmodf);
#endif

	Curpage = page;
	Cpstart = page->pdata;
	Curmodf = false;
	Curplen = Curpage->plen;
}

bool bisend(void)
{
	return (Curpage == Curbuff->lastp) && (Curchar >= Curplen);
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
	Curpage->plen = Curplen;
	for (tpage = Curbuff->firstp; tpage->nextp; tpage = tpage->nextp)
		if (tpage->plen >= offset)
			break;
		else
			offset -= tpage->plen;

	makecur(tpage);
	makeoffset(offset);
}

void set_umark(struct mark *tmark)
{
	if (Curbuff->umark == NULL) {
		if (freeumark) {
			Curbuff->umark = freeumark;
			freeumark = NULL;
		} else if (!(Curbuff->umark = bcremrk()))
			return;
	}

	if (tmark)
		mrktomrk(Curbuff->umark, tmark);
	else
		bmrktopnt(Curbuff->umark);
}

void clear_umark(void)
{
	if (Curbuff->umark) {
		if (freeumark)
			unmark(Curbuff->umark);
		else
			freeumark = Curbuff->umark;
		Curbuff->umark = NULL;
	}
}

int bgetstats(char *str, int len)
{
	return snprintf(str, len,
					"Buffers: %d  Pages: %d  Marks: %d",
					NumBuffs, NumPages, NumMarks);
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

#ifdef ONE_PAGE
/* Create a new memory page or resize old page. */
static struct page *newpage(struct buff *tbuff,
			    struct page *ppage, struct page *npage)
{
	if (ppage)
		/* For breadfile: we just want a bigger page */
		return pagesplit() ? Curpage : NULL;

	/* Brand new page. */
	struct page *page = (struct page *)calloc(1, sizeof(struct page));
	if (!page)
		return NULL;
	page->pdata = malloc(PSIZE);
	if (!page->pdata) {
		free(page);
		return NULL;
	}
	tbuff->firstp = tbuff->lastp = page;
	page->psize = PSIZE;

	++NumPages;

	return page;
}

/* Make a full page larger. */
static bool pagesplit(void)
{
	Byte *newp = realloc(Curpage->pdata, Curpage->psize + PSIZE);
	if (!newp)
		return false;
	Curpage->pdata = newp;
	Curpage->psize += PSIZE;
	Cpstart = newp;
	Curcptr = newp + Curchar;
	++NumPages;
	return true;
}

/* Free a memory page */
static void freepage(struct buff *tbuff, struct page *page)
{
	NumPages -= page->psize / PSIZE;
	free(page->pdata);
	free(page);
	tbuff->firstp = tbuff->lastp = NULL;
	--NumPages;
}
#else
#define HALFP		(PSIZE / 2)	/* half the page size */

/* Create a new memory page and link into chain */
static struct page *newpage(struct buff *tbuff,
			    struct page *ppage, struct page *npage)
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

	page->nextp = npage;
	page->prevp = ppage;
	npage ? (npage->prevp = page) : (tbuff->lastp = page);
	ppage ? (ppage->nextp = page) : (tbuff->firstp = page);
	++NumPages;

	return page;
}

/* Split the current (full) page. */
static bool pagesplit(void)
{
	struct page *newp;
	struct mark *btmark;

	newp = newpage(Curbuff, Curpage, Curpage->nextp);
	if (newp == NULL)
		return false;

#ifdef DOS_EMS
	ems_pagesplit(newp, Curmodf);
#else
	memmove(newp->pdata, Cpstart + HALFP, HALFP);
#endif
	Curmodf = true;
	Curplen = HALFP;
	newp->plen = HALFP;
	for (btmark = Mrklist; btmark; btmark = btmark->prev)
		if (btmark->mpage == Curpage && btmark->moffset >= HALFP) {
			btmark->mpage = newp;
			btmark->moffset -= HALFP;
		}
	if (Curchar >= HALFP) {
		/* new page has Point in it */
		makecur(newp);
		makeoffset(Curchar - HALFP);
	}
	return true;
}

/* Free a memory page */
static void freepage(struct buff *tbuff, struct page *page)
{
#ifdef DOS_EMS
	ems_freepage(page);
#endif

	if (page->nextp)
		page->nextp->prevp = page->prevp;
	else
		tbuff->lastp = page->prevp;
	if (page->prevp)
		page->prevp->nextp = page->nextp;
	else
		tbuff->firstp = page->nextp;
	free((char *)page);
	--NumPages;
}

#ifdef DOS_EMS
/* ems.c needs to know the page structure */
#include "win32/ems.c"
#endif
#endif
