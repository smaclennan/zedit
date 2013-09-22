/* support.c - Zedit support routines
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
#include <stdarg.h>
#include <sys/time.h>
#include <sys/poll.h>

/* ask Yes/No question.
 * Returns YES, NO, BANG, or ABORT
 */
int ask2(char *msg, Boolean allow_bang)
{
	int rc = BADCHAR;
	unsigned cmd;

	putpaw("%s", msg);
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
	int rc = delay(500);
	if (rc)
		putpaw(msg);
	return rc;
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

/* Put a string into the PAW. */
void putpaw(const char *fmt, ...)
{
	int trow, tcol;
	char str[STRMAX];
	va_list ap;

	if (InPaw)
		return;

	va_start(ap, fmt);
	vsnprintf(str, sizeof(str), fmt, ap);
	va_end(ap);

	trow = Prow; tcol = Pcol;
	tsetpoint(tmaxrow() - 1, 0);
	tprntstr(str);
	tcleol();
	tsetpoint(trow, tcol);
	tforce();
	tflush();
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
	putpaw("%s", str);
}

/* Find the correct path for the config files.
 * We check HOME and then CONFIGDIR.
 */
int findpath(char *p, char *f)
{
	if (isfile(p, Home, f, TRUE))
		return 2;
	else if (isfile(p, ConfigDir, f, TRUE))
		return 1;
	else
		return 0;
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
	while (arg-- > 0)
		binsert(' ');
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

/* Return a pointer to the start of the last part of fname */
char *lastpart(char *fname)
{
	char *p = strrchr(fname, '/');
	if (p)
		return p + 1;
	else
		return fname;
}
