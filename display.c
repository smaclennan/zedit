/* display.c - Zedit main display update
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

#include "z.h"
#include <setjmp.h>

/** @addtogroup zedit
 * @{
*/

static int innerdsp(int from, int to, struct mark *pmark);
static void modeflags(struct wdo *wdo);
static char *setmodes(zbuff_t *);
static void pawdisplay(struct mark *, struct mark *);

struct mark *Sstart;			/* Screen start */
static struct mark *Psstart;	/* Screen 'prestart' */
static struct mark *Send;		/* Screen end */
static int Tlrow;				/* Last row displayed */

static int NESTED;		/* zrefresh can go recursive... */
static Byte tline[COLMAX + 1];

#define EXTRA_MARKS 4
static struct mark Scrnmarks[ROWMAX + EXTRA_MARKS];	/* Screen marks - one per line */
static bool Scrnmodf[ROWMAX];
static struct mark *was;	/* last location of user mark for zrefresh() */

static void (*printchar)(Byte ichar) = tprntchar;

#if HUGE_FILES

#if HUGE_THREADED
#include <samthread.h>

static struct huge_event {
	struct buff *buff;
	int rc;
	struct huge_event *next;
} *events, *events_tail;

static struct huge_event event_fail = { .rc = EIO };

static mutex_t *event_mutex;

static void add_event(struct buff *buff, int rc)
{
	if (event_fail.buff)
		return;

	if (event_mutex == NULL) {
		event_mutex = mutex_create();
		if (!event_mutex)
			goto failed;
	}

	mutex_lock(event_mutex);
	struct huge_event *event = calloc(1, sizeof(struct huge_event));
	if (!event)
		goto failed;

	event->buff = buff;
	event->rc = rc;
	if (events == NULL)
		events = event;
	else
		events_tail->next = event;
	events_tail = event;
	return;

failed:
	event_fail.buff = buff; // mark as failed
	events = &event_fail;
	mutex_unlock(event_mutex);
}

static void do_modeline_invalidate(struct buff *buff, int rc);

void check_events(void)
{
	mutex_lock(event_mutex);
	while (events) {
		struct huge_event *next = events->next;
		do_modeline_invalidate(events->buff, events->rc);
		if (events != &event_fail)
			free(events);
		events = next;
	}
	mutex_unlock(event_mutex);
}

#endif

static jmp_buf zrefresh_jmp;

static void huge_file_modified(struct buff *buff)
{
	zbuff_t *zbuff = cfindzbuff(buff);
	if (!zbuff) { /* can't happen */
		terror("\n\nUnable to get zbuff for huge buffer\n\n\n");
		exit(1);
	}

again:
	strconcat(PawStr, PAWSTRLEN,
			  "WARNING: huge file ", lastpart(zbuff->fname),
			  " has been modified. Re-read? ", NULL);
	if (ask(PawStr) != YES) {
		if (ask("Delete buffer? ") != YES)
			goto again;
		cdelbuff(zbuff);
		return;
	}

	bempty(buff);
	breadhuge(buff, zbuff->fname);
	btostart(buff);
	longjmp(zrefresh_jmp, 1);
}

static void huge_file_io(struct buff *buff)
{
	zbuff_t *zbuff = cfindzbuff(buff);
	if (!zbuff) { /* can't happen */
		terror("\n\nUnable to get zbuff for huge buffer\n\n\n");
		exit(1);
	}

	strconcat(PawStr, PAWSTRLEN, "FATAL: huge file ", lastpart(zbuff->fname),
			  " had an I/O error. ", NULL);
	ask(PawStr);
	cdelbuff(zbuff);
	longjmp(zrefresh_jmp, 1);
}

static void do_modeline_invalidate(struct buff *buff, int rc)
{
	switch (rc) {
	case 0:
	{
		struct wdo *wdo = findwdo(buff);
		if (wdo)
			wdo->modeflags = INVALID;
		return; /* success */
	}
	case EAGAIN:
		return;
	case EBADF:
		huge_file_modified(buff);
		return;
	case EIO:
		huge_file_io(buff);
		return;
	default:
		default_huge_file_cb(buff, rc);
	}
}

static void modeline_invalidate(struct buff *buff, int rc)
{
#if HUGE_THREADED
	if (buff->lock) {
		/* we are in the thread */
		add_event(buff, rc);
		return;
	}
#endif

	do_modeline_invalidate(buff, rc);
}
#endif

void display_init(struct mark *mrk)
{
	int cnt;

	/* Set the screen marks */
	Scrnmarks[0].next = &Scrnmarks[1];
	for (cnt = 1; cnt < ROWMAX + EXTRA_MARKS; ++cnt) {
		Scrnmarks[cnt].prev  = &Scrnmarks[cnt - 1];
		Scrnmarks[cnt].next  = &Scrnmarks[cnt + 1];
	}
	Scrnmarks[cnt - 1].next = NULL;

	/* Now point the display marks at extra screen marks */
	was     = &Scrnmarks[ROWMAX + 0];
	Sstart	= &Scrnmarks[ROWMAX + 1];
	Psstart	= &Scrnmarks[ROWMAX + 2];
	Send	= &Scrnmarks[ROWMAX + 3];

	/* init the mark list */
	Marklist = Send;

	if (mrk)
		/* user provided a start mark */
		set_sstart(mrk);

#if HUGE_FILES
	huge_file_cb = modeline_invalidate;
#endif
}

/* True if user mark moved */
static bool umarkmoved(struct mark *tmark)
{
	return	UMARK_SET && !mrkatmrk(tmark, UMARK);
}

void set_umark(struct mark *tmark)
{
	if (!UMARK_SET)
		if (!(UMARK = bcremark(Bbuff)))
			return;

	if (tmark)
		mrktomrk(UMARK, tmark);
	else
		bmrktopnt(Bbuff, UMARK);
}

void clear_umark(void)
{
	if (UMARK_SET) {
		int i;
		for (i = 0; i < ROWMAX; ++i)
			Scrnmodf[i] = true;
		Tlrow = -1;

		bdelmark(UMARK);
		UMARK = NULL;
	}
}

/* True if buffer at user mark */
static bool bisatumark(void)
{
	return  UMARK_SET && bisatmrk(Bbuff, UMARK);
}

/** Returns true if point is between start and end. This has to walk
 * all the pages between start and end. So it is most efficient for
 * small ranges and terrible if end is before start.
 *
 * Note: point == start == end returns false: it is not between.
 */
static bool bisbetweenmrks(struct buff *buff, struct mark *start, struct mark *end)
{
	struct page *tp;
	bool found = false;

	if (start->mbuff != buff || end->mbuff != buff)
		return false;

	if (buff->curpage == start->mpage)
		if (buff->curchar < start->moffset)
			return false;

	for (tp = start->mpage; tp; tp = tp->nextp)
		if (tp == buff->curpage) {
			if (tp == end->mpage)
				return buff->curchar < end->moffset;
			found = true;
		} else if (tp == end->mpage)
			return found;

	return false;
}

/* Set Sstart and Psstart. Mark Send as invalid. */
void set_sstart(struct mark *mrk)
{
	mrktomrk(Sstart, mrk);
	mrktomrk(Psstart, mrk);
	/* Psstart - 1 */
	if (Psstart->moffset)
		--Psstart->moffset;
	else if (Psstart->mpage->prevp) {
		Psstart->mpage = Psstart->mpage->prevp;
		Psstart->moffset = Psstart->mpage->plen;
	} else
		Psstart->mbuff = NULL; /* mark as invalid */
	Send->mbuff = NULL; /* mark as invalid */
}

/* Mark screen invalid */
void redisplay(void)
{
	struct wdo *wdo;
	int i;

	foreachwdo(wdo)
		wdo->modeflags = INVALID;

	tclrwind();
	for (i = 0; i < Rowmax - 2; ++i)
		Scrnmodf[i] = true;
	Tlrow = -1;

	/* This is needed to set Comstate to false */
	uncomment(NULL);
}

/* Do the actual display update from the buffer */
void zrefresh(void)
{
	int pntrow, col;
	struct mark pmark;
	struct wdo *wdo;
	int tsave;

#if HUGE_FILES
	setjmp(zrefresh_jmp);
#endif

	bmrktopnt(Bbuff, &pmark);
	if (InPaw) {
		pawdisplay(&pmark, was);
		return;
	}
	if (++NESTED > 10)
		hang_up(0);

	setmodes(Curbuff);	/* SAM make sure OK */

	if (umarkmoved(was)) {
		/* the user mark has moved! */
		vsetmrk(was);
		vsetmrk(UMARK);
		mrktomrk(was, UMARK);
	}

	if (!bisbetweenmrks(Bbuff, Sstart, Send))
		/* The cursor has moved before/after the screen marks */
		reframe();
	else if (mrkatmrk(Sstart, Psstart) && !bisstart(Bbuff))
		/* Deleted first char in window that is not at buffer start */
		reframe();

	bpnttomrk(Bbuff, Sstart);
	pntrow = innerdsp(Curwdo->first, Curwdo->last, &pmark);
	/* Buffer is almost always after mark.. this is more efficient for
	 * huge buffers.
	 */
	if (!bisaftermrk(Bbuff, &pmark) || bisatmrk(Bbuff, &pmark)) {
		/* Point is off the end of the screen. Mainly for yanks. */
		bpnttomrk(Bbuff, &pmark);
		reframe();
		zrefresh();
		--NESTED;
		return;
	}

	/* update the other windows except Curwdo */
	tsave = Tabsize;
	foreachwdo(wdo)
		if (wdo != Curwdo) {
			struct mark point;
			zswitchto(wdo->wbuff);
			settabsize(Curbuff->bmode);
			bmrktopnt(Bbuff, &point);
			bpnttomrk(Bbuff, wdo->wstart);
			innerdsp(wdo->first, wdo->last, NULL);
			modeflags(wdo);
			bpnttomrk(Bbuff, &point);
			zswitchto(Curwdo->wbuff);
		}
	Tabsize = tsave;

	/* position the cursor */
	col = 0;
	bpnttomrk(Bbuff, &Scrnmarks[pntrow]);
	while (bisbeforemrk(Bbuff, &pmark)) {
		col += chwidth(Buff(), col, false);
		bmove1(Bbuff);
	}
	t_goto(pntrow, col);

	/*
	 * If we display the cursor on the mark, they both disappear.
	 * This code checks for this case: if true it removes the mark
	 * and invalidates its position so it will be updated when the
	 * cursor moves on...
	 */
	if (bisatumark()) {
		tstyle(T_NORMAL);
		tprntchar((bisend(Bbuff) || ISNL(Buff())) ? ' ' : Buff());
		t_goto(pntrow, col);
		was->moffset = -1; /* Invalidate it */
	}

	modeflags(Curwdo);
	setmodes(Curbuff);	/* displaying other windows can blow modes */
	tflush();
	tstyle(T_NORMAL);

	--NESTED;
}

static int buff_col(void)
{	/* Current column after current buffer char */
	return Pcol + chwidth(Buff(), Pcol, false);
}

static void extendedlinemarker(void)
{
	int col;

	for (col = Pcol; col < Colmax - 1; ++col)
		tprntchar(' ');
	tstyle(T_BOLD);
	tprntchar('>');
	tstyle(T_NORMAL);
}

static bool in_region(struct mark *pmark)
{
	if (!UMARK_SET || !pmark)
		return false;

#if 0
	if (mrkaftermrk(pmark, UMARK))
		return bisbetweenmrks(Bbuff, UMARK, pmark);
	else
		return bisbetweenmrks(Bbuff, pmark, UMARK);
#else
	return bisatmrk(Bbuff, UMARK);
#endif
}

/* Fairly special routine. Pushes the char one past the end of the
 * buffer. */
static void bshove(void)
{
	btoend(Bbuff);
	++Bbuff->curcptr;
	++Bbuff->curchar;
}

/*
 * Do the actual screen update.
 * Curwdo is not valid.
 */
static int innerdsp(int from, int to, struct mark *pmark)
{
	static int pntrow;
	int trow;
	Byte *lptr;
	int needpnt = true, col;

	if (VAR(VCOMMENTS))
		resetcomments();

	if (UMARK_SET)
		tsetcursor();

	for (trow = from; trow < to; ++trow) {
		if (Scrnmodf[trow] || !bisatmrk(Bbuff, &Scrnmarks[trow]) || UMARK_SET) {
			Scrnmodf[trow] = false;
			bmrktopnt(Bbuff, &Scrnmarks[trow]); /* Do this before tkbrdy */
			lptr = tline;
			col = 0;
			t_goto(trow, col);
			while (!bisend(Bbuff) && !ISNL(Buff()) &&
				   (col = buff_col()) < Colmax) {
				if (in_region(pmark)) {
					tstyle(T_REGION);
					tprntchar(Buff());
					tstyle(T_NORMAL);
				} else if (trow == Tlrow &&
					Buff() == *lptr &&
					Buff() != (Byte)'\376')
					Pcol = col;
				else {
					if (bisatumark())
						setmark(true);
					else
						/* usually tprntchar */
						printchar(Buff());
					if (trow == Tlrow &&
						(!ZISPRINT(*lptr) ||
						 !ZISPRINT(Buff())))
						Tlrow = -1;
				}
				*lptr++ = Buff();
				bmove1(Bbuff);
			}
			tcleol();
			if (bisatumark() &&
				(ISNL(Buff()) || bisstart(Bbuff) || bisend(Bbuff)))
				setmark(false);
			if (col >= Colmax)
				extendedlinemarker();
			memset(lptr, '\376', Colmax - (lptr - tline));
			Tlrow = trow;
			if (Pcol < Colmax) {
				if (bisend(Bbuff))
					bshove();
				else if (ISNL(Buff()))
					bmove1(Bbuff);
			}
		} else
			bpnttomrk(Bbuff, &Scrnmarks[trow + 1]);
		if (needpnt && pmark && bisaftermrk(Bbuff, pmark)) {
			pntrow = trow;
			needpnt = false;
		}
	}
	bmrktopnt(Bbuff, &Scrnmarks[trow]);
	if (pmark) {
		bmrktopnt(Bbuff, Send);
		if (needpnt) {
			/* the user has typed past the end of the screen */
			reframe();
			zrefresh();
		}
	}

	tstyle(T_NORMAL);

	if (UMARK_SET)
		tsetcursor();

	return pntrow;
}

/* Work for centering redisplay */
void reframe(void)
{
	int cnt;
	struct mark pmark, new_start;

	bmrktopnt(Bbuff, &pmark);
	for (cnt = prefline(); cnt > 0 && bcrsearch(Bbuff, NL); --cnt)
			cnt -= bgetcol(true, 0) / Colmax;
	if (cnt < 0)
		bmakecol((-cnt) * Colmax);
	else
		tobegline(Bbuff);
	bmrktopnt(Bbuff, &new_start);
	set_sstart(&new_start);
	bpnttomrk(Bbuff, &pmark);
}

static inline void modeline_style(void)
{
	tstyle(ring_bell ? T_BELL : T_STANDOUT);
}

/* Redraw the modeline except for flags. */
static void modeline(struct wdo *wdo)
{
	char str[COLMAX + 1]; /* can't use PawStr because of setmodes */
	int len;

	t_goto(wdo->last, 0);
	modeline_style();
	strconcat(str, sizeof(str), ZSTR, " ", VERSION, "  (", setmodes(wdo->wbuff),
			  ")  ", wdo->wbuff->bname, NULL);
	tprntstr(str);
	if (wdo->wbuff->fname) {
		char *fname = wdo->wbuff->fname;

		len = strlen(str) + 4;
		tprntchar(':');
		if (strncmp(fname, Home, Homelen) == 0) {
			fname += Homelen;
			len++;
			tprntchar('~');
		}
		tprntstr(limit(fname, len));
	}
	wdo->modecol = Pcol;

	/* space pad the line */
	for (len = Colmax - Pcol; len > 0; --len)
		tprntchar(' ');
	tstyle(T_NORMAL);
}

/* This routine will call modeline if wdo->modeflags == INVALID */
static void modeflags(struct wdo *wdo)
{
	unsigned trow, tcol;
	int mask;

	trow = Prow;
	tcol = Pcol;

	if (wdo->modeflags == INVALID)
		modeline(wdo);

	modeline_style();

	mask = delcmd() | (wdo->wbuff->buff->bmodf ? 2 : 0);
	if (!InPaw && wdo->modeflags != mask) {
		t_goto(wdo->last, wdo->modecol);
		tprntchar(mask & 2 ? '*' : ' ');
		tprntchar(mask & 1 ? '+' : ' ');
		wdo->modeflags = mask;
	}

	tstyle(T_NORMAL);
	t_goto(trow, tcol);
}

#define PAWCAT(s) strcat(PawStr, s)

/* local routine to set PawStr to the correct mode */
static char *setmodes(zbuff_t *buff)
{
	*PawStr = 0;

	if (!InPaw)	/* we should never be in the Paw but .... */
		Curcmds = 0;

	/* set all keys back to default */
	Keys[CR] = ZNEWLINE;
	Keys[' '] = Keys['}'] = Keys['#'] = Keys[':'] = Keys['/'] = ZINSERT;
	Keys['\t'] = ZTAB;
	printchar = tprntchar;

	/* Set PawStr to majour mode and setup any special keys */
	switch (buff->bmode & MAJORMODE) {
	case CMODE:
		PAWCAT("C");
		Keys[CR] = ZC_INDENT;
		Keys['}'] = Keys['#'] = Keys[':'] = ZC_INSERT;
		if (VAR(VCOMMENTS)) {
			Keys['/'] = ZC_INSERT;
			printchar = cprntchar;
		}
		break;
	case SHMODE:
		PAWCAT("sh");
		Keys[CR] = ZSH_INDENT;
		if (VAR(VCOMMENTS))
			printchar = cprntchar;

		shell_init();
		break;
	case PYMODE:
		PAWCAT("py");
		Keys[CR] = ZPY_INDENT;
		if (VAR(VCOMMENTS))
			printchar = cprntchar;
		break;
	case TXTMODE:
		PAWCAT("Text");
		Keys[' '] = Keys[CR] = ZFILL_CHECK;
		break;
	default:
		PAWCAT("Normal");
	}

	if (buff->bmode & VIEW)
		PAWCAT(" RO");
#if HUGE_FILES
	if (buff->buff->fd != -1)
		PAWCAT(" H");
#endif
	if (buff->bmode & FILE_COMPRESSED)
		PAWCAT(" Z");
	if (buff->bmode & FILE_CRLF)
		PAWCAT(" CR");
	if (buff->bmode & OVERWRITE)
		PAWCAT(" OVWRT");

	settabsize(buff->bmode);
	return PawStr;
}

/* Set one windows modified flags. */
static void subset(struct buff *buff, int from, int to)
{
	struct mark *btmark;
	int row;

	if (Scrnmarks[from].mbuff != buff)
		return;

	for (row = from; row <= to && Scrnmarks[row].mpage != buff->curpage; ++row) ;

	if (row > to) {
		for (row = from, btmark = &Scrnmarks[from];
			 row <= to && (btmark->mbuff != buff || bisaftermrk(buff, btmark));
			 ++btmark, ++row)
			;
		if (row > from) {
			while (Scrnmarks[--row].mbuff != buff) ;
			Scrnmodf[row] = true;
		}
	} else {
		btmark = &Scrnmarks[row];
		while (btmark->mpage == buff->curpage && btmark->moffset <= buff->curchar && row <= to) {
			++btmark;
			++row;
		}
		if (--row >= from)
			Scrnmodf[row] = true;
		while (row > from && bisatmrk(buff, &Scrnmarks[row])) {
			Scrnmodf[--row] = true;
		}
	}
}

/* Insert the correct modified flags. */
void vsetmod_callback(struct buff *buff)
{
	struct wdo *wdo;

	if (buff == NULL) buff = Bbuff;

	foreachwdo(wdo)
		if (wdo->wbuff == Curbuff)
			subset(buff, wdo->first, wdo->last);
}

void vsetmrk(struct mark *mrk)
{
	if (mrk) {
		int row;

		Tlrow = -1; /* Needed if mark and point on same row */

		for (row = 0; row < Rowmax - 1; ++row)
			if (mrkaftermrk(&Scrnmarks[row], mrk)) {
				if (row > 0)
					Scrnmodf[row - 1] = true;
				return;
			}
	}
}

void invalidate_scrnmarks(unsigned from, unsigned to)
{
	unsigned i;

	for (i = from; i < to; ++i)
		Scrnmodf[i] = true;
}

#define SHIFT	(Colmax / 4 + 1)

static void pawdisplay(struct mark *pmark, struct mark *was)
{
	int bcol = 0, i, nested = 0;
	bool mrkmoved = umarkmoved(was);

	Prow = Rowmax - 1;
pawshift:
	btostart(Bbuff); bmove(Bbuff, Pshift);
	for (i = 0, Pcol = Pawcol;
		 Pcol < Colmax - 2 && !bisend(Bbuff);
		 bmove1(Bbuff), ++i) {
		if (bisatmrk(Bbuff, pmark))
			bcol = Pcol;
		if (mrkmoved && (bisatumark() || bisatmrk(Bbuff, was))) {
			if (bisatumark())
				tstyle(T_REVERSE);
			tprntchar(Buff());
			tstyle(T_NORMAL);
			tline[i] = Buff();
		} else if (tline[i] == Buff())
			Pcol += chwidth(Buff(), 0, 0);
		else {
			tline[i] = Buff();
			tprntchar(Buff());
		}
	}
	memset(&tline[i], '\376', &tline[COLMAX] - &tline[i]);
	tcleol();

	if (bisend(Bbuff)) {
		if (bisatumark()) {
			setmark(false);
			--Pcol;		/* space always 1 character! */
		} else if (bisatmrk(Bbuff, pmark))
			bcol = Pcol;
	}

	if (!bcol) {
		if (Pshift) {
			/* shift right */
			Pshift -= SHIFT;
			if (Pshift < 0)
				Pshift = 0;
			if (++nested == 1)
				goto pawshift;
			else
				Dbg("Shift right nested too deep!\n");
		} else if (Pcol >= Colmax - 2) {
			/* shift left */
			Pshift += SHIFT;
			if (++nested == 1)
				goto pawshift;
			else
				Dbg("Shift left nested too deep!\n");
		}
	}

	if (bcol)
		Pcol = bcol;
	bpnttomrk(Bbuff, pmark);
	if (UMARK_SET)
		mrktomrk(was, UMARK);

	/*
	 * If we display the cursor on the mark, they both disappear.
	 * This code checks for this case: if true it removes the mark
	 * and invalidates its position so it will be updated when the
	 * cursor moves on...
	 */
	if (bisatumark()) {
		i = Pcol;
		tprntchar(bisend(Bbuff) ? ' ' : Buff());
		Pcol = i;
		was->moffset = -1;		/* Invalidate it */
	}

	--NESTED;
	t_goto(Prow, Pcol);
	tflush();
}

void makepaw(char *word, bool start)
{
	zswitchto(Paw);
	bempty(Bbuff);
	binstr(Bbuff, word);
	tcleol();
	memset(tline, '\376', COLMAX);	/* invalidate it */
	if (start)
		btostart(Bbuff);
}
/* @} */
