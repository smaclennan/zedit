/* buff.h - low level buffer defines
 * Copyright (C) 1988-2010 Sean MacLennan
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


/* THE BUFFER STRUCTURES */

/* The bigger the page size the faster the editor.
 * However, every buffer has at least one page.
 */
#define PSIZE		4096		/* size of page */
#define HALFP		(PSIZE / 2)	/* half the page size */

#define BMODF		1		/* Buffer modified */
#define BMODF_HINT	2		/* Modified since last display */
#define MODIFIED	(BMODF | BMODF_HINT)

typedef struct _page {
	Byte pdata[PSIZE];		/* the page data */
	int plen;			/* current length of the page */
	int lines;			/* number of lines in page */
	struct _page *nextp, *prevp;	/* list of pages in buffer */
} Page;


typedef struct _mrk {
	Page *mpage;			/* page in the buffer */
	struct _buff *mbuff;		/* buffer the mark is in */
	int moffset;			/* offset in the page */
	Boolean modf;			/* screen mark modified flags */
	struct _mrk *prev, *next;	/* list of marks */
} Mark;

#if COMMENTBOLD
typedef struct _comment {
	Mark *start;
	Mark *end;
	int  type;
	struct _comment *next;
} Comment;
#endif

typedef struct _buff {
	Boolean bmodf;			/* buffer modified? */
	Page *firstp, *lastp;		/* describe the pages */
	Page *pnt_page;			/* the position of the point */
	unsigned pnt_offset;
	Mark *mark;			/* position of mark in this buffer */
	unsigned bmode;			/* buffer mode */
	char *bname;			/* buffer name */
	char *fname;			/* file name associated with buffer */
	long mtime;			/* file time at read */
	struct _buff *prev, *next;	/* list of buffers */
#if PIPESH
	int child;			/* PID of shell or EOF */
	int in_pipe;			/* the pipes */
	FILE *out_pipe;
#endif
#if COMMENTBOLD
	Comment *comments;		/* list of comments in file */
	Comment *ctail;
	int comstate;			/* comment state */
	int comchar;			/* single char comment character */
#endif
#if UNDO
	void *undo_tail;
#endif
} Buffer;

typedef struct _wdo {
	Buffer	*wbuff;			/* buffer window looks on */
	Mark	*wpnt;			/* saved Point */
	Mark	*wmrk;			/* saved Mark */
	Mark	*wstart;		/* screen start */
	int	first, last;		/* screen line boundries */
	int	modecol;		/* column for Modeflags */
	int	modeflags;		/* flags for Modeflags */
#if XWINDOWS && !defined(BORDER3D)
	ulong	vscroll, vthumb;	/* windows for vert scrollbar */
	int		vheight;	/* height of vert scrollbar */
	ulong	hscroll, hthumb;	/* windows for horiz scrollbar */
#endif
	struct _wdo	*prev, *next;
} WDO;

extern Byte *Curcptr, *Cpstart;
extern int Curchar, Curplen;
extern Buffer *Curbuff;
extern Page *Curpage;
extern Mark *Mrklist;
extern WDO *Curwdo, *Whead;
extern int Curmodf;

#define MRKSIZE		(sizeof(Mark) - (sizeof(Mark *) << 1))

#define Buff()		(*Curcptr)
#define Bisstart()	((Curpage == Curbuff->firstp) && (Curchar == 0))
#define Bisend()	((Curpage == Curbuff->lastp) && (Curchar >= Curplen))
#define Bisatmrk(m)	((Curpage == (m)->mpage) && (Curchar == (m)->moffset))
#define Mrktomrk(m1, m2) memcpy(m1, m2, MRKSIZE)
#define Bfname()	(Curbuff->fname)

/* Return the character a mark points to. */
#define Markch(mrk)	((mrk)->mpage->pdata[(mrk)->moffset])

#endif
