/* comms.c - Zedit commands
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

#include "z.h"
#include "keys.h"

int Arg;
Boolean Argp;

/* Word COMMANDS */

/* Find the start of a word. In normal/text, we always move to the
 * start of a word if we are in the middle. In program mode, if we are
 * in a word, we do not move.
 */
static Boolean Findstart(void)
{
	if ((Curbuff->bmode & PROGMODE))
		moveto(bistoken, FORWARD);
	else {
		while (!bisstart() && bistoken())
			bmove(-1);
		while (!bisend() && !isalpha(*Curcptr))
			bmove1();
	}
	return !bisend();
}

void Zcapword(void)
{
	if (Findstart()) {
		*Curcptr = toupper(*Curcptr);
		Curbuff->bmodf = Curmodf = MODIFIED;
		for (bmove1(); !bisend() && bistoken(); bmove1())
			*Curcptr = tolower(*Curcptr);
		vsetmod(FALSE);
	}
}


void Zlowword(void)
{
	if (Findstart()) {
		for ( ; !bisend() && bistoken(); bmove1()) {
			Curbuff->bmodf = Curmodf = MODIFIED;
			*Curcptr = tolower(*Curcptr);
		}
		vsetmod(FALSE);
	}
}


void Zupword(void)
{
	if (Findstart()) {
		for ( ; !bisend() && bistoken(); bmove1()) {
			Curbuff->bmodf = Curmodf = MODIFIED;
			*Curcptr = toupper(*Curcptr);
		}
		vsetmod(FALSE);
	}
}

/* Move a block of chars around point and "from" to "to".
 * Assumes point is before "from".
*/
static void blockmove(struct mark *from, struct mark *to)
{
	char tmp;

	while (bisbeforemrk(from)) {
		tmp = *Curcptr;
		bswappnt(to);
		binsert(tmp);
		bswappnt(to);
		bdelete(1);
	}
}


void Zswapword(void)
{
	struct mark *tmark, *tmp;

	moveto(bistoken, FORWARD);
	if (bisend())
		return;
	tmark = bcremrk();
	movepast(bistoken, FORWARD);
	tmp = bcremrk();
	bpnttomrk(tmark);
	moveto(bistoken, BACKWARD);
	blockmove(tmark, tmp);
	movepast(bistoken, BACKWARD);
	blockmove(tmark, tmp);
	bpnttomrk(tmp);
	unmark(tmark);
	unmark(tmp);
}


void Zcenter(void)
{
	int tmp;

	tobegline();
	Zdelwhite();
	toendline();
	tmp = bgetcol(TRUE, 0);
	if (tmp <= VAR(VFILLWIDTH)) {
		tobegline();
		tindent((VAR(VFILLWIDTH) - tmp) / 2);
		toendline();
	}
}


/* C MODE COMMANDS */

static void handle_close_bracket(struct mark *tmark, int crfound)
{
	int cnt;

	tobegline();
	if (VAR(VMATCH) & 0x100) {
		/* show the rest of the line in the PAW */
		struct mark t;
		int i, col;

		bmrktopnt(&t);
		for (i = col = 0;
		     !bisend() && !ISNL(*Curcptr) && col < Colmax;
		     ++i, bmove1()) {
			col += chwidth(*Curcptr, col, FALSE);
			PawStr[i] = *Curcptr;
		}
		PawStr[i] = '\0';
		putpaw("%s", PawStr);
		bpnttomrk(&t);
	}
	movepast(biswhite, FORWARD);
	cnt = bgetcol(TRUE, 0);
	bpnttomrk(tmark);
	if (crfound) {
		Zdelwhite();
		tindent(cnt);
	}
	binsert(Cmd);
}


/* This code must handle any char so that expansion will work */
void Zcinsert(void)
{
	int cnt, crfound = FALSE;
	struct mark tmark;

	if (STRIP(Cmd) == '}') {
		/* line it up with last unmatched '{' */
		Arg = 0;
		bmrktopnt(&tmark);	/* save current position */
		for (cnt = 0, bmove(-1); !bisstart(); bmove(-1))
			if (STRIP(*Curcptr) == '{') {
				if (cnt)
					--cnt;
				else {  /* matched */
					handle_close_bracket(&tmark, crfound);
					return;
				}
			} else if (STRIP(*Curcptr) == '}')
				++cnt;
			else if (ISNL(*Curcptr))
				crfound = TRUE;
		bpnttomrk(&tmark);	/* no match - go back */
		tbell();				/* and warn user */
	} else if (STRIP(Cmd) == '#') {
		struct mark *tmark = bcremrk();

		do
			bmove(-1);
		while (!bisstart() && biswhite());

		if (ISNL(*Curcptr) && !bisstart()) {
			bmove1();		/* skip NL */
			bdeltomrk(tmark);
		} else
			bpnttomrk(tmark);
		unmark(tmark);
	} else if (STRIP(Cmd) == ':') {
		/* for c++ look for public/protected */
		char word[16];

		getbword(word, 15, bisword);
		if (strcmp(word, "public")    == 0 ||
		   strcmp(word, "private")   == 0 ||
		   strcmp(word, "protected") == 0) {
			struct mark *tmark = bcremrk();
			tobegline();
			while (bisspace())
				bdelete(1);
			bpnttomrk(tmark);
			unmark(tmark);
		}
	}

	binsert(Cmd);

#if COMMENTBOLD
	if (STRIP(Cmd) == '/') {
		/* SAM What about overwrite mode? */
		Boolean comment;

		bmove(-2);
		comment = *Curcptr == '*';
		bmove(2);
		if (comment)
			addcomment();
	}
#if WANT_CPPS
	else if (STRIP(Cmd) == '#') {
		if (bmove(-2) == 0) {
			/* # is first character in buffer */
			bmove1();
			addcpp();
		} else {
			Boolean cpp = ISNL(*Curcptr);
			bmove(2);
			if (cpp)
				addcpp();
		}
	}
#endif
#endif
}


/* 'C' indent. Called for NL. */
void Zcindent(void)
{
	int width;
	struct mark tmark;

	if ((Curbuff->bmode & OVERWRITE))
		bcsearch(NL);
	else {
		int sawstart = 0;

		bmrktopnt(&tmark);
		do {
			if (STRIP(*Curcptr) == '#')
				bmove(-1);
			tobegline();
		} while (STRIP(*Curcptr) == '#' && !bisstart());
		movepast(biswhite, FORWARD);
		if (bisaftermrk(&tmark))
			bpnttomrk(&tmark);
		for (width = bgetcol(TRUE, 0); bisbeforemrk(&tmark); bmove1())
			if (STRIP(*Curcptr) == '{') {
				width += Tabsize;
				++sawstart;
			} else if (STRIP(*Curcptr) == '}' && sawstart) {
				width -= Tabsize;
				--sawstart;
			}
		bpnttomrk(&tmark);
		binsert(NL);
		tindent(width);
	}
}


/* This is called from Zfillpara if we are in C mode.
 * It is used to reformat comments.
 *
 * It is fairly deadly, so I break all rules and ask "Are you sure?"
 *
 * We assume comments are as follows:
 *
 *		[<ws>] <start_comment> [*]
 *		[<ws>] [*] [<text>]
 *			...
 *		[<ws>] [*] <end_comment> [<text>]
 */
static void Zfillcomment(void)
{
	struct mark *start, *end, *tmp, tmark;
	int col;

	if (ask("Are you sure? ") != YES)
		return;

	if (Curbuff->bmodf)
		Zfilesave();

	bmrktopnt(&tmark);

	/* find start of comment */
	if (!bstrsearch("/*", FALSE)) {
		bpnttomrk(&tmark);
		error("Unable to find start of comment.");
		return;
	}
	col = bgetcol(TRUE, 0) + 1;
	bcsearch(NL);
	start = bcremrk();

	/* find end of comment */
	if (!bstrsearch("*/", TRUE)) {
		unmark(start);
		bpnttomrk(&tmark);
		error("Unable to find end of comment.");
		return;
	}
	Zdelwhite();
	tindent(col);
	bcrsearch(NL);
	end = bcremrk();

	/* Remove leading ws and '*'s */
	for (bpnttomrk(start); bisbeforemrk(end); bcsearch(NL)) {
		Zdelwhite();
		if (*Curcptr == '*') {
			bdelete(1);
			Zdelwhite();
		}
	}

	/* do it! */
	bpnttomrk(start);
	do {	/* mark the end of the paragraph and move the point to
		 * the start */
		Zfpara();
		movepast(bisspace, BACKWARD);
		tmp = bcremrk();
		if (mrkaftermrk(tmp, end))
			mrktomrk(tmp, end);
		Zbpara();
		if (bisbeforemrk(start))
			bpnttomrk(start);
		tindent(col); binstr("* ");

		/* main loop */
		while (bisbeforemrk(tmp)) {
			moveto(bisspace, FORWARD);
			if (bgetcol(TRUE, 0) > VAR(VFILLWIDTH)) {
				moveto(bisspace, BACKWARD);
				Zdelwhite();
				binsert(NL);
				tindent(col); binstr("* ");
			}
			movepast(biswhite, FORWARD);
			if (*Curcptr == NL && bisbeforemrk(tmp)) {
				/* convert NL to space */
				bdelete(1);
				Zdelwhite();
				binsert(' ');
			}
		}
		unmark(tmp);

		movepast(bisspace, FORWARD); /* setup for next iteration */
	} while (bisbeforemrk(end));

	/* Fill in empty lines. */
	for (bpnttomrk(start); bisbeforemrk(end); bcsearch(NL))
		if (*Curcptr == NL) {
			tindent(col);
			binsert('*');
		}
	if (*Curcptr == NL) {
		tindent(col);
		binsert('*');
	}
}

/* FILL MODE COMMANDS */

void Zfillchk(void)
{
	Boolean tmp;
	struct mark *tmark;

	if (Cmd == CR)
		Cmd = NL;
	if (bgetcol(TRUE, 0) < VAR(VFILLWIDTH) || InPaw)
		CMD(Cmd == NL ? ZNEWLINE : ZINSERT);
	else {
		tmark = bcremrk();
		while (bgetcol(TRUE, 0) > VAR(VFILLWIDTH)) {
			moveto(bisspace, BACKWARD);
			movepast(bisspace, BACKWARD);
		}
		Zdelwhite();
		tmp = !bisatmrk(tmark);
		binsert(NL);
		tindent(VAR(VMARGIN));
		if (tmp) {
			bpnttomrk(tmark);
			binsert(Cmd);
		}
		unmark(tmark);
	}
}

void Zdelwhite(void)
{
	while (!bisend() && biswhite())
		bdelete(1);
	while (!bisstart()) {
		bmove(-1);
		if (biswhite())
			bdelete(1);
		else {
			bmove1();
			break;
		}
	}
}

/* Not reentrant - must iterate for arg */
void Zfillpara(void)
{
	Boolean all;
	struct mark *tmark, *tmp;

	if (Curbuff->bmode & CMODE) {
		Zfillcomment();
		return;
	}
	if (Curbuff->bmode & PROGMODE) {
		echo("Not in program mode");
		tbell();
		return;
	}
	tmark = bcremrk();		/* save the current point */
	all = Arg == 0;
	if (all == TRUE)
		btostart();
	echo("Reformatting...");
	do {
		/* mark the end of the paragraph and move the point to
		 * the start */
		Zfpara();
		movepast(bisspace, BACKWARD);
		tmp = bcremrk();
		Zbpara();
		if (*Curcptr == '.')
			bcsearch('\n');	/* for nroff */

		/* main loop */
		while (bisbeforemrk(tmp)) {
			moveto(bisspace, FORWARD);
			if (bgetcol(TRUE, 0) > VAR(VFILLWIDTH)) {
				moveto(bisspace, BACKWARD);
				Zdelwhite();
				binsert(NL);
				if (VAR(VMARGIN))
					tindent(VAR(VMARGIN));
				moveto(bisspace, FORWARD);
			}
			movepast(biswhite, FORWARD);
			if (*Curcptr == NL && bisbeforemrk(tmp)) {
				bdelete(1);
				Zdelwhite();
				binsert(' ');
			}
		}

		unmark(tmp);
		movepast(bisspace, FORWARD); /* setup for next iteration */
	} while ((all || --Arg > 0) && !bisend() && !tkbrdy());

	clrecho();
	if (Arg > 0 || (all && !bisend())) {
		echo("Aborted");
		tgetcmd();
	}

	bpnttomrk(tmark); /* restore point */
	unmark(tmark);
}

void Zfpara(void)
{
	char pc = '\0';

	/* Only go back if between paras */
	if (bisspace())
		movepast(bisspace, BACKWARD);
	while (!bisend() && !Ispara(pc, *Curcptr)) {
		pc = *Curcptr;
		bmove1();
	}
	movepast(bisspace, FORWARD);
}

void Zbpara(void)
{
	char pc = '\0';

	movepast(bisspace, BACKWARD);
	while (!bisstart() && !Ispara(*Curcptr, pc)) {
		pc = *Curcptr;
		bmove(-1);
	}
	movepast(bisspace, FORWARD);
}

Boolean Ispara(char pc, char ch)
{
	/* We consider a FF, VT, or two NLs in a row to mark a paragraph.
	 * A '.' at the start of a line also marks a paragraph (for nroff)
	 */
	return ch == '\f' || ch == '\13' ||
		(pc == NL && (ch == NL || ch == '.'));
}


/* MISC COMMANDS */

void Zprintpos(void)
{
	unsigned long mark, point;
	unsigned line;

	bswappnt(Curbuff->mark);
	mark = blocation(&line);
	bswappnt(Curbuff->mark);
	point = blocation(&line);
	putpaw("Line: %u  Column: %u  Point: %lu  Mark: %lu  Length: %lu",
	       line, bgetcol(FALSE, 0) + 1, point, mark, blength(Curbuff));
}


/* Key has no binding */
void Znotimpl(void)
{
	tbell();
}


void Zsetmrk(void)
{
	bmrktopnt(Curbuff->mark);
	echo("Mark Set.");
}

static void cleanup(void)
{	/* Mainly for valgrind */
	vfini();
	wfini();
	bfini();
	ufini(); /* must be after bfini */

	free(Cwd);
}

/* Exit the editor.
 * Warn about makes in progress.
 * If a buffer is modified, ask to write it out.
 * Dosen't save the system buffers.
 */
void Zexit(void)
{
	struct buff *tbuff;
	Boolean modf = FALSE;
#ifdef PIPESH
	struct buff *make = cfindbuff(MAKEBUFF);

	if (make && make->child != EOF)
		if (ask("You have a make running. Kill it?") != YES)
			return;
#endif

	if (!saveall(Argp))
		return;

	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (tbuff->bmodf && !(tbuff->bmode & SYSBUFF))
			modf = TRUE;
	if (modf && ask("Modified buffers. quit anyway? ") != YES)
		return;

#ifdef PIPESH
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (tbuff->child != EOF)
			unvoke(tbuff, FALSE);
	checkpipes(0);		/* make sure waited for ALL children */
#endif

	tfini();
	cleanup();

	exit(0);
}

/* Prompt to save buffer if the buffer has been modified.
 * Always saves if 'must' is TRUE or saveOnExit is set.
 * Returns FALSE if the user ABORTS the prompt.
 */
static Boolean promptsave(struct buff *tbuff, Boolean must)
{
	static int save_all;
	char str[BUFNAMMAX + 20];
	int ok = YES;

	if (tbuff->bmodf) {
		if (!must && !save_all) {
			sprintf(str, "save buffer %s? ", tbuff->bname);
			ok = ask2(str, TRUE);
			if (ok == BANG)
				save_all = 1;
			else if (ok == ABORT)
				return FALSE;
		}

		if (ok == YES || save_all) {
			bswitchto(tbuff);
			if (filesave() != TRUE)
				return FALSE;
		}
	}
	return TRUE;
}

/* Prompt to save ALL modified non-system buffers.
 *
 * If the user aborts a prompt, he is left in the buffer he aborted in
 * and the routine returns false.
 * Else the user returns to the buffer he started in.
*/
Boolean saveall(Boolean must)
{
	struct buff *tbuff, *bsave;

	bsave = Curbuff;
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (!(tbuff->bmode & SYSBUFF) && !promptsave(tbuff, must)) {
			Curwdo->modeflags = INVALID;
			return FALSE;
		}
	bswitchto(bsave);
	return TRUE;
}

static void mshow(unsigned ch)
{
	Byte match;
	int cnt = 0;
	struct mark save;

	if (!(Curbuff->bmode & PROGMODE) || InPaw || tkbrdy())
		return;
	if (VAR(VMATCH) & 1) {
		switch (ch) {
		case ')':
			match = '('; break;
		case ']':
			match = '['; break;
		case '}':
			match = '{'; break;
		default:
			return;
		}
		bmrktopnt(&save);
		do {
			bmove(-1);
			if (*Curcptr == match)
				--cnt;
			else if (*Curcptr == ch)
				++cnt;
		} while (cnt && !bisstart());
		if (cnt)
			tbell();
		else {
			zrefresh();
			showcursor(TRUE);	/* show the match! */
			delay();
			showcursor(FALSE);
		}
		bpnttomrk(&save);
	} else if (VAR(VMATCH) & 2) {
		switch (ch) {
		case '(':
			match = ')'; break;
		case '[':
			match = ']'; break;
		case '{':
			match = '}'; break;
		default:
			return;
		}
		binsert(match);
		bmove(-1);
	}
}

/* Self inserting commands */
void Zinsert(void)
{
	if (Curbuff->bmode & OVERWRITE) {
		if (!bisend() && *Curcptr != NL)
			bdelete(1);
	}

	binsert(Cmd);
	mshow(Cmd);
}

/*
 * Handle the CR or NL keys.
 * In overwrite mode, a NL goes to start of next line.
 * In insert mode, its just inserted.
 * This also causes the current line of text to be sent to the down the pipe.
 */
void Znewline(void)
{
	if (Curbuff->bmode & OVERWRITE)
		bcsearch(NL);
	else
		binsert(NL);
#ifdef PIPESH
	if (Curbuff->out_pipe)
		sendtopipe();
#endif
}

void Zoverin(void)
{
	Curbuff->bmode ^= OVERWRITE;
	if (!InPaw)
		Curwdo->modeflags = INVALID;
	Arg = 0;
}

void Zcase(void)
{
	Curbuff->bmode ^= EXACT;
	if (InPaw && Insearch) {
		tsetpoint(tmaxrow() - 1, 0);
		tprntstr(nocase(NULL));
	} else
		echo(Curbuff->bmode & EXACT ? "Exact Set" : "Exact Reset");
	Arg = 0;
}

void Zarg(void)
{
	char str[STRMAX], *p;

	Argp = TRUE;
	Arg = 0;
	strcpy(str, "Arg: 0");
	p = str + 5;	/* point to first digit */
	putpaw(str);
	while ((Cmd = tgetcmd()) >= '0' && Cmd <= '9') {
		Arg = Arg * 10 + Cmd - '0';
		*p++ = Cmd;
		*p = '\0';
		putpaw(str);
	}
	clrecho();
	CMD(Keys[Cmd]);
}


/* Process Meta (ESC) commands. */
/* Note: The delayprompt for Zmeta doesn't work. The tgetkb in Tgetcmd
 * reads the ESC, but matches it in the term entries. It then waits
 * for a second key. This means we do not get to the Zmeta without a
 * key waiting and therefore the delayprompt exits immediately. It is
 * left in for when we add a timeout to the read in tgetkb. */
void Zmeta(void)
{
	Boolean tmp;

	tmp = delayprompt("Meta: ");
	Cmd = tgetkb() | 128;
	if (tmp)
		clrecho();
	CMD(Cmd < SPECIAL_START ? Keys[Cmd] : ZNOTIMPL);
}

/* Process ^X commands. */
void Zctrlx(void)
{
	Boolean tmp;

	tmp = delayprompt("C-X: ");
	Cmd = tgetcmd() | 256;
	if (tmp)
		clrecho();
	CMD(Cmd < SPECIAL_START ? Keys[Cmd] : ZNOTIMPL);
}

/* Process the M-X command */
void Zmetax(void)
{
	int rc = getplete("M-X: ", NULL, (char **)Cnames, CNAMESIZE, NUMFUNCS);
	if (rc != -1) {
		Cmd = Cnames[rc].fnum;
		Lfunc = ZMETAX;
		for (Arg = Arg == 0 ? 1 : Arg; Arg > 0; --Arg)
			CMD(Cnames[rc].fnum);
	}
}

void Zabort(void)
{
	tbell();
	Arg = 0;
	if (InPaw)
		InPaw = ABORT;
}

void Zquote(void)
{
	Boolean tmp;
	char n[3];

	tmp = delayprompt("Quote: ");
	Cmd = tgetkb();
	if (isxdigit(Cmd)) {
		n[0] = Cmd;
		n[1] = tgetkb();
		n[2] = '\0';
		Cmd = (int) strtol(n, NULL, 16);
	}
	while (Arg-- > 0)
		if (InPaw)
			pinsert();
		else
			binsert(Cmd);
	if (tmp)
		clrecho();
	Arg = 0;
}

void Zhexout(void)
{
	char str[STRMAX], *p;

	if (Arg > 25)
		Arg = 25;
	strcpy(str, "Hex: ");
	for (p = str; Arg-- > 0 && !bisend(); bmove1()) {
		p = strchr(p, '\0');
		sprintf(p, " %02x", *Curcptr & 0xff);
	}
	putpaw("%s", str);
}

void Zswapchar(void)
{
	int tmp;

	if (bisend() || *Curcptr == NL)
		bmove(-1);
	if (!bisstart())
		bmove(-1);
	tmp = *Curcptr;
	bdelete(1);
	bmove1();
	binsert(tmp);
}

void Ztab(void)
{
	int tcol;

	if (VAR(VSPACETAB)) {
		tcol = Tabsize - (bgetcol(FALSE, 0) % Tabsize);
		sindent(tcol);
	} else
		binsert('\t');
}

void tobegline(void)
{
	if (bcrsearch(NL))
		bmove1();
}

void toendline(void)
{
	if (bcsearch(NL))
		bmove(-1);
}
