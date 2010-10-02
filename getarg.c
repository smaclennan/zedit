/* getarg.c - paw commands
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

/* globals for getarg */
Boolean InPaw, First;
int Pawcol, Pawlen, Pshift;
struct buff *Paw, *Buff_save;

/* globals for getplete */
static char **Carray;
static int Csize, Cnum = 0, Cret;
unsigned Nextpart = ZNOTIMPL;

/* General purpose string argument input routine which recursively calls the
 * editor through the PAW buffer.
 * Returns 0 if ok, ABORT if user aborted, 1 if empty string.
 * Prompt the user for an argument, with an optional default in arg.
 * Only allow max chars.
 * Arg is NOT overwritten if the user aborts, or returns a null string.
 */
#ifdef XWINDOWS
char *PromptString;
#endif


Boolean getarg(char *prompt, char *arg, int max)
{
	char *ptr;
	int argp_save, arg_save, rc;
	int tcol, trow;

	tcol = Pcol; trow = Prow;
#ifdef XWINDOWS
	/* We need this global so we can redisplay on an exposure event */
	PromptString = prompt;
	tstyle(T_NORMAL);		/* always display paw in normal */
#endif
	tgoto(tmaxrow() - 1 , 0);			/* display the prompt */
	tprntstr(prompt);
	Pawcol = Pcol = strlen(prompt); /* prompts are always simple ascii */

	argp_save = Argp;
	arg_save = Arg;
	Buff_save = Curbuff;
	Paw->bmode = Curbuff->bmode;
	InPaw = TRUE;
	Curcmds = 2;
	Keys[CR] = ZNEWLINE; /* in case we are in a VIEW buff */
	Pshift = 0;
	Pawlen = max;
	makepaw(arg, FALSE);
	First = TRUE;
	while (InPaw == TRUE)
		Execute();
	if (InPaw != ABORT) {
		/* get the argument */
		btostart();
		for (ptr = arg; !bisend() && Buff() != '\0'; bmove1())
			*ptr++ = Buff();
		*ptr = '\0';
		rc = ptr == arg;	/* set to 1 if string empty */
	} else
		rc = ABORT;
	InPaw = FALSE;
	Insearch = FALSE; /* used by Zcase when in search command */
	Argp = argp_save;
	Arg = arg_save;
	bswitchto(Buff_save);					/* go back */
	Curbuff->bmode = Paw->bmode; /* mainly for EXACT mode */
#if 0
	/* SAM ??? */
	Clrcol[tgetrow()] = tmaxcol();		/* force clear */
#endif
	tgoto(trow, tcol);
	Curwdo->modeflags = INVALID;
	Curcmds = (Curbuff->bmode & VIEW) ? 1 : 0;	/* SAM */
	clrecho();
#ifdef XWINDOWS
	tflush();
	showcursor(TRUE);
	showcursor(FALSE);
#endif
	return rc;
}

/* General purpose ask for an argument with completion given a struct
 * or char * array.
 * Returns offset of entry in array if found, else -1.
 */
int getplete(char *prompt, char *def, char **array, int size, int num)
{
	char cmdstr[STRMAX + 1];

	Carray = array;
	Csize = size / sizeof(char *);
	Cnum = num;
	if (def)
		strcpy(cmdstr, def);
	else
		*cmdstr = '\0';
	if (getarg(prompt, cmdstr, STRMAX))
		Cret = -1;
	Cnum = 0;
	return Cret;
}

static int pcmdplete(Boolean show)
{
	char cmd[STRMAX + 1], **cca, *mstr = NULL;
	int i = 0, len, len1 = 0, rc;

	Cret = -1;
	getbtxt(cmd, STRMAX);
	len = strlen(cmd);
	cca = Carray;
	if (show && !len)
		for (; i < Cnum; ++i, cca += Csize)
			pout(*cca, TRUE);
	else
		for (; i < Cnum && (rc = strncasecmp(*cca, cmd, len)) <= 0;
				++i, cca += Csize)
			if (rc == 0) {
				if (show)
					pout(*cca, TRUE);
				else if (mstr) {
					Cret = -1;
					len1 = nmatch(mstr, *cca);
				} else {
					Cret = i;
					len1 = strlen(mstr = *cca);
				}
			}
	if (mstr && !show) {
		strncpy(cmd, mstr, len1);
		cmd[len1] = '\0';
		makepaw(cmd, FALSE);
		return Cret != -1 ? 2 : 1;
	}
	return 0;
}


static int p_row, p_col;
static int p_ncols = PNUMCOLS;

static void pclear(void)
{
	int i;
	struct wdo *wdo = Whead;

	for (i = 0; i < Rowmax - 2; ++i) {
		tsetpoint(i, 0);
		tcleol();
		Scrnmarks[i].modf = TRUE;
		if (wdo->last == i) {
			wdo->modeflags = INVALID;
			if (wdo->next)
				wdo = wdo->next;
		}
	}
	pset(0, 0, PNUMCOLS);
}

void pset(int row, int col, int ncols)
{
	p_row = row;
	p_col = col;
	p_ncols = ncols;
}

void pout(char *str, Boolean check)
{
	tsetpoint(p_row, p_col * PCOLSIZE);
	Scrnmarks[p_row].modf = TRUE;
	if (!check || p_row < tmaxrow() - 2) {
		tprntstr(str);
		tcleol();
	}
	if (++p_col >= p_ncols) {
		++p_row;
		p_col = 0;
	}
}

/* Use instead of Zinsert when in PAW */
void pinsert(void)
{
	char savech;
	struct mark tmark;
	int width;

	if (Cmd == '?' && Cnum) {
		/* Help!!!!*/
		pclear();
		pcmdplete(TRUE);
		Dline(p_col ? ++p_row : p_row);
		return;
	}

	if (First) {
		bdelete(Curplen);
		First = FALSE;
		Tlrow = -1;
	}

	width = twidth(Cmd);
	if (bgetcol(FALSE, 0) + width <= Pawlen) {
		savech = Buff();	/* in case overwrite mode */
		Zinsert();

		bmrktopnt(&tmark);
		btoend();
		if (bgetcol(FALSE, 0) > Pawlen) {
			/* Insert in middle pushed text past end */
			bmove(-width);
			bdelete(width);
		}
		bpnttomrk(&tmark);

		if (Cnum)
			switch (pcmdplete(FALSE)) {
			case 0:		/* no match - remove char */
				bmove(-1);
				bdelete(1);
				if (Paw->bmode & OVERWRITE) {
					binsert(savech);
					bmove(-1);
				}
				break;
			case 1:		/* partial match */
				break;
			case 2:		/* full match */
				if (VAR(VEXECONMATCH))
					InPaw = FALSE;
				break;
			}
	} else
		tbell();
}

/* Use instead of Znewline when in PAW */
void pnewline(void)
{
	char cmdstr[STRMAX + 1], **ptr;

	if (Cnum) {
		getbtxt(cmdstr, STRMAX);
		for (Cret = 0, ptr = Carray;
		     Cret < Cnum && strcasecmp(cmdstr, *ptr);
		     ++Cret, ptr += Csize)
			;
		if (Cret == Cnum) {
			tbell();
			return;
		}
	}
	InPaw = FALSE;
}

void Zpart(void)
{
	char word[STRMAX + 1];
	struct buff *tbuff;

#ifdef TAGS
	if (Nextpart == ZFINDTAG) {
		bswitchto(Cfindbuff(TAGBUFNAME));
		for (bcsearch(NL); !bisend(); bcsearch(NL)) {
			getbword(word, STRMAX, bistoken);
			if (stristr(word, savetag)) {
				makepaw(word, TRUE);
				return;
			}
		}
		bswitchto(Paw);
		tbell();
		return;
	}
#endif
	if (Nextpart == ZSWITCHTO) {
			getbtxt(word, STRMAX);
		tbuff = Cfindbuff(word);
		if (!tbuff)
			tbuff = Curbuff;
		if (tbuff->next)
			tbuff = tbuff->next;
		else
			tbuff = Bufflist;
		makepaw(tbuff->bname, TRUE);
	} else
		tbell();
}

void makepaw(char *word, Boolean start)
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
