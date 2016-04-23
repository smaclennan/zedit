/* buff.h - low level buffer defines
 * Copyright (C) 1988-2016 Sean MacLennan
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

#ifndef _buff_h
#define _buff_h

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdbool.h>
#include <stdint.h>

#ifdef UNSIGNED_BYTES
#define Byte unsigned char
#else
#define Byte char
#endif

#ifdef WIN32
#define inline __inline
#define snprintf _snprintf
#endif

#include "mark.h"

/* THE BUFFER STRUCTURES */

/** Main buffer structure. */
struct buff {
	struct page *firstp;		/**< List of pages. */
	struct page *curpage;		/**< Point page. */
	unsigned curchar;			/**< Point index in the page. */
	Byte *curcptr;				/**< Point position in the page. */
	bool bmodf;					/**< Buffer modified? */
#if defined(UNDO) && UNDO
	bool in_undo;				/**< Are we currently performing an undo? */
	void *undo_tail;			/**< list of undos */
#endif
#ifdef HAVE_BUFFER_MARKS
	struct mark *marks;			/**< Buffer dynamic marks. */
#endif
};

/* This is used a lot */
#define curplen(b) ((b)->curpage->plen)

extern void (*bsetmod)(struct buff *buff);

struct buff *bcreate(void);
void bdelbuff(struct buff *);
bool binsert(struct buff *, Byte);
void bdelete(struct buff *, unsigned);
bool bmove(struct buff *, int);
void btostart(struct buff *);
void btoend(struct buff *);
void tobegline(struct buff *);
void toendline(struct buff *);
unsigned long blength(struct buff *);
unsigned long blocation(struct buff *);
bool bcsearch(struct buff *, Byte);
bool bcrsearch(struct buff *, Byte);
void bempty(struct buff *buff);
Byte bpeek(struct buff *buff);
void boffset(struct buff *buff, unsigned long offset);

bool binstr(struct buff *buff, const char *str, ...);
void bmovepast(struct buff *buff, int (*pred)(int), bool forward);
void bmoveto(struct buff *buff, int (*pred)(int), bool forward);

/* bfile.c */
#define FILE_COMPRESSED		0x10000
#define FILE_CRLF			0x20000
int breadfile(struct buff *buff, const char *, int *compressed);
bool bwritefile(struct buff *buff, char *, int mode);

/* bmsearch.c */
bool bm_search(struct buff *buff, const char *str, bool sensitive);
bool bm_rsearch(struct buff *buff, const char *str, bool sensitive);

/* bsocket.c */
/** Max iovs used for one writev in bwrite(). */
#define MAX_IOVS 16
int bread(struct buff *buff, int fd);
int bwrite(struct buff *buff, int fd, int size);
int bappend(struct buff *buff, const Byte *, int);
int bindata(struct buff *buff, Byte *, int);

/* undo.c */
#if defined(UNDO) && UNDO
extern unsigned long undo_total; /* Not thread safe */

void undo_add(struct buff *buff, int size, bool clumped);
void undo_del(struct buff *buff, int size);
void undo_clear(struct buff *buff);
int do_undo(struct buff *buff);

/* These must be implemented by the app. It is safe to return 0. */
int undo_add_clumped(struct buff *buff, int size);
int undo_del_clumped(struct buff *buff, int size);
#else
#define undo_add(b, s, c)
#define undo_del(b, s)
#define undo_clear(b)
#define do_undo(b) 0
#endif

/* dbg.c */
const char *Dbgfname(const char *fname);
void Dbg(const char *fmt, ...);

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MIN(a, b)	(a < b ? a : b)
#define MAX(a, b)	(a > b ? a : b)

/* These are for stats only and are not thread safe */
extern int NumBuffs, NumPages;

/* Page struct and functions. */

/** Page size. Generally, the bigger the page size the faster the
 * editor however the more wasted memory. A page size of 1k seems to
 * be a very good trade off.
 */
#define PSIZE		1024
/** Half a page for pagesplit(). */
#define HALFP		(PSIZE / 2)

/** Describes one page in memory. */
struct page {
	Byte pdata[PSIZE];		/**< Page data. */
	unsigned plen;			/**< Current length of the page. */
	struct page *prevp;		/**< List of pages. */
	struct page *nextp;		/**< List of pages. */
};

struct page *newpage(struct page *curpage);
void freepage(struct buff *buff, struct page *page);
struct page *pagesplit(struct buff *buff, unsigned dist);

/** Is the point at the start of the buffer? */
static inline bool bisstart(struct buff *buff)
{
	return buff->curpage == buff->firstp && buff->curchar == 0;
}

/** Is the point at the end of the buffer? */
static inline bool bisend(struct buff *buff)
{
	return buff->curpage->nextp == NULL && buff->curchar >= buff->curpage->plen;
}

/** Make page current at offset. */
static inline void makecur(struct buff *buff, struct page *page, int dist)
{
	buff->curpage = page;
	buff->curchar = dist;
	buff->curcptr = page->pdata + dist;
}

/** Move current page to offset. */
static inline void makeoffset(struct buff *buff, int dist)
{
	buff->curchar = dist;
	buff->curcptr = buff->curpage->pdata + dist;
}

/** Is this the last page in the buffer? */
static inline bool lastp(struct page *page)
{
	return page->nextp == NULL;
}

/** Moves the point forward one. This function is highly optimized. */
static inline void bmove1(struct buff *buff)
{
	if (++buff->curchar < curplen(buff))
		/* within current page */
		++buff->curcptr;
	else if (buff->curpage->nextp)
		/* goto start of next page */
		makecur(buff, buff->curpage->nextp, 0);
	else
		/* Already at EOB */
		makeoffset(buff, curplen(buff));
}

#endif
