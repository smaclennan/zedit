/* bcremark.c - create a mark
 * Copyright (C) 1988-2018 Sean MacLennan <seanm@seanm.ca>
 *
 * This program is free software; you can redistribute it and/or modify
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

#include "buff.h"

/** @addtogroup buffer
 * @{
 */

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
 *
 * @param buff The buffer to create the mark in.
 * @param tail The tail of the mark list to add the mark to.
 * @return The newly created mark or NULL.
 */
struct mark *_bcremark(struct buff *buff, struct mark **tail)
{
	struct mark *mark;

#ifdef HAVE_ATOMIC
	mark = atomic_exchange(&freemark, freemark, NULL);
	if (!mark)
#endif
		mark = (struct mark *)calloc(1, sizeof(struct mark));
	if (mark) {
#if defined(HAVE_GLOBAL_MARKS) || defined(HAVE_BUFFER_MARKS)
		if (tail) {
			mark->prev = *tail; /* add to end of list */
			if (*tail)
				(*tail)->next = mark;
			*tail = mark;
		}
#endif
		bmrktopnt(buff, mark);
	}
	return mark;
}

/** Create a mark in the specified buffer.
 *
 * If dynamic marks are enabled (#HAVE_GLOBAL_MARKS or
 * #HAVE_BUFFER_MARKS) then the buffer code will keep the mark in place
 * as bytes are inserted or deleted.
 *
 * @param buff The buffer to create the mark in.
 * @return The newly created mark or NULL.
 */
struct mark *bcremark(struct buff *buff)
{
#ifdef HAVE_BUFFER_MARKS
	return _bcremark(buff, &buff->marks);
#else
	return _bcremark(buff, NULL);
#endif
}

/** Low-level delete mark. You probably want bdelmark().
 * @param mark The mark to delete. Can be NULL.
 * @param tail The mark list to remove the mark from.
 */
void _bdelmark(struct mark *mark, struct mark **tail)
{
	if (mark) {
#if defined(HAVE_GLOBAL_MARKS) || defined(HAVE_BUFFER_MARKS)
		if (tail) {
			if (mark == *tail)
				*tail = mark->prev;
			if (mark->prev)
				mark->prev->next = mark->next;
			if (mark->next)
				mark->next->prev = mark->prev;
		}
		mark->prev = mark->next = NULL;
#endif
#ifdef HAVE_ATOMIC
		if (atomic_exchange(&freemark, NULL, mark))
#endif
			free((char *)mark);
	}
}

/** Free up the given mark and remove it from the list.  The mark
 * should be created with bcremark().
 *
 * @param mark The mark to delete. Can be NULL.
 */
void bdelmark(struct mark *mark)
{
#ifdef HAVE_BUFFER_MARKS
	if (mark)
		_bdelmark(mark, &mark->mbuff->marks);
#else
	_bdelmark(mark, NULL);
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
/* @} */
