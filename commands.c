/* commands.c - Zedit commands
 * Copyright (C) 1988-2016 Sean MacLennan
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
static bool findstart(void)
{
	while (!bisstart(Bbuff) && bistoken(Buff()))
		bmove(Bbuff, -1);
	while (!bisend(Bbuff) && !isalpha(Buff()))
		bmove1(Bbuff);
	return !bisend(Bbuff);
}

static void bconvert(int (*to)(int c))
{
	Buff() = to(Buff());
	Bbuff->bmodf = true;
}

void Zcapitalize_word(void)
{
	if (findstart()) {
		bconvert(toupper);
		for (bmove1(Bbuff); !bisend(Bbuff) && bistoken(Buff()); bmove1(Bbuff))
			bconvert(tolower);
		vsetmod();
	}
}


void Zlowercase_word(void)
{
	if (findstart()) {
		for (; !bisend(Bbuff) && bistoken(Buff()); bmove1(Bbuff))
			bconvert(tolower);
		vsetmod();
	}
}

void Zuppercase_word(void)
{
	if (findstart()) {
		for (; !bisend(Bbuff) && bistoken(Buff()); bmove1(Bbuff))
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
		tmp = Buff();
		bswappnt(Bbuff, to);
		binsert(Bbuff, tmp);
		bswappnt(Bbuff, to);
		bdelete(Bbuff, 1);
	}
}

void Zswap_words(void)
{
	struct mark *tmark, *tmp;

	tmark = bcremark(Bbuff);
	tmp = bcremark(Bbuff);
	if (tmark && tmp) {
		moveto(bistoken, FORWARD);
		if (!bisend(Bbuff)) {
			bmrktopnt(Bbuff, tmark);
			movepast(bistoken, FORWARD);
			bmrktopnt(Bbuff, tmp);
			bpnttomrk(Bbuff, tmark);
			moveto(bistoken, BACKWARD);
			blockmove(tmark, tmp);
			movepast(bistoken, BACKWARD);
			blockmove(tmark, tmp);
			bpnttomrk(Bbuff, tmp);
		}
	} else
		tbell();

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
	struct mark *tmark = NULL;
	char word[16];

	switch (Cmd) {
	case '}':
		/* line it up with last unmatched '{' */
		Arg = 0;
		if (!(tmark = bcremark(Bbuff))) {
			tbell();
			break;
		}
		bmrktopnt(Bbuff, tmark);	/* save current position */
		for (cnt = 0, bmove(Bbuff, -1); !bisstart(Bbuff); bmove(Bbuff, -1))
			if (Buff() == '{') {
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
			} else if (Buff() == '}')
				++cnt;
			else if (ISNL(Buff()))
				crfound = true;
		bpnttomrk(Bbuff, tmark);	/* no match - go back */
		tbell();		/* and warn user */
		break;

	case '#':
		if (!(tmark = bcremark(Bbuff))) {
			tbell();
			break;
		}

		do
			bmove(Bbuff, -1);
		while (!bisstart(Bbuff) && biswhite(Buff()));

		if (ISNL(Buff()) && !bisstart(Bbuff)) {
			bmove1(Bbuff);		/* skip NL */
			bdeltomrk(tmark);
		} else
			bpnttomrk(Bbuff, tmark);
		break;

	case ':':
		/* for c++ look for public/protected */
		getbword(word, 15, bisword);
		if (strcmp(word, "public")    == 0 ||
		   strcmp(word, "private")   == 0 ||
		   strcmp(word, "protected") == 0) {
			if (!(tmark = bcremark(Bbuff))) {
				tbell();
				break;
			}
			tobegline(Bbuff);
			while (isspace(Buff()))
				bdelete(Bbuff, 1);
			bpnttomrk(Bbuff, tmark);
		}
		break;

	case '/':
		if (bpeek(Bbuff) == '*')
			uncomment(Curbuff);
		break;
	}

	binsert(Bbuff, Cmd);
	unmark(tmark);
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
			if (Buff() == '#')
				bmove(Bbuff, -1);
			tobegline(Bbuff);
		} while (Buff() == '#' && !bisstart(Bbuff));
		movepast(biswhite, FORWARD);
		if (lookingat(Bbuff, "if") || lookingat(Bbuff, "while")) {
			width += Tabsize;
			did_indent = 1;
		}
		if (bisaftermrk(Bbuff, &tmark))
			bpnttomrk(Bbuff, &tmark);
		for (width += bgetcol(true, 0); bisbeforemrk(Bbuff, &tmark); bmove1(Bbuff))
			if (Buff() == '{') {
				if (did_indent == 0 || sawstart > 0)
					width += Tabsize;
				++sawstart;
			} else if (Buff() == '}' && sawstart) {
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

#ifdef HAVE_PCRE
#define RE_IF "\\bif\\b"
#define RE_FI "\\bfi\\b"
#define RE_WHILE "\\bwhile\\b"
#define RE_DONE "\\bdone\\b"
#else
#define RE_IF "\\<if\\>"
#define RE_FI "\\<fi\\>"
#define RE_WHILE "\\<while\\>"
#define RE_DONE "\\<done\\>"
#endif

void Zsh_indent(void)
{
	int width, fixup = 0;
	struct mark *save, *tmark;

	if (!(save = bcremark(Bbuff))) {
		Znewline();
		return;
	}

	tobegline(Bbuff);
	movepast(biswhite, FORWARD);
	tmark = bcremark(Bbuff);
	width = bgetcol(true, 0);

	if (lookingat(Bbuff, RE_IF)) {
		if (find_line(RE_FI) == 0)
			width += Taboffset;
	} else if (lookingat(Bbuff, RE_WHILE)) {
		if (find_line(RE_DONE) == 0)
			width += Taboffset;
	} else if (lookingat(Bbuff, RE_FI)) {
		width -= Taboffset;
		fixup = 1;
	} else if (lookingat(Bbuff, RE_DONE)) {
		width -= Taboffset;
		fixup = 1;
	} else if (Buff() == '}') {
		width -= Taboffset;
		fixup = 1;
	} else if (lookingat(Bbuff, ".*\\{[ \t]*$")) {
		/* Won't work if there is a comment */
		width += Taboffset;
	}

	if (fixup && tmark) {
		tobegline(Bbuff);
		bdeltomrk(tmark);
		tindent(width);
	}

	bpnttomrk(Bbuff, save);
	binsert(Bbuff, NL);
	Lfunc = ZINSERT; /* for undo */
	tindent(width);
	bdelmark(tmark);
	bdelmark(save);
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
		tmark = bcremark(Bbuff);
		if (!tmark) {
			tbell();
			return;
		}
		while (bgetcol(true, 0) > VAR(VFILLWIDTH)) {
			moveto(isspace, BACKWARD);
			movepast(isspace, BACKWARD);
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
	while (!bisend(Bbuff) && biswhite(Buff()))
		bdelete(Bbuff, 1);
	while (!bisstart(Bbuff)) {
		bmove(Bbuff, -1);
		if (biswhite(Buff()))
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
	tmp = bcremark(Bbuff);
	tmark = bcremark(Bbuff);		/* save the current point */
	if (!tmp || !tmark) {
		unmark(tmp);
		unmark(tmark);
		tbell();
		return;
	}
	putpaw("Reformatting...");
	do {
		/* mark the end of the paragraph and move the point to
		 * the start */
		Znext_paragraph();
		movepast(isspace, BACKWARD);
		bmrktopnt(Bbuff, tmp);
		Zprevious_paragraph();
		if (Buff() == '.')
			bcsearch(Bbuff, '\n');	/* for nroff */

		/* main loop */
		while (bisbeforemrk(Bbuff, tmp)) {
			moveto(isspace, FORWARD);
			if (bgetcol(true, 0) > VAR(VFILLWIDTH)) {
				moveto(isspace, BACKWARD);
				Ztrim_white_space();
				binsert(Bbuff, NL);
				moveto(isspace, FORWARD);
			}
			movepast(biswhite, FORWARD);
			if (Buff() == NL && bisbeforemrk(Bbuff, tmp)) {
				bdelete(Bbuff, 1);
				Ztrim_white_space();
				binsert(Bbuff, ' ');
			}
		}

		movepast(isspace, FORWARD); /* setup for next iteration */
	} while (Argp && !bisend(Bbuff) && !tkbrdy());

	clrpaw();
	if (Argp && !bisend(Bbuff)) {
		putpaw("Aborted");
		tgetkb();
	}

	bpnttomrk(Bbuff, tmark); /* restore point */
	unmark(tmark);
	unmark(tmp);
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
	if (isspace(Buff()))
		movepast(isspace, BACKWARD);
	while (!bisend(Bbuff) && !ispara(pc, Buff())) {
		pc = Buff();
		bmove1(Bbuff);
	}
	movepast(isspace, FORWARD);
}

void Zprevious_paragraph(void)
{
	char pc = '\0';

	movepast(isspace, BACKWARD);
	while (!bisstart(Bbuff) && !ispara(Buff(), pc)) {
		pc = Buff();
		bmove(Bbuff, -1);
	}
	movepast(isspace, FORWARD);
}

/* MISC COMMANDS */

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
		if (Bbuff->bmodf)
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
		if (Buff() == match)
			--cnt;
		else if (Buff() == ch)
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
		if (!bisend(Bbuff) && Buff() != NL)
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
	tsetcursor();
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
	while ((Cmd = tgetkb()) >= '0' && Cmd <= '9') {
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

	if (bisend(Bbuff) || Buff() == NL)
		bmove(Bbuff, -1);
	if (!bisstart(Bbuff))
		bmove(Bbuff, -1);
	tmp = Buff();
	bdelete(Bbuff, 1);
	bmove1(Bbuff);
	binsert(Bbuff, tmp);
}

void Zcount(void)
{
	bool word, swapped = false;
	unsigned l, w, c;
	struct mark *tmark = bcremark(Bbuff);

	if (!tmark) {
		tbell();
		return;
	}

	Arg = 0;
	if (UMARK_SET) {
		swapped = bisaftermrk(Bbuff, UMARK);
		if (swapped)
			bswappnt(Bbuff, UMARK);
		bmrktopnt(Bbuff, tmark);
	} else
		btostart(Bbuff);
	l = w = c = 0;
	putpaw("Counting...");
	word = false;
	for (; UMARK_SET ? bisbeforemrk(Bbuff, UMARK) : !bisend(Bbuff); bmove1(Bbuff), ++c) {
		if (ISNL(Buff()))
			++l;
		if (!bistoken(Buff()))
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
		toggle_mode(modes[rc].mode);
		Curwdo->modeflags = INVALID;
		if (settabsize(modes[rc].mode))
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
			 strncmp((char *)Bbuff->curcptr, "#!/", 3) == 0)
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
		Buff() = (*convert)(Buff());

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

	if (!(psave = bcremark(Bbuff))) {
		tbell();
		return;
	}
	if (bisaftermrk(Bbuff, UMARK)) {
		bswappnt(Bbuff, UMARK);
		if (!(msave = bcremark(Bbuff))) {
			tbell();
			unmark(psave);
			return;
		}
	}
	bcrsearch(Bbuff, NL);
	while (bisbeforemrk(Bbuff, UMARK)) {
		if (flag) {
			/* skip comment lines */
			if (Buff() != '#')
				for (i = 0; i < Arg; ++i)
					binsert(Bbuff, '\t');
		} else
			for (i = 0; i < Arg && Buff() == '\t'; ++i)
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
	Cmd = tgetkb();
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

void Zcalc(void)
{
	static char Calc_str[STRMAX + 1];
	char str[STRMAX];

	Arg = 0;
	if (getarg("Calc: ", Calc_str, STRMAX - 1))
		return;

	struct calc *c = calloc(1, sizeof(struct calc));
	if (c) {
		c->ops = calloc(MAX_OPS, sizeof(char));
		c->nums = calloc(MAX_OPS, sizeof(union number));
		c->max_ops = MAX_OPS;
	}
	if (!c || !c->ops || !c->nums) {
		if (c) free(c);
		if (c->ops) free(c->ops);
		if (c->nums) free(c->nums);
		error("Out of memory!");
		return;
	}

	/* We modify the string, leave Calc_str alone */
	strcpy(str, Calc_str);
	strcat(str, "=");

	int n = calc(c, str);
	switch (n) {
	case 0:
		if (c->is_float)
			putpaw("= %g", c->result.f);
		else {
			long n = c->result.i;
			putpaw("= %ld (%lx)", n, n);
		}
		break;
	case CALC_STACK_OVERFLOW:
		error("Stack overflow.");
		break;
	case CALC_SYNTAX_ERROR:
		error("Syntax error.");
		break;
	default:
		error("Calc internal error.");
	}

	free(c->ops);
	free(c->nums);
	free(c);
}


#if UNDO
int undo_add_clumped(struct buff *buff, int size)
{	/* commands we clump together */
	switch (Lfunc) {
	case ZINSERT:
	case ZNEWLINE:
	case ZTAB:
	case ZC_INSERT:
	case ZC_INDENT:
	case ZSH_INDENT:
		return 1;
	default:
		return 0;
	}
}

int undo_del_clumped(struct buff *buff, int size)
{
	if (size == 1)
		switch (Lfunc) {
		case ZDELETE_CHAR: return 1;
		case ZDELETE_PREVIOUS_CHAR: return -1;
		}
	return 0;
}


void Zundo(void)
{
	if (do_undo(Bbuff))
		tbell();
	else if (!Bbuff->undo_tail)
		/* Last undo */
		Bbuff->bmodf = false;
}
#else
void Zundo(void) { tbell(); }
#endif
