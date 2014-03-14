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
	while (!bisstart() && bistoken())
		bmove(-1);
	while (!bisend() && !isalpha(*Curcptr))
		bmove1();
	return !bisend();
}

void Zcapitalize_word(void)
{
	if (Findstart()) {
		*Curcptr = toupper(*Curcptr);
		Curbuff->bmodf = Curmodf = true;
		for (bmove1(); !bisend() && bistoken(); bmove1())
			*Curcptr = tolower(*Curcptr);
		vsetmod(false);
	}
}


void Zlowercase_word(void)
{
	if (Findstart()) {
		for (; !bisend() && bistoken(); bmove1()) {
			Curbuff->bmodf = Curmodf = true;
			*Curcptr = tolower(*Curcptr);
		}
		vsetmod(false);
	}
}

void Zuppercase_word(void)
{
	if (Findstart()) {
		for (; !bisend() && bistoken(); bmove1()) {
			Curbuff->bmodf = Curmodf = true;
			*Curcptr = toupper(*Curcptr);
		}
		vsetmod(false);
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

void Zswap_words(void)
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
	Ztrim_white_space();
	toendline();
	tmp = bgetcol(true, 0);
	if (tmp <= VAR(VFILLWIDTH)) {
		tobegline();
		tindent((VAR(VFILLWIDTH) - tmp) / 2);
		toendline();
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
		tmark = bcremrk();
		bmrktopnt(tmark);	/* save current position */
		for (cnt = 0, bmove(-1); !bisstart(); bmove(-1))
			if (*Curcptr == '{') {
				if (cnt)
					--cnt;
				else {  /* matched */
					tobegline();
					movepast(biswhite, FORWARD);
					cnt = bgetcol(true, 0);
					bpnttomrk(tmark);
					if (crfound) {
						Ztrim_white_space();
						tindent(cnt);
					}
					binsert(Cmd);
					unmark(tmark);
					return;
				}
			} else if (*Curcptr == '}')
				++cnt;
			else if (ISNL(*Curcptr))
				crfound = true;
		bpnttomrk(tmark);	/* no match - go back */
		unmark(tmark);
		tbell();		/* and warn user */
		break;

	case '#':
		tmark = bcremrk();

		do
			bmove(-1);
		while (!bisstart() && biswhite());

		if (ISNL(*Curcptr) && !bisstart()) {
			bmove1();		/* skip NL */
			bdeltomrk(tmark);
		} else
			bpnttomrk(tmark);
		unmark(tmark);
		break;

	case ':':
		/* for c++ look for public/protected */
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
		break;

	case '/':
		if (bpeek() == '*')
			uncomment(Curbuff);
		break;
	}

	binsert(Cmd);
}

void Zc_indent(void)
{
	int width = 0;
	struct mark tmark;

	if ((Curbuff->bmode & OVERWRITE))
		bcsearch(NL);
	else {
		int sawstart = 0, did_indent = 0;

		bmrktopnt(&tmark);
		do {
			if (*Curcptr == '#')
				bmove(-1);
			tobegline();
		} while (*Curcptr == '#' && !bisstart());
		movepast(biswhite, FORWARD);
		if (looking_at("if") || looking_at("while")) {
			width += Tabsize;
			did_indent = 1;
		}
		if (bisaftermrk(&tmark))
			bpnttomrk(&tmark);
		for (width += bgetcol(true, 0); bisbeforemrk(&tmark); bmove1())
			if (*Curcptr == '{') {
				if (did_indent == 0 || sawstart > 0)
					width += Tabsize;
				++sawstart;
			} else if (*Curcptr == '}' && sawstart) {
				width -= Tabsize;
				--sawstart;
			}
		bpnttomrk(&tmark);
		binsert(NL);
		tindent(width);
	}
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
		tmark = bcremrk();
		while (bgetcol(true, 0) > VAR(VFILLWIDTH)) {
			moveto(bisspace, BACKWARD);
			movepast(bisspace, BACKWARD);
		}
		Ztrim_white_space();
		tmp = !bisatmrk(tmark);
		binsert(NL);
		if (tmp) {
			bpnttomrk(tmark);
			binsert(Cmd);
		}
		unmark(tmark);
	}
}

void Ztrim_white_space(void)
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

void Zfill_paragraph(void)
{	/* Not reentrant - must iterate for arg */
	bool all;
	struct mark *tmark, *tmp;

	if (Curbuff->bmode & PROGMODE) {
		putpaw("Not in program mode");
		tbell();
		return;
	}
	tmark = bcremrk();		/* save the current point */
	all = Arg == 0;
	if (all == true)
		btostart();
	putpaw("Reformatting...");
	do {
		/* mark the end of the paragraph and move the point to
		 * the start */
		Znext_paragraph();
		movepast(bisspace, BACKWARD);
		tmp = bcremrk();
		Zprevious_paragraph();
		if (*Curcptr == '.')
			bcsearch('\n');	/* for nroff */

		/* main loop */
		while (bisbeforemrk(tmp)) {
			moveto(bisspace, FORWARD);
			if (bgetcol(true, 0) > VAR(VFILLWIDTH)) {
				moveto(bisspace, BACKWARD);
				Ztrim_white_space();
				binsert(NL);
				moveto(bisspace, FORWARD);
			}
			movepast(biswhite, FORWARD);
			if (*Curcptr == NL && bisbeforemrk(tmp)) {
				bdelete(1);
				Ztrim_white_space();
				binsert(' ');
			}
		}

		unmark(tmp);
		movepast(bisspace, FORWARD); /* setup for next iteration */
	} while ((all || --Arg > 0) && !bisend() && !tkbrdy());

	clrpaw();
	if (Arg > 0 || (all && !bisend())) {
		putpaw("Aborted");
		tgetcmd();
	}

	bpnttomrk(tmark); /* restore point */
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
	while (!bisend() && !ispara(pc, *Curcptr)) {
		pc = *Curcptr;
		bmove1();
	}
	movepast(bisspace, FORWARD);
}

void Zprevious_paragraph(void)
{
	char pc = '\0';

	movepast(bisspace, BACKWARD);
	while (!bisstart() && !ispara(*Curcptr, pc)) {
		pc = *Curcptr;
		bmove(-1);
	}
	movepast(bisspace, FORWARD);
}

/* MISC COMMANDS */

void Zposition(void)
{
	unsigned long mark, point;
	unsigned line;

	if (Curbuff->umark) {
		bswappnt(Curbuff->umark);
		mark = blocation(&line);
		bswappnt(Curbuff->umark);
		point = blocation(&line);
		putpaw("Line: %u  Column: %u  Point: %lu  Mark: %lu  Length: %lu",
		       line, bgetcol(false, 0) + 1, point, mark, blength(Curbuff));
	} else {
		point = blocation(&line);
		putpaw("Line: %u  Column: %u  Point: %lu  Mark: unset  Length: %lu",
		       line, bgetcol(false, 0) + 1, point, blength(Curbuff));
	}
}

void Znotimpl(void) { tbell(); }

void Zset_mark(void)
{
	set_umark(NULL); /* set to point */
	putpaw("Mark Set.");
}

void Zexit(void)
{
	struct buff *tbuff;
	bool modf = false;

	if (!saveall(Argp))
		return;

	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (tbuff->bmodf && !(tbuff->bmode & SYSBUFF))
			modf = true;
	if (modf && ask("Modified buffers. quit anyway? ") != YES)
		return;

	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (tbuff->child != EOF)
			unvoke(tbuff, false);
	checkpipes(0);		/* make sure waited for ALL children */

	tfini();

	/* For valgrind */
	wfini();
	bfini();
	cleanup_bookmarks();

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
bool promptsave(struct buff *tbuff, bool must)
{
	static int save_all;
	char str[BUFNAMMAX + 20];
	int ok = YES;

	if (tbuff->bmodf) {
		if (!must && !save_all) {
			sprintf(str, "save buffer %s? ", tbuff->bname);
			ok = ask2(str, true);
			if (ok == BANG)
				save_all = 1;
			else if (ok == ABORT)
				return false;
		}

		if (ok == YES || save_all) {
			bswitchto(tbuff);
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
	struct buff *tbuff, *bsave;

	bsave = Curbuff;
	for (tbuff = Bufflist; tbuff; tbuff = tbuff->next)
		if (!(tbuff->bmode & SYSBUFF) && !promptsave(tbuff, must)) {
			Curwdo->modeflags = INVALID;
			return false;
		}
	bswitchto(bsave);
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
	else { /* show the match! */
		zrefresh();
		tdelay(999);
	}
	bpnttomrk(&save);
}

void Zinsert(void)
{
	if (Curbuff->bmode & OVERWRITE) {
		if (!bisend() && *Curcptr != NL)
			bdelete(1);
	}

	binsert(Cmd);
	mshow(Cmd);
}

void Znewline(void)
{
	if (Curbuff->bmode & OVERWRITE)
		bcsearch(NL);
	else
		binsert(NL);
}

void Ztab(void)
{
	if (VAR(VSPACETAB)) {
		int tcol = Tabsize - (bgetcol(false, 0) % Tabsize);
		while (tcol-- > 0)
			binsert(' ');
	} else
		binsert('\t');
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
		tsetpoint(tmaxrow() - 1, 0);
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
			binsert(Cmd);
}

void Zswap_chars(void)
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

void tobegline(void)
{
	if (Curchar > 0 && *(Curcptr - 1) == NL)
		return;
	if (bcrsearch(NL))
		bmove1();
}

void toendline(void)
{
	if (bcsearch(NL))
		bmove(-1);
}

void Zcount(void)
{
	bool word, swapped = false;
	unsigned l, w, c;
	struct mark *tmark;

	Arg = 0;
	if (Argp) {
		NEED_UMARK;
		swapped = bisaftermrk(Curbuff->umark);
		if (swapped)
			bswappnt(Curbuff->umark);
		tmark = bcremrk();
	} else {
		tmark = bcremrk();
		btostart();
	}
	l = w = c = 0;
	putpaw("Counting...");
	word = false;
	for (; Argp ? bisbeforemrk(Curbuff->umark) : !bisend(); bmove1(), ++c) {
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
		mrktomrk(Curbuff->umark, tmark);
	else
		bpnttomrk(tmark);
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
	strcpy(tmp, bfname());

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
	NEED_UMARK;

	bmove1();	/* make sure we are not at the start of a paragraph */
	Zprevious_paragraph();
	bmrktopnt(Curbuff->umark);
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

	swapped = bisaftermrk(Curbuff->umark);
	if (swapped)
		bswappnt(Curbuff->umark);
	bmrktopnt(&tmark);

	for (; bisbeforemrk(Curbuff->umark); bmove1())
		*Curcptr = (*convert)(*Curcptr);

	if (swapped)
		mrktomrk(Curbuff->umark, &tmark);
	else
		bpnttomrk(&tmark);
	Curbuff->bmodf = true;

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

	psave = bcremrk();
	if (bisaftermrk(Curbuff->umark)) {
		bswappnt(Curbuff->umark);
		msave = bcremrk();
	}
	bcrsearch(NL);
	while (bisbeforemrk(Curbuff->umark)) {
		if (flag) {
			/* skip comment lines */
			if (*Curcptr != '#')
				for (i = 0; i < Arg; ++i)
					binsert('\t');
		} else
			for (i = 0; i < Arg && *Curcptr == '\t'; ++i)
				bdelete(1);
		bcsearch(NL);
	}
	bpnttomrk(psave);
	unmark(psave);
	if (msave) {
		mrktomrk(Curbuff->umark, msave);
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

	/* putenv cannot be passed an automatic: malloc the space */
	p = (char *)malloc(strlen(env) + strlen(set));
	if (p) {
		strcpy(p, env);
		strcat(p, set);
		if (putenv(p))
			error("Unable to set environment variable.");
	} else
		error("Out of memory.");
}
