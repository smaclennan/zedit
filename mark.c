#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#include "buff.h"

#ifdef HAVE_GLOBAL_MARKS
struct mark *Marklist;	/* the marks list tail */
#endif

#ifdef HAVE_FREEMARK
/* Keeping one freemark is a huge win for very little code in single
 * threaded apps.
 */
struct mark *freemark;
#endif

int NumMarks;

struct mark *_bcremark(struct buff *buff, struct mark **tail)
{
	struct mark *mptr;

#ifdef HAVE_FREEMARK
	if (freemark) {
		mptr = freemark;
		freemark = NULL;
	} else
#endif
		mptr = (struct mark *)calloc(1, sizeof(struct mark));
	if (mptr) {
#if defined(HAVE_GLOBAL_MARKS) || defined(HAVE_BUFFER_MARKS)
		if (tail) {
			mptr->prev = *tail; /* add to end of list */
			if (*tail)
				(*tail)->next = mptr;
			*tail = mptr;
		}
#endif
		bmrktopnt(buff, mptr);
		++NumMarks;
	}
	return mptr;
}

void _bdelmark(struct mark *mptr, struct mark **tail)
{
	if (mptr) {
#if defined(HAVE_GLOBAL_MARKS) || defined(HAVE_BUFFER_MARKS)
		if (tail) {
			if (mptr == *tail)
				*tail = mptr->prev;
			if (mptr->prev)
				mptr->prev->next = mptr->next;
			if (mptr->next)
				mptr->next->prev = mptr->prev;
		}
#endif
#ifdef HAVE_FREEMARK
		if (freemark == NULL) {
			freemark = mptr;
			freemark->prev = freemark->next = NULL;
		} else
#endif
			free((char *)mptr);
		--NumMarks;
	}
}

/* Create a mark at the current point and add it to the list. */
struct mark *bcremark(struct buff *buff)
{
#ifdef HAVE_BUFFER_MARKS
	return _bcremark(buff, &buff->marks);
#else
	return _bcremark(buff, NULL);
#endif
}

/* Free up the given mark and remove it from the list. */
void bdelmark(struct mark *mptr)
{
#ifdef HAVE_BUFFER_MARKS
	_bdelmark(mptr, &mptr->mbuff->marks);
#else
	_bdelmark(mptr, NULL);
#endif
}

/* Returns true if point is after the mark. */
bool bisaftermrk(struct buff *buff, struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != buff)
		return false;
	if (tmark->mpage == buff->curpage)
		return buff->curchar > tmark->moffset;
	for (tp = buff->curpage->prevp; tp && tp != tmark->mpage; tp = tp->prevp)
		;
	return tp != NULL;
}

/* True if the point precedes the mark. */
bool bisbeforemrk(struct buff *buff, struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != buff)
		return false;
	if (tmark->mpage == buff->curpage)
		return buff->curchar < tmark->moffset;
	for (tp = buff->curpage->nextp; tp && tp != tmark->mpage; tp = tp->nextp)
		;
	return tp != NULL;
}

/* Put the mark where the point is. */
void bmrktopnt(struct buff *buff, struct mark *tmark)
{
	tmark->mbuff   = buff;
	tmark->mpage   = buff->curpage;
	tmark->moffset = buff->curchar;
}

/* Put the current buffer point at the mark */
bool bpnttomrk(struct buff *buff, struct mark *tmark)
{
	if (tmark->mbuff != buff)
		return false;
	if (tmark->mpage)
		makecur(buff, tmark->mpage, tmark->moffset);
	return true;
}

void mrktomrk(struct mark *m1, struct mark *m2)
{
	m1->mbuff = m2->mbuff;
	m1->mpage = m2->mpage;
	m1->moffset = m2->moffset;
}

/* Swap the point and the mark. */
bool bswappnt(struct buff *buff, struct mark *tmark)
{
	if (tmark->mbuff != buff)
		return false;

	struct mark tmp;
	bmrktopnt(buff, &tmp);
	bpnttomrk(buff, tmark);
	mrktomrk(tmark, &tmp);
	return true;
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

/* Copy from Point to mark to buffer 'to'. Returns bytes copied. */
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

	if (!(ltmrk = bcremark(from)))
		return 0;

	while (bisbeforemrk(from, tmark)) {
		if (from->curpage == tmark->mpage)
			srclen = tmark->moffset - from->curchar;
		else
			srclen = curplen(from) - from->curchar;

		dstlen = PSIZE - curplen(to);
		if (dstlen == 0) {
			if (pagesplit(to, HALFP))
				dstlen = PSIZE - curplen(to);
			else
				break;
		}
		if (srclen < dstlen)
			dstlen = srclen;
		/* Make a gap */
		memmove(to->curcptr + dstlen, to->curcptr, curplen(to) - to->curchar);
		/* and fill it in */
		memmove(to->curcptr, from->curcptr, dstlen);
		curplen(to) += dstlen;
		copied += dstlen;
		foreach_global_pagemark(to, btmrk, to->curpage)
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

/* Delete from the point to the Mark. Returns bytes deleted. */
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
			amount = curplen(buff) = buff->curchar;
		bdelete(buff, amount);
		deleted += amount;
	}

	return deleted;
}
