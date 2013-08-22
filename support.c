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
#include <sys/time.h>
#include <sys/poll.h>

/* ask Yes/No question.
 * Returns YES, NO, BANG, or ABORT
 */
int ask2(char *msg, Boolean allow_bang)
{
	int rc = BADCHAR;
	unsigned cmd;

	echo(msg);
	while (rc == BADCHAR)
		switch (cmd = tgetcmd()) {
		case 'y':
		case 'Y':
			rc = YES;
			break;
		case 'N':
		case 'n':
			rc = NO;
			break;
		case '!':
			if (allow_bang)
				rc = BANG;
			else
				tbell();
			break;
		default:
			tbell();
			if (Keys[cmd] == ZABORT)
				rc = ABORT;
		}
	clrecho();
	return rc;
}

/* ask Yes/No question.
 * Returns YES, NO, or ABORT
 */
int ask(char *msg)
{
	return ask2(msg, FALSE);
}

Boolean delayprompt(char *msg)
{
	int rc;

	rc = delay();
	if (rc)
		putpaw(msg, 2);
	return rc;
}

Boolean delay(void)
{
	struct pollfd ufd;

	if (InPaw || tkbrdy())
		return FALSE;

	ufd.fd = 1;
	ufd.events = POLLIN;
	return poll(&ufd, 1, 1000) != 1;
}

/* Was the last command a delete to kill buffer command? */
Boolean delcmd(void)
{
	return	Lfunc == ZDELEOL  || Lfunc == ZDELLINE  || Lfunc == ZDELRGN   ||
		Lfunc == ZCOPYRGN || Lfunc == ZDELWORD  || Lfunc == ZRDELWORD ||
		Lfunc == ZMAKEDEL || Lfunc == ZGETBWORD;
}

/* Was the last command a delete of any type? */
Boolean delcmdall(void)
{
	return delcmd() || Lfunc == ZDELCHAR || Lfunc == ZRDELCHAR;
}

char PawStr[COLMAX + 10];

/*
Put a string into the PAW.
type is:	0 for echo			echo()		macro
		1 for error			error()		macro
		2 for save pos
*/
void putpaw(char *str, int type)
{
	int trow, tcol;

	if (type == 1)
		tbell();
	if (!InPaw) {
		trow = Prow; tcol = Pcol;
		tsetpoint(tmaxrow() - 1, 0);
		tprntstr(str);
		tcleol();
		if (type != 1)
			tsetpoint(trow, tcol);
		tforce();
		tflush();
		if (type == 1)
			tgetcmd();
	}
}

/* echo 'str' to the paw and as the filename for 'buff' */
void message(struct buff *buff, char *str)
{
	struct wdo *wdo;

	if (buff->fname)
		free(buff->fname);
	buff->fname = strdup(str);
	for (wdo = Whead; wdo; wdo = wdo->next)
		if (wdo->wbuff == buff)
			wdo->modeflags = INVALID;
	echo(str);
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
int findpath(char *p, char *f, int s, Boolean m)
{
	switch (s) {
	case 4:
		if (isfile(p, Thispath, f, m))
			return 4;
	case 3:
		if (isfile(p, Me->pw_dir, f, m))
			return 3;
	case 2:
		if (isfile(p, ".", f, m))
			return 2;
	case 1:
		if (isfile(p, ConfigDir, f, m))
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
Boolean getbword(char word[], int max, int (*valid)())
{
	int i;
	struct mark tmark;

	bmrktopnt(&tmark);
	moveto(bistoken, FORWARD);
	if (bisend())
		moveto(bistoken, BACKWARD);
	movepast(bistoken, BACKWARD);
	for (i = 0; !bisend() && valid() && i < max; ++i, bmove1())
		word[i] = Buff();
	word[i] = '\0';
	bpnttomrk(&tmark);
	return i;
}

/* Get the current buffer text and store in 'txt'.
 * Get at the most 'max' characters.
 * Leaves the point alone.
 */
char *getbtxt(char txt[], int max)
{
	int i;
	struct mark tmark;

	bmrktopnt(&tmark);
	for (btostart(), i = 0; !bisend() && i < max; bmove1(), ++i)
		txt[i] = Buff();
	txt[i] = '\0';
	bpnttomrk(&tmark);
	return txt;
}

/* Go forward or back past a thingy */
void movepast(int (*pred)(), Boolean forward)
{
	if (!forward)
		bmove(-1);
	while (!(forward ? bisend() : bisstart()) && (*pred)())
		bmove(forward ? 1 : -1);
	if (!forward && !(*pred)())
		bmove1();
}

/* Go forward or back to a thingy */
void moveto(int (*pred)(), Boolean forward)
{
	if (!forward)
		bmove(-1);
	while (!(forward ? bisend() : bisstart()) && !(*pred)())
		bmove(forward ? 1 : -1);
	if (!forward && !bisstart())
		bmove1();
}

char *strup(char *str)
{
	char *ptr;
	for (ptr = str; *ptr; ++ptr)
		*ptr = toupper(*ptr);
	return str;
}


/* Put in the right number of tabs and spaces */
void tindent(int arg)
{
	if (VAR(VSPACETAB) == 0)
		for (; arg >= Tabsize; arg -= Tabsize)
			binsert('\t');
	sindent(arg);
}

int bisspace(void)
{
	return isspace(Buff());
}

int bisword(void)
{
	return  isalnum(Buff()) || Buff() == '_' || Buff() == '.' ||
		Buff() == '$';
}

/* Must be a real function. $ for PL/M */
int bistoken(void)
{
	return isalnum(Buff()) || Buff() == '_' || Buff() == '.' ||
		Buff() == '$' || Buff() == '/';
}

int biswhite(void)
{
	return STRIP(Buff()) == ' ' || STRIP(Buff()) == '\t';
}

#if 0
/* Simple rename */
int rename(char *from, char *to)
{
	int rc;

	unlink(to);
	rc = link(from, to);
	if (rc == 0)
		unlink(from);
	return rc;
}
#endif

/* Limit a filename to at most tmaxcol() - 'num' cols */
char *limit(char *fname, int num)
{
	int off;

	off = strlen(fname) - (tmaxcol() - num);
	return off > 0 ? fname + off : fname;
}

/* Find first occurance in str1 of str2. NULL if not found.
 * Case insensitive!
 */
char *stristr(char *str1, char *str2)
{
	int i, len, max;

	len = strlen(str2);
	max = strlen(str1) - len;
	for (i = 0; i <= max; ++i)
		if (strncasecmp(&str1[i], str2, len) == 0)
			return &str1[i];
	return NULL;
}
