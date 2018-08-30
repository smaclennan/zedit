/* mark.h - low level mark defines
 * Copyright (C) 1988-2017 Sean MacLennan
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

#ifndef _mark_h
#define _mark_h

#include <string.h>

/**
 * The static and/or dynamic mark structure.
 */
struct mark {
	struct buff *mbuff;			/**< Buffer the mark is in. */
	struct page *mpage;			/**< Page in the buffer. */
	unsigned moffset;			/**< Offset in the page. */
#if defined(HAVE_GLOBAL_MARKS) || defined(HAVE_BUFFER_MARKS)
	struct mark *prev;			/**< List of marks. */
	struct mark *next;			/**< List of marks. */
#endif
};
/** Size of mark struct without list pointers */
#define __MRKSIZE (sizeof(void *) * 2 + sizeof(unsigned))

struct mark *bcremark(struct buff *);
void bdelmark(struct mark *);

struct mark *_bcremark(struct buff *buff, struct mark **tail);
void _bdelmark(struct mark *mark, struct mark **tail);

/** Returns 1 if buffer is at mark */
static inline int bisatmrk(struct buff *b, struct mark *m)
{
	return b->curpage == m->mpage && b->curchar == m->moffset;
}
int bisaftermrk(struct buff *, struct mark *);
int bisbeforemrk(struct buff *, struct mark *);
int bpnttomrk(struct buff *, struct mark *);
int bswappnt(struct buff *, struct mark *);
void mrkfini(void);

int mrkaftermrk(struct mark *, struct mark *);
int mrkbeforemrk(struct mark *, struct mark *);

/** Move the mark to the point. */
static inline void bmrktopnt(struct buff *buff, struct mark *tmark)
{
	tmark->mbuff   = buff;
	tmark->mpage   = buff->curpage;
	tmark->moffset = buff->curchar;
}

/** Move mark 1 to mark 2 */
static inline void mrktomrk(struct mark *m1, struct mark *m2)
{
	memcpy(m1, m2, __MRKSIZE);
}

/** True if mark1 is at mark2 */
static inline int mrkatmrk(struct mark *m1, struct mark *m2)
{
	return memcmp(m1, m2, __MRKSIZE) == 0;
}

#ifdef HAVE_GLOBAL_MARKS
extern struct mark *Marklist;

/** Walk through all the global marks that match page */
#define foreach_global_pagemark(mark, page)		       \
	for ((mark) = Marklist; (mark); (mark) = (mark)->prev) \
		if ((mark)->mpage == (page))

/** Walk through all the global marks that match buff */
#define foreach_global_buffmark(buff, mark)		       \
	for ((mark) = Marklist; (mark); (mark) = (mark)->prev) \
		if ((mark)->mbuff == (buff))
#else
#define foreach_global_pagemark(mark, page) if (0)
#define foreach_global_buffmark(buff, mark) if (0)
#endif

#ifdef HAVE_BUFFER_MARKS
/** Walk through all the buffer marks in buff that match page */
#define foreach_pagemark(buff, mark, page)				\
	for ((mark) = (buff)->marks; (mark); (mark) = (mark)->prev)	\
		if ((mark)->mpage == (page))

/** Walk through all the buffer marks in buff */
#define foreach_buffmark(buff, mark)					\
	for ((mark) = (buff)->marks; (mark); (mark) = (mark)->prev)
#else
#define foreach_pagemark(buff, mark, page) if (0)
#define foreach_buffmark(buff, mark) if (0)
#endif

#endif /* _mark_h */