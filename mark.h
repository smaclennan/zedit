/* mark.h - low level mark defines
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

#ifndef _mark_h
#define _mark_h

#include <stdbool.h>

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

extern int NumMarks; /* stats */

struct mark *bcremark(struct buff *);
void bdelmark(struct mark *);

struct mark *_bcremark(struct buff *buff, struct mark **tail);
void _bdelmark(struct mark *mark, struct mark **tail);

#define bisatmrk(b, m)	(((b)->curpage == (m)->mpage) && ((b)->curchar == (m)->moffset))
bool bisaftermrk(struct buff *, struct mark *);
bool bisbeforemrk(struct buff *, struct mark *);
void bmrktopnt(struct buff *, struct mark *);
bool bpnttomrk(struct buff *, struct mark *);
bool bswappnt(struct buff *, struct mark *);

void mrktomrk(struct mark *, struct mark *);
bool mrkaftermrk(struct mark *, struct mark *);
bool mrkatmrk(struct mark *, struct mark *);
bool mrkbeforemrk(struct mark *, struct mark *);

long bcopyrgn(struct mark *, struct buff*);
long bdeltomrk(struct mark *);

#ifdef HAVE_GLOBAL_MARKS
/**
 * Global mark list. The buffer code keeps the marks in this list
 * up to date.
 */
extern struct mark *Marklist;

#define foreach_global_pagemark(buff, mark, page)				   \
	for ((mark) = Marklist; (mark); (mark) = (mark)->prev) \
		if ((mark)->mpage == (page))

#define foreach_global_buffmark(buff, mark)				   \
	for ((mark) = Marklist; (mark); (mark) = (mark)->prev) \
		if ((mark)->mbuff == (buff))
#else
#define foreach_global_pagemark(buff, mark, page) if (0)
#define foreach_global_buffmark(buff, mark) if (0)
#endif

#ifdef HAVE_BUFFER_MARKS
#define foreach_pagemark(buff, mark, page)						 \
	for ((mark) = (buff)->marks; (mark); (mark) = (mark)->prev)	 \
		if ((mark)->mpage == (page))

#define foreach_buffmark(buff, mark)							\
	for ((mark) = (buff)->marks; (mark); (mark) = (mark)->prev)
#else
#define foreach_pagemark(buff, mark, page) if (0)
#define foreach_buffmark(buff, mark) if (0)
#endif

#ifdef HAVE_FREEMARK
/**
 * The freemark keeps one mark around so that bcremark() can use the
 * freemark rather than allocating a new mark. Since a lot of
 * functions just need to allocate one mark, this is a huge win for
 * very little code.
 */
extern struct mark *freemark;
#endif

#endif /* _mark_h */
