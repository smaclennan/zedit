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
#define snprintf _snprintf
#endif

#include "mark.h"

#define MAX_IOVS 16

/* THE BUFFER STRUCTURES */

struct buff {
	struct page *firstp;		/* the pages */
	struct page *curpage;		/* the position of the point */
	unsigned curchar;
	Byte *curcptr;
	bool bmodf;					/* buffer modified? */
#ifdef HAVE_BUFFER_MARKS
	struct mark *marks;			/* buffer marks */
#endif
};

/* This is used a lot */
#define curplen(b) ((b)->curpage->plen)

extern void (*bsetmod)(struct buff *buff);

struct buff *bcreate(void);
void bdelbuff(struct buff *);
bool binsert(struct buff *, Byte);
void bdelete(struct buff *, int);
bool bmove(struct buff *, int);
void bmove1(struct buff *);
bool bisstart(struct buff *);
bool bisend(struct buff *);
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
int bappend(struct buff *buff, Byte *, int);
int bindata(struct buff *buff, Byte *, int);

void binstr(struct buff *buff, const char *str, ...);

/* reg.c */
#ifdef BUILTIN_REG
#define ESIZE		256			/* reg exp buffer size */

typedef struct regex {
	uint8_t ep[ESIZE];
} regex_t;

#define REG_EXTENDED	0
#define REG_ICASE		0
#define REG_NOSUB		0
#define REG_NEWLINE		0

int regerror(int errcode, const regex_t *preg, char *errbuf,
				int errbuf_size);
static inline void regfree(regex_t *re) {}
#endif

int compile(struct buff *buff, regex_t *re, const char *regex, int cflags);
bool step(struct buff *buff, regex_t *re, struct mark *REstart);
/* regerror */
/* regfree */
bool lookingat(struct buff *buff, Byte *str);

/* bmsearch.c */
bool bm_search(struct buff *buff, const char *str, bool sensitive);
bool bm_rsearch(struct buff *buff, const char *str, bool sensitive);

/* bsocket.c */
int bread(struct buff *buff, int fd);
int bwrite(struct buff *buff, int fd, int size);

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MIN(a, b)	(a < b ? a : b)
#define MAX(a, b)	(a > b ? a : b)

/* These are for stats only and are not thread safe */
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
	unsigned plen;			/* current length of the page */
	struct page *nextp, *prevp;	/* list of pages */
};

#define lastp(pg) ((pg)->nextp == NULL)

struct page *newpage(struct page *curpage);
void freepage(struct buff *buff, struct page *page);
struct page *pagesplit(struct buff *buff, unsigned dist);

#endif
