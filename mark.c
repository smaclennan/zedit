/* mark.c - low level mark functions
 * Copyright (C) 1988-2016 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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

#include "buff.h"

#ifdef HAVE_GLOBAL_MARKS
/** Global mark list. The buffer code keeps the marks in this list
 * up to date.
 */
struct mark *Marklist;	/* the marks list tail */
#endif

#ifdef HAVE_ATOMIC
/**
 * The freemark keeps one mark around so that bcremark() can use the
 * freemark rather than allocating a new mark. Since a lot of
 * functions just need to allocate one mark, this is a huge win for
 * very little code.
 */
static struct mark *freemark;
#endif

/**
 * If you are pedantic about freeing all memory you should call this
 * at exit.
 */
void mrkfini(void)
{
#ifdef HAVE_ATOMIC
	if (freemark) {
		free(freemark);
		freemark = NULL;
	}
#endif
}

/** Low-level create mark.
 *
 * This is a helper function that creates a mark on a mark list. For
 * example, it could be used to create marks on the global
 * #Marklist. Normally you would use bcremark().
 */
struct mark *_bcremark(struct buff *buff, struct mark **tail)
{
	struct mark *mptr;

#ifdef HAVE_ATOMIC
	mptr = atomic_exchange(&freemark, freemark, NULL);
	if (!mptr)
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
	}
	return mptr;
}

/** Low-level delete mark. You probably want bdelmark(). */
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
		mptr->prev = mptr->next = NULL;
#endif
#ifdef HAVE_ATOMIC
		if (atomic_exchange(&freemark, NULL, mptr))
#endif
			free((char *)mptr);
	}
}

/** Create a mark in the specified buffer.
 *
 * If dynamic marks are enabled (#HAVE_GLOBAL_MARKS or
 * #HAVE_BUFFER_MARKS) then the buffer code will keep the mark in place
 * as bytes are inserted or deleted.
 */
struct mark *bcremark(struct buff *buff)
{
#ifdef HAVE_BUFFER_MARKS
	return _bcremark(buff, &buff->marks);
#else
	return _bcremark(buff, NULL);
#endif
}

/** Free up the given mark and remove it from the list.  The mark
 * should be created with bcremark(). The mark can be NULL.
 */
void bdelmark(struct mark *mptr)
{
#ifdef HAVE_BUFFER_MARKS
	if (mptr)
		_bdelmark(mptr, &mptr->mbuff->marks);
#else
	_bdelmark(mptr, NULL);
#endif
}

/** Returns true if point is after the mark. */
bool bisaftermrk(struct buff *buff, struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != buff)
		return false;
	if (tmark->mpage == buff->curpage)
		return buff->curchar > tmark->moffset;
	if (tmark->mpage == buff->curpage->nextp)
		return false;
	for (tp = buff->curpage->prevp; tp && tp != tmark->mpage; tp = tp->prevp)
		;
	return tp != NULL;
}

/** Return true if the point precedes the mark. */
bool bisbeforemrk(struct buff *buff, struct mark *tmark)
{
	struct page *tp;

	if (!tmark->mpage || tmark->mbuff != buff)
		return false;
	if (tmark->mpage == buff->curpage)
		return buff->curchar < tmark->moffset;
	if (tmark->mpage == buff->curpage->prevp)
		return false;
	for (tp = buff->curpage->nextp; tp && tp != tmark->mpage; tp = tp->nextp)
		;
	return tp != NULL;
}

/** Returns true if point is between start and end. This has to walk
 * all the pages between start and end. So it is most efficient for
 * small ranges and terrible if end is before start.
 *
 * Note: point == start == end returns false: it is not between.
 */
bool bisbetweenmrks(struct buff *buff, struct mark *start, struct mark *end)
{
	struct page *tp;
	bool found = false;

	if (start->mbuff != buff || end->mbuff != buff)
		return false;

	if (buff->curpage == start->mpage)
		if (buff->curchar < start->moffset)
			return false;

	for (tp = start->mpage; tp; tp = tp->nextp)
		if (tp == buff->curpage) {
			if (tp == end->mpage)
				return buff->curchar < end->moffset;
			found = true;
		} else if (tp == end->mpage)
			return found;

	return false;
}

/** Put the current buffer point at the mark */
bool bpnttomrk(struct buff *buff, struct mark *tmark)
{
	if (tmark->mbuff != buff)
		return false;
	if (tmark->mpage)
		makecur(buff, tmark->mpage, tmark->moffset);
	return true;
}

/** Swap the point and the mark. */
bool bswappnt(struct buff *buff, struct mark *tmark)
{
	struct mark tmp;

	if (tmark->mbuff != buff)
		return false;

	bmrktopnt(buff, &tmp);
	bpnttomrk(buff, tmark);
	mrktomrk(tmark, &tmp);
	return true;
}

/** True if mark1 precedes mark2 */
bool mrkbeforemrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return false;        /* Marks not in same buffer */

	if (mark1->mpage == mark2->mpage)
		return mark1->moffset < mark2->moffset;
	if (mark1->mpage->prevp == mark2->mpage)
		return false;

	for (tpage = mark1->mpage->nextp;
		 tpage && tpage != mark2->mpage;
		 tpage = tpage->nextp)
		;
	return tpage != NULL;
}

/** True if mark1 follows mark2 */
bool mrkaftermrk(struct mark *mark1, struct mark *mark2)
{
	struct page *tpage;

	if (!mark1->mpage || !mark2->mpage || mark1->mbuff != mark2->mbuff)
		return false;        /* marks in different buffers */

	if (mark1->mpage == mark2->mpage)
		return mark1->moffset > mark2->moffset;
	if (mark1->mpage->nextp == mark2->mpage)
		return false;

	for (tpage = mark1->mpage->prevp;
		 tpage && tpage != mark2->mpage;
		 tpage = tpage->prevp)
		;
	return tpage != NULL;
}
