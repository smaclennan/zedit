/* commands.c - Zedit commands
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

int Arg;
bool Argp;

/* Word COMMANDS */

/* Find the start of a word. */
static bool Findstart(void)
{
	while (!bisstart(Bbuff) && bistoken())
		bmove(Bbuff, -1);
	while (!bisend(Bbuff) && !isalpha(*Curcptr))
		bmove1(Bbuff);
	return !bisend(Bbuff);
}

static void bconvert(int (*to)(int c))
{
	*Curcptr = to(*Curcptr);
	Bbuff->bmodf = true;
}

void Zcapitalize_word(void)
{
	if (Findstart()) {
		bconvert(toupper);
		for (bmove1(Bbuff); !bisend(Bbuff) && bistoken(); bmove1(Bbuff))
			bconvert(tolower);
		vsetmod();
	}
}


void Zlowercase_word(void)
{
	if (Findstart()) {
		for (; !bisend(Bbuff) && bistoken(); bmove1(Bbuff))
			bconvert(tolower);
		vsetmod();
	}
}

void Zuppercase_word(void)
{
	if (Findstart()) {
		for (; !bisend(Bbuff) && bistoken(); bmove1(Bbuff))
			bconvert(toupper);
		vsetmod();
	}
}

/* Move a block of chars around point and "from" to "to".
 * Assumes point is before "from".
*/
static void blockmove(struct mark *from, struct mark *to)
{
	char tmp;

	while (bisbeforemrk(Bbuff, from)) {
		tmp = *Curcptr;
		bswappnt(Bbuff, to);
		binsert(Bbuff, tmp);
		bswappnt(Bbuff, to);
		bdelete(Bbuff, 1);
	}
}

void Zswap_words(void)
{
	struct mark *tmark, *tmp;

	moveto(bistoken, FORWARD);
	if (bisend(Bbuff))
		return;
	tmark = zcreatemrk();
	movepast(bistoken, FORWARD);
	tmp = zcreatemrk();
	bpnttomrk(Bbuff, tmark);
	moveto(bistoken, BACKWARD);
	blockmove(tmark, tmp);
	movepast(bistoken, BACKWARD);
	blockmove(tmark, tmp);
	bpnttomrk(Bbuff, tmp);
	unmark(tmark);
	unmark(tmp);
}

void Zcenter(void)
{
	int tmp;

	tobegline(Bbuff);
	Ztrim_white_space();
	toendline(Bbuff);
	tmp = bgetcol(true, 0);
	if (tmp <= VAR(VFILLWIDTH)) {
		tobegline(Bbuff);
		tindent((VAR(VFILLWIDTH) - tmp) / 2);
		toendline(Bbuff);
	}
}

/* C MODE COMMANDS */

void Zc_insert(void)
{	/* This code must handle any char so that expansion will work */
	int cnt, crfound = false;
	struct mark *tmark;
	char word[16];

	switch (Cmd) {
	case '}':
		/* line it up with last unmatched '{' */
		Arg = 0;
		tmark = zcreatemrk();
		bmrktopnt(Bbuff, tmark);	/* save current position */
		for (cnt = 0, bmove(Bbuff, -1); !bisstart(Bbuff); bmove(Bbuff, -1))
			if (*Curcptr == '{') {
				if (cnt)
					--cnt;
				else {  /* matched */
					tobegline(Bbuff);
					movepast(biswhite, FORWARD);
					cnt = bgetcol(true, 0);
					bpnttomrk(Bbuff, tmark);
					if (crfound) {
						Ztrim_white_space();
						tindent(cnt);
					}
					binsert(Bbuff, Cmd);
					unmark(tmark);
					return;
				}
			} else if (*Curcptr == '}')
				++cnt;
			else if (ISNL(*Curcptr))
				crfound = true;
		bpnttomrk(Bbuff, tmark);	/* no match - go back */
		unmark(tmark);
		tbell();		/* and warn user */
		break;

	case '#':
		tmark = zcreatemrk();

		do
			bmove(Bbuff, -1);
		while (!bisstart(Bbuff) && biswhite());

		if (ISNL(*Curcptr) && !bisstart(Bbuff)) {
			bmove1(Bbuff);		/* skip NL */
			bdeltomrk(tmark);
		} else
			bpnttomrk(Bbuff, tmark);
		unmark(tmark);
		break;

	case ':':
		/* for c++ look for public/protected */
		getbword(word, 15, bisword);
		if (strcmp(word, "public")    == 0 ||
		   strcmp(word, "private")   == 0 ||
		   strcmp(word, "protected") == 0) {
			struct mark *tmark = zcreatemrk();
			tobegline(Bbuff);
			while (bisspace())
				bdelete(Bbuff, 1);
			bpnttomrk(Bbuff, tmark);
			unmark(tmark);
		}
		break;

	case '/':
		if (bpeek(Bbuff) == '*')
			uncomment(Curbuff);
		break;
	}

	binsert(Bbuff, Cmd);
}

void Zc_indent(void)
{
	int width = 0;
	struct mark tmark;

	if ((Curbuff->bmode & OVERWRITE))
		bcsearch(Bbuff, NL);
	else {
		int sawstart = 0, did_indent = 0;

		bmrktopnt(Bbuff, &tmark);
		do {
			if (*Curcptr == '#')
				bmove(Bbuff, -1);
			tobegline(Bbuff);
		} while (*Curcptr == '#' && !bisstart(Bbuff));
		movepast(biswhite, FORWARD);
		if (looking_at("if") || looking_at("while")) {
			width += Tabsize;
			did_indent = 1;
		}
		if (bisaftermrk(Bbuff, &tmark))
			bpnttomrk(Bbuff, &tmark);
		for (width += bgetcol(true, 0); bisbeforemrk(Bbuff, &tmark); bmove1(Bbuff))
			if (*Curcptr == '{') {
				if (did_indent == 0 || sawstart > 0)
					width += Tabsize;
				++sawstart;
			} else if (*Curcptr == '}' && sawstart) {
				width -= Tabsize;
				--sawstart;
			}
		bpnttomrk(Bbuff, &tmark);
		binsert(Bbuff, NL);
		Lfunc = ZINSERT; /* for undo */
		tindent(width);
	}
}

/* SHELL MODE COMMANDS */

/* The builtin reg cannot handle extended expressions. But if you are
 * on windows, why are you editing shell scripts?
 */
#if !defined(BUILTIN_REG) && !defined(WIN32)
static bool find_line(char *str)
{
	struct mark end, save;

	bmrktopnt(Bbuff, &save);
	bmrktopnt(Bbuff, &end);
	toendline(Bbuff);
	bswappnt(Bbuff, &end);

	while (bisbeforemrk(Bbuff, &end)) {
		if (lookingat(Bbuff, str)) {
			bpnttomrk(Bbuff, &save);
			return true;
		}
		Znext_word();
	}

	bpnttomrk(Bbuff, &save);
	return false;
}
#endif

void Zsh_indent(void)
{
	int width, fixup = 0;
	struct mark *tmark;

	tobegline(Bbuff);
	movepast(biswhite, FORWARD);
	tmark = bcremrk(Bbuff);
	width = bgetcol(true, 0);

#if !defined(BUILTIN_REG) && !defined(WIN32)
	if (lookingat(Bbuff, "\\<if\\>")) {
		if (find_line("\\<fi\\>") == 0)
			width += Tabsize;
	} else if (lookingat(Bbuff, "\\<while\\>")) {
		if (find_line("\\<done\\>") == 0)
			width += Tabsize;
	} else if (lookingat(Bbuff, "\\<fi\\>")) {
		width -= Tabsize;
		fixup = 1;
	} else if (lookingat(Bbuff, "\\<done\\>")) {
		width -= Tabsize;
		fixup = 1;
	} else if (*Curcptr == '}') {
		width -= Tabsize;
		fixup = 1;
	} else if (lookingat(Bbuff, ".*\\{[ \t]*$")) {
		/* Won't work if there is a comment */
		width += Tabsize;
	}
#endif

	if (fixup && tmark) {
		tobegline(Bbuff);
		bdeltomrk(tmark);
		tindent(width);
	}

	toendline(Bbuff);
	binsert(Bbuff, NL);
	Lfunc = ZINSERT; /* for undo */
	tindent(width);
	bdelmark(tmark);
}

/* FILL MODE COMMANDS */

void Zfill_check(void)
{
	bool tmp;
	struct mark *tmark;

	if (Cmd == CR)
		Cmd = NL;
	if (bgetcol(true, 0) < VAR(VFILLWIDTH) || InPaw)
		CMD(Cmd == NL ? ZNEWLINE : ZINSERT);
	else {
		tmark = zcreatemrk();
		while (bgetcol(true, 0) > VAR(VFILLWIDTH)) {
			moveto(bisspace, BACKWARD);
			movepast(bisspace, BACKWARD);
		}
		Ztrim_white_space();
		tmp = !bisatmrk(Bbuff, tmark);
		binsert(Bbuff, NL);
		if (tmp) {
			bpnttomrk(Bbuff, tmark);
			binsert(Bbuff, Cmd);
		}
		unmark(tmark);
	}
}

void Ztrim_white_space(void)
{
	while (!bisend(Bbuff) && biswhite())
		bdelete(Bbuff, 1);
	while (!bisstart(Bbuff)) {
		bmove(Bbuff, -1);
		if (biswhite())
			bdelete(Bbuff, 1);
		else {
			bmove1(Bbuff);
			break;
		}
	}
}

void Zfill_paragraph(void)
{	/* Not reentrant - must iterate for arg */
	struct mark *tmark, *tmp;

	if (Curbuff->bmode & PROGMODE) {
		putpaw("Not in program mode");
		tbell();
		return;
	}
	tmark = zcreatemrk();		/* save the current point */
	putpaw("Reformatting...");
	do {
		/* mark the end of the paragraph and move the point to
		 * the start */
		Znext_paragraph();
		movepast(bisspace, BACKWARD);
		tmp = zcreatemrk();
		Zprevious_paragraph();
		if (*Curcptr == '.')
			bcsearch(Bbuff, '\n');	/* for nroff */

		/* main loop */
		while (bisbeforemrk(Bbuff, tmp)) {
			moveto(bisspace, FORWARD);
			if (bgetcol(true, 0) > VAR(VFILLWIDTH)) {
				moveto(bisspace, BACKWARD);
				Ztrim_white_space();
				binsert(Bbuff, NL);
				moveto(bisspace, FORWARD);
			}
			movepast(biswhite, FORWARD);
			if (*Curcptr == NL && bisbeforemrk(Bbuff, tmp)) {
				bdelete(Bbuff, 1);
				Ztrim_white_space();
				binsert(Bbuff, ' ');
			}
		}

		unmark(tmp);
		movepast(bisspace, FORWARD); /* setup for next iteration */
	} while (Argp && !bisend(Bbuff) && !tkbrdy());

	clrpaw();
	if (Argp && !bisend(Bbuff)) {
		putpaw("Aborted");
		tgetcmd();
	}

	bpnttomrk(Bbuff, tmark); /* restore point */
	unmark(tmark);
}

static bool ispara(char pc, char ch)
{
	/* We consider a FF, VT, or two NLs in a row to mark a paragraph.
	 * A '.' at the start of a line also marks a paragraph (for nroff)
	 */
	return ch == '\f' || ch == '\13' ||
		(pc == NL && (ch == NL || ch == '.'));
}

void Znext_paragraph(void)
{
	char pc = '\0';

	/* Only go back if between paras */
	if (bisspace())
		movepast(bisspace, BACKWARD);
	while (!bisend(Bbuff) && !ispara(pc, *Curcptr)) {
		pc = *Curcptr;
		bmove1(Bbuff);
	}
	movepast(bisspace, FORWARD);
}

void Zprevious_paragraph(void)
{
	char pc = '\0';

	movepast(bisspace, BACKWARD);
	while (!bisstart(Bbuff) && !ispara(*Curcptr, pc)) {
		pc = *Curcptr;
		bmove(Bbuff, -1);
	}
	movepast(bisspace, FORWARD);
}

/* MISC COMMANDS */

/* Return the current line of the point. */
unsigned long bline(struct buff *buff)
{
	struct mark tmark;
	unsigned long line = 1;

	bmrktopnt(buff, &tmark);
	btostart(buff);
	while (bcsearch(buff, '\n') && !bisaftermrk(buff, &tmark))
		++line;
	bpnttomrk(buff, &tmark);
	return line;
}

void Zposition(void)
{
	unsigned long mark, point = blocation(Bbuff);

	if (UMARK_SET) {
		bswappnt(Bbuff, UMARK);
		mark = blocation(Bbuff);
		bswappnt(Bbuff, UMARK);
		putpaw("Line: %u  Column: %u  Point: %lu  Mark: %lu  Length: %lu",
			   bline(Bbuff), bgetcol(false, 0) + 1, point, mark, blength(Bbuff));
	} else
		putpaw("Line: %u  Column: %u  Point: %lu  Mark: unset  Length: %lu",
			   bline(Bbuff), bgetcol(false, 0) + 1, point, blength(Bbuff));
}

void Znotimpl(void) { tbell(); }

void Zset_mark(void)
{
	set_umark(NULL); /* set to point */
	putpaw("Mark Set.");
}

void Zexit(void)
{
	struct zbuff *tbuff;
	bool modf = false;

	if (!saveall(Argp))
		return;

	foreachbuff(tbuff)
		if (tbuff->buff->bmodf && !(tbuff->bmode & SYSBUFF))
			modf = true;
	if (modf && ask("Modified buffers. quit anyway? ") != YES)
		return;

	unvoke(NULL);
	checkpipes(0);		/* make sure waited for ALL children */

	exit(0);
}

void Zsave_and_exit(void)
{
	if ((Curbuff->bmode & SYSBUFF) == 0)
		Zsave_file();
	Zexit();
}

/* Prompt to save buffer if the buffer has been modified.
 * Always saves if 'must' is true or saveOnExit is set.
 * Returns false if the user ABORTS the prompt.
 */
bool promptsave(struct zbuff *tbuff, bool must)
{
	static int save_all;
	char str[BUFNAMMAX + 20];
	int ok = YES;

	if (tbuff->buff->bmodf) {
		if (!must && !save_all) {
			sprintf(str, "save buffer %s? ", tbuff->bname);
			ok = ask2(str, true);
			if (ok == BANG)
				save_all = 1;
			else if (ok == ABORT)
				return false;
		}

		if (ok == YES || save_all) {
			zswitchto(tbuff);
			if (filesave() != true)
				return false;
		}
	}
	return true;
}

/* Prompt to save ALL modified non-system buffers.
 *
 * If the user aborts a prompt, he is left in the buffer he aborted in
 * and the routine returns false.
 * Else the user returns to the buffer he started in.
*/
bool saveall(bool must)
{
	struct zbuff *tbuff, *bsave;

	bsave = Curbuff;
	foreachbuff(tbuff)
		if (!(tbuff->bmode & SYSBUFF) && !promptsave(tbuff, must)) {
			Curwdo->modeflags = INVALID;
			return false;
		}
	zswitchto(bsave);
	return true;
}

static void mshow(unsigned ch)
{
	Byte match;
	int cnt = 0;
	struct mark save;

	if (!(Curbuff->bmode & PROGMODE) || InPaw || tkbrdy())
		return;

	switch (ch) {
	case ')':
		/* This is irritating in case statements */
		if (Curbuff->bmode & SHMODE) return;
		match = '('; break;
	case ']':
		match = '['; break;
	case '}':
		match = '{'; break;
	default:
		return;
	}
	bmrktopnt(Bbuff, &save);
	do {
		bmove(Bbuff, -1);
		if (*Curcptr == match)
			--cnt;
		else if (*Curcptr == ch)
			++cnt;
	} while (cnt && !bisstart(Bbuff));
	if (cnt)
		tbell();
	else { /* show the match! */
		zrefresh();
		tdelay(999);
	}
	bpnttomrk(Bbuff, &save);
}

void Zinsert(void)
{
	if (Curbuff->bmode & OVERWRITE) {
		if (!bisend(Bbuff) && *Curcptr != NL)
			bdelete(Bbuff, 1);
	}

	binsert(Bbuff, Cmd);
	mshow(Cmd);
}

void Znewline(void)
{
	if (Curbuff->bmode & OVERWRITE)
		bcsearch(Bbuff, NL);
	else
		binsert(Bbuff, NL);
}

void Ztab(void)
{
	if (VAR(VSPACETAB)) {
		int tcol = Tabsize - (bgetcol(false, 0) % Tabsize);
		while (tcol-- > 0)
			binsert(Bbuff, ' ');
	} else
		binsert(Bbuff, '\t');
}

void Zinsert_overwrite(void)
{
	Curbuff->bmode ^= OVERWRITE;
	if (!InPaw)
		Curwdo->modeflags = INVALID;
	Arg = 0;
	tsetcursor(false);
}

void Ztoggle_case(void)
{
	Curbuff->bmode ^= EXACT;
	if (InPaw && Insearch) {
		tsetpoint(Rowmax - 1, 0);
		tprntstr(nocase(NULL));
	} else
		putpaw(Curbuff->bmode & EXACT ? "Exact Set" : "Exact Reset");
	Arg = 0;
}

void Zarg(void)
{
	char str[STRMAX], *p;

	Argp = true;
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
	clrpaw();
	CMD(Keys[Cmd]);
}

static void do_prefix_cmd(const char *prompt, int mask)
{
	Cmd = delayprompt(prompt);
	if (Cmd < 128) {
		Cmd = toupper(Cmd) | mask;
		CMD(Keys[Cmd]);
	} else
		Zabort();
}

void Zmeta(void)
{
	do_prefix_cmd("Meta: ", M(0));
}

void Zctrl_x(void)
{
	do_prefix_cmd("C-X: ", CX(0));
}

void Zmeta_x(void)
{
	static char cmd[40];

	int rc = getplete("M-X: ", cmd, (char **)Cnames, CNAMESIZE, NUMFUNCS);
	if (rc != -1) {
		strcpy(cmd, Cnames[rc].name);
		Cmd = Cnames[rc].fnum;
		Lfunc = ZMETA_X;
		for (Arg = Arg == 0 ? 1 : Arg; Arg > 0; --Arg)
			CMD(Cnames[rc].fnum);
	}
}

void Zabort(void)
{
	Arg = 0;
	if (InPaw)
		InPaw = ABORT;
	CLEAR_UMARK;
}

void Zquote(void)
{
	Cmd = delayprompt("Quote: ");
	while (Arg-- > 0)
		if (InPaw)
			pinsert();
		else
			binsert(Bbuff, Cmd);
}

void Zswap_chars(void)
{
	int tmp;

	if (bisend(Bbuff) || *Curcptr == NL)
		bmove(Bbuff, -1);
	if (!bisstart(Bbuff))
		bmove(Bbuff, -1);
	tmp = *Curcptr;
	bdelete(Bbuff, 1);
	bmove1(Bbuff);
	binsert(Bbuff, tmp);
}

void Zcount(void)
{
	bool word, swapped = false;
	unsigned l, w, c;
	struct mark *tmark;

	Arg = 0;
	if (UMARK_SET) {
		swapped = bisaftermrk(Bbuff, UMARK);
		if (swapped)
			bswappnt(Bbuff, UMARK);
		tmark = zcreatemrk();
	} else {
		tmark = zcreatemrk();
		btostart(Bbuff);
	}
	l = w = c = 0;
	putpaw("Counting...");
	word = false;
	for (; UMARK_SET ? bisbeforemrk(Bbuff, UMARK) : !bisend(Bbuff); bmove1(Bbuff), ++c) {
		if (ISNL(*Curcptr))
			++l;
		if (!bistoken())
			word = false;
		else if (!word) {
			++w;
			word = true;
		}
	}
	putpaw("Lines: %u   Words: %u   Characters: %u", l, w, c);
	if (swapped)
		bswappnt(Bbuff, UMARK);
	else
		bpnttomrk(Bbuff, tmark);
	unmark(tmark);
}

/* this struct must be sorted */
static struct _amode
{
	const char *str;
	int mode;
} modes[] = {
	{ "C",		CMODE	},
	{ "Normal",	NORMAL	},
	{ "Shell",	SHMODE	},
	{ "Text",	TXTMODE	},
};
#define AMODESIZE	sizeof(struct _amode)
#define NUMMODES	((int)(sizeof(modes) / AMODESIZE))

void Zmode(void)
{
	int i, rc;

	/* find the current mode for default */
	for (i = 0; i < NUMMODES - 1; ++i)
		if (modes[i].mode & Curbuff->bmode)
			break;
	rc = getplete("Mode: ", modes[i].str, (char **)modes,
			  AMODESIZE, NUMMODES);
	if (rc != -1) {
		int tsave = Tabsize;
		toggle_mode(modes[rc].mode);
		Curwdo->modeflags = INVALID;
		if (settabsize(modes[rc].mode) != tsave)
			redisplay();
	}
}

static bool matchit(char *extstr, char *str)
{
	if (str) {
		char *p = strstr(extstr, str);
		if (p) {
			p += strlen(str);
			return *p == '.' || *p == '\0';
		}
	}
	return false;
}

/* Toggle to 'mode'. Passed 0 to set for readone */
void toggle_mode(int mode)
{
	char tmp[PATHMAX], *ext;
	strcpy(tmp, Curbuff->fname);

	ext = strrchr(tmp, '.');
#if ZLIB
	if (ext) {
		if (strcmp(ext, ".gz") == 0) {
			*ext = '\0';
			ext = strrchr(tmp, '.');
		}
	}
#endif

	if (mode == 0) {
		if (matchit(VARSTR(VCEXTS), ext))
			mode = CMODE;
		else if (matchit(VARSTR(VSEXTS), ext) ||
			 strncmp((char *)Curcptr, "#!/", 3) == 0)
			mode = SHMODE;
		else if (matchit(VARSTR(VTEXTS), ext))
			mode = TXTMODE;
		else if (!VAR(VNORMAL))
			mode = TXTMODE;
		else
			mode = NORMAL;
	}

	if (mode == SHMODE) {
		if (ext && strcmp(ext, ".el") == 0)
			Curbuff->comchar = ';';
		else
			Curbuff->comchar = '#';
	} else
		Curbuff->comchar = 0;

	Curbuff->bmode = (Curbuff->bmode & ~MAJORMODE) | mode;
}

void Zmark_paragraph(void)
{
	bmove1(Bbuff);	/* make sure we are not at the start of a paragraph */
	Zprevious_paragraph();
	set_umark(NULL);
	while (Arg-- > 0)
		Znext_paragraph();
	Arg = 0;
}

static void setregion(int (*convert)(int))
{
	bool swapped;
	struct mark tmark;

	if (Curbuff->bmode & PROGMODE) {
		putpaw("Not in program mode");
		tbell();
		return;
	}

	NEED_UMARK;

	swapped = bisaftermrk(Bbuff, UMARK);
	if (swapped)
		bswappnt(Bbuff, UMARK);
	bmrktopnt(Bbuff, &tmark);

	for (; bisbeforemrk(Bbuff, UMARK); bmove1(Bbuff))
		*Curcptr = (*convert)(*Curcptr);

	if (swapped)
		mrktomrk(UMARK, &tmark);
	else
		bpnttomrk(Bbuff, &tmark);
	Bbuff->bmodf = true;

	CLEAR_UMARK;
	redisplay();
}

void Zuppercase_region(void)
{
	setregion(toupper);
}

void Zlowercase_region(void)
{
	setregion(tolower);
}

static void indent(bool flag)
{
	struct mark *psave, *msave = NULL;
	int i;

	NEED_UMARK;

	psave = zcreatemrk();
	if (bisaftermrk(Bbuff, UMARK)) {
		bswappnt(Bbuff, UMARK);
		msave = zcreatemrk();
	}
	bcrsearch(Bbuff, NL);
	while (bisbeforemrk(Bbuff, UMARK)) {
		if (flag) {
			/* skip comment lines */
			if (*Curcptr != '#')
				for (i = 0; i < Arg; ++i)
					binsert(Bbuff, '\t');
		} else
			for (i = 0; i < Arg && *Curcptr == '\t'; ++i)
				bdelete(Bbuff, 1);
		bcsearch(Bbuff, NL);
	}
	bpnttomrk(Bbuff, psave);
	unmark(psave);
	if (msave) {
		mrktomrk(UMARK, msave);
		unmark(msave);
	}

	CLEAR_UMARK;
}

void Zindent(void)
{
	indent(true);
}

void Zundent(void)
{
	indent(false);
}

void Zsetenv(void)
{
	char env[STRMAX + 2], set[STRMAX + 1], *p;

	*env = '\0';
	if (getarg("Env: ", env, STRMAX))
		return;
	p = getenv(env);
	if (p) {
		if (strlen(p) >= STRMAX) {
			error("Variable is too long.");
			return;
		}
		strcpy(set, p);
	} else
		*set = '\0';

	strcat(env, "=");	/* turn it into prompt */
	if (getarg(env, set, STRMAX))
		return;

	/* putenv cannot be passed an automatic: alloc the space */
	p = (char *)calloc(1, strlen(env) + strlen(set));
	if (p) {
		strcpy(p, env);
		strcat(p, set);
		if (putenv(p))
			error("Unable to set environment variable.");
	} else
		error("Out of memory.");
}

void Zzap_to_char(void)
{
	Cmd = tgetcmd();
	if (Keys[Cmd] == ZABORT) {
		tbell();
		return;
	}

	set_umark(NULL); /* set to point */
	while (Arg-- > 0)
		if (!bcsearch(Bbuff, Cmd)) {
			bpnttomrk(Bbuff, UMARK);
			clear_umark();
			tbell();
			return;
		}

	redisplay();
}
