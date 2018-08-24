#include "buff.h"

#ifdef HAVE_ATOMIC
/**
 * The freemark keeps one mark around so that bcremark() can use the
 * freemark rather than allocating a new mark. Since a lot of
 * functions just need to allocate one mark, this is a huge win for
 * very little code.
 */
static struct mark *freemark;
#endif

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

/**
 * If you are pedantic about freeing all memory you should call this
 * at exit.
 */
void mrkfini(void)
{	/* This is only here since it needs to know about freemark */
#ifdef HAVE_ATOMIC
	FREE(freemark);
	freemark = NULL;
#endif
}
