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

/* globals for Getarg */
Boolean InPaw, First;
int Pawcol, Pawlen, Pshift;
struct buff *Paw, *Buff_save;

/* globals for Getplete */
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


Boolean Getarg(char *prompt, char *arg, int max)
{
	char *ptr;
	int argp_save, arg_save, rc;
	int tcol, trow;

	tcol = Pcol; trow = Prow;
#ifdef XWINDOWS
	/* We need this global so we can redisplay on an exposure event */
	PromptString = prompt;
	Tstyle(T_NORMAL);		/* always display paw in normal */
#endif
	Tgoto(Tmaxrow() - 1 , 0);			/* display the prompt */
	Tprntstr(prompt);
	Pawcol = Pcol = strlen(prompt); /* prompts are always simple ascii */

	argp_save = Argp;
	arg_save = Arg;
	Buff_save = Curbuff;
	Paw->bmode = Curbuff->bmode;
	InPaw = TRUE;
	Funcs = Pawcmds;
	Keys[CR] = ZNEWLINE; /* in case we are in a VIEW buff */
	Pshift = 0;
	Pawlen = max;
	Makepaw(arg, FALSE);
	First = TRUE;
	while (InPaw == TRUE)
		Execute();
	if (InPaw != ABORT) {
		/* get the argument */
		Btostart();
		for (ptr = arg; !Bisend() && Buff() != '\0'; Bmove1())
			*ptr++ = Buff();
		*ptr = '\0';
		rc = ptr == arg;	/* set to 1 if string empty */
	} else
		rc = ABORT;
	InPaw = FALSE;
	Insearch = FALSE; /* used by Zcase when in search command */
	Argp = argp_save;
	Arg = arg_save;
	Bswitchto(Buff_save);					/* go back */
	Curbuff->bmode = Paw->bmode; /* mainly for EXACT mode */
#if 0
	/* SAM ??? */
	Clrcol[Tgetrow()] = Tmaxcol();		/* force clear */
#endif
	Tgoto(trow, tcol);
	Curwdo->modeflags = INVALID;
	Funcs = (Curbuff->bmode & VIEW) ? Vcmds : Cmds;	/* SAM */
	Clrecho();
#ifdef XWINDOWS
	Tflush();
	ShowCursor(TRUE);
	ShowCursor(FALSE);
#endif
	return rc;
}

/* General purpose ask for an argument with completion given a struct
 * or char * array.
 * Returns offset of entry in array if found, else -1.
 */
int Getplete(char *prompt, char *def, char **array, int size, int num)
{
	char cmdstr[STRMAX + 1];

	Carray = array;
	Csize = size / sizeof(char *);
	Cnum = num;
	if (def)
		strcpy(cmdstr, def);
	else
		*cmdstr = '\0';
	if (Getarg(prompt, cmdstr, STRMAX))
		Cret = -1;
	Cnum = 0;
	return Cret;
}

int Pcmdplete(Boolean show)
{
	char cmd[STRMAX + 1], **cca, *mstr = NULL;
	int i = 0, len, len1 = 0, rc;

	Cret = -1;
	Getbtxt(cmd, STRMAX);
	len = strlen(cmd);
	cca = Carray;
	if (show && !len)
		for (; i < Cnum; ++i, cca += Csize)
			Pout(*cca, TRUE);
	else
		for (; i < Cnum && (rc = Strnicmp(*cca, cmd, len)) <= 0;
				++i, cca += Csize)
			if (rc == 0) {
				if (show)
					Pout(*cca, TRUE);
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
		Makepaw(cmd, FALSE);
		return Cret != -1 ? 2 : 1;
	}
	return 0;
}


static int p_row, p_col;
static int p_ncols = PNUMCOLS;

static void Pclear(void)
{
	int i;
	struct wdo *wdo = Whead;

	for (i = 0; i < Rowmax - 2; ++i) {
		Tsetpoint(i, 0);
		Tcleol();
		Scrnmarks[i].modf = TRUE;
		if (wdo->last == i) {
			wdo->modeflags = INVALID;
			if (wdo->next)
				wdo = wdo->next;
		}
	}
	Pset(0, 0, PNUMCOLS);
}


void Pset(int row, int col, int ncols)
{
	p_row = row;
	p_col = col;
	p_ncols = ncols;
}

void Pout(char *str, Boolean check)
{
	Tsetpoint(p_row, p_col * PCOLSIZE);
	Scrnmarks[p_row].modf = TRUE;
	if (!check || p_row < Tmaxrow() - 2) {
		Tprntstr(str);
		Tcleol();
	}
	if (++p_col >= p_ncols) {
		++p_row;
		p_col = 0;
	}
}

/* Use instead of Zinsert when in PAW */
void Pinsert(void)
{
	char savech;
	struct mark tmark;
	int width;

	if (Cmd == '?' && Cnum) {
		/* Help!!!!*/
		Pclear();
		Pcmdplete(TRUE);
		Dline(p_col ? ++p_row : p_row);
		return;
	}

	if (First) {
		Bdelete(Curplen);
		First = FALSE;
		Tlrow = -1;
	}

	width = Twidth(Cmd);
	if (Bgetcol(FALSE, 0) + width <= Pawlen) {
		savech = Buff();	/* in case overwrite mode */
		Zinsert();

		Bmrktopnt(&tmark);
		Btoend();
		if (Bgetcol(FALSE, 0) > Pawlen) {
			/* Insert in middle pushed text past end */
			Bmove(-width);
			Bdelete(width);
		}
		Bpnttomrk(&tmark);

		if (Cnum)
			switch (Pcmdplete(FALSE)) {
			case 0:		/* no match - remove char */
				Bmove(-1);
				Bdelete(1);
				if (Paw->bmode & OVERWRITE) {
					Binsert(savech);
					Bmove(-1);
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
		Tbell();
}

/* Use instead of Znewline when in PAW */
void Pnewline(void)
{
	char cmdstr[STRMAX + 1], **ptr;

	if (Cnum) {
		Getbtxt(cmdstr, STRMAX);
		for (Cret = 0, ptr = Carray;
		     Cret < Cnum && Stricmp(cmdstr, *ptr);
		     ++Cret, ptr += Csize)
			;
		if (Cret == Cnum) {
			Tbell();
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
		Bswitchto(Cfindbuff(TAGBUFNAME));
		for (Bcsearch(NL); !Bisend(); Bcsearch(NL)) {
			Getbword(word, STRMAX, Istoken);
			if (Strstr(word, Savetag)) {
				Makepaw(word, TRUE);
				return;
			}
		}
		Bswitchto(Paw);
		Tbell();
		return;
	}
#endif
	if (Nextpart == ZSWITCHTO) {
			Getbtxt(word, STRMAX);
		tbuff = Cfindbuff(word);
		if (!tbuff)
			tbuff = Curbuff;
		if (tbuff->next)
			tbuff = tbuff->next;
		else
			tbuff = Bufflist;
		Makepaw(tbuff->bname, TRUE);
	} else
		Tbell();
}

void Makepaw(char *word, Boolean start)
{
	Bswitchto(Paw);
	Btostart();
	Bdelete(Curplen);
	Binstr(word);
	Tcleol();
	memset(tline, '\376', COLMAX);	/* invalidate it */
	if (start)
		Btostart();
}
