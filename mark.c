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
#define MARKS_TAIL(buff) Marklist

/* Keeping one freemark is a huge win for very little code. However,
 * it is really only a gain when single threaded.
 */
static struct mark *freemark;
#elif defined(HAVE_BUFFER_MARKS)
#define MARKS_TAIL(buff) ((buff)->marks)
#endif

int NumMarks;

/* Create a mark at the current point and add it to the list. */
struct mark *bcremark(struct buff *buff)
{
	struct mark *mrk;

#ifdef HAVE_GLOBAL_MARKS
	if (freemark) {
		mrk = freemark;
		freemark = NULL;
	} else
#endif
		mrk = (struct mark *)calloc(1, sizeof(struct mark));
	if (mrk) {
#ifdef MARKS_TAIL
		struct mark **head = &MARKS_TAIL(buff);
		mrk->prev = *head; /* add to end of list */
		if (*head) (*head)->next = mrk;
		*head = mrk;
#endif
		bmrktopnt(buff, mrk);
		++NumMarks;
	}
	return mrk;
}

/* Free up the given mark and remove it from the list. */
void bdelmark(struct mark *mptr)
{
	if (mptr) {
#ifdef MARKS_TAIL
		if (mptr == MARKS_TAIL(mptr->mbuff))
			MARKS_TAIL(mptr->mbuff) = mptr->prev;
		if (mptr->prev)
			mptr->prev->next = mptr->next;
		if (mptr->next)
			mptr->next->prev = mptr->prev;
#endif
#ifdef HAVE_GLOBAL_MARKS
		if (freemark == NULL) {
			freemark = mptr;
			freemark->prev = freemark->next = NULL;
		} else
#endif
			free((char *)mptr);
		--NumMarks;
	}
}

void bdeleteallmarks(void)
{
	while (Marklist)
		bdelmark(Marklist);

#ifdef HAVE_GLOBAL_MARKS
	if (freemark) {
		free(freemark);
		freemark = NULL;
	}
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
