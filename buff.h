/* buff.h - low level buffer defines
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

#ifndef _buff_h
#define _buff_h

#include <stdbool.h>

#if defined(WIN32)
#include "zwin32.h"
#else
#include <unistd.h>
#endif

#include <time.h>

#ifdef ZEDIT
#define Byte unsigned char
#else
#define Byte char
#endif

#define MAX_IOVS 16

/* THE BUFFER STRUCTURES */

struct page;

struct buff {
	char *bname;				/* buffer name */
	bool bmodf;					/* buffer modified? */
	struct page *firstp;		/* the pages */
	struct page *curpage;		/* the position of the point */
	unsigned curchar;
	Byte *curcptr;
#if defined(HAVE_MARKS) && !defined(HAVE_GLOBAL_MARKS)
	struct mark *marks;			/* buffer marks */
#endif
#ifdef HAVE_FILES
	char *fname;				/* file name associated with buffer */
	unsigned bmode;				/* buffer mode */
	time_t mtime;				/* file time at read */
#endif
	void *app;					/* app specific data */
	struct buff *prev, *next;	/* list of buffers */
};

/* This is used a lot */
#define curplen(b) ((b)->curpage->plen)

/* If set, this function will be called on _bdelbuff */
extern void (*app_cleanup)(struct buff *buff);

extern void (*bsetmod)(struct buff *buff);

struct buff *_bcreate(void);
void _bdelbuff(struct buff *);
bool _binsert(struct buff *, Byte);
void _bdelete(struct buff *, int);
bool _bmove(struct buff *, int);
void _bmove1(struct buff *);
bool _bisstart(struct buff *);
bool _bisend(struct buff *);
void _btostart(struct buff *);
void _btoend(struct buff *);
void _tobegline(struct buff *);
void _toendline(struct buff *);
unsigned long _blength(struct buff *);
unsigned long _blocation(struct buff *);
bool _bcsearch(struct buff *, Byte);
bool _bcrsearch(struct buff *, Byte);
void _bempty(struct buff *buff);
Byte _bpeek(struct buff *buff);
void _boffset(struct buff *buff, unsigned long offset);
int _bappend(struct buff *buff, Byte *, int);
int _bindata(struct buff *buff, Byte *, int);
int _bread(struct buff *buff, int fd, int size);
int _bwrite(struct buff *buff, int fd, int size);

/* bmsearch.c */
bool _bm_search(struct buff *buff, const char *str, bool sensitive);
bool _bm_rsearch(struct buff *buff, const char *str, bool sensitive);

#ifndef HAVE_THREADS
extern struct buff *Curbuff;
#define Curpage (Curbuff->curpage)
#define Curchar (Curbuff->curchar)
#define Curcptr (Curbuff->curcptr)
#define Cpstart (Curbuff->curpage->pdata)

#define Buff()		(*Curcptr)

#define binsert(c) _binsert(Curbuff, (c))
#define bdelete(n) _bdelete(Curbuff, (n))
#define bmove(n) _bmove(Curbuff, (n))
#define bmove1() _bmove1(Curbuff)
#define bisstart() _bisstart(Curbuff)
#define bisend() _bisend(Curbuff)
#define btostart() _btostart(Curbuff)
#define btoend() _btoend(Curbuff)
#define tobegline() _tobegline(Curbuff)
#define toendline() _toendline(Curbuff)
#define blength _blength
#define blocation _blocation
#define bcsearch(c) _bcsearch(Curbuff, (c))
#define bcrsearch(c) _bcrsearch(Curbuff, (c))
#define bempty() _bempty(Curbuff)
#define boffset(n) _boffset(Curbuff, n)
#define bappend(d, n) _bappend(Curbuff, (d), (n))
#define bindata(d, n) _bindata(Curbuff, (d), (n))
#define bread(fd, n) _bread(Curbuff, (fd), (n))
#define bwrite(fd, n) _bwrite(Curbuff, (fd), (n))

#define binstr(s) _bindata(Curbuff, (Byte *)(s), strlen(s));

struct buff *bcreate(void);
bool bdelbuff(struct buff *);
void bswitchto(struct buff *);

/* bfile.c */
int breadfile(const char *);
bool bwritefile(char *);
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MIN(a, b)	(a < b ? a : b)
#define MAX(a, b)	(a > b ? a : b)

extern int NumBuffs, NumPages;

/* These should be called from buffer/mark code only */
void makecur(struct buff *buff, struct page *, int);
#define makeoffset(buff, dist) makecur((buff), (buff)->curpage, (dist))

/* Page struct and functions. */

/* Generally, the bigger the page size the faster the editor however
 * the more wasted memory. A page size of 1k seems to be a very good trade off.
 */
#define PSIZE		1024		/* size of page */
#define HALFP		(PSIZE / 2)	/* half the page size */

struct page {
	Byte pdata[PSIZE];		/* the page data */
	int plen;			/* current length of the page */
	struct page *nextp, *prevp;	/* list of pages */
};

#define lastp(pg) ((pg)->nextp == NULL)

struct page *newpage(struct page *curpage);
void freepage(struct buff *buff, struct page *page);
struct page *pagesplit(struct buff *buff, int dist);

#endif
