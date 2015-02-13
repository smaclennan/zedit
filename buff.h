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

struct buff {
	bool bmodf;			/* buffer modified? */
	struct page *firstp;	/* the pages */
	struct page *curpage;	/* the position of the point */
	unsigned curchar;
	Byte *curcptr;
	unsigned bmode;			/* buffer mode */
	char *bname;			/* buffer name */
	char *fname;			/* file name associated with buffer */
	time_t mtime;			/* file time at read */
	void *app;				/* app specific data */
	struct buff *prev, *next;	/* list of buffers */
};

/* If set, this function will be called on bdelbuff */
extern void (*app_cleanup)(struct buff *buff);

extern struct buff *Curbuff;
extern bool Curmodf;

#define Curpage (Curbuff->curpage)
#define Curchar (Curbuff->curchar)
#define Curcptr (Curbuff->curcptr)

extern int NumBuffs, NumPages;

#define Buff()		(*Curcptr)
#define bisstart()	((Curpage == Curbuff->firstp) && (Curchar == 0))

struct buff *_bcreate(void);
struct buff *bcreate(void);
bool bdelbuff(struct buff *);

#ifdef THREAD_SAFE
#define binsert _binsert
#define bdelete _bdelete
#define bmove _bmove
#else
#define binsert(c) _binsert(Curbuff, (c))
#define bdelete(n) _bdelete(Curbuff, (n))
#define bmove(n) _bmove(Curbuff, (n))
#endif
bool _binsert(struct buff *, Byte);
void _bdelete(struct buff *, int);
bool _bmove(struct buff *, int);

bool bcsearch(Byte);
bool bcrsearch(Byte);
void bempty(void);
void bgoto_char(long offset);
bool bappend(Byte *, int);
unsigned long blength(struct buff *);
unsigned long blocation(void);
int breadfile(const char *);
void bswitchto(struct buff *);
void btoend(void);
void btostart(void);
bool bwritefile(char *);

void tobegline(void);
void toendline(void);

bool bisend(void);
Byte bpeek(void);
int batoi(void);

void bmove1(void);
void boffset(unsigned long off);

/* These should be called from buffer/mark code only */
void makecur(struct buff *buff, struct page *, int);
#define makeoffset(buff, dist) makecur((buff), (buff)->curpage, (dist))

/* bmsearch.c */
bool bm_search(const char *str, bool sensitive);
bool bm_rsearch(const char *str, bool sensitive);

#ifndef O_BINARY
#define O_BINARY 0
#endif
#ifndef NULL
#define NULL ((void *)0)
#endif

#define MIN(a, b)	(a < b ? a : b)
#define MAX(a, b)	(a > b ? a : b)

#endif
