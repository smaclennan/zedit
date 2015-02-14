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

#ifdef HAVE_MARKS
#ifndef _mark_h
#define _mark_h

struct mark {
	struct buff *mbuff;			/* buffer the mark is in */
	struct page *mpage;			/* page in the buffer */
	int moffset;				/* offset in the page */
	struct mark *prev, *next;	/* list of marks */
};

extern struct mark *Marklist;
extern int NumMarks; /* stats */

#define MRKSIZE		(sizeof(struct mark) - (sizeof(struct mark *) << 1))

#define bisatmrk(m)	((Curpage == (m)->mpage) && (Curchar == (m)->moffset))
#define mrktomrk(m1, m2) memcpy(m1, m2, MRKSIZE)

/* optional - zedit uses it to preallocate screen marks */
void minit(struct mark *preallocated);
struct mark *bcremrk(void);
void unmark(struct mark *);

bool bisaftermrk(struct mark *);
bool bisbeforemrk(struct mark *);
bool mrkaftermrk(struct mark *, struct mark *);
bool mrkatmrk(struct mark *, struct mark *);
bool mrkbeforemrk(struct mark *, struct mark *);

void bmrktopnt(struct mark *);
void bpnttomrk(struct mark *);
void bswappnt(struct mark *);

/* reg.c - requires marks */
int compile(Byte*, Byte*, Byte*);
bool step(Byte *, struct mark *REstart);
const char *regerr(int);

#define foreach_pagemark(mark, page) \
	for ((mark) = Curbuff->marks; (mark); (mark) = (mark)->prev) \
		if ((mark)->mpage == (page))

#define foreach_globalpagemark(mark, page) \
	for ((mark) = Marklist; (mark); (mark) = (mark)->prev) \
		if ((mark)->mpage == (page))

#define foreach_buffmark(mark, buff) \
	for ((mark) = Curbuff->marks; (mark); (mark) = (mark)->prev)

#define foreach_globalbuffmark(mark, buff)				   \
	for ((mark) = Marklist; (mark); (mark) = (mark)->prev) \
		if ((mark)->mbuff == (buff))

#endif /* _mark_h */
#endif /* HAVE_MARKS */
