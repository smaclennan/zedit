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

struct mark {
	struct buff *mbuff;			/* buffer the mark is in */
	struct page *mpage;			/* page in the buffer */
	unsigned moffset;			/* offset in the page */
#if defined(HAVE_GLOBAL_MARKS) || defined(HAVE_BUFFER_MARKS)
	struct mark *prev, *next;	/* list of marks */
#endif
};

extern int NumMarks; /* stats */

struct mark *bcremark(struct buff *);
void bdelmark(struct mark *);

#define bisatmrk(b, m)	(((b)->curpage == (m)->mpage) && ((b)->curchar == (m)->moffset))
bool bisaftermrk(struct buff *, struct mark *);
bool bisbeforemrk(struct buff *, struct mark *);
void bmrktopnt(struct buff *, struct mark *);
bool bpnttomrk(struct buff *, struct mark *);
bool bswappnt(struct buff *, struct mark *);

void mrktomrk(struct mark *m1, struct mark *m2);
bool mrkaftermrk(struct mark *, struct mark *);
bool mrkatmrk(struct mark *, struct mark *);
bool mrkbeforemrk(struct mark *, struct mark *);

#ifdef HAVE_GLOBAL_MARKS
extern struct mark *Marklist;
#define MARKS_HEAD(buff) Marklist

#define foreach_pagemark(buff, mark, page)				   \
	for ((mark) = Marklist; (mark); (mark) = (mark)->prev) \
		if ((mark)->mpage == (page))

#define foreach_buffmark(buff, mark)				   \
	for ((mark) = Marklist; (mark); (mark) = (mark)->prev) \
		if ((mark)->mbuff == (buff))
#elif defined(HAVE_BUFFER_MARKS)
#define MARKS_HEAD(buff) ((buff)->marks)

#define foreach_pagemark(buff, mark, page)						 \
	for ((mark) = (buff)->marks; (mark); (mark) = (mark)->prev)	 \
		if ((mark)->mpage == (page))

#define foreach_buffmark(buff, mark)							\
	for ((mark) = (buff)->marks; (mark); (mark) = (mark)->prev)
#else
#define foreach_pagemark(buff, mark, page) if (0)
#define foreach_buffmark(buff, mark) if (0)
#endif

#endif /* _mark_h */
