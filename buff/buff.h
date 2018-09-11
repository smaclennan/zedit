/* buff.h - low level buffer defines
 * Copyright (C) 1988-2018 Sean MacLennan
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

#ifdef __STRICT_ANSI__
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef UNSIGNED_BYTES
#define Byte unsigned char
#else
#define Byte char
#endif

#ifdef WIN32
#include "bwin32.h"
#else
#include <unistd.h>
#endif

/** @defgroup buffer Core buffer routines
 * The core buffer routines including the mark functions.
 */

/** @defgroup misc Miscellaneous routines */

/* THE BUFFER STRUCTURES */

/** @addtogroup buffer
 * @{
*/

/** Main buffer structure. */
struct buff {
#ifdef HAVE_BUFFER_MARKS
	struct mark *marks;			/**< Buffer dynamic marks. */
#endif
	struct page *firstp;		/**< List of pages. */
	struct page *curpage;		/**< Point page. */
	Byte *curcptr;				/**< Point position in the page. */
	unsigned curchar;			/**< Point index in the page. */
	int bmodf;					/**< Buffer modified? */
#if defined(UNDO) && UNDO
	int in_undo;		/**< Are we currently performing an undo? */
	void *undo_tail;	/**< List of undos. */
#endif
#if HUGE_FILES
	struct stat *stat;	/**< Stat buffer for huge files. */
	void *lock;		/**< Lock for huge files. */
	int fd;			/**< File descriptor for huge files. */
	int n_huge;		/**< Number of huge pages to read. */
#endif
};

#define BUFF(b) (*(b)->curcptr)

/* mark.h needs the buffer structure */
#include "mark.h"

/** Current page length.
 * @param b Buffer to check.
 * @return Current page length.
 */
#define curplen(b) ((b)->curpage->plen)

extern void (*bsetmod)(struct buff *buff);

struct buff *bcreate(void);
void bdelbuff(struct buff *);
int binsert(struct buff *, Byte);
void bdelete(struct buff *, unsigned);
int bmove(struct buff *, int);
int bstrline(struct buff *buff, char *str, int len);
void btoend(struct buff *);
void tobegline(struct buff *);
void toendline(struct buff *);
unsigned long blength(struct buff *);
unsigned long blocation(struct buff *);
unsigned long bline(struct buff *buff);
int bcsearch(struct buff *, Byte);
int bcrsearch(struct buff *, Byte);
void bempty(struct buff *buff);
Byte bpeek(struct buff *buff);
void boffset(struct buff *buff, unsigned long offset);
long bcopyrgn(struct mark *, struct buff*);
long bdeltomrk(struct mark *);

int binstr(struct buff *buff, const char *fmt, ...);
int strfmt(char *str, int len, const char *fmt, ...);
int strfmt_ap(char *str, int len, const char *fmt, va_list ap);
void bmovepast(struct buff *buff, int (*pred)(int), int forward);
void bmoveto(struct buff *buff, int (*pred)(int), int forward);

/* bfile.c */
#define FILE_COMPRESSED		0x10000 /**< File compressed flag for bwritefile(). */
#define FILE_CRLF			0x20000 /**< DOS file flag for bwritefile(). */
int breadfile(struct buff *buff, const char *, int *compressed);
int bwritefile(struct buff *buff, char *, int mode);

/* bmsearch.c */
int bm_search(struct buff *buff, const char *str, int sensitive);
int bm_rsearch(struct buff *buff, const char *str, int sensitive);

/* bsocket.c */
/** Max iovs used for one writev in bwrite(). */
#define MAX_IOVS 16
int bread(struct buff *buff, int fd);
int bwrite(struct buff *buff, int fd, unsigned size);
int bappend(struct buff *buff, const Byte *, int);
int bindata(struct buff *buff, Byte *, unsigned);

/* undo.c */
#if defined(UNDO) && UNDO
void undo_add(struct buff *buff, int size);
void undo_del(struct buff *buff, int size);
void undo_clear(struct buff *buff);
int do_undo(struct buff *buff);
#else
/* \cond skip */
#define undo_add(b, s)
#define undo_del(b, s)
#define undo_clear(b)
#define do_undo(b) 0
/* \endcond */
#endif

/* util.c */
#if defined(__linux__) || defined(WIN32)
size_t strlcpy(char *dst, const char *src, size_t dstsize);
size_t strlcat(char *dst, const char *src, size_t dstsize);
#endif
int strconcat(char *str, int len, ...);

#ifndef NULL
#define NULL ((void *)0)
#endif
#ifndef EOF
#define EOF -1
#endif

/** Min of a and b. */
#define MIN(a, b)	(a < b ? a : b)
/** Max of a and b. */
#define MAX(a, b)	(a > b ? a : b)

#ifndef O_BINARY
#define O_BINARY 0
#endif

/* Page struct and functions. */

/** Page size. Generally, the bigger the page size the faster the
 * editor however the more wasted memory. A page size of 1k seems to
 * be a very good trade off. 4k probably better on modern systems.
 */
#ifndef PGSIZE
#define PGSIZE		4096
#endif
/** Half a page for pagesplit(). */
#define HALFP		(PGSIZE / 2)

/** Describes one page in memory. */
struct page {
	Byte pdata[PGSIZE];	/**< Page data. */
#if HUGE_FILES
	unsigned pgoffset;      /**< Huge files offset or 0 if in memory. */
#endif
	unsigned plen;          /**< Current length of the page. */
	struct page *prevp;	/**< List of pages. */
	struct page *nextp;	/**< List of pages. */
};

struct page *newpage(struct page *curpage);
void freepage(struct buff *buff, struct page *page);
struct page *pagesplit(struct buff *buff, unsigned dist);

/** Is the point at the start of the buffer?
 * @param bbuff The buffer to check.
 * @return 1 if at start.
 */
static inline int bisstart(struct buff *buff)
{
	return buff->curpage == buff->firstp && buff->curchar == 0;
}

/** Is the point at the end of the buffer?
 * @param bbuff The buffer to check.
 * @return 1 if at end.
 */
static inline int bisend(struct buff *buff)
{
	return buff->curpage->nextp == NULL &&
		buff->curchar >= buff->curpage->plen;
}

/* Helper function - always use makecur */
static inline void __makecur(struct buff *buff, struct page *page, int dist)
{
	buff->curpage = page;
	buff->curchar = dist;
	buff->curcptr = page->pdata + dist;
}

/** Make page current at offset. */
#if HUGE_FILES
void makecur(struct buff *buff, struct page *page, int dist);
#else
#define makecur __makecur
#endif

/** Move point to start of buffer.
 * @param buff Buffer to move Point in.
 */
static inline void btostart(struct buff *buff)
{
	makecur(buff, buff->firstp, 0);
}

/** Move current page to offset.
 * @param buff Buffer to move offset in.
 * @param dist Amount to move offset (can be negative).
 */
static inline void makeoffset(struct buff *buff, int dist)
{
	buff->curchar = dist;
	buff->curcptr = buff->curpage->pdata + dist;
}

/** Is this the last page in the buffer?
 * @param page Page to check.
 */
static inline int lastp(struct page *page)
{
	return page->nextp == NULL;
}

/** Moves the point forward one. This function is highly optimized.
 * @param buff Buffer to move Point in.
 */
static inline void bmove1(struct buff *buff)
{
	if (++buff->curchar < curplen(buff))
		/* within current page */
		++buff->curcptr;
	else if (buff->curpage->nextp)
		/* goto start of next page */
		makecur(buff, buff->curpage->nextp, 0);
	else
		/* already at eob - paw needs this */
		makeoffset(buff, curplen(buff));
}

/* returns the start of the string */
char *bitoa(int val, char *out);
char *butoa(unsigned val, char *out);
/* returns the end of the string */
char *_bitoa(int val, char *out);
char *_butoa(unsigned val, char *out);

#if HUGE_FILES
extern void (*huge_file_cb)(struct buff *buff, int rc);
void default_huge_file_cb(struct buff *buff, int rc);
int breadhuge(struct buff *buff, const char *fname);
void bhugecleanup(struct buff *buff);
#endif

#ifdef __GNUC__ /* also clang */
#define HAVE_ATOMIC
#define atomic_exchange  __sync_val_compare_and_swap
#define atomic_add(a, n) __sync_fetch_and_add(&a, n)
#define atomic_sub(a, n) __sync_fetch_and_sub(&a, n)
#define atomic_inc(a) __sync_fetch_and_add(&a, 1)
#define atomic_dec(a) __sync_fetch_and_sub(&a, 1)
#elif defined(WIN32)
#define HAVE_ATOMIC
#define atomic_exchange InterlockedCompareExchangePointer
#define atomic_add(a, n) InterlockedAdd(&a, n)
#define atomic_sub(a, n) InterlockedAdd(&a, -n)
#define atomic_inc(a) InterlockedIncrement(&a)
#define atomic_dec(a) InterlockedDecrement(&a)
#else
#warning no atomic functions
#define atomic_add(a, n) ((a) += n)
#define atomic_sub(a, n) ((a) -= n)
#define atomic_inc(a) (++(a))
#define atomic_dec(a) (--(a))
#endif
/* @} buffer */

/** @addtogroup misc
 * @{
*/
const char *Dbgfname(const char *fname);
void Dbg(const char *fmt, ...);
/* @} misc */

#endif
