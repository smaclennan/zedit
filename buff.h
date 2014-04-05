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

#include <setjmp.h>
extern jmp_buf zenv;

#define Byte unsigned char


/* THE BUFFER STRUCTURES */

/* Generally, the bigger the page size the faster the editor however
 * the more wasted memory. A page size of 1k seems to be a very good trade off.
 * NOTE: DOS *requires* 1k pages for DOS_EMS.
 */
#define PSIZE		1024		/* size of page */
#define HALFP		(PSIZE / 2)	/* half the page size */

struct page {
#ifdef DOS_EMS
	Byte *pdata;			/* the page data */
	Byte emmpage;			/* 16k page */
	Byte emmoff;			/* offset in page */
#else
	Byte pdata[PSIZE];		/* the page data */
#endif
	int plen;			/* current length of the page */
	struct page *nextp, *prevp;	/* list of pages in buffer */
};

struct mark {
	struct page *mpage;		/* page in the buffer */
	struct buff *mbuff;		/* buffer the mark is in */
	int moffset;			/* offset in the page */
	bool modf;			/* screen mark modified flags */
	struct mark *prev, *next;	/* list of marks */
};

struct comment {
	struct mark *start, *end;
	struct comment *next;
};

struct buff {
	bool bmodf;			/* buffer modified? */
	struct page *firstp, *lastp;	/* describe the pages */
	struct page *pnt_page;		/* the position of the point */
	unsigned pnt_offset;
	struct mark *umark;		/* position of mark in this buffer */
	unsigned bmode;			/* buffer mode */
	char *bname;			/* buffer name */
	char *fname;			/* file name associated with buffer */
	time_t mtime;			/* file time at read */
	pid_t child;			/* PID of shell or EOF */
	int in_pipe;			/* the pipe */
	struct comment *chead, *ctail;	/* list of comments in file */
	Byte comchar;			/* single char comment character */
	void *undo_tail;
	struct buff *prev, *next;	/* list of buffers */
};

struct wdo {
	struct buff *wbuff;		/* buffer window looks on */
	struct mark *wpnt;		/* saved Point */
	struct mark *wmrk;		/* saved Mark */
	struct mark *wstart;		/* screen start */
	int umark_set;
	int first, last;		/* screen line boundries */
	int modecol;			/* column for modeflags */
	int modeflags;			/* flags for modeflags */
	struct wdo *next;
};

#define foreachwdo(wdo) for (wdo = Whead; wdo; wdo = wdo->next)
#define wheight() (Curwdo->last - Curwdo->first)

extern Byte *Curcptr, *Cpstart;
extern int Curchar, Curplen;
extern struct buff *Curbuff;
extern struct page *Curpage;
extern struct mark *Mrklist;
extern struct wdo *Curwdo, *Whead;
extern bool Curmodf;

#define MRKSIZE		(sizeof(struct mark) - (sizeof(struct mark *) << 1))

#define Buff()		(*Curcptr)
#define bisstart()	((Curpage == Curbuff->firstp) && (Curchar == 0))
#define bisend()	((Curpage == Curbuff->lastp) && (Curchar >= Curplen))
#define bisatmrk(m)	((Curpage == (m)->mpage) && (Curchar == (m)->moffset))
#define mrktomrk(m1, m2) memcpy(m1, m2, MRKSIZE)
#define bfname()	(Curbuff->fname)

Byte bpeek(void);
int batoi(void);

bool bmove(int);
void bmove1(void);
void boffset(unsigned long off);

/* Return the character a mark points to. */
#define markch(mrk)	((mrk)->mpage->pdata[(mrk)->moffset])

#define NEED_UMARK do if (Curbuff->umark == NULL) { tbell(); return; } while (0)
/* This does not need to be a macro... just makes it easier to see */
#define CLEAR_UMARK clear_umark()

#endif
