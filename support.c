/* support.c - Zedit support routines
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

/* Ask Yes/No question.
 * Returns YES, NO, or ABORT
 */
int Ask(char *msg)
{
	int rc;
	unsigned cmd;

	Echo(msg);
	do
		switch (cmd = Tgetcmd()) {
		case 'y':
		case 'Y':
			rc = YES;
			break;
		case 'N':
		case 'n':
			rc = NO;
			break;
		default:
			Tbell();
			rc = (Keys[cmd] == ZABORT) ? ABORT : BADCHAR;
			break;
		}
	while (rc == BADCHAR);
	Clrecho();
	return rc;
}

/* Move a block of chars around point and "from" to "to".
 * Assumes point is before "from".
*/
void Blockmove(struct mark *from, struct mark *to)
{
	char tmp;

	while (Bisbeforemrk(from)) {
		tmp = Buff();
		Bswappnt(to);
		Binsert(tmp);
		Bswappnt(to);
		Bdelete(1);
	}
}

#ifndef XWINDOWS
/* more efficient to not make it a macro */
void Clrecho() { PutPaw("", 2); }
#endif

Boolean Isext(char *fname, char *ext)
{
	char *ptr;

	return fname && (ptr = strrchr(fname, '.')) && strcmp(ptr, ext) == 0;
}

/* Was the last command a "cursor" command? */
Boolean zCursor()
{
	return  Lfunc == ZPREVLINE || Lfunc == ZNEXTLINE ||
		Lfunc == ZPREVPAGE || Lfunc == ZNEXTPAGE;
}

Boolean Delayprompt(char *msg)
{
	int rc;

	rc = Delay();
	if (rc)
		PutPaw(msg, 2);
	return rc;
}

#ifndef XWINDOWS
#include <sys/time.h>
#include <sys/poll.h>

Boolean Delay()
{
	static struct pollfd ufd = { .fd = 1, .events = POLLIN };

	return InPaw || Tkbrdy() || poll(&ufd, 1, 1000) == 1 ? 1 : 0;
}

#else

Boolean Delay()
{
	long t;

	if (InPaw)
		return FALSE;
	t = time((long *)0) + 2;	/* at least 1 second */
	do
		if (Tkbrdy())
			return FALSE;
	while (time(NULL) < t);
	return TRUE;
}
#endif

/* Was the last command a delete to kill buffer command? */
Boolean Delcmd()
{
	return	Lfunc == ZDELEOL  || Lfunc == ZDELLINE  || Lfunc == ZDELRGN   ||
		Lfunc == ZCOPYRGN || Lfunc == ZDELWORD  || Lfunc == ZRDELWORD ||
		Lfunc == ZMAKEDEL || Lfunc == ZGETBWORD;
}

/* Was the last command a delete of any type? */
Boolean DelcmdAll()
{
	return Delcmd() || Lfunc == ZDELCHAR || Lfunc == ZRDELCHAR;
}

char PawStr[COLMAX + 10];

#ifndef XWINDOWS
/*
Put a string into the PAW.
type is:	0 for echo			Echo()		macro
		1 for error			Error()		macro
		2 for save pos
*/
void PutPaw(char *str, int type)
{
	int trow, tcol;

	if (type == 1)
		Tbell();
	if (!InPaw) {
		trow = Prow; tcol = Pcol;
		Tsetpoint(Tmaxrow() - 1, 0);
		Tprntstr(str);
		Tcleol();
		if (type != 1)
			Tsetpoint(trow, tcol);
		Tforce();
		Tflush();
		if (type == 1)
			Tgetcmd();
	}
}
#endif


/* Echo 'str' to the paw and as the filename for 'buff' */
void Message(struct buff *buff, char *str)
{
	struct wdo *wdo;

	if (buff->fname)
		free(buff->fname);
	buff->fname = strdup(str);
	for (wdo = Whead; wdo; wdo = wdo->next)
		if (wdo->wbuff == buff)
			wdo->modeflags = INVALID;
	Echo(str);
}

/*
 * Find the correct path for the config files.
 * There are 4 config files: bindings, config, help, and save
 *	bindings	- $EXE  + $HOME + . + ConfigDir
 *	config		- $EXE  + $HOME + . + ConfigDir
 *	help		- $EXE  | $HOME | . | ConfigDir
 *	save		- $HOME	| . | ConfigDir
 *
 * returns:	4 for $EXE
 *		3 for $HOME
 *		2 for .
 *		1 for ConfigDir
 *		0 for not found, 'path' set to 'fname'
 */
int Findpath(char *p, char *f, int s, Boolean m)
{
	switch (s) {
	case 4:
		if (Isfile(p, Thispath, f, m))
			return 4;
	case 3:
		if (Isfile(p, Me->pw_dir, f, m))
			return 3;
	case 2:
		if (Isfile(p, ".", f, m))
			return 2;
	case 1:
		if (Isfile(p, ConfigDir, f, m))
			return 1;
	default:
		strcpy(p, f);
		return 0;
	}
}

/* Get the word at the current buffer point and store in 'word'.
 *  Get at the most 'max' characters.
 * Leaves the point alone.
 */
Boolean Getbword(char word[], int max, int (*valid)())
{
	int i;
	struct mark tmark;

	Bmrktopnt(&tmark);
	Moveto(Istoken, FORWARD);
	if (Bisend())
		Moveto(Istoken, BACKWARD);
	Movepast(Istoken, BACKWARD);
	for (i = 0; !Bisend() && valid() && i < max; ++i, Bmove1())
		word[i] = Buff();
	word[i] = '\0';
	Bpnttomrk(&tmark);
	return i;
}

/* Get the current buffer text and store in 'txt'.
 * Get at the most 'max' characters.
 * Leaves the point alone.
 */
char *Getbtxt(char txt[], int max)
{
	int i;
	struct mark tmark;

	Bmrktopnt(&tmark);
	for (Btostart(), i = 0; !Bisend() && i < max; Bmove1(), ++i)
		txt[i] = Buff();
	txt[i] = '\0';
	Bpnttomrk(&tmark);
	return txt;
}

void Killtomrk(struct mark *tmark)
{
	Copytomrk(tmark);
	Bdeltomrk(tmark);
}

/* Go forward or back past a thingy */
void Movepast(int (*pred)(), Boolean forward)
{
	if (!forward)
		Bmove(-1);
	while (!(forward ? Bisend() : Bisstart()) && (*pred)())
		Bmove(forward ? 1 : -1);
	if (!forward && !(*pred)())
		Bmove1();
}

/* Go forward or back to a thingy */
void Moveto(int (*pred)(), Boolean forward)
{
	if (!forward)
		Bmove(-1);
	while (!(forward ? Bisend() : Bisstart()) && !(*pred)())
		Bmove(forward ? 1 : -1);
	if (!forward && !Bisstart())
		Bmove1();
}

char *Strup(char *str)
{
	char *ptr;
	for (ptr = str; *ptr; ++ptr)
		*ptr = Toupper(*ptr);
	return str;
}


/* Put in the right number of tabs and spaces */
void Tindent(int arg)
{
	if (VAR(VSPACETAB) == 0)
		for (; arg >= Tabsize; arg -= Tabsize)
			Binsert('\t');
	Sindent(arg);
}

int Toupper(int c)
{
	return TOUPPER(c);
}

int Tolower(int c)
{
	return TOLOWER(c);
}

int Isspace()
{
	return isspace(Buff());
}

int Isword()
{
	return  isalnum(Buff()) || Buff() == '_' || Buff() == '.' ||
		Buff() == '$';
}

/* Must be a real function. $ for PL/M */
int Istoken()
{
	return isalnum(Buff()) || Buff() == '_' || Buff() == '.' ||
		Buff() == '$' || Buff() == '/';
}

int Iswhite()
{
	return STRIP(Buff()) == ' ' || STRIP(Buff()) == '\t';
}

char *Bakname(char *bakname, char *fname)
{
	strcpy(bakname, fname);
	strcat(bakname, "~");
	return bakname;
}


Boolean Mv(char *from, char *to)
{
	Boolean rc;

	unlink(to);
	rc = link(from, to);
	if (rc == 0)
		unlink(from);
	return rc;
}


Boolean Cp(char *from, char *to)
{
	FILE *in, *out;
	char buf[1024];
	int n, rc = TRUE;

	in = fopen(from, "r");
	out = fopen(to, "w");
	if (!in || !out) {
		if (!in)
			fclose(in);
		return FALSE;
	}
	while ((n = fread(buf, 1, 1024, in)) > 0)
		if (fwrite(buf, 1, n, out) != n) {
			rc = FALSE;
			break;
		}
	fclose(in);
	fclose(out);
	return rc;
}


#ifdef XWINDOWS
/* Move the buffer point to an absolute row, col */
void Pntmove(int row, int col)
{
	struct wdo *wdo;
	int i;

	if (InPaw) {
		/* Can't move out of paw */
		if (row != Rowmax - 1 || col < Pawcol)
			Tbell();
		else {
			col = Bmakecol(col - Pawcol, FALSE);
			Tsetpoint(Rowmax - 1, col);
		}
		return;
	}

	/* don't move Point into Paw or mode row */
	for (wdo = Whead; wdo; wdo = wdo->next)
		if (row >= wdo->first && row < wdo->last) {
			/* find offset in window */
			for (i = wdo->first; i < row; ++i) {
				Wswitchto(wdo);
				Bpnttomrk(&Scrnmarks[i]);
				if (Bisend()) {
					/* at end of buffer - stop */
					if (i > wdo->first)
						--i;
					col = Colmax;
					break;
				}
			}
			Bpnttomrk(&Scrnmarks[i]);
			col = Bmakecol(col, FALSE);
			Tsetpoint(i, col);
			return;
		}
	Tbell();
}
#endif

/* Limit a filename to at most Tmaxcol() - 'num' cols */
char *Limit(char *fname, int num)
{
	int off;

	off = strlen(fname) - (Tmaxcol() - num);
	return off > 0 ? fname + off : fname;
}

/* called at startup when out of memory */
void NoMem()
{
	Error("Out of memory.");
	exit(1);
}

/* Find first occurance in str1 of str2. NULL if not found.
 * Case insensitive!
 */
char *Strstr(char *str1, char *str2)
{
	int i, len, max;

	len = strlen(str2);
	max = strlen(str1) - len;
	for (i = 0; i <= max; ++i)
		if (Strnicmp(&str1[i], str2, len) == 0)
			return &str1[i];
	return NULL;
}
