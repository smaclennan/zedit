/* buff.c - low level buffer commands for Zedit
 * Copyright (C) 1988-2010 Sean MacLennan
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
#include <sys/stat.h>
#include <time.h>

Boolean Curmodf;		/* page modified?? */
Byte *Cpstart;			/* pim data start */
Byte *Curcptr;			/* current character */
int Curchar;			/* current offset in Cpstart */
int Curplen;			/* current page length */
struct buff *Bufflist;		/* the buffer list */
struct buff *Curbuff;		/* the current buffer */
struct mark *Mrklist;		/* the marks list tail */
struct page *Curpage;		/* the current page */

static int NumPages;

static struct page *newpage(struct buff *tbuff,
			    struct page *ppage, struct page *npage);
static void freepage(struct buff *tbuff, struct page *page);
static Boolean pagesplit();
static Boolean xbput(int fd, Byte *addr, unsigned len);

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
			free(Bufflist->bname);
		/* bdelbuff will update Bufflist */
		bdelbuff(Bufflist);
	}
	free(Bnames);

	/* Do not unmark the Scrnmarks */
	while (Mrklist && Mrklist != mhead)
		unmark(Mrklist);
}

/* Copy from Point to tmark to tbuff. Returns number of bytes
 * copied. Caller must handle undo. */
int bcopyrgn(struct mark *tmark, struct buff *tbuff)
{
	struct buff *sbuff;
	struct mark *ltmrk, *btmrk;
	Boolean flip;
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
		Curmodf = TRUE;
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
		copied += dstlen;
		for (btmrk = Mrklist; btmrk; btmrk = btmrk->prev)
			if (btmrk->mpage == Curpage && btmrk->moffset > Curchar)
					btmrk->moffset += dstlen;
		makeoffset(Curchar + dstlen);
		vsetmod(FALSE);
		Curmodf = TRUE;
		Curbuff->bmodf = MODIFIED;
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
	struct buff *new;
	struct page *fpage;

	new = calloc(1, sizeof(struct buff));
	if (new) {
		fpage = newpage(new, NULL, NULL);
		if (!fpage) {
			/* bad news, de-allocate */
			free((char *)new);
			return NULL;
		}
		new->mark = bcremrk();
		new->mark->mbuff = new;
		new->pnt_page = new->mark->mpage = fpage;
		new->bmode = (VAR(VNORMAL) ? NORMAL : TEXT) |
			(VAR(VEXACT) ? EXACT     : 0) |
			(VAR(VOVWRT) ? OVERWRITE : 0);
#ifdef PIPESH
		new->child = EOF;
#endif
	}

	return new;
}

/* Create a mark at the current point and add it to the list.
 * If we are unable to alloc, longjmp.
 */
struct mark *bcremrk(void)
{
	struct mark *new = calloc(1, sizeof(struct mark));

	if (!new)
		longjmp(zenv, -1);	/* ABORT */
	bmrktopnt(new);
	new->prev = Mrklist;		/* add to end of list */
	new->next = NULL;
	if (Mrklist)
		Mrklist->next = new;
	Mrklist = new;
	return new;
}

Boolean bcrsearch(Byte what)
{
	while (1) {
		if (Curchar <= 0)
			if (Curpage == Curbuff->firstp)
				return FALSE;
			else {
				makecur(Curpage->prevp);
				makeoffset(Curplen - 1);
			}
		else
			makeoffset(Curchar - 1);
		if (*Curcptr == what)
			return TRUE;
	}
}

Boolean bcsearch(Byte what)
{
	Byte *n;

	if (bisend())
		return FALSE;

	while ((n = (Byte *)memchr(Curcptr, what, Curplen - Curchar)) == NULL)
		if (Curpage == Curbuff->lastp) {
			makeoffset(Curplen);
			return FALSE;
		} else {
			makecur(Curpage->nextp);
			makeoffset(0);
		}

	makeoffset(n - Cpstart);
	bmove1();
	return TRUE;
}

/* Delete the buffer and its pages. */
Boolean bdelbuff(struct buff *tbuff)
{
	if (!tbuff)
		return TRUE;

	if (tbuff == Curbuff) { /* switch to a safe buffer */
		if (tbuff->next)
			bswitchto(tbuff->next);
		else if (tbuff->prev)
			bswitchto(tbuff->prev);
		else {
			error("Last Buffer.");
			return FALSE;
		}
	}

#ifdef PIPESH
	if (tbuff->child != EOF)
		unvoke(tbuff, TRUE);
#endif

	while (tbuff->firstp)	/* delete the pages */
		freepage(tbuff, tbuff->firstp);
	unmark(tbuff->mark);	/* free the user mark */
	if (tbuff == Bufflist)	/* unlink from the list */
		Bufflist = tbuff->next;
	if (tbuff->prev)
		tbuff->prev->next = tbuff->next;
	if (tbuff->next)
		tbuff->next->prev = tbuff->prev;
	free((char *)tbuff);	/* free the buffer proper */

	undo_clear(tbuff);

	return TRUE;
}

/* Delete quantity characters. */
void bdelete(unsigned quantity)
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
		Curplen -= quan;

		undo_del(quan);

		memmove(Curcptr, Curcptr + quan, Curplen - Curchar);
		if (Curpage == Curbuff->lastp)
			quantity = 0;
		else
			quantity -= quan;
		Curbuff->bmodf = MODIFIED;
		Curmodf = TRUE;
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
	vsetmod(TRUE);
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
int bgetcol(Boolean flag, int col)
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
void binsert(Byte new)
{
	register struct mark *btmark;

	if (Curplen == PSIZE && !pagesplit())
		return;
	memmove(Curcptr + 1, Curcptr, Curplen - Curchar);
	*Curcptr++ = new;
	++Curplen;
	++Curchar;
	Curbuff->bmodf = MODIFIED;
	Curmodf = TRUE;

	undo_add(1);

	for (btmark = Mrklist; btmark; btmark = btmark->prev)
		if (btmark->mpage == Curpage && btmark->moffset >= Curchar)
			++(btmark->moffset);
	vsetmod(FALSE);

}


/* Insert a string into the current buffer. */
void binstr(char *str)
{
	while (*str)
		binsert(*str++);
}


/* Returns TRUE if point is after the mark. */
Boolean bisaftermrk(struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != Curbuff)
		return FALSE;
	if (tmark->mpage == Curpage)
		return Curchar > tmark->moffset;
	for (tp = Curpage->prevp; tp && tp != tmark->mpage; tp = tp->prevp)
		;
	return tp != NULL;
}


/* True if the point precedes the mark. */
Boolean bisbeforemrk(struct mark *tmark)
{
	register struct page *tp;

	if (!tmark->mpage || tmark->mbuff != Curbuff)
		return FALSE;
	if (tmark->mpage == Curpage)
		return Curchar < tmark->moffset;
	for (tp = Curpage->nextp; tp && tp != tmark->mpage; tp = tp->nextp)
		;
	return tp != NULL;
}


/* Returns the length of the buffer. */
long blength(struct buff *tbuff)
{
	register struct page *tpage;
	struct page *spage;
	register long len;

	Curpage->plen = Curplen;
	spage = Curpage;
	for (len = 0, tpage = tbuff->firstp; tpage; tpage = tpage->nextp) {
		if (tpage->lines == EOF)
			makecur(tpage);
		len += tpage->plen;
	}
	makecur(spage);
	return len;
}


/* Return the current position of the point. */
unsigned long blocation(unsigned *lines)
{
	unsigned long len;
	struct page *tpage, *spage;

	spage = Curpage;
	len = 0l;
	if (lines)
		*lines = 1;
	for (tpage = Curbuff->firstp; tpage != spage; tpage = tpage->nextp) {
		if (tpage->lines == EOF) {
			makecur(tpage);
			tpage->lines = cntlines(Curplen);
		}
		if (lines)
			*lines += tpage->lines;
		len += tpage->plen;
	}
	makecur(spage);
	if (lines)
		*lines += cntlines(Curchar);
	return len + Curchar;
}


/* Number of lines in buffer */
long blines(struct buff *buff)
{
	unsigned long lines;
	struct page *tpage, *spage;

	if (Curmodf)
		Curpage->lines = EOF;
	spage = Curpage;
	lines = 1;

	for (tpage = buff->firstp; tpage; tpage = tpage->nextp) {
		if (tpage->lines == EOF) {
			makecur(tpage);
			tpage->lines = cntlines(Curplen);
		}
		lines += tpage->lines;
	}

	makecur(spage);
	return lines;
}


/* Try to put Point in a specific column.
 * Returns actual Point column.
 */
int bmakecol(int col, Boolean must)
{
	int tcol = 0;

	if (bcrsearch(NL))
		bmove1();
	while (tcol < col && !ISNL(*Curcptr) && !bisend()) {
		tcol += chwidth(*Curcptr, tcol, !must);
		bmove1();
	}
	if (must && tcol < col) {
		int wid = bwidth('\t', tcol);
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
Boolean bmove1(void)
{
	if (++Curchar < Curplen) {
		/* within current page */
		++Curcptr;
		return TRUE;
	}

	if (Curpage->nextp) {
		/* goto start of next page */
		makecur(Curpage->nextp);
		Curchar = 0;
		Curcptr = Cpstart;
		return TRUE;
	}

	/* At EOB */
	makeoffset(Curplen);
	return FALSE;
}

Boolean bmove(int dist)
{
	while (dist) {
		dist += Curchar;
		if (dist >= 0 && dist < Curplen) {
			/* within current page makeoffset dist */
			Curchar = dist;
			Curcptr = Cpstart + dist;
			return TRUE;
		}
		if (dist < 0) { /* goto previous page */
			if (Curpage == Curbuff->firstp) {
				/* past start of buffer */
				makeoffset(0);
				return FALSE;
			}
			makecur(Curpage->prevp);
			Curchar = Curplen; /* makeoffset Curplen */
			Curcptr = Cpstart + Curplen;
		} else {	/* goto next page */
			if (Curpage == Curbuff->lastp) {
				/* past end of buffer */
				makeoffset(Curplen);
				return FALSE;
			}
			dist -= Curplen; /* must use this Curplen */
			makecur(Curpage->nextp);
			Curchar = 0; /* makeoffset 0 */
			Curcptr = Cpstart;
		}
	}
	return TRUE;
}

/* bmove can only move the Point +/-32767 bytes. This routine overcomes
 * this limitation.
 * NOTE: Can only move forward.
 */
#define MAXMOVE		(0x7fff - 1024)

void boffset(unsigned long off)
{
	btostart();
	for (; off > MAXMOVE; off -= MAXMOVE)
		bmove(MAXMOVE);
	bmove(off);
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
	register struct mark *btmark;

	makecur(Curbuff->firstp);
	while (Curpage->nextp)
		freepage(Curbuff, Curpage->nextp);
	for (btmark = Mrklist; btmark; btmark = btmark->prev)
		if (btmark->mpage && btmark->mbuff == Curbuff) {
			btmark->mpage = Curpage;
			btmark->moffset = 0;
			btmark->modf = TRUE;
		}
	Curplen = Curchar = 0;		/* reset to start of page */
	Curcptr = Cpstart;
	Curmodf = TRUE;

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


void bswitchto(struct buff *new)
{
	if (new && new != Curbuff) {
		if (Curbuff) {
			Curbuff->pnt_page   = Curpage;
			Curbuff->pnt_offset = Curchar;
		}
		makecur(new->pnt_page);
		makeoffset(new->pnt_offset);
		Curbuff = new;
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


#define xbread(fd, blk, addr) read(fd, addr, PSIZE)

/*
Load the file 'fname' into the current buffer.
Returns  0	successfully opened file
		 1  ENOENT		no such file
		-1	EACCESS		file share violation
		-2	EMFILE		out of fds
*/
int breadfile(char *fname)
{
	char msg[PATHMAX + 20];
	struct stat sbuf;
	int fd, len, blk;

	fd = open(fname, READ_MODE);
	if (fd < 0  || fstat(fd, &sbuf) == EOF) {
		if (fd >= 0)
			close(fd);

		switch (errno) {
		case EACCES:
			sprintf(msg, "No read access: %s", fname);
			error(msg);
			return -1;
		case EMFILE:
			error("Out of File Descriptors.");
			return -2;
		default:
			return 1;
		}
	}

	Curbuff->mtime = sbuf.st_mtime;		/* save the modified time */
	sprintf(msg, "Reading %s", lastpart(fname));
	echo(msg);

	bempty();

	for (blk = 0; (len = xbread(fd, blk, Cpstart)) > 0; ++blk) {
		Curplen = len;
		makeoffset(0);
		Curmodf = TRUE;
		if (!newpage(Curbuff, Curpage, NULL))
			break;
		makecur(Curpage->nextp);
	}
	if (blk > 0 && Curplen == 0)
		freepage(Curbuff, Curpage); /* whoops - alloced 1 too many */
	(void)close(fd);

	btostart();
	Curbuff->bmodf = FALSE;
	clrecho();

	return 0;
}


/*	Write the current buffer to an open file descriptor.
 *	Returns:	TRUE	if write successfull
 *			FALSE	if write failed
 */
int bwritefd(int fd)
{
	struct mark pmark;				/* no mallocs! */
	struct page *tpage;
	struct stat sbuf;
	int status = TRUE;

	bmrktopnt(&pmark);
	for (tpage = Curbuff->firstp; tpage && status; tpage = tpage->nextp) {
		makecur(tpage);
		status = xbput(fd, Cpstart, Curplen);
	}
	/* flush the buffers */
	status &= xbput(fd, NULL, EOF);
	if (status) {
		/* get the time here - on some machines (SUN) 'time'
		 * incorrect */
		fstat(fd, &sbuf);
		Curbuff->mtime = sbuf.st_mtime;
	} else
		error("Unable to write file.");
	(void)close(fd);
	bpnttomrk(&pmark);

	if (status)
		Curbuff->bmodf = FALSE;

	return status;
}

static char *make_bakname(char *bakname, char *fname)
{
	strcpy(bakname, fname);
	strcat(bakname, "~");
	return bakname;
}

static Boolean cp(char *from, char *to)
{
	FILE *in, *out;
	char buf[1024];
	int n, rc = TRUE;

	in = fopen(from, "r");
	out = fopen(to, "w");
	if (!in || !out) {
		if (!in)
			fclose(in);
		return FALSE;
	}
	while ((n = fread(buf, 1, 1024, in)) > 0)
		if (fwrite(buf, 1, n, out) != n) {
			rc = FALSE;
			break;
		}
	fclose(in);
	fclose(out);
	return rc;
}

/*	Write the current buffer to the file 'fname'.
 *	Handles the backup scheme according to VAR(VBACKUP).
 *	Returns:	TRUE	if write successfull
 *				FALSE	if write failed
 *				ABORT	if user didn't want to overwrite
 */
int bwritefile(char *fname)
{
	char bakname[PATHMAX + 1];
	int fd, mode, status = TRUE, bak = FALSE;
	struct stat sbuf;
	int nlink;

	if (!fname)
		return TRUE;

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
	fd = open(fname, WRITE_MODE, mode);
	if (fd != EOF)
		status = bwritefd(fd);
	else {
		if (errno == EACCES)
			error("File is read only.");
		else
			error("Unable to open file.");
		status = FALSE;
	}

	/* cleanup */
	if (status)
		clrecho();
	else if (bak) {
		if (sbuf.st_nlink) {
			cp(bakname, fname);
			unlink(bakname);
		} else
			rename(bakname, fname);
	}

	return status;
}

/* count the lines (NLs) in the current page up to offset 'stop' */
int cntlines(int stop)
{
	Byte *p, *n;
	int lines = 0, end;

	for (p = Cpstart, end = stop;
	     (n = (Byte *)memchr(p, NL, end));
	     ++lines, p = n) {
		++n;
		end -= n - p;
	}
	return lines;
}

/* Make the point be dist chars into the page. */
void makeoffset(int dist)
{
	Curchar = dist;
	Curcptr = Cpstart + dist;
}

/* True if mark1 follows mark2 */
Boolean mrkaftermrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return FALSE;        /* marks in different buffers */
	if (mark1->mpage == mark2->mpage)
		return mark1->moffset > mark2->moffset;
	for (tpage = mark1->mpage->prevp;
	     tpage && tpage != mark2->mpage;
	     tpage = tpage->prevp)
		;

	return tpage != NULL;
}

/* True if mark1 is at mark2 */
Boolean mrkatmrk(struct mark *mark1, struct mark *mark2)
{
	return  mark1->mbuff == mark2->mbuff &&
		mark1->mpage == mark2->mpage &&
		mark1->moffset == mark2->moffset;
}

/* True if mark1 precedes mark2 */
Boolean mrkbeforemrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return FALSE;        /* Marks not in same buffer */
	if (mark1->mpage == mark2->mpage)
		return mark1->moffset < mark2->moffset;
	for (tpage = mark1->mpage->nextp;
	     tpage && tpage != mark2->mpage;
	     tpage = tpage->nextp)
		;
	return tpage != NULL;
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
		free((char *)mptr);
	}
}


/* Low level memory buffer routines */

/* Create a new memory page and link into chain */
static struct page *newpage(struct buff *tbuff,
			    struct page *ppage, struct page *npage)
{
	struct page *new = calloc(1, sizeof(struct page));

	if (new) {
		new->nextp = npage;
		new->prevp = ppage;
		npage ? (npage->prevp = new) : (tbuff->lastp = new);
		ppage ? (ppage->nextp = new) : (tbuff->firstp = new);
		new->lines = EOF;	/* undefined */
		++NumPages;
	}

	return new;
}

/* Free a memory page */
static void freepage(struct buff *tbuff, struct page *page)
{
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
	if (Curpage) {
		Curpage->plen = Curplen;
		if (Curmodf || Curpage->lines == EOF)
			Curpage->lines = cntlines(Curplen);
	}
	Curpage = page;
	Cpstart = page->pdata;
	Curmodf = FALSE;
	Curplen = Curpage->plen;
}

/* Split the current (full) page. */
static Boolean pagesplit(void)
{
	struct page *new;
	struct mark *btmark;

	new = newpage(Curbuff, Curpage, Curpage->nextp);
	if (new == NULL)
		return FALSE;

	memmove(new->pdata, Cpstart + HALFP, HALFP);
	Curmodf = TRUE;
	Curplen = HALFP;
	new->plen = HALFP;
	for (btmark = Mrklist; btmark; btmark = btmark->prev)
		if (btmark->mpage == Curpage && btmark->moffset >= HALFP) {
			btmark->mpage = new;
			btmark->moffset -= HALFP;
		}
	if (Curchar >= HALFP) {
		/* new page has Point in it */
		makecur(new);
		makeoffset(Curchar - HALFP);
	}
	return TRUE;
}

/* High level write for non-block writes.
 * SLOW_DISK version buffers up the input until a PSIZE block is
 * reached, then sends it to XBwrite.  Can only be used on ONE file at
 * a time!  A 'xbput(fd, NULL, EOF)' call should be made before
 * closing the file to handle AddNL.
 */
#if SLOW_DISK
static Boolean xbput(int fd, Byte *addr, unsigned len)
{
	static Byte buf[PSIZE];
	static int buflen, lastch;
	int wrlen, rc = TRUE;

	if (len == 0)
		return TRUE;
	if (len == EOF) {
		/* flush the buffer and reset */
		if (VAR(VADDNL)) {
			if (buflen > 0)
				lastch = buf[buflen - 1];
			if (lastch != '\n')
				xbput(fd, (Byte *)"\n", 1);
		}
		rc = write(fd, buf, buflen) == buflen;
		buflen = lastch = 0;
	} else {
		wrlen = (buflen + len > PSIZE) ? (PSIZE - buflen) : len;
		memcpy(&buf[buflen], addr, wrlen);
		buflen += wrlen;
		if (buflen == PSIZE) {
			rc = write(fd, buf, PSIZE) == PSIZE;
			lastch = buf[PSIZE - 1];
			buflen = 0;
			rc &= xbput(fd, &addr[wrlen], len - wrlen);
		}
	}
	return rc;
}
#else
static Boolean xbput(int fd, Byte *addr, unsigned len)
{
	static int lastch;

	if (len == 0)
		return TRUE;
	if (len == EOF) {
		/* handle ADDNL */
		if (VAR(VADDNL) && lastch != '\n') {
			char buf = '\n';
			return write(fd, &buf, 1) == 1;
		} else {
			lastch = 0;
			return TRUE;
		}
	}

	lastch = addr[len - 1];
	return write(fd, addr, len) == len;
}
#endif

void Zstat(void)
{
	sprintf(PawStr, "Buffers: %d   Pages: %d", Numbuffs, NumPages);
	echo(PawStr);
}
