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
static char *setmodes(struct buff *);
static void pawdisplay(struct mark *, struct mark *);

struct mark *Sstart, *Psstart;	/* Screen start and 'prestart' */
struct mark *Send;		/* Screen end */
bool Sendp;			/* Screen end set */
struct mark Scrnmarks[ROWMAX + 1];	/* Screen marks - one per line */
int Tlrow;			/* Last row displayed */

static int NESTED;		/* zrefresh can go recursive... */
static Byte tline[COLMAX + 1];

static void (*printchar)(Byte ichar) = tprntchar;

/* True if user mark moved */
static bool umarkmoved(struct mark *tmark)
{
	return	Curbuff->umark &&
		(tmark->moffset != Curbuff->umark->moffset ||
		 tmark->mpage != Curbuff->umark->mpage ||
		 tmark->mbuff != Curbuff->umark->mbuff);
}

/* True if buffer at user mark */
static bool bisatumark(void)
{
	return  Curbuff->umark &&
		Curpage == Curbuff->umark->mpage &&
		Curchar == Curbuff->umark->moffset;
}

/* Mark screen invalid */
void redisplay(void)
{
	struct wdo *wdo;
	int i;

	foreachwdo(wdo)
		wdo->modeflags = INVALID;

	tclrwind();
	for (i = 0; i < tmaxrow() - 2; ++i)
		Scrnmarks[i].modf = true;
	Tlrow = -1;

	/* This is needed to set Comstate to false */
	uncomment(NULL);
}

/* Do the actual display update from the buffer */
void zrefresh(void)
{
	int pntrow, col, bcol;
	struct mark *pmark;
	struct wdo *wdo;
	int tsave;
	static struct mark *was;	/* last location of user mark */

	if (was == NULL)
		was = bcremrk();
	pmark = bcremrk();
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
		vsetmrk(Curbuff->umark);
		mrktomrk(was, Curbuff->umark);
	}

	if (bisbeforemrk(Sstart) || (Sendp && !bisbeforemrk(Send)) ||
	   Sstart->mbuff != Curbuff)
		/* The cursor has moved before/after the screen marks */
		reframe();
	bpnttomrk(Sstart);
	if (bisatmrk(Psstart) && !bisstart()) {
		/* Deleted first char in window that is not at buffer start */
		bpnttomrk(pmark);
		reframe();
		bpnttomrk(Sstart);
	}
	pntrow = innerdsp(Curwdo->first, Curwdo->last, pmark);
	if (bisbeforemrk(pmark) && !tkbrdy()) {
		bpnttomrk(pmark);
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
			bswitchto(wdo->wbuff);
			settabsize(Curbuff->bmode);
			point = bcremrk();
			bpnttomrk(wdo->wstart);
			innerdsp(wdo->first, wdo->last, NULL);
			modeflags(wdo);
			bpnttomrk(point);
			unmark(point);
			bswitchto(Curwdo->wbuff);
		}
	Tabsize = tsave;

	bpnttomrk(pmark);
	unmark(pmark);
	bcol = bgetcol(true, 0);
	/* position the cursor */
	col = bcol % (tmaxcol() - 1);
	/* special case for NL or bisend at column 80 */
	if (col == 0 && bcol && (ISNL(Buff()) || bisend()))
		col = tmaxcol() - 1;
	else if (!bisend() && (col + chwidth(Buff(), col, false) >= tmaxcol()))
		col = 0;
	t_goto(pntrow, col);

	/*
	 * If we display the cursor on the mark, they both disappear.
	 * This code checks for this case: if true it removes the mark
	 * and invalidates its position so it will be updated when the
	 * cursor moves on...
	 */
	if (bisatumark()) {
		tstyle(T_NORMAL);
		tprntchar((bisend() || ISNL(Buff())) ? ' ' : Buff());
		t_goto(pntrow, col);
		was->moffset = PSIZE + 1; /* Invalidate it */
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

	for (col = tgetcol(); col < tmaxcol() - 1; ++col)
		tprntchar(' ');
	tstyle(T_BOLD);
	tprntchar('>');
	tstyle(T_NORMAL);
}

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

	for (trow = from; trow < to; ++trow) {
		if (Scrnmarks[trow].modf || !bisatmrk(&Scrnmarks[trow])) {
			Scrnmarks[trow].modf = false;
			bmrktopnt(&Scrnmarks[trow]); /* Do this before tkbrdy */
			lptr = tline;
			col = 0;
			tsetpoint(trow, col);
			while (!bisend() && !ISNL(Buff()) &&
			       (col = buff_col()) < tmaxcol()) {
				if (trow == Tlrow &&
				    Buff() == *lptr &&
				    Buff() != (Byte)'\376')
					tgetcol() = col;
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
				bmove1();
			}
			tcleol();
			if (bisatumark() && (ISNL(Buff()) || bisstart() || bisend()))
					setmark(false);
			if (col >= tmaxcol())
				extendedlinemarker();
			memset(lptr, '\376', Colmax - (lptr - tline));
			Tlrow = trow;
			if (tgetcol() < tmaxcol()) {
				if (bisend())
					bshoveit();
				else if (ISNL(Buff()))
					bmove1();
			}
		} else
			bpnttomrk(&Scrnmarks[trow + 1]);
		if (pmark && bisaftermrk(pmark) && needpnt) {
			pntrow = trow;
			needpnt = false;
		}
	}
	bmrktopnt(&Scrnmarks[trow]);
	if (pmark) {
		bmrktopnt(Send);
		Sendp = true;
		if (needpnt) {
			/* the user has typed past the end of the screen */
			reframe();
			zrefresh();
		}
	}

	tstyle(T_NORMAL);

	return pntrow;
}

/* Work for centering redisplay */
void reframe(void)
{
	int cnt;
	struct mark *pmark;

	pmark = bcremrk();
	for (cnt = prefline(); cnt > 0 && bcrsearch(NL); --cnt)
			cnt -= bgetcol(true, 0) / tmaxcol();
	if (cnt < 0)
		bmakecol((-cnt) * tmaxcol(), false);
	else
		tobegline();
	bmrktopnt(Sstart);
	bmove(-1);
	bmrktopnt(Psstart);
	Sendp = false;
	bpnttomrk(pmark);
	unmark(pmark);
}

/* Redraw the modeline except for flags. */
static void modeline(struct wdo *wdo)
{
	char str[COLMAX + 1]; /* can't use PawStr because of setmodes */
	int len;

	tsetpoint(wdo->last, 0);
	tstyle(T_STANDOUT);
	sprintf(str, "%s %s  (%s)  %s: ", ZSTR, VERSION,
		setmodes(wdo->wbuff), wdo->wbuff->bname);
	tprntstr(str);
	if (wdo->wbuff->fname) {
		len = (VAR(VLINES) ? 13 : 3) + strlen(str);
		tprntstr(limit(wdo->wbuff->fname, len));
	}
	wdo->modecol = tgetcol();

	/* space pad the line */
	for (len = tmaxcol() - tgetcol(); len > 0; --len)
		tprntchar(' ');
	tstyle(T_NORMAL);
}

/* This routine will call modeline if wdo->modeflags == INVALID */
static void modeflags(struct wdo *wdo)
{
	unsigned trow, tcol, line, col;
	int mask;

	trow = tgetrow();
	tcol = tgetcol();

	if (wdo->modeflags == INVALID)
		modeline(wdo);

	tstyle(T_STANDOUT);

	if (VAR(VLINES)) {
		struct buff *was = Curbuff;
		bswitchto(wdo->wbuff);
		blocation(&line);
		col = bgetcol(false, 0) + 1;
		if (col > 999)
			sprintf(PawStr, "%5u:???", line);
		else
			sprintf(PawStr, "%5u:%-3u", line, col);
		PawStr[9] = '\0';
		tsetpoint(wdo->last, tmaxcol() - 9);
		tprntstr(PawStr);
		bswitchto(was);
	}

	mask = delcmd() | (wdo->wbuff->bmodf ? 2 : 0);
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
static char *setmodes(struct buff *buff)
{
	if (!InPaw)	/* we should never be in the Paw but .... */
		Curcmds = 0;

	/* set all keys back to default */
	Keys[CR] = CRdefault;
	Keys[' '] = Keys['}'] = Keys['#'] = Keys[':'] = Keys['/'] = ZINSERT;
	Keys['\t'] = ZTAB;
	printchar = tprntchar;

	/* Set PawStr to majour mode and setup any special keys */
	switch (buff->bmode & MAJORMODE) {
	case CMODE:
		strcpy(PawStr, "C");
		Keys[CR] = ZC_INDENT;
		Keys['}'] = Keys['#'] = Keys[':'] = Keys['\t'] = ZC_INSERT;
		if (VAR(VCOMMENTS)) {
			Keys['/'] = ZC_INSERT;
			printchar = cprntchar;
		}
		break;
	case SHMODE:
		strcpy(PawStr, "sh");
		Keys[CR] = ZC_INDENT;
		Keys['\t'] = ZC_INSERT;
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

	if (buff->bmode & VIEW)
		strcat(PawStr, " RO");
	if (buff->bmode & COMPRESSED)
		strcat(PawStr, " Z");
	if (buff->bmode & OVERWRITE)
		strcat(PawStr, " OVWRT");

	settabsize(buff->bmode);
	return PawStr;
}

/* Set one windows modified flags. */
static void subset(int from, int to, bool flag)
{
	struct mark *btmark, *ltmark;

	if (Scrnmarks[from].mbuff != Curbuff)
		return;
	for (btmark = &Scrnmarks[from], ltmark = &Scrnmarks[to];
		 btmark <= ltmark && btmark->mpage != Curpage;
		 ++btmark)
		;
	if (btmark > ltmark) {
		for (btmark = &Scrnmarks[from];
		     btmark <= ltmark &&
			     (btmark->mbuff != Curbuff || bisaftermrk(btmark));
		     ++btmark)
			;
		if (btmark > &Scrnmarks[from]) {
			while ((--btmark)->mbuff != Curbuff)
				;
			btmark->modf = true;
		}
	} else {
		while (btmark->mpage == Curpage && btmark->moffset <= Curchar &&
			   btmark <= ltmark)
			++btmark;
		if (--btmark >= &Scrnmarks[from])
			btmark->modf = true;
		if (flag)
			while (btmark > &Scrnmarks[from] &&
			       btmark->mpage == Curpage &&
			       btmark->moffset == Curchar)
				(--btmark)->modf = true;
	}
}

/* Insert the correct modified flags. */
void vsetmod(bool flag)
{
	struct wdo *wdo;

	foreachwdo(wdo)
		if (wdo->wbuff == Curbuff)
			subset(wdo->first, wdo->last, flag);
}

void vsetmrk(struct mark *mrk)
{
	int row;

	Tlrow = -1; /* Needed if mark and point on same row */

	for (row = 0; row < tmaxrow() - 1; ++row)
		if (mrkaftermrk(&Scrnmarks[row], mrk)) {
			if (row > 0)
				Scrnmarks[row - 1].modf = true;
			return;
		}
}

#define SHIFT	(Colmax / 4 + 1)

static void pawdisplay(struct mark *pmark, struct mark *was)
{
	int bcol = 0, i, nested = 0;
	bool mrkmoved = umarkmoved(was);

	Prow = Rowmax - 1;
pawshift:
	btostart(); bmove(Pshift);
	for (i = 0, Pcol = Pawcol;
	     Pcol < Colmax - 2 && !bisend();
	     bmove1(), ++i) {
		if (bisatmrk(pmark))
			bcol = Pcol;
		if (mrkmoved && (bisatumark() || bisatmrk(was))) {
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

	if (bisend()) {
		if (bisatumark()) {
			setmark(false);
			--Pcol;		/* space always 1 character! */
		} else if (bisatmrk(pmark))
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
	bpnttomrk(pmark);
	if (Curbuff->umark)
		mrktomrk(was, Curbuff->umark);

	/*
	 * If we display the cursor on the mark, they both disappear.
	 * This code checks for this case: if true it removes the mark
	 * and invalidates its position so it will be updated when the
	 * cursor moves on...
	 */
	if (bisatumark()) {
		i = Pcol;
		tprntchar(bisend() ? ' ' : Buff());
		Pcol = i;
		was->moffset = PSIZE + 1;		/* Invalidate it */
	}

	unmark(pmark);
	--NESTED;
	tforce();
	tflush();
}

void initscrnmarks(void)
{
	int cnt;

	/* Set the screen marks */
	memset((char *)Scrnmarks, 0, sizeof(Scrnmarks));
	Scrnmarks[0].next = &Scrnmarks[1];
	for (cnt = 1; cnt < ROWMAX; ++cnt) {
		Scrnmarks[cnt].prev  = &Scrnmarks[cnt - 1];
		Scrnmarks[cnt].next  = &Scrnmarks[cnt + 1];
	}
	Scrnmarks[ROWMAX - 1].next = NULL;

	/* init the Mrklist */
	Mrklist = &Scrnmarks[ROWMAX - 1];
}

void makepaw(char *word, bool start)
{
	bswitchto(Paw);
	btostart();
	bdelete(Curplen);
	binstr(word);
	tcleol();
	memset(tline, '\376', COLMAX);	/* invalidate it */
	if (start)
		btostart();
}
