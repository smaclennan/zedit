/* spell.c - spell command
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

#if SPELL
#include <signal.h>

#define SPELLSTRING "<space>(skip)  A(ccept)  I(nsert)  R(eplace)  #"
#define PROMPT		"Replace with: "

static void mclear(void);
static int ispell(FILE *, FILE *, char *, char *);

static int bisalpha(void)
{
	return isalpha(Buff());
}

/* replace with new, assumes Region contains old word */
static void sreplace(char *new)
{
	bdeltomrk(Curbuff->mark);
	binstr(new);
}

/***
 * This command is an interactive interface to the "ispell" program. It
 * allows you to perform spell checking on the current buffer. A Universal
 * Argument limits the spell check to the Region. The default is to check
 * the entire buffer.
 */
void Zspell(void)
{
	static char *argv[] = { "ispell", "-a", NULL };
	struct buff *was;
	struct buff *sbuff;
	struct mark *emark;
	struct mark *point, *mark;
	Byte cmd;
	char send[STRMAX + 2], buff[1024], *p, *e;
	char word[6][STRMAX + 1];
	int m, n, rc;
	FILE *in, *out;

	/* save current Buff, Point, and Mark */
	was = Curbuff;
	point = bcremrk();
	mark = bcremrk();
	mrktomrk(mark, Curbuff->mark);

	/* set Point and emark */
	if (Argp) {
		/* use Region */
		emark = bcremrk();
		mrktomrk(emark, Curbuff->mark);
		if (bisaftermrk(emark))
			bswappnt(emark);
	} else {
		/* use entire buffer */
		btoend();
		bmove(-1);
		emark = bcremrk();
		btostart();
	}

	sbuff = cmakebuff(SPELLBUFF, NULL);
	if (!sbuff || !invoke(sbuff, argv))
		return;

	in = fdopen(sbuff->in_pipe, "r");
	out = sbuff->out_pipe;

	/* make the paw 3 lines */
	bswitchto(was);
	paw_resize(-2);
	zrefresh();
	mclear();
	putpaw("Checking...");
	while (bisbeforemrk(emark)) {
		/* get the next word */
		moveto(bisalpha, FORWARD);
		if (bisend())
			break;	/* incase no alphas left */
		bmrktopnt(Curbuff->mark);
		for (p = send + 1; bisalpha() && !bisend(); ++p, bmove1())
			*p = Buff();
		*p++ = '\n'; *p = '\0';

		mclear();

		/* process the word */
		n = ispell(in, out, send + 1, buff);
		if (n <= 0)
			continue;
		m = -1;
		switch (*buff) {
		case '*':		/* ok */
		case '+':		/* ok with suffix removed */
			break;

		case '&':		/* close matches */
			pset(tmaxrow(), 0, PNUMCOLS);
			for (m = 0, p = buff + 2; m < 6; ++m) {
				e = strchr(p, ' ');
				if (!e)
					e = strchr(p, '\n');
				if (!e || e == p)
					break;
				*e++ = '\0';
				word[m][0] = m + '0';
				word[m][1] = ' ';
				strcpy(&word[m][2], p);
				pout(word[m], false);
				p = e;
			}
			/* 'm' is now the number of matches -
			 * drop thru */

		case '#':		/* no match */
			putpaw(SPELLSTRING);
			zrefresh();		/* update mark */
			cmd = tgetcmd();
			switch (cmd) {
			case ' ':		/* skip it */
				break;

			case 'A':		/* accept */
			case 'a':
				*send = '@';
				ispell(in, out, send, buff);
				break;

			case 'I':	/* insert in dictionary */
			case 'i':
				*send = '*';
				ispell(in, out, send, buff);
				break;

			case 'R':		/* replace */
			case 'r':
				*word[0] = '\0';
				rc = getarg(PROMPT, word[0], STRMAX);
				if (rc == ABORT) {
					bswappnt(Curbuff->mark);
					continue;
				}
				sreplace(word[0]);
				break;

			default:
				if (isdigit(cmd)) {
					n = cmd - '0';
					if (n < m)
						sreplace(word[n] + 2);
				} else {
					tbell();
					if (Keys[cmd] == ZABORT)
						goto abort;
				}
				break;
			}
			break;

		default:		/* invalid */
			error("Unable to start ispell");
			goto abort;
		}
	}
abort:
	fclose(in);		/* from fdopen */
	delbuff(sbuff);
	bswitchto(was);
	bpnttomrk(point);
	mrktomrk(Curbuff->mark, mark);
	unmark(point);
	unmark(mark);
	unmark(emark);
	paw_resize(2);
}


static void mclear(void)
{
	/* clear the matches area */
	Clrcol[Rowmax] = Clrcol[Rowmax + 1] = COLMAX + 1;
	tsetpoint(Rowmax, 0);
	tcleol();
	tsetpoint(Rowmax + 1, 0);
	tcleol();
}

static int ispell(FILE *in, FILE *out, char *send, char *receive)
{
	fputs(send, out);
	fflush(out);
	return fgets(receive, 1024, in) ? strlen(receive) : 0;
}
#else
void Zspell(void) { tbell(); }
#endif
