/* Copyright (C) 1988-2018 Sean MacLennan */

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

#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

/** @addtogroup buffer
 * @{
 */

#define LIBBUFF_MAJOR 1
#define LIBBUFF_MINOR 0

/* THE BUFFER STRUCTURES */

#if HUGE_FILES
#include <sys/stat.h>

struct huge_file {
	struct stat stat;	/**< Stat buffer for huge files. */
	void *lock;		/**< Lock for huge files. */
	int fd;			/**< File descriptor for huge files. */
	int n_huge;		/**< Number of huge pages to read. */
};
#endif

/** Main buffer structure. */
struct buff {
	struct page *firstp;	/**< List of pages. */
	struct page *lastp;	/**< Last page in buffer. */
	struct page *curpage;	/**< Point page. */
	Byte *curcptr;		/**< Point position in the page. */
	unsigned int curchar;	/**< Point index in the page. */
	int bmodf;		/**< Buffer modified? */
#ifdef BUFFER_MARKS
	struct mark *marks;	/**< Buffer dynamic marks. */
#endif
#if defined(UNDO) && UNDO
	int in_undo;		/**< Are we currently performing an undo? */
	void *undo_tail;	/**< List of undos. */
#endif
#if HUGE_FILES
	struct huge_file *huge; /**< Huge file if huge file. */
#endif
};

/**
 * The static and/or dynamic mark structure.
 */
struct mark {
	struct buff *mbuff;			/**< Buffer the mark is in. */
	struct page *mpage;			/**< Page in the buffer. */
	unsigned int moffset;			/**< Offset in the page. */
#if defined(GLOBAL_MARKS) || defined(BUFFER_MARKS)
	struct mark *prev;			/**< List of marks. */
	struct mark *next;			/**< List of marks. */
#endif
};

/** Current page length.
 * @param b Buffer to check.
 * @return Current page length.
 */
#define curplen(b) ((b)->curpage->plen)

extern void (*bsetmod)(struct buff *buff);

struct buff *bcreate(void);
void bdelbuff(struct buff *buff);
int binsert(struct buff *buff, Byte ch);
void bdelete(struct buff *buff, unsigned int n);
int bmove(struct buff *buff, int n);
int bstrline(struct buff *buff, char *str, int len);
void tobegline(struct buff *buff);
void toendline(struct buff *buff);
unsigned long blength(struct buff *buff);
unsigned long blocation(struct buff *buff);
unsigned long bline(struct buff *buff);
int bcsearch(struct buff *buff, Byte ch);
int bcrsearch(struct buff *buff, Byte ch);
void bempty(struct buff *buff);
Byte bpeek(struct buff *buff);
void boffset(struct buff *buff, unsigned long offset);
long bcopyrgn(struct mark *mark, struct buff *buff);
long bdeltomrk(struct mark *mark);

int binstr(struct buff *buff, const char *fmt, ...);
int strfmt(char *str, int len, const char *fmt, ...);
int strfmt_ap(char *str, int len, const char *fmt, va_list ap);
char *octal2str(long val, char *out);
char *int2str(long val, char *out);
char *uint2str(unsigned long val, char *out);
char *hex2str(unsigned long val, char *out);
void bmovepast(struct buff *buff, int (*pred)(int), int forward);
void bmoveto(struct buff *buff, int (*pred)(int), int forward);

/* bfile.c */
#define FILE_COMPRESSED	0x10000 /**< File compressed flag for bwritefile(). */
#define FILE_CRLF	0x20000 /**< DOS file flag for bwritefile(). */
int breadfile(struct buff *buff, const char *fname, int *compressed);
int bwritefile(struct buff *buff, char *fname, int mode);

/* bmsearch.c */
int bm_search(struct buff *buff, const char *str, int sensitive);
int bm_rsearch(struct buff *buff, const char *str, int sensitive);

/* bsocket.c */
/** Max iovs used for one writev in bwrite(). */
#define MAX_IOVS 16
int breadv(struct buff *buff, int fd);
int bwritev(struct buff *buff, int fd, unsigned int size);
int bappend(struct buff *buff, const Byte *data, int len);
int bindata(struct buff *buff, Byte *data, unsigned int len);

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

/* strlcpy.c */
#if defined(__linux__) || defined(WIN32)
size_t strlcpy(char *dst, const char *src, size_t dstsize);
size_t strlcat(char *dst, const char *src, size_t dstsize);
#endif
int strconcat(char *str, int len, ...);

/* \cond skip */
#ifndef NULL
#define NULL ((void *)0)
#endif
#ifndef EOF
#define EOF -1
#endif
#ifndef O_BINARY
#define O_BINARY 0
#endif
/* \endcond */

/** Min of a and b. */
#define MIN(a, b)	(a < b ? a : b)
/** Max of a and b. */
#define MAX(a, b)	(a > b ? a : b)

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
_Static_assert((PGSIZE & 1) == 0, "PGSIZE must be even.");

/** Describes one page in memory. */
struct page {
	Byte pdata[PGSIZE];	/**< Page data. */
#if HUGE_FILES
	unsigned int pgoffset;  /**< Huge files offset or 0 if in memory. */
#endif
	unsigned int plen;      /**< Current length of the page. */
	struct page *prevp;	/**< List of pages. */
	struct page *nextp;	/**< List of pages. */
};

struct page *newpage(struct buff *buff);
void freepage(struct buff *buff, struct page *page);
struct page *pagesplit(struct buff *buff, unsigned int dist);

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

#if HUGE_FILES
/** Make page current at offset. */
void makecur(struct buff *buff, struct page *page, int dist);
#else
/** Low level function to make page current at offset.
 * Does not validate that page is in the buffer or that dist is valid!
 * @param buff The buffer to use.
 * @param page The page to make current.
 * @param dist The offset in the page.
 */
static inline void makecur(struct buff *buff, struct page *page, int dist)
{
	buff->curpage = page;
	buff->curchar = dist;
	buff->curcptr = page->pdata + dist;
}

#define bhugecleanup(b)
#endif

/** Low level function to move current page to offset. Does not
 * validate dist.
 * @param buff Buffer to move offset in.
 * @param dist Amount to move offset.
 */
static inline void makeoffset(struct buff *buff, int dist)
{
	buff->curchar = dist;
	buff->curcptr = buff->curpage->pdata + dist;
}

/** Move point to start of buffer.
 * @param buff Buffer to move Point in.
 */
static inline void btostart(struct buff *buff)
{
	makecur(buff, buff->firstp, 0);
}

/** Move point to end of buffer.
 * @param buff Buffer to move Point in.
 */
static inline void btoend(struct buff *buff)
{
	makecur(buff, buff->lastp, buff->lastp->plen);
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

#if HUGE_FILES
extern void (*huge_file_cb)(struct buff *buff, int rc);
void default_huge_file_cb(struct buff *buff, int rc);
int breadhuge(struct buff *buff, const char *fname);
void bhugecleanup(struct buff *buff);
#endif

#ifdef __GNUC__ /* also clang */
#define HAVE_ATOMIC
#define atomic_exchange_ptr __sync_val_compare_and_swap
#define atomic_exchange __sync_val_compare_and_swap
#elif defined(WIN32)
#define HAVE_ATOMIC
#define atomic_exchange_ptr InterlockedCompareExchangePointer
#define atomic_exchange InterlockedCompareExchange
#elif defined(__x86__) || defined(__x86_64__)
#define HAVE_ATOMIC
static inline void *
atomic_exchange_ptr(void **ptr, void *old, void *new)
{
	uint64_t _ret;
	uint64_t *_ptr = (uint64_t *)ptr;
	uint64_t _old = (uint64_t)old;
	uint64_t _new = (uint64_t)new;

	asm volatile("lock cmpxchgq %2,%1"
			     : "=a" (_ret), "+m" (*_ptr)
			     : "r" (_new), "0" (_old)
			     : "memory");
	return (void *)_ret;
}

static inline uint64_t
atomic_exchange(uint64_t *ptr, uint64_t old, uint64_t new)
{
	uint64_t ret;

	asm volatile("lock cmpxchgq %2,%1"
			     : "=a" (ret), "+m" (*ptr)
			     : "r" (new), "0" (old)
			     : "memory");
	return ret;
}
#else
#warning no atomic functions
#endif

/* Mark functions */

struct mark *bcremark(struct buff *buff);
void bdelmark(struct mark *mark);

struct mark *_bcremark(struct buff *buff, struct mark **tail);
void _bdelmark(struct mark *mark, struct mark **tail);

/** Is buffer Point at mark?
 * @param buff Buffer to check.
 * @param mark Mark to check.
 * @return 1 if buffer Point is at mark.
 */
static inline int bisatmrk(struct buff *buff, struct mark *mark)
{
	return buff->curpage == mark->mpage && buff->curchar == mark->moffset;
}
/** Is buffer Point at mark? Mark can be NULL.
 * @param buff Buffer to check.
 * @param mark Mark to check.
 * @return 1 if buffer Point is at mark.
 */
static inline int bisatmrk_safe(struct buff *buff, struct mark *mark)
{
	return mark && bisatmrk(buff, mark);
}

int bisaftermrk(struct buff *buff, struct mark *mark);
int bisbeforemrk(struct buff *buff, struct mark *mark);
int bpnttomrk(struct buff *buff, struct mark *mark);
int bswappnt(struct buff *buff, struct mark *mark);
void mrkfini(void);

int mrkaftermrk(struct mark *mark1, struct mark *mark2);
int mrkbeforemrk(struct mark *mark1, struct mark *mark2);

/** Move the mark to the point.
 * @param buff The buffer Point to move the mark to.
 * @param mark The mark to set.
 */
static inline void bmrktopnt(struct buff *buff, struct mark *mark)
{
	mark->mbuff   = buff;
	mark->mpage   = buff->curpage;
	mark->moffset = buff->curchar;
}

/** Move mark 1 to mark 2
 * @param m1 Destination mark.
 * @param m2 Source mark.
 */
static inline void mrktomrk(struct mark *m1, struct mark *m2)
{
	m1->mbuff = m2->mbuff;
	m1->mpage = m2->mpage;
	m1->moffset = m2->moffset;
}

/** Is mark1 at mark2?
 * @param m1 First mark.
 * @param m2 Second mark.
 * @return mark1 == mark2.
 */
static inline int mrkatmrk(struct mark *m1, struct mark *m2)
{
	return  m1->moffset == m2->moffset &&
		m1->mpage == m2->mpage &&
		m1->mbuff == m2->mbuff;
}

#ifdef GLOBAL_MARKS
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

#ifdef BUFFER_MARKS
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

/** The libbuff version as a string. */
extern const char *libbuff_version;
extern const char *libbuff_marker;

/* @} buffer */

/** @addtogroup misc
 * @{
 */
const char *Dbgfname(const char *fname);
void Dbg(const char *fmt, ...);
/* @} misc */

#endif

/* Ironically, I need LONG_LINE_COMMENT for the local variable! */
/*
 * Local Variables:
 * my-checkpatch-ignores: "SPDX_LICENSE_TAG,COMPLEX_MACRO,MULTISTATEMENT_MACRO_USE_DO_WHILE,LONG_LINE_COMMENT"
 * End:
 */
