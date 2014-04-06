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

#include "z.h"
#include <setjmp.h>
#include <time.h>

#if ZLIB
#undef Byte
#include <zlib.h>

#define bread(a, b, c) gzread(gz, b, c)
#define bclose(a) gzclose(gz)
#else
#define bread(a, b, c) read(a, b, c)
#define bclose(a) close(a)
#endif

static bool Curmodf;		/* page modified?? */
static Byte *Cpstart;		/* pim data start */
Byte *Curcptr;			/* current character */
int Curchar;			/* current offset in Cpstart */
int Curplen;			/* current page length */
struct buff *Bufflist;		/* the buffer list */
struct buff *Curbuff;		/* the current buffer */
struct mark *Mrklist;		/* the marks list tail */
struct page *Curpage;		/* the current page */

/* Keeping just one mark around is a HUGE win for a trivial amount of code. */
static struct mark *freemark;
/* Same for user mark */
static struct mark *freeumark;

/* stats only */
static int NumBuffs;
static int NumPages;
static int NumMarks;

static struct page *newpage(struct buff *tbuff,
			    struct page *ppage, struct page *npage);
static void freepage(struct buff *tbuff, struct page *page);
static bool pagesplit(void);

void binit(void)
{
#ifdef DOS_EMS
	ems_init();
#endif
}

void bfini(void)
{
	struct mark *mhead = &Scrnmarks[ROWMAX - 1];

	Curbuff = NULL;

	bdelbuff(Killbuff);
	bdelbuff(Paw);

	while (Bufflist) {
		if (Bufflist->fname)
			free(Bufflist->fname);
		if (Bufflist->bname)
			delbname(Bufflist->bname);
		/* bdelbuff will update Bufflist */
		bdelbuff(Bufflist);
	}

	/* Do not unmark the Scrnmarks */
	while (Mrklist && Mrklist != mhead)
		unmark(Mrklist);

	if (freemark)
		free(freemark);

#ifdef DOS_EMS
	ems_free();
#endif
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

	if (tbuff == Curbuff)
		return 0;

	flip = bisaftermrk(tmark);
	if (flip)
		bswappnt(tmark);

	ltmrk = bcremrk();
	sbuff = Curbuff;
	while (bisbeforemrk(tmark)) {
		if (Curpage == tmark->mpage)
			srclen = tmark->moffset - Curchar;
		else
			srclen = Curplen - Curchar;
		Curmodf = true;
		spnt = Curcptr;

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
		tbuff->blen += dstlen;
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


/* Create a buffer.   Returns a pointer to the buffer descriptor. */
struct buff *bcreate(void)
{
	struct buff *buf = (struct buff *)calloc(1, sizeof(struct buff));
	if (buf) {
		struct page *fpage = newpage(buf, NULL, NULL);
		if (!fpage) {
			/* bad news, de-allocate */
			free(buf);
			return NULL;
		}
		buf->pnt_page = fpage;
		buf->bmode = (VAR(VNORMAL) ? NORMAL : TXTMODE) |
			(VAR(VEXACT) ? EXACT     : 0);
		buf->child = EOF;
		++NumBuffs;
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
			longjmp(zenv, -1);	/* ABORT */
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
		else {
			error("Last Buffer.");
			return false;
		}
	}

	while (tbuff->child != EOF) {
		unvoke(tbuff);
		checkpipes(1);
	}

	uncomment(tbuff);
	undo_clear(tbuff);

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

		undo_del(quan);

		Curplen -= quan;
		Curbuff->blen -= quan;

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


/* Return current screen col of point. */
int bgetcol(bool flag, int col)
{
	struct mark pmark;

	bmrktopnt(&pmark);
	if (bcrsearch(NL))
		bmove1();
	while (!bisatmrk(&pmark) && !bisend()) {
		col += chwidth(*Curcptr, col, flag);
		bmove1();
	}
	return col;
}


/* Insert a character in the current buffer. */
void binsert(Byte byte)
{
	struct mark *btmark;

	if (Curplen == PSIZE && !pagesplit())
		return;
	memmove(Curcptr + 1, Curcptr, Curplen - Curchar);
	*Curcptr++ = byte;
	++Curplen;
	++Curchar;
	++Curbuff->blen;
	Curbuff->bmodf = true;
	Curmodf = true;

	undo_add(1);

	for (btmark = Mrklist; btmark; btmark = btmark->prev)
		if (btmark->mpage == Curpage && btmark->moffset >= Curchar)
			++(btmark->moffset);
	vsetmod(false);
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

	if (len != tbuff->blen) Dbg("PROBLEMS: %lu != %lu\n", len, tbuff->blen); // SAM DBG

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
	while (bcsearch(NL) && !bisaftermrk(&tmark))
		++line;
	bpnttomrk(&tmark);
	return line;
}

#ifdef INT_IS_16BITS
#define MAXMOVE		(0x7fff - 1024)

void boffset(unsigned long off)
{
	btostart();
	for (; off > MAXMOVE; off -= MAXMOVE)
		bmove(MAXMOVE);
	bmove(off);
}
#else
void boffset(unsigned long off)
{
	btostart();
	bmove(off);
}
#endif

/* Try to put Point in a specific column.
 * Returns actual Point column.
 */
int bmakecol(int col, bool must)
{
	int tcol = 0;

	if (bcrsearch(NL))
		bmove1();
	while (tcol < col && !ISNL(*Curcptr) && !bisend()) {
		tcol += chwidth(*Curcptr, tcol, !must);
		bmove1();
	}
	if (must && tcol < col) {
		int wid = chwidth('\t', tcol, true);
		if (tcol + wid < col)
			tcol -= Tabsize - wid;
		tindent(col - tcol);
	}
	return tcol;
}


/* Move the point relative to its current position.
 *
 * This routine is the most time-consuming routine in the editor.
 * Because of this, it is highly optimized. makeoffset() calls have
 * been inlined here.
 *
 * Since bmove(1) is used the most, a special call has been made.
 */
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
	Curbuff->blen = 0;
	Curcptr = Cpstart;
	Curmodf = true;

	undo_clear(Curbuff);
}

/* Point to off the end of the buffer */
void bshoveit(void)
{
	makecur(Curbuff->lastp);
	makeoffset(Curplen + 1);
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

#ifdef DOS
static int guess_mode(char *fname, char *buf)
{
	int text = 0, n, fd = open(fname, READ_MODE | O_BINARY);
	if (fd < 0)
		return fd;

	n = bread(fd, buf, PSIZE / 2);
	close(fd);
	if (n > 0) {
		buf[n] = '\0';
		text = strchr(buf, '\r') != NULL;
	}

	if (text) {
		fd = open(fname, READ_MODE);
		Curbuff->bmode |= CRLF;
	} else
		fd = open(fname, READ_MODE | O_BINARY);

	return fd;
}
#endif

/*
Load the file 'fname' into the current buffer.
Returns  0  successfully opened file
	 1  no such file
	-1  on error
*/
int breadfile(char *fname)
{
	char buf[PSIZE];
	struct stat sbuf;
	int fd, len;

#ifdef DOS
	fd = guess_mode(fname, buf);
#else
	fd = open(fname, READ_MODE);
#endif
	if (fd < 0) {
		if (errno == ENOENT)
			return 1;
		error("%s: %s", fname, strerror(errno));
		return -1;
	}

	if (fstat(fd, &sbuf) == 0)
		Curbuff->mtime = sbuf.st_mtime;
	else
		Curbuff->mtime = -1;

	putpaw("Reading %s", lastpart(fname));
	bempty();

#if ZLIB
	gzFile gz = gzdopen(fd, "rb");
	if (!gz) {
		close(fd);
		error("gzdopen %s", fname);
		return -1;
	}

	if (gzdirect(gz) == 0)
		Curbuff->bmode |= COMPRESSED;
#endif

	while ((len = bread(fd, buf, PSIZE)) > 0) {
		Curmodf = true;
		if (Curplen) {
			if (!newpage(Curbuff, Curpage, NULL)) {
				bempty();
				error("Out of page memory!");
				bclose(fd);
				return -ENOMEM;
			}
			makecur(Curpage->nextp);
		}
		memcpy(Cpstart, buf, len);
		Curplen = len;
		Curbuff->blen += len;
	}
	(void)bclose(fd);

	btostart();
	Curbuff->bmodf = false;
	clrpaw();

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
#ifdef DOS_EMS
	struct page *cur = Curpage;
#endif
	struct page *tpage;
	int n, status = true;

	Curpage->plen = Curplen;
	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp)
		if (tpage->plen) {
#ifdef DOS_EMS
			makecur(tpage);
#endif
			n = write(fd, tpage->pdata, tpage->plen);
			status = n == tpage->plen;
		}

	close(fd);

#ifdef DOS_EMS
	makecur(cur);
#endif

	return status;
}

static char *make_bakname(char *bakname, char *fname)
{
	strcpy(bakname, fname);
	strcat(bakname, "~");
	return bakname;
}

static bool cp(char *from, char *to)
{
	FILE *in, *out;
	char buf[1024];
	int rc = true;
	size_t n;

	in = fopen(from, "r");
	out = fopen(to, "w");
	if (!in || !out) {
		if (!in)
			fclose(in);
		return false;
	}
	while ((n = fread(buf, 1, 1024, in)) > 0)
		if (fwrite(buf, 1, n, out) != n) {
			rc = false;
			break;
		}
	fclose(in);
	fclose(out);
	return rc;
}

/*	Write the current buffer to the file 'fname'.
 *	Handles the backup scheme according to VAR(VBACKUP).
 *	Returns:	true	if write successful
 *				false	if write failed
 *				ABORT	if user didn't want to overwrite
 */
int bwritefile(char *fname)
{
	static int Cmask;
	char bakname[PATHMAX + 1];
	int fd, mode, status = true, bak = false;
	struct stat sbuf;
	int nlink;

	if (!fname)
		return true;

	/* If the file existed, check to see if it has been modified. */
	if (Curbuff->mtime && stat(fname, &sbuf) == 0) {
		if (sbuf.st_mtime > Curbuff->mtime) {
			sprintf(PawStr,
				"WARNING: %s has been modified. Overwrite? ",
				lastpart(fname));
			if (ask(PawStr) != YES)
				return ABORT;
		}
		mode  = sbuf.st_mode;
		nlink = sbuf.st_nlink;
	} else {
		if (Cmask == 0) {
			Cmask = umask(0);	/* get the current umask */
			umask(Cmask);		/* set it back */
			Cmask = ~Cmask & 0666;	/* make it usable */
		}
		mode  = Cmask;
		nlink = 1;
	}

	/* check for links and handle backup file */
	make_bakname(bakname, fname);
	if (nlink > 1) {
		sprintf(PawStr, "WARNING: %s is linked. Preserve? ",
			lastpart(fname));
		switch (ask(PawStr)) {
		case YES:
			if (VAR(VBACKUP))
				bak = cp(fname, bakname);
			break;
		case NO:
			if (VAR(VBACKUP))
				bak = rename(fname, bakname);
			else
				unlink(fname);	/* break link */
			break;
		case ABORT:
			return ABORT;
		}
	} else if (VAR(VBACKUP))
		bak = rename(fname, bakname);

	/* Write the output file */
	if (Curbuff->bmode & CRLF)
		fd = open(fname, WRITE_MODE, mode);
	else
		fd = open(fname, WRITE_MODE | O_BINARY, mode);
	if (fd != EOF) {
#if ZLIB
		if (Curbuff->bmode & COMPRESSED)
			status = bwritegzip(fd);
		else
#endif
			status = bwritefd(fd);
	} else {
		if (errno == EACCES)
			error("File is read only.");
		else
			error("Unable to open file.");
		status = false;
	}

	/* cleanup */
	if (status) {
		struct stat sbuf;

		if (stat(fname, &sbuf) == 0)
			Curbuff->mtime = sbuf.st_mtime;
		else
			Curbuff->mtime = -1;
		clrpaw();
		/* If we saved the file... it isn't read-only */
		Curbuff->bmode &= ~VIEW;
		Curbuff->bmodf = false;
	} else {
		error("Unable to write file.");
		if (bak) {
			if (sbuf.st_nlink) {
				cp(bakname, fname);
				unlink(bakname);
			} else
				rename(bakname, fname);
		}
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


/* Low level memory buffer routines */

/* Create a new memory page and link into chain */
static struct page *newpage(struct buff *tbuff,
			    struct page *ppage, struct page *npage)
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

	page->nextp = npage;
	page->prevp = ppage;
	npage ? (npage->prevp = page) : (tbuff->lastp = page);
	ppage ? (ppage->nextp = page) : (tbuff->firstp = page);
	++NumPages;

	return page;
}

/* Free a memory page */
static void freepage(struct buff *tbuff, struct page *page)
{
#ifdef DOS_EMU
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

/* Peek the previous byte */
Byte bpeek(void)
{
	if (Curchar > 0)
		return *(Curcptr - 1);
	else if (bisstart())
		/* Pretend we are at the start of a line.
		 * Needed for delete-to-eol and step in reg.c. */
		return NL;
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

	while (biswhite())
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
		} else
			Curbuff->umark = bcremrk();
	}

	if (tmark)
		mrktomrk(Curbuff->umark, tmark);
	else
		bmrktopnt(Curbuff->umark);
}

void clear_umark(void)
{
	if (Curbuff->umark) {
		vsetmrk(Curbuff->umark);
		if (freeumark)
			unmark(Curbuff->umark);
		else
			freeumark = Curbuff->umark;
		Curbuff->umark = NULL;
	}
}

void Zstats(void)
{
	struct buff *b;
	struct page *p;
	unsigned long use = 0, total = 0;
	int n;

	Curpage->plen = Curplen;
	for (b = Bufflist; b; b = b->next)
		for (p = b->firstp; p; p = p->nextp) {
			use += p->plen;
			++total;
		}

	n = snprintf(PawStr, Colmax,
		     "Buffers: %d  Pages: %d  Marks: %d",
		     NumBuffs, NumPages, NumMarks);
#ifdef DOS_EMS
	n += snprintf(PawStr + n, Colmax - n, "  EMS: %d", ems_pages);
#endif
#if UNDO
	n += snprintf(PawStr + n, Colmax - n, "  Undos: %lu%c",
		      (undo_total + 521) / 1024, undo_total ? 'K' : ' ');
#endif
	_putpaw(PawStr);
}
