/* comms1.c - Zedit commands continued
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

static char *SaveFileName(char *fname)
{
	sprintf(fname, "%s.%d", ZSFILE, (int)Me->pw_uid);
	return fname;
}

void Save(Buffer *bsave)
{
	WDO *wdo;
	Buffer *tbuff;
	char fname[30];
	unsigned junk;
	unsigned long ploc, mloc;
	FILE *fp = fopen(SaveFileName(fname), "w");
	if (fp == NULL)
		return;

	/* save the globals */
	fwrite(G_start, G_end - G_start, 1, fp);

	/* save buffers */
	for (tbuff = Bufflist; tbuff->next; tbuff = tbuff->next)
		;
	for (; tbuff; tbuff = tbuff->prev)
		/* don't save the system buffers */
		if (tbuff->fname && !(tbuff->bmode & SYSBUFF)) {
			unsigned mode;

			Bswitchto(tbuff);
			ploc = Blocation(&junk);
			Bpnttomrk(tbuff->mark);
			mloc = Blocation(&junk);
#if COMMENTBOLD
			/* put the comchar in the upper 8 bits of mode */
			mode = tbuff->bmode | (tbuff->comchar << 24);
#else
			mode = tbuff->bmode;
#endif
			fprintf(fp, "B %s %s %lu %lu %u\n",
				tbuff->bname, tbuff->fname, ploc, mloc, mode);
		}

	/* end of buffers marker */
	fputs("M M M 0 0 0\n", fp);

#ifndef BORDER3D
	/* save the windows */
	for (wdo = Whead; wdo; wdo = wdo->next) {
		Bswitchto(wdo->wbuff);
		Bpnttomrk(wdo->wstart);
		mloc = Blocation(&junk);
		fprintf(fp, "W %s %u %u %lu %u\n",
			wdo->wbuff->bname, wdo->first, wdo->last, mloc,
			wdo == Curwdo);
	}
#endif

	fclose(fp);
}

void Loadsaved()
{
	FILE *fp;
	char bname[BUFNAMMAX + 1], fname[PATHMAX + 1], save[PATHMAX + 1];
	char ch;
	int mode;
	unsigned long ploc, mloc, sloc;

	if (!Vars[VDOSAVE].val)
		return;

	fp = fopen(SaveFileName(fname), "r");
	if (fp == NULL)
		return;

	/* check the version */
	if (fread(save, 1, 4, fp) != 4 || strcmp(save, VERSTR)) {
		fclose(fp);
		return;
	}

	/* read the global variables */
	rewind(fp);
	if (fread(G_start, G_end - G_start, 1, fp) != 1) {
		fclose(fp);
		return;
	}

	/* load the buffers */
	strcpy(save, Lbufname);
	while (fscanf(fp, "%c %s %s %lu %lu %u\n",
			&ch, bname, fname, &ploc, &mloc, &mode) == 6 &&
	       ch == 'B') {
#if COMMENTBOLD
		char comchar;
#endif

		Readone(bname, fname);
		Boffset(mloc);
		Bmrktopnt(Curbuff->mark);
		Boffset(ploc);
#if COMMENTBOLD
		/* strip the comchar off the mode */
		comchar = mode >> 24;
		if (comchar) {
			Curbuff->comchar = comchar;
			mode &= 0x00ffffff;
		}
#endif
		/* use 'mode' except for the VIEW bit */
		Curbuff->bmode = (mode & ~VIEW) | (Curbuff->bmode & VIEW);
	}

	/* load the windows */
	while (fscanf(fp, "W %s %lu %lu %lu %u\n",
		bname, &ploc, &mloc, &sloc, &mode) == 5)
			Wload(bname, ploc, mloc, sloc, mode);

	fclose(fp);
	strcpy(Lbufname, save);
}


/* Bmove can only move the Point +/-32767 bytes. This routine overcomes
 * this limitation.
 * NOTE: Can only move forward.
 */
#define MAXMOVE		(0x7fff - 1024)

void Boffset(unsigned long off)
{
	Btostart();
	for (; off > MAXMOVE; off -= MAXMOVE)
		Bmove(MAXMOVE);
	Bmove(off);
}

Proc Zcount()
{
	Boolean word, swapped = FALSE;
	char str[STRMAX];
	unsigned l, w, c;
	Mark *tmark;

	Arg = 0;
	if (Argp) {
		tmark = Bcremrk();
		Btostart();
	} else {
		swapped = Bisaftermrk(Curbuff->mark);
		if (swapped)
			Bswappnt(Curbuff->mark);
		tmark = Bcremrk();
	}
	l = w = c = 0;
	Echo("Counting...");
	word = FALSE;
	for (; Argp ? !Bisend() : Bisbeforemrk(Curbuff->mark); Bmove1(), ++c) {
		if (ISNL(Buff()))
			++l;
		if (!Istoken())
			word = FALSE;
		else if (!word) {
			++w;
			word = TRUE;
		}
	}
	sprintf(str, "Lines: %u   Words: %u   Characters: %u", l, w, c);
	Echo(str);
	if (swapped)
		Mrktomrk(Curbuff->mark, tmark);
	else
		Bpnttomrk(tmark);
	Unmark(tmark);
}

Proc Zispace()
{
	Binsert(' ');
	Bmove(-1);
}

/* this struct must be sorted */
static struct _amode
{
	char *str;
	int mode;
} modes[] = {
	{ "ASM",	ASMMODE },
	{ "C",		CMODE	},
	{ "Normal",	NORMAL	},
	{ "TCL",	TCL		},
	{ "Text",	TEXT	},
	{ "View",	VIEW	},
#define TEXTMODE	4
#define VIEWMODE	5
};
#define AMODESIZE	sizeof(struct _amode)
#define NUMMODES	(sizeof(modes) / AMODESIZE)

Proc Zmode()
{
	int i, rc;

	/* find the current mode for default */
	for (i = 0; i < NUMMODES && !(modes[i].mode & Curbuff->bmode); ++i)
		;
	if (Curbuff->bmode & VIEW)
		i = VIEWMODE;
	else if (i == NUMMODES)
		i = TEXTMODE;
	rc = Getplete("Mode: ", modes[i].str, (char **)modes,
		      AMODESIZE, NUMMODES);
	if (rc == VIEWMODE) {
		Curbuff->bmode ^= VIEW;
		Curwdo->modeflags = INVALID;
	} else if (rc != -1)
		Toggle_mode(modes[rc].mode);
}

/* we allow 8 extensions per type */
static int NoExt;
static char *cexts[9] = { 0 };
static char *texts[9] = { 0 };
static char *sexts[9] = { 0 };	/* s for shell */
static char *asexts[9] = { 0 };

static int get_mode(int mode, char ***exts)
{
	switch (mode & PROGMODE) {
	case CMODE:
		mode = CMODE;	*exts = cexts; break;
	case ASMMODE:
		mode = ASMMODE;	*exts = asexts; break;
	case TCL:
		mode = TCL;	*exts = sexts; break;
	case TEXT:
	default:
		mode = TEXT;	*exts = texts; break;
	}

	return mode;
}

/* This is called to set the cexts/texts/aexts array */
void parsem(char *in, int mode)
{
	char **o, *str, *start;
	int i = 0;

	mode = get_mode(mode, &o);
	start = str = strdup(in);
	if (str) {
		str = strtok(str, ":");
		if (str) {
			do
				if (strcmp(str, ".") == 0)
					NoExt = mode;
				else
					o[i++] = strdup(str);
			while (i < 8 && (str = strtok(NULL, ":")) != 0)
				;
			o[i] = NULL;
		}
		free(start);
	}
}

static Boolean extmatch(char *str, Boolean mode)
{
	char **o;
	int i;

	if (!str)
		return FALSE;

	mode = get_mode(mode, &o);
	str = strrchr(str, '.');
	if (!str)
		return NoExt == mode;
	else
		for (i = 0; o[i]; ++i)
			if (strcmp(o[i], str) == 0)
				return TRUE;
	return FALSE;
}


/* Toggle from/to 'mode'. Passed 0 to set for Readone */
void Toggle_mode(int mode)
{
	int new, tsave;

	if ((Curbuff->bmode & mode) || mode == 0)
		/* Toggle out of 'mode' - decide which to switch to */
		if (mode != CMODE && extmatch(Bfname(), CMODE))
			new = CMODE;
		else if (mode != ASMMODE && extmatch(Bfname(), ASMMODE))
			new = ASMMODE;
		else if (mode != TCL && extmatch(Bfname(), TCL))
			new = TCL;
		else if (mode != TEXT &&
			 (!Vars[VNORMAL].val || extmatch(Bfname(), TEXT)))
			new = TEXT;
		else
			new = NORMAL;
	else
		new = mode;

#if COMMENTBOLD
	if (mode == 0)
		Curbuff->comchar = *(char *)Vars[VASCHAR].val;
#endif

	Curbuff->bmode = (Curbuff->bmode & MODEMASK) | new;
	if (mode) {
		Curwdo->modeflags = INVALID;
		tsave = Tabsize;
		if (Settabsize(new) != tsave)
			Zredisplay();
	}
}

Proc Zmrkpara()
{
	Bmove1();	/* make sure we are not at the start of a paragraph */
	Zbpara();
	Bmrktopnt(Curbuff->mark);
	while (Arg-- > 0)
		Zfpara();
	Arg = 0;
}

#define MAXDATE	80

Proc Zdate()
{
	char date[MAXDATE + 1];
	long t;

	time(&t);
	strftime(date, MAXDATE, (char *)Vars[VDATESTR].val, localtime(&t));
	if ((Argp || (Curbuff->bmode & VIEW)) && !InPaw)
		Echo(date);
	else
		Binstr(date);
	Arg = 0;
}

static void Setregion(int (*convert)())
{
	Boolean swapped;
	Mark tmark;

	if (Curbuff->bmode & PROGMODE) {
		Echo("Not in program mode");
		Tbell();
		return;
	}

	swapped = Bisaftermrk(Curbuff->mark);
	if (swapped)
		Bswappnt(Curbuff->mark);
	Bmrktopnt(&tmark);

	for (; Bisbeforemrk(Curbuff->mark); Bmove1())
		Buff() = (*convert)(Buff());

	if (swapped)
		Mrktomrk(Curbuff->mark, &tmark);
	else
		Bpnttomrk(&tmark);
	Curbuff->bmodf = MODIFIED;
	Zredisplay();
}

Proc Zupregion()
{
	Setregion(Toupper);
}

Proc Zlowregion()
{
	Setregion(Tolower);
}

static void Indent(Boolean flag)
{
	Mark *psave, *msave = NULL;
	int i;

	psave = Bcremrk();
	if (Bisaftermrk(Curbuff->mark)) {
		Bswappnt(Curbuff->mark);
		msave = Bcremrk();
	}
	Bcrsearch(NL);
	while (Bisbeforemrk(Curbuff->mark)) {
		if (flag) {
			/* skip comment lines */
			if (Buff() != '#')
				for (i = 0; i < Arg; ++i)
					Binsert('\t');
		} else
			for (i = 0; i < Arg && Buff() == '\t'; ++i)
				Bdelete(1);
		Bcsearch(NL);
	}
	Bpnttomrk(psave);
	Unmark(psave);
	if (msave) {
		Mrktomrk(Curbuff->mark, msave);
		Unmark(msave);
	}
}

Proc Zindent()
{
	Indent(TRUE);
}

Proc Zundent()
{
	Indent(FALSE);
}

Proc Mshow(ch)
unsigned ch;
{
	Byte match;
	int cnt = 0;
	Mark save;

	if (!(Curbuff->bmode & PROGMODE) || InPaw || Tkbrdy())
		return;
	if (Vars[VMATCH].val & 1) {
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
		Bmrktopnt(&save);
		do {
			Bmove(-1);
			if (Buff() == match)
				--cnt;
			else if (Buff() == ch)
				++cnt;
		} while (cnt && !Bisstart());
		if (cnt)
			Tbell();
		else {
			Refresh();
			ShowCursor(TRUE);	/* show the match! */
			Delay();
			ShowCursor(FALSE);
		}
		Bpnttomrk(&save);
	} else if (Vars[VMATCH].val & 2) {
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
		Binsert(match);
		Bmove(-1);
	}
}

Proc Zsetenv()
{
	char env[STRMAX + 2], set[STRMAX + 1], *p;

	*env = '\0';
	if (Getarg("Env: ", env, STRMAX))
		return;
	p = getenv(env);
	if (p) {
		if (strlen(p) >= STRMAX) {
			Error("Variable is too long.");
			return;
		}
		strcpy(set, p);
	} else
		*set = '\0';

	strcat(env, "=");	/* turn it into prompt */
	if (Getarg(env, set, STRMAX))
		return;

	/* putenv cannot be passed an automatic: malloc the space */
	p = malloc(strlen(env) + strlen(set));
	if (p) {
		strcpy(p, env);
		strcat(p, set);
		if (putenv(p))
			Error("Unable to set environment variable.");
	} else
		Error("Out of memory.");
}
