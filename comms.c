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
		Moveto(Istoken, FORWARD);
	else {
		while (!Bisstart() && Istoken())
			bmove(-1);
		while (!Bisend() && !isalpha(Buff()))
			bmove1();
	}
	return !Bisend();
}

void Zcapword(void)
{
	if (Findstart()) {
		Buff() = Toupper(Buff());
		Curbuff->bmodf = Curmodf = MODIFIED;
		for (bmove1(); !Bisend() && Istoken(); bmove1())
			Buff() = Tolower(Buff());
		Vsetmod(FALSE);
	}
}


void Zlowword(void)
{
	if (Findstart()) {
		for ( ; !Bisend() && Istoken(); bmove1()) {
			Curbuff->bmodf = Curmodf = MODIFIED;
			Buff() = Tolower(Buff());
		}
		Vsetmod(FALSE);
	}
}


void Zupword(void)
{
	if (Findstart()) {
		for ( ; !Bisend() && Istoken(); bmove1()) {
			Curbuff->bmodf = Curmodf = MODIFIED;
			Buff() = Toupper(Buff());
		}
		Vsetmod(FALSE);
	}
}


void Zswapword(void)
{
	struct mark *tmark, *tmp;

	Moveto(Istoken, FORWARD);
	if (Bisend())
		return;
	tmark = bcremrk();
	Movepast(Istoken, FORWARD);
	tmp = bcremrk();
	bpnttomrk(tmark);
	Moveto(Istoken, BACKWARD);
	Blockmove(tmark, tmp);
	Movepast(Istoken, BACKWARD);
	Blockmove(tmark, tmp);
	bpnttomrk(tmp);
	unmark(tmark);
	unmark(tmp);
}


void Zcenter(void)
{
	int tmp;

	Tobegline();
	Zdelwhite();
	Toendline();
	tmp = bgetcol(TRUE, 0);
	if (tmp <= VAR(VFILLWIDTH)) {
		Tobegline();
		Tindent((VAR(VFILLWIDTH) - tmp) / 2);
		Toendline();
	}
}


/* C MODE COMMANDS */

static void handle_close_bracket(struct mark *tmark, int crfound)
{
	int cnt;

	Tobegline();
	if (VAR(VMATCH) & 0x100) {
		/* show the rest of the line in the PAW */
		struct mark t;
		int i, col;

		bmrktopnt(&t);
		for (i = col = 0;
		     !Bisend() && !ISNL(Buff()) && col < Colmax;
		     ++i, bmove1()) {
			col += chwidth(Buff(), col, FALSE);
			PawStr[i] = Buff();
		}
		PawStr[i] = '\0';
		Echo(PawStr);
		bpnttomrk(&t);
	}
	Movepast(Iswhite, FORWARD);
	cnt = bgetcol(TRUE, 0);
	bpnttomrk(tmark);
	if (crfound) {
		Zdelwhite();
		Tindent(cnt);
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
		for (cnt = 0, bmove(-1); !Bisstart(); bmove(-1))
			if (STRIP(Buff()) == '{') {
				if (cnt)
					--cnt;
				else {  /* matched */
					handle_close_bracket(&tmark, crfound);
					return;
				}
			} else if (STRIP(Buff()) == '}')
				++cnt;
			else if (ISNL(Buff()))
				crfound = TRUE;
		bpnttomrk(&tmark);	/* no match - go back */
		tbell();				/* and warn user */
	} else if (STRIP(Cmd) == '#') {
		struct mark *tmark = bcremrk();

		do
			bmove(-1);
		while (!Bisstart() && Iswhite());

		if (ISNL(Buff()) && !Bisstart()) {
			bmove1();		/* skip NL */
			bdeltomrk(tmark);
		} else
			bpnttomrk(tmark);
		unmark(tmark);
	} else if (STRIP(Cmd) == ':') {
		/* for c++ look for public/protected */
		char word[16];

		Getbword(word, 15, Isword);
		if (strcmp(word, "public")    == 0 ||
		   strcmp(word, "private")   == 0 ||
		   strcmp(word, "protected") == 0) {
			struct mark *tmark = bcremrk();
			Tobegline();
			while (Isspace())
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
		comment = Buff() == '*';
		bmove(2);
		if (comment)
			AddComment();
	} else if (STRIP(Cmd) == '#') {
		if (bmove(-2) == 0) {
			/* # is first character in buffer */
			bmove1();
			AddCPP();
		} else {
			Boolean cpp = ISNL(Buff());
			bmove(2);
			if (cpp)
				AddCPP();
		}
	}
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
			if (STRIP(Buff()) == '#')
				bmove(-1);
			Tobegline();
		} while (STRIP(Buff()) == '#' && !Bisstart());
		Movepast(Iswhite, FORWARD);
		if (bisaftermrk(&tmark))
			bpnttomrk(&tmark);
		for (width = bgetcol(TRUE, 0); bisbeforemrk(&tmark); bmove1())
			if (STRIP(Buff()) == '{') {
				width += Tabsize;
				++sawstart;
			} else if (STRIP(Buff()) == '}' && sawstart) {
				width -= Tabsize;
				--sawstart;
			}
		bpnttomrk(&tmark);
		binsert(NL);
		Tindent(width);
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

	if (Ask("Are you sure? ") != YES)
		return;

	if (Curbuff->bmodf)
		Zfilesave();

	bmrktopnt(&tmark);

	/* find start of comment */
	if (!Bsearch("/*", FALSE)) {
		bpnttomrk(&tmark);
		Error("Unable to find start of comment.");
		return;
	}
	col = bgetcol(TRUE, 0) + 1;
	bcsearch(NL);
	start = bcremrk();

	/* find end of comment */
	if (!Bsearch("*/", TRUE)) {
		unmark(start);
		bpnttomrk(&tmark);
		Error("Unable to find end of comment.");
		return;
	}
	Zdelwhite();
	Tindent(col);
	bcrsearch(NL);
	end = bcremrk();

	/* Remove leading ws and '*'s */
	for (bpnttomrk(start); bisbeforemrk(end); bcsearch(NL)) {
		Zdelwhite();
		if (Buff() == '*') {
			bdelete(1);
			Zdelwhite();
		}
	}

	/* do it! */
	bpnttomrk(start);
	do {	/* mark the end of the paragraph and move the point to
		 * the start */
		Zfpara();
		Movepast(Isspace, BACKWARD);
		tmp = bcremrk();
		if (mrkaftermrk(tmp, end))
			Mrktomrk(tmp, end);
		Zbpara();
		if (bisbeforemrk(start))
			bpnttomrk(start);
		Tindent(col); binstr("* ");

		/* main loop */
		while (bisbeforemrk(tmp)) {
			Moveto(Isspace, FORWARD);
			if (bgetcol(TRUE, 0) > VAR(VFILLWIDTH)) {
				Moveto(Isspace, BACKWARD);
				Zdelwhite();
				binsert(NL);
				Tindent(col); binstr("* ");
			}
			Movepast(Iswhite, FORWARD);
			if (Buff() == NL && bisbeforemrk(tmp)) {
				/* convert NL to space */
				bdelete(1);
				Zdelwhite();
				binsert(' ');
			}
		}
		unmark(tmp);

		Movepast(Isspace, FORWARD); /* setup for next iteration */
	} while (bisbeforemrk(end));

	/* Fill in empty lines. */
	for (bpnttomrk(start); bisbeforemrk(end); bcsearch(NL))
		if (Buff() == NL) {
			Tindent(col);
			binsert('*');
		}
	if (Buff() == NL) {
		Tindent(col);
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
			Moveto(Isspace, BACKWARD);
			Movepast(Isspace, BACKWARD);
		}
		Zdelwhite();
		tmp = !Bisatmrk(tmark);
		binsert(NL);
		Tindent(VAR(VMARGIN));
		if (tmp) {
			bpnttomrk(tmark);
			binsert(Cmd);
		}
		unmark(tmark);
	}
}

void Zdelwhite(void)
{
	while (!Bisend() && Iswhite())
		bdelete(1);
	while (!Bisstart()) {
		bmove(-1);
		if (Iswhite())
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
		Echo("Not in program mode");
		tbell();
		return;
	}
	tmark = bcremrk();		/* save the current point */
	all = Arg == 0;
	if (all == TRUE)
		btostart();
	Echo("Reformatting...");
	do {
		/* mark the end of the paragraph and move the point to
		 * the start */
		Zfpara();
		Movepast(Isspace, BACKWARD);
		tmp = bcremrk();
		Zbpara();
		if (Buff() == '.')
			bcsearch('\n');	/* for nroff */

		/* main loop */
		while (bisbeforemrk(tmp)) {
			Moveto(Isspace, FORWARD);
			if (bgetcol(TRUE, 0) > VAR(VFILLWIDTH)) {
				Moveto(Isspace, BACKWARD);
				Zdelwhite();
				binsert(NL);
				if (VAR(VMARGIN))
					Tindent(VAR(VMARGIN));
				Moveto(Isspace, FORWARD);
			}
			Movepast(Iswhite, FORWARD);
			if (Buff() == NL && bisbeforemrk(tmp)) {
				bdelete(1);
				Zdelwhite();
				binsert(' ');
			}
		}

		unmark(tmp);
		Movepast(Isspace, FORWARD); /* setup for next iteration */
	} while ((all || --Arg > 0) && !Bisend() && !Tkbrdy());

	Clrecho();
	if (Arg > 0 || (all && !Bisend())) {
		Echo("Aborted");
		Tgetcmd();
	}

	bpnttomrk(tmark); /* restore point */
	unmark(tmark);
}

void Zfpara(void)
{
	char pc = '\0';

	/* Only go back if between paras */
	if (Isspace())
		Movepast(Isspace, BACKWARD);
	while (!Bisend() && !Ispara(pc, Buff())) {
		pc = Buff();
		bmove1();
	}
	Movepast(Isspace, FORWARD);
}

void Zbpara(void)
{
	char pc = '\0';

	Movepast(Isspace, BACKWARD);
	while (!Bisstart() && !Ispara(Buff(), pc)) {
		pc = Buff();
		bmove(-1);
	}
	Movepast(Isspace, FORWARD);
}

Boolean Ispara(char pc, char ch)
{
	/* We consider a FF, VT, or two NLs in a row to mark a paragraph.
	 * A '.' at the start of a line also marks a paragraph (for nroff)
	 */
	return ch == '\f' || ch == '\13' ||
		(pc == NL && (ch == NL || ch == '.'));
}


/* FORM MODE COMMANDS */

void Zformtab(void)
{
	if (Bsearch(FORMSTRING, TRUE))
		bdelete(strlen(FORMSTRING));
	else
		tbell();
}

/* MISC COMMANDS */

void Zprintpos(void)
{
	char str[STRMAX];
	unsigned long mark, point;
	unsigned line;

	bswappnt(Curbuff->mark);
	mark = blocation(&line);
	bswappnt(Curbuff->mark);
	point = blocation(&line);
	sprintf(str, "Line: %u  Column: %u  Point: %lu  Mark: %lu  Length: %lu",
		line, bgetcol(FALSE, 0) + 1, point, mark, blength(Curbuff));
	Echo(str);
}


/* Key has no binding */
void Znotimpl(void)
{
	tbell();
}


void Zsetmrk(void)
{
	bmrktopnt(Curbuff->mark);
	Echo("Mark Set.");
}

/* This does the real work of quiting. */
void Quit(void)
{
#ifdef PIPESH
	struct buff *tbuff;

	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (tbuff->child != EOF)
			Unvoke(tbuff, FALSE);
	Checkpipes(0);		/* make sure waited for ALL children */
#endif

	if (VAR(VDOSAVE))
		Save(Curbuff);
	Exitflag = TRUE;
	tfini();

#ifdef XWINDOWS
	closeSockets();
#endif

	exit(0);
}

void Zquit(void)
{
	struct buff *tbuff;
	Boolean modf = FALSE;

	if (!Argp) {
		for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
			if (tbuff->bmodf && !(tbuff->bmode & SYSBUFF))
				modf = TRUE;
		if (modf && Ask("Modified buffers. Quit anyway? ") != YES)
			return;
	}
	Quit();
}


/* Exit the editor.
 * Warn about makes in progress.
 * If a buffer is modified, ask to write it out.
 * Dosen't save the system buffers.
 */
void Zexit(void)
{
#ifdef PIPESH
	struct buff *make = Cfindbuff(MAKEBUFF);

	if (make && make->child != EOF)
		if (Ask("You have a make running. Kill it?") != YES)
			return;
#endif

	if (Saveall(Argp))
		Quit();
}

/* Prompt to save buffer if the buffer has been modified.
 * Always saves if 'must' is TRUE or SaveOnExit is set.
 * Returns FALSE if the user ABORTS the prompt.
 */
static Boolean Promptsave(struct buff *tbuff, Boolean must)
{
	char str[BUFNAMMAX + 20];
	int ok = YES;

	if (tbuff->bmodf) {
		if (!must && !VAR(VSAVE)) {
			sprintf(str, "Save buffer %s? ", tbuff->bname);
			ok = Ask(str);
			if (ok == ABORT)
				return FALSE;
		}

		if (ok == YES || VAR(VSAVE)) {
			bswitchto(tbuff);
			if (Filesave() != TRUE)
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
Boolean Saveall(Boolean must)
{
	struct buff *tbuff, *bsave;

	bsave = Curbuff;
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (!(tbuff->bmode & SYSBUFF) && !Promptsave(tbuff, must)) {
			Curwdo->modeflags = INVALID;
			return FALSE;
		}
	bswitchto(bsave);
	return TRUE;
}

/* Self inserting commands */
void Zinsert(void)
{
	if (Curbuff->bmode & OVERWRITE) {
		if (!Bisend() && Buff() != NL)
			bdelete(1);
	}

	binsert(Cmd);
	Mshow(Cmd);
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
		Sendtopipe();
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
		Tsetpoint(Tmaxrow() - 1, 0);
		tprntstr(Nocase(NULL));
	} else
		Echo(Curbuff->bmode & EXACT ? "Exact Set" : "Exact Reset");
	Arg = 0;
}

void Zarg(void)
{
	char str[STRMAX], *p;

	Argp = TRUE;
	Arg = 0;
	strcpy(str, "Arg: 0");
	p = str + 5;	/* point to first digit */
	PutPaw(str, 2);
	while ((Cmd = Tgetcmd()) >= '0' && Cmd <= '9') {
		Arg = Arg * 10 + Cmd - '0';
		*p++ = Cmd;
		*p = '\0';
		PutPaw(str, 2);
	}
	Clrecho();
	CMD(Keys[Cmd]);
}


/* voidess Meta (ESC) commands. */
void Zmeta(void)
{
	Boolean tmp;

	tmp = Delayprompt("Meta: ");
	Cmd = Tgetkb() | 128;
	if (tmp)
		Clrecho();
	CMD(Cmd < SPECIAL_START ? Keys[Cmd] : ZNOTIMPL);
}

/* voidess ^X commands. */
void Zctrlx(void)
{
	Boolean tmp;

	tmp = Delayprompt("C-X: ");
	Cmd = Tgetcmd() | 256;
	if (tmp)
		Clrecho();
	CMD(Cmd < SPECIAL_START ? Keys[Cmd] : ZNOTIMPL);
}

/* voidess the M-X command */
void Zmetax(void)
{
	int rc = Getplete("M-X: ", NULL, (char **)Cnames, CNAMESIZE, NUMFUNCS);
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

	tmp = Delayprompt("Quote: ");
	Cmd = Tgetkb();
	if (isxdigit(Cmd)) {
		n[0] = Cmd;
		n[1] = Tgetkb();
		n[2] = '\0';
		Cmd = (int) strtol(n, NULL, 16);
	}
	while (Arg-- > 0)
		if (InPaw)
			Pinsert();
		else
			binsert(Cmd);
	if (tmp)
		Clrecho();
	Arg = 0;
}

void Zhexout(void)
{
	char str[STRMAX], *p;

	if (Arg > 25)
		Arg = 25;
	strcpy(str, "Hex: ");
	for (p = str; Arg-- > 0 && !Bisend(); bmove1()) {
		p = strchr(p, '\0');
		sprintf(p, " %02x", Buff() & 0xff);
	}
	Echo(str);
}

void Zswapchar(void)
{
	int tmp;

	if (Bisend() || Buff() == NL)
		bmove(-1);
	if (!Bisstart())
		bmove(-1);
	tmp = Buff();
	bdelete(1);
	bmove1();
	binsert(tmp);
}

void Ztab(void)
{
	int tcol;

	if (VAR(VSPACETAB)) {
		tcol = Tabsize - (bgetcol(FALSE, 0) % Tabsize);
		Sindent(tcol);
	} else
		binsert('\t');
}

#ifndef XWINDOWS
void Zzoom(void)	{ tbell(); }
#endif
