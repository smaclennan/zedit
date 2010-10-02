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

static char *savefilename(char *fname)
{
	sprintf(fname, "%s.%d", ZSFILE, (int)Me->pw_uid);
	return fname;
}

void save(struct buff *bsave)
{
	struct wdo *wdo;
	struct buff *tbuff;
	char fname[30];
	unsigned junk;
	unsigned long ploc, mloc;
	FILE *fp = fopen(savefilename(fname), "w");
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

			bswitchto(tbuff);
			ploc = blocation(&junk);
			bpnttomrk(tbuff->mark);
			mloc = blocation(&junk);
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

	/* save the windows */
	for (wdo = Whead; wdo; wdo = wdo->next) {
		bswitchto(wdo->wbuff);
		bpnttomrk(wdo->wstart);
		mloc = blocation(&junk);
		fprintf(fp, "W %s %u %u %lu %u\n",
			wdo->wbuff->bname, wdo->first, wdo->last, mloc,
			wdo == Curwdo);
	}

	fclose(fp);
}

void loadsaved(void)
{
	FILE *fp;
	char bname[BUFNAMMAX + 1], fname[PATHMAX + 1], save[PATHMAX + 1];
	char ch;
	int mode;
	unsigned long ploc, mloc, sloc;

	if (!VAR(VDOSAVE))
		return;

	fp = fopen(savefilename(fname), "r");
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
		boffset(mloc);
		bmrktopnt(Curbuff->mark);
		boffset(ploc);
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

void Zcount(void)
{
	Boolean word, swapped = FALSE;
	char str[STRMAX];
	unsigned l, w, c;
	struct mark *tmark;

	Arg = 0;
	if (Argp) {
		tmark = bcremrk();
		btostart();
	} else {
		swapped = bisaftermrk(Curbuff->mark);
		if (swapped)
			bswappnt(Curbuff->mark);
		tmark = bcremrk();
	}
	l = w = c = 0;
	Echo("Counting...");
	word = FALSE;
	for (; Argp ? !bisend() : bisbeforemrk(Curbuff->mark); bmove1(), ++c) {
		if (ISNL(Buff()))
			++l;
		if (!bistoken())
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
		bpnttomrk(tmark);
	unmark(tmark);
}

void Zispace(void)
{
	binsert(' ');
	bmove(-1);
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

void Zmode(void)
{
	int i, rc;

	/* find the current mode for default */
	for (i = 0; i < NUMMODES && !(modes[i].mode & Curbuff->bmode); ++i)
		;
	if (Curbuff->bmode & VIEW)
		i = VIEWMODE;
	else if (i == NUMMODES)
		i = TEXTMODE;
	rc = getplete("Mode: ", modes[i].str, (char **)modes,
		      AMODESIZE, NUMMODES);
	if (rc == VIEWMODE) {
		Curbuff->bmode ^= VIEW;
		Curwdo->modeflags = INVALID;
	} else if (rc != -1)
		toggle_mode(modes[rc].mode);
}

/* we allow 8 extensions per type */
static int NoExt;
static char *cexts[9];
static char *texts[9];
static char *sexts[9];	/* s for shell */
static char *asexts[9];

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
			do {
				if (strcmp(str, ".") == 0)
					NoExt = mode;
				else
					o[i++] = strdup(str);
			} while (i < 8 && (str = strtok(NULL, ":")));
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
void toggle_mode(int mode)
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
			 (!VAR(VNORMAL) || extmatch(Bfname(), TEXT)))
			new = TEXT;
		else
			new = NORMAL;
	else
		new = mode;

#if COMMENTBOLD
	if (mode == 0)
		Curbuff->comchar = *(char *)VAR(VASCHAR);
#endif

	Curbuff->bmode = (Curbuff->bmode & MODEMASK) | new;
	if (mode) {
		Curwdo->modeflags = INVALID;
		tsave = Tabsize;
		if (Settabsize(new) != tsave)
			Zredisplay();
	}
}

void Zmrkpara(void)
{
	bmove1();	/* make sure we are not at the start of a paragraph */
	Zbpara();
	bmrktopnt(Curbuff->mark);
	while (Arg-- > 0)
		Zfpara();
	Arg = 0;
}

#define MAXDATE	80

void Zdate(void)
{
	char date[MAXDATE + 1];
	long t;

	time(&t);
	strftime(date, MAXDATE, VARSTR(VDATESTR), localtime(&t));
	if ((Argp || (Curbuff->bmode & VIEW)) && !InPaw)
		Echo(date);
	else
		binstr(date);
	Arg = 0;
}

static void setregion(int (*convert)(int))
{
	Boolean swapped;
	struct mark tmark;

	if (Curbuff->bmode & PROGMODE) {
		Echo("Not in program mode");
		tbell();
		return;
	}

	swapped = bisaftermrk(Curbuff->mark);
	if (swapped)
		bswappnt(Curbuff->mark);
	bmrktopnt(&tmark);

	for (; bisbeforemrk(Curbuff->mark); bmove1())
		Buff() = (*convert)(Buff());

	if (swapped)
		Mrktomrk(Curbuff->mark, &tmark);
	else
		bpnttomrk(&tmark);
	Curbuff->bmodf = MODIFIED;
	Zredisplay();
}

void Zupregion(void)
{
	setregion(toupper);
}

void Zlowregion(void)
{
	setregion(tolower);
}

static void indent(Boolean flag)
{
	struct mark *psave, *msave = NULL;
	int i;

	psave = bcremrk();
	if (bisaftermrk(Curbuff->mark)) {
		bswappnt(Curbuff->mark);
		msave = bcremrk();
	}
	bcrsearch(NL);
	while (bisbeforemrk(Curbuff->mark)) {
		if (flag) {
			/* skip comment lines */
			if (Buff() != '#')
				for (i = 0; i < Arg; ++i)
					binsert('\t');
		} else
			for (i = 0; i < Arg && Buff() == '\t'; ++i)
				bdelete(1);
		bcsearch(NL);
	}
	bpnttomrk(psave);
	unmark(psave);
	if (msave) {
		Mrktomrk(Curbuff->mark, msave);
		unmark(msave);
	}
}

void Zindent(void)
{
	indent(TRUE);
}

void Zundent(void)
{
	indent(FALSE);
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
			Error("Variable is too long.");
			return;
		}
		strcpy(set, p);
	} else
		*set = '\0';

	strcat(env, "=");	/* turn it into prompt */
	if (getarg(env, set, STRMAX))
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
