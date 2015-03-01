#ifdef HAVE_MARKS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#include "buff.h"
#include "mark.h"

#ifdef HAVE_GLOBAL_MARKS
struct mark *Marklist;	/* the marks list tail */
static struct mark *mhead;
#endif

int NumMarks;

#ifndef HAVE_THREADS
/* Keeping just one mark around is a HUGE win for a trivial amount of code. */
static struct mark *freemark;

static void mfini(void)
{
#ifdef HAVE_GLOBAL_MARKS
	if (mhead) {
		if (mhead->next)
			mhead->next->prev = NULL;
		else
			Marklist = NULL;
	}

	while (Marklist)
		unmark(Marklist);
#endif

	if (freemark)
		free(freemark); /* don't unmark */

}

void minit(struct mark *preallocated)
{
#ifdef HAVE_GLOBAL_MARKS
	Marklist = preallocated;
	mhead = preallocated;
#endif
	atexit(mfini);
}
#endif /* !HAVE_THREADS */

/* Create a mark at the current point and add it to the list. */
struct mark *bcremrk(struct buff *buff)
{
	struct mark *mrk;

#ifdef HAVE_THREADS
	mrk = (struct mark *)calloc(1, sizeof(struct mark));
#else
	if (freemark) {
		mrk = freemark;
		freemark = NULL;
	} else
		mrk = (struct mark *)calloc(1, sizeof(struct mark));
#endif
	if (mrk) {
#ifdef HAVE_GLOBAL_MARKS
		struct mark **head = &Marklist;
#else
		struct mark **head = &buff->marks;
#endif

		bmrktopnt(buff, mrk);
		mrk->prev = *head; /* add to end of list */
		mrk->next = NULL;
		if (*head) (*head)->next = mrk;
		*head = mrk;
		++NumMarks;
	}
	return mrk;
}

/* Free up the given mark and remove it from the list. */
void unmark(struct mark *mptr)
{
	if (mptr) {
#ifdef HAVE_GLOBAL_MARKS
		if (mptr == Marklist)
			Marklist = mptr->prev;
#else
		if (mptr == mptr->mbuff->marks)
			mptr->mbuff->marks = mptr->prev;
#endif
		if (mptr->prev)
			mptr->prev->next = mptr->next;
		if (mptr->next)
			mptr->next->prev = mptr->prev;

#ifndef HAVE_THREADS
		if (!freemark)
			freemark = mptr;
		else
#endif
			free((char *)mptr);
		--NumMarks;
	}
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
	/* SAM WARNING: Zedit used to do bswitchto if tmark->mbuff != buff */
	if (tmark->mbuff != buff)
		return false;
	if (tmark->mpage)
		makecur(buff, tmark->mpage, tmark->moffset);
	return true;
}

/* Swap the point and the mark. */
void bswappnt(struct buff *buff, struct mark *tmark)
{
	struct mark tmp;

	tmp.mbuff	= buff; /* Point not moved out of its buffer */
	tmp.mpage	= tmark->mpage;
	tmp.moffset	= tmark->moffset;
	bmrktopnt(buff, tmark);
	bpnttomrk(buff, &tmp);
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


#endif /* HAVE_MARKS */
