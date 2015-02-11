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

#ifndef DOS
#include <stdbool.h>
#endif

#if defined(WIN32) || defined(DOS)
#include "zwin32.h"
#else
#include <unistd.h>
#endif

#include <time.h>

#define Byte unsigned char

#define READ_MODE	(O_RDONLY | O_BINARY)
#define WRITE_MODE	(O_WRONLY | O_CREAT | O_TRUNC | O_BINARY)

/* THE BUFFER STRUCTURES */

struct page;

struct mark {
	struct buff *mbuff;			/* buffer the mark is in */
	struct page *mpage;			/* page in the buffer */
	int moffset;				/* offset in the page */
	struct mark *prev, *next;	/* list of marks */
};

struct buff {
	bool bmodf;			/* buffer modified? */
	struct page *firstp, *lastp;	/* describe the pages */
	struct page *pnt_page;		/* the position of the point */
	unsigned pnt_offset;
	unsigned bmode;			/* buffer mode */
	char *bname;			/* buffer name */
	char *fname;			/* file name associated with buffer */
	time_t mtime;			/* file time at read */
	void *app;				/* app specific data */
	struct buff *prev, *next;	/* list of buffers */
};

extern Byte *Curcptr;
extern int Curchar;
extern struct buff *Curbuff;
extern struct page *Curpage;

#define MRKSIZE		(sizeof(struct mark) - (sizeof(struct mark *) << 1))

#define Buff()		(*Curcptr)
#define bisstart()	((Curpage == Curbuff->firstp) && (Curchar == 0))
#define bisatmrk(m)	((Curpage == (m)->mpage) && (Curchar == (m)->moffset))
#define mrktomrk(m1, m2) memcpy(m1, m2, MRKSIZE)

bool bisend(void);
Byte bpeek(void);
int batoi(void);

bool bmove(int);
void bmove1(void);
void bshove(void);
void boffset(unsigned long off);

/* optional - zedit uses it to preallocate screen marks */
void minit(struct mark *preallocated);
int bcopyrgn(struct mark *, struct buff*);
struct buff *_bcreate(void);
struct buff *bcreate(void);
struct mark *bcremrk(void);
bool bcrsearch(Byte);
bool bcsearch(Byte);
bool bdelbuff(struct buff *);
void bdelete(int);
void bdeltomrk(struct mark *);
void bempty(void);
void bgoto_char(long offset);
bool binsert(Byte);
bool bappend(Byte *, int);
void bconvert(int (*to)(int));
void binstr(const char *);
bool bisaftermrk(struct mark *);
bool bisbeforemrk(struct mark *);
unsigned long blength(struct buff *);
unsigned long blocation(void);
unsigned long bline(void);
void bmrktopnt(struct mark *);
void bpnttomrk(struct mark *);
int breadfile(const char *);
void bswappnt(struct mark *);
void bswitchto(struct buff *);
void btoend(void);
void btostart(void);
bool bwritefile(char *);
void unmark(struct mark *);
void makecur(struct page *);
void makeoffset(int);

int bgetstats(char *str, int len);
void tobegline(void);
void toendline(void);

/* bmsearch.c */
bool bm_search(const char *str, bool sensitive);
bool bm_rsearch(const char *str, bool sensitive);

/* reg.c */
extern struct mark *REstart;
int compile(Byte*, Byte*, Byte*);
bool step(Byte *);
const char *regerr(int);

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef NULL
#define NULL ((void *)0)
#endif

#define MIN(a, b)	(a < b ? a : b)
#define MAX(a, b)	(a > b ? a : b)

#endif
