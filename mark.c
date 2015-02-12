#ifdef HAVE_MARKS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#include "buff.h"
#include "mark.h"
#include "page.h"

#ifdef ZEDIT
#include "z.h"
#else
static inline void vsetmod(bool flag) {}
#endif

struct mark *Mrklist;	/* the marks list tail */
static struct mark *mhead;
int NumMarks;

/* Keeping just one mark around is a HUGE win for a trivial amount of code. */
static struct mark *freemark;

static void mfini(void)
{
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
}

void minit(struct mark *preallocated)
{
	Mrklist = preallocated;
	mhead = preallocated;

	atexit(mfini);
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
			if (bpagesplit())
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

#endif /* HAVE_MARKS */
