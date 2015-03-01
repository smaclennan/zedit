/* display.c - Zedit main display update
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

#include "z.h"
#include "assert.h"

static int innerdsp(int from, int to, struct mark *pmark);
static void modeflags(struct wdo *wdo);
static char *setmodes(struct zbuff *);
static void pawdisplay(struct mark *, struct mark *);

struct mark *Sstart, *Psstart;	/* Screen start and 'prestart' */
struct mark *Send;		/* Screen end */
bool Sendp;			/* Screen end set */
int Tlrow;			/* Last row displayed */

static int NESTED;		/* zrefresh can go recursive... */
static Byte tline[COLMAX + 1];
static struct mark Scrnmarks[ROWMAX + 3];	/* Screen marks - one per line */
static bool Scrnmodf[ROWMAX];

/* Keeping just one mark around is a HUGE win for a trivial amount of code. */
static struct mark *freeumark;

static void (*printchar)(Byte ichar) = tprntchar;

void display_init(void)
{
	int cnt;

	/* Set the screen marks */
	Scrnmarks[0].next = &Scrnmarks[1];
	for (cnt = 1; cnt < ROWMAX + 3; ++cnt) {
		Scrnmarks[cnt].prev  = &Scrnmarks[cnt - 1];
		Scrnmarks[cnt].next  = &Scrnmarks[cnt + 1];
	}
	Scrnmarks[cnt - 1].next = NULL;

	/* Now point the display marks at the last 3 screen marks */
	Sstart	= &Scrnmarks[ROWMAX];
	Psstart	= &Scrnmarks[ROWMAX + 1];
	Send	= &Scrnmarks[ROWMAX + 2];
	Sendp	= false;

	/* init the mark list */
	minit(Send);
}

/* True if user mark moved */
static bool umarkmoved(struct mark *tmark)
{
	return	UMARK_SET && !mrkatmrk(tmark, UMARK);
}

void set_umark(struct mark *tmark)
{
	if (!UMARK_SET) {
		if (freeumark) {
			UMARK = freeumark;
			freeumark = NULL;
		} else if (!(UMARK = bcremrk(Bbuff)))
			return;
	}

	if (tmark)
		mrktomrk(UMARK, tmark);
	else
		bmrktopnt(Bbuff, UMARK);
}

void clear_umark(void)
{
	if (UMARK_SET) {
#if SHOW_REGION
		int i;
		for (i = 0; i < ROWMAX; ++i)
			Scrnmodf[i] = true;
		Tlrow = -1;
#else
		vsetmrk(UMARK);
#endif

		if (freeumark)
			unmark(UMARK);
		else
			freeumark = UMARK;
		UMARK = NULL;
	}
}

/* True if buffer at user mark */
static bool bisatumark(void)
{
	return  UMARK_SET && bisatmrk(Bbuff, UMARK);
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
	struct mark *pmark;
	struct wdo *wdo;
	int tsave;
	static struct mark *was;	/* last location of user mark */

	if (was == NULL)
		was = zcreatemrk();
	pmark = zcreatemrk();
	if (InPaw) {
		pawdisplay(pmark, was);
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

	if (bisbeforemrk(Bbuff, Sstart) || (Sendp && !bisbeforemrk(Bbuff, Send)) ||
	   Sstart->mbuff != Bbuff)
		/* The cursor has moved before/after the screen marks */
		reframe();
	bpnttomrk(Bbuff, Sstart);
	if (bisatmrk(Bbuff, Psstart) && !bisstart(Bbuff)) {
		/* Deleted first char in window that is not at buffer start */
		bpnttomrk(Bbuff, pmark);
		reframe();
		bpnttomrk(Bbuff, Sstart);
	}
	pntrow = innerdsp(Curwdo->first, Curwdo->last, pmark);
	if (bisbeforemrk(Bbuff, pmark) && !tkbrdy()) {
		bpnttomrk(Bbuff, pmark);
		unmark(pmark);
		reframe();
		zrefresh();
		--NESTED;
		return;
	}

	/* update the other windows except Curwdo */
	tsave = Tabsize;
	foreachwdo(wdo)
		if (wdo != Curwdo) {
			struct mark *point;
			zswitchto(wdo->wbuff);
			settabsize(zapp(Curbuff)->bmode);
			point = zcreatemrk();
			bpnttomrk(Bbuff, wdo->wstart);
			innerdsp(wdo->first, wdo->last, NULL);
			modeflags(wdo);
			bpnttomrk(Bbuff, point);
			unmark(point);
			zswitchto(Curwdo->wbuff);
		}
	Tabsize = tsave;

	/* position the cursor */
	col = 0;
	bpnttomrk(Bbuff, &Scrnmarks[pntrow]);
	while (bisbeforemrk(Bbuff, pmark)) {
		col += chwidth(Buff(), col, false);
		bmove1(Bbuff);
	}
	t_goto(pntrow, col);
	unmark(pmark);

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
#if SHOW_REGION
	if (!UMARK_SET || !pmark)
		return false;

	if (bisaftermrk(Bbuff, UMARK) && bisbeforemrk(Bbuff, pmark))
		return true;

	if (bisaftermrk(Bbuff, pmark) && bisbeforemrk(Bbuff, UMARK))
		return true;
#endif

	return false;
}

/* Fairly special routine. Pushes the char one past the end of the
 * buffer. */
static void bshove(void)
{
	btoend(Bbuff);
	++Curcptr;
	++Curchar;
}

#if SHOW_REGION
#define REGION_ON UMARK_SET
#else
#define REGION_ON false
#endif

/*
 * Do the acutal screen update.
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
		tsetcursor(true);

	for (trow = from; trow < to; ++trow) {
		if (Scrnmodf[trow] || !bisatmrk(Bbuff, &Scrnmarks[trow]) || REGION_ON) {
			Scrnmodf[trow] = false;
			bmrktopnt(Bbuff, &Scrnmarks[trow]); /* Do this before tkbrdy */
			lptr = tline;
			col = 0;
			tsetpoint(trow, col);
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
		if (pmark && bisaftermrk(Bbuff, pmark) && needpnt) {
			pntrow = trow;
			needpnt = false;
		}
	}
	bmrktopnt(Bbuff, &Scrnmarks[trow]);
	if (pmark) {
		bmrktopnt(Bbuff, Send);
		Sendp = true;
		if (needpnt) {
			/* the user has typed past the end of the screen */
			reframe();
			zrefresh();
		}
	}

	tstyle(T_NORMAL);

	if (UMARK_SET)
		tsetcursor(false);

	return pntrow;
}

/* Work for centering redisplay */
void reframe(void)
{
	int cnt;
	struct mark pmark;

	bmrktopnt(Bbuff, &pmark);
	for (cnt = prefline(); cnt > 0 && bcrsearch(Bbuff, NL); --cnt)
			cnt -= bgetcol(true, 0) / Colmax;
	if (cnt < 0)
		bmakecol((-cnt) * Colmax, false);
	else
		tobegline(Bbuff);
	bmrktopnt(Bbuff, Sstart);
	bmove(Bbuff, -1);
	bmrktopnt(Bbuff, Psstart);
	Sendp = false;
	bpnttomrk(Bbuff, &pmark);
}

/* Redraw the modeline except for flags. */
static void modeline(struct wdo *wdo)
{
	char str[COLMAX + 1]; /* can't use PawStr because of setmodes */
	int len;

	tsetpoint(wdo->last, 0);
	tstyle(T_STANDOUT);
	sprintf(str, "%s %s  (%s)  %s: ", ZSTR, VERSION,
			setmodes(wdo->wbuff), zapp(wdo->wbuff)->bname);
	tprntstr(str);
	if (zapp(wdo->wbuff)->fname) {
		len = strlen(str) + 3;
		tprntstr(limit(zapp(wdo->wbuff)->fname, len));
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

	tstyle(T_STANDOUT);

	mask = delcmd() | (wdo->wbuff->buff->bmodf ? 2 : 0);
	if (!InPaw && wdo->modeflags != mask) {
		tsetpoint(wdo->last, wdo->modecol);
		tprntchar(mask & 2 ? '*' : ' ');
		tprntchar(mask & 1 ? '+' : ' ');
		wdo->modeflags = mask;
	}

	tstyle(T_NORMAL);
	t_goto(trow, tcol);
}

/* local routine to set PawStr to the correct mode */
static char *setmodes(struct zbuff *buff)
{
	if (!InPaw)	/* we should never be in the Paw but .... */
		Curcmds = 0;

	/* set all keys back to default */
	Keys[CR] = CRdefault;
	Keys[' '] = Keys['}'] = Keys['#'] = Keys[':'] = Keys['/'] = ZINSERT;
	Keys['\t'] = ZTAB;
	printchar = tprntchar;

	/* Set PawStr to majour mode and setup any special keys */
	switch (zapp(buff)->bmode & MAJORMODE) {
	case CMODE:
		strcpy(PawStr, "C");
		Keys[CR] = ZC_INDENT;
		Keys['}'] = Keys['#'] = Keys[':'] = ZC_INSERT;
		if (VAR(VCOMMENTS)) {
			Keys['/'] = ZC_INSERT;
			printchar = cprntchar;
		}
		break;
	case SHMODE:
		strcpy(PawStr, "sh");
		Keys[CR] = ZC_INDENT;
		if (VAR(VCOMMENTS))
			printchar = cprntchar;
		break;
	case TXTMODE:
		strcpy(PawStr, "Text");
		Keys[' '] = Keys[CR] = ZFILL_CHECK;
		break;
	default:
		strcpy(PawStr, "Normal");
	}

	if (zapp(buff)->bmode & VIEW)
		strcat(PawStr, " RO");
	if (zapp(buff)->bmode & COMPRESSED)
		strcat(PawStr, " Z");
	if (zapp(buff)->bmode & CRLF)
		strcat(PawStr, " CR");
	if (zapp(buff)->bmode & OVERWRITE)
		strcat(PawStr, " OVWRT");

	settabsize(zapp(buff)->bmode);
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
	int i;

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

	unmark(pmark);
	--NESTED;
	tforce();
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
